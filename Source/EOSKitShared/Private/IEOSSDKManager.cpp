// Copyright (C) 2024, All Rights Reserved.

#include "IEOSSDKManager.h"
#include "EOSKitSettings.h"
#include <cstdint>
#include "Containers/Ticker.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Containers/StringConv.h"
#include "Misc/ConfigCacheIni.h"

#if WITH_EOS_SDK
#include "eos_sdk.h"
#include "eos_init.h"
#include "eos_logging.h"
#include "eos_types.h"
// Note: EOS_Platform_RTCOptions is defined in eos_types.h, no need for eos_rtc.h here

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/eos_Windows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#endif

DEFINE_LOG_CATEGORY_STATIC(LogEOSSDKManager, Log, All);

// Initialize static instance pointer
IEOSSDKManager* IEOSSDKManager::Instance = nullptr;

#if WITH_EOS_SDK

namespace
{

class FEOSSDKManager final : public IEOSSDKManager
{
public:
	FEOSSDKManager();
	virtual ~FEOSSDKManager() override;

	virtual void Tick(float DeltaTime) override;
	virtual void* GetPlatformHandle() const override { return PlatformHandle; }
	virtual bool IsInitialized() const override { return bPlatformInitialized; }
	virtual std::int32_t GetTickBudgetMs() const override { return TickBudgetMs; }
private:
	bool InitializeSDK();
	bool CreatePlatform();
	bool HandleTicker(float DeltaTime);

	bool bEOSInitialized = false;
	bool bPlatformInitialized = false;
	std::int32_t TickBudgetMs = 0;
	EOS_HPlatform PlatformHandle = nullptr;
	FTSTicker::FDelegateHandle TickHandle;
};

TUniquePtr<FEOSSDKManager> GEOSSDKManagerInstance;

} // namespace

FEOSSDKManager::FEOSSDKManager()
{
	bPlatformInitialized = InitializeSDK() && CreatePlatform();

	if (bPlatformInitialized)
	{
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FEOSSDKManager::HandleTicker));
	}
	else
	{
		UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: Failed to initialize EOS SDK platform handle."));
	}
}

FEOSSDKManager::~FEOSSDKManager()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	}

	if (PlatformHandle)
	{
		EOS_Platform_Release(PlatformHandle);
		PlatformHandle = nullptr;
	}

	if (bEOSInitialized)
	{
		EOS_EResult ShutdownResult = EOS_Shutdown();
		if (ShutdownResult != EOS_EResult::EOS_Success)
		{
			UE_LOG(LogEOSSDKManager, Warning, TEXT("EOSKit: EOS_Shutdown returned %d"), static_cast<int32>(ShutdownResult));
		}
	}
}

bool FEOSSDKManager::InitializeSDK()
{
	EOS_InitializeOptions InitOptions = {};
	InitOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	InitOptions.ProductName = "EOSKit";
	InitOptions.ProductVersion = "1.0";

	EOS_EResult InitResult = EOS_Initialize(&InitOptions);
	if (InitResult == EOS_EResult::EOS_Success)
	{
		bEOSInitialized = true;
		return true;
	}

	if (InitResult == EOS_EResult::EOS_AlreadyConfigured)
	{
		UE_LOG(LogEOSSDKManager, Verbose, TEXT("EOSKit: EOS SDK already initialized."));
		return true;
	}

	UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: EOS_Initialize failed (%d)."), static_cast<int32>(InitResult));
	return false;
}

bool FEOSSDKManager::CreatePlatform()
{
	FEOSArtifact ActiveArtifact;
	if (!UEOSKitSettings::GetSettingsForArtifactFromIni(TEXT(""), ActiveArtifact))
	{
		UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: Failed to read artifact credentials from config. Check DefaultEngine.ini [/Script/EOSKitShared.EOSKitSettings]."));
		return false;
	}
	if (ActiveArtifact.ProductId.IsEmpty() ||
		ActiveArtifact.SandboxId.IsEmpty() ||
		ActiveArtifact.DeploymentId.IsEmpty() ||
		ActiveArtifact.ClientId.IsEmpty() ||
		ActiveArtifact.ClientSecret.IsEmpty() ||
		ActiveArtifact.EncryptionKey.IsEmpty())
	{
		UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: EOSKit settings are incomplete. Please configure the artifact in Project Settings > EOS Kit."));
		return false;
	}

	// EOS SDK requires an ABSOLUTE path for CacheDirectory
	FString CacheDirectory = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("EOSCache")));
	IFileManager::Get().MakeDirectory(*CacheDirectory, true);

	TickBudgetMs = 0;
	if (GConfig)
	{
		GConfig->GetInt(TEXT("/Script/EOSKitShared.EOSKitSettings"), TEXT("TickBudgetInMilliseconds"), TickBudgetMs, GEngineIni);
	}

	// Convert to UTF8 with persistent storage to guarantee lifetime during EOS_Platform_Create call
	FTCHARToUTF8 ProductId(*ActiveArtifact.ProductId);
	FTCHARToUTF8 SandboxId(*ActiveArtifact.SandboxId);
	FTCHARToUTF8 DeploymentId(*ActiveArtifact.DeploymentId);
	FTCHARToUTF8 ClientId(*ActiveArtifact.ClientId);
	FTCHARToUTF8 ClientSecret(*ActiveArtifact.ClientSecret);
	FTCHARToUTF8 EncryptionKey(*ActiveArtifact.EncryptionKey);
	FTCHARToUTF8 CacheDirUtf8(*CacheDirectory);

	// Basic sanity logs to help diagnose access violations from bad config values
	UE_LOG(LogEOSSDKManager, Verbose, TEXT("EOSKit: Creating EOS Platform (ProductId=%s SandboxId=%s DeploymentId=%s TickBudget=%dms)"), *ActiveArtifact.ProductId, *ActiveArtifact.SandboxId, *ActiveArtifact.DeploymentId, TickBudgetMs);

	// Read overlay settings from config
	// Read directly from GConfig to ensure we get the actual config values
	bool bEnableOverlay = false;
	bool bEnableSocialOverlay = false;
	bool bEnableEditorOverlay = false;
	
	if (GConfig)
	{
		const TCHAR* SettingsSection = TEXT("/Script/EOSKitShared.EOSKitSettings");
		GConfig->GetBool(SettingsSection, TEXT("bEnableOverlay"), bEnableOverlay, GEngineIni);
		GConfig->GetBool(SettingsSection, TEXT("bEnableSocialOverlay"), bEnableSocialOverlay, GEngineIni);
		GConfig->GetBool(SettingsSection, TEXT("bEnableEditorOverlay"), bEnableEditorOverlay, GEngineIni);
	}

	// Set platform flags based on overlay settings
	uint64_t PlatformFlags = 0;
	
	// Check if we're in editor
#if WITH_EDITOR
	const bool bIsInEditor = true;
#else
	const bool bIsInEditor = false;
#endif

	// EOS flag hierarchy:
	// EOS_PF_LOADING_IN_EDITOR implies EOS_PF_DISABLE_OVERLAY
	// EOS_PF_DISABLE_OVERLAY implies EOS_PF_DISABLE_SOCIAL_OVERLAY
	
	// If in editor and editor overlay is disabled, set loading in editor flag (disables everything)
	if (bIsInEditor && !bEnableEditorOverlay)
	{
		PlatformFlags |= EOS_PF_LOADING_IN_EDITOR;
	}
	// Otherwise, check overlay settings
	else
	{
		// If overlay is disabled, disable it (this also disables social overlay)
		if (!bEnableOverlay)
		{
			PlatformFlags |= EOS_PF_DISABLE_OVERLAY;
		}
		// If overlay is enabled but social overlay is disabled, only disable social overlay
		else if (!bEnableSocialOverlay)
		{
			PlatformFlags |= EOS_PF_DISABLE_SOCIAL_OVERLAY;
		}
	}

	EOS_Platform_Options PlatformOptions = {};
	PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	PlatformOptions.ProductId = ProductId.Get();
	PlatformOptions.SandboxId = SandboxId.Get();
	PlatformOptions.DeploymentId = DeploymentId.Get();
	PlatformOptions.ClientCredentials.ClientId = ClientId.Get();
	PlatformOptions.ClientCredentials.ClientSecret = ClientSecret.Get();
	PlatformOptions.CacheDirectory = CacheDirUtf8.Get();
	PlatformOptions.EncryptionKey = EncryptionKey.Get();
	PlatformOptions.TickBudgetInMilliseconds = TickBudgetMs;
	PlatformOptions.Flags = PlatformFlags;

	// Enable RTC (Real-Time Communication) for voice chat support
	// This allows lobbies to have RTC rooms for voice communication
	// Note: RTC is optional - if XAudio DLL is not found, we skip RTC to avoid platform creation failures
	EOS_Platform_RTCOptions RtcOptions = { 0 };
	RtcOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;
	bool bRTCEnabled = false;
	
	// Windows-specific RTC options (XAudio2_9 DLL path)
#if PLATFORM_WINDOWS
	static EOS_Windows_RTCOptions WindowsRTCOptions = { 0 };
	WindowsRTCOptions.ApiVersion = EOS_WINDOWS_RTCOPTIONS_API_LATEST;
	
	// Get XAudio2_9 DLL path from engine
	const FString XAudioPath = FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Windows/XAudio2_9"), 
		PLATFORM_64BITS ? TEXT("x64") : TEXT("x86"), TEXT("xaudio2_9redist.dll"));
	const FString XAudioAbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*XAudioPath);
	
	if (FPaths::FileExists(XAudioAbsolutePath))
	{
		static FTCHARToUTF8 Utf8XAudioPath(*XAudioAbsolutePath);
		WindowsRTCOptions.XAudio29DllPath = Utf8XAudioPath.Get();
		RtcOptions.PlatformSpecificOptions = &WindowsRTCOptions;
		bRTCEnabled = true;
		UE_LOG(LogEOSSDKManager, Log, TEXT("EOSKit: RTC enabled with XAudio2_9 path: %s"), *XAudioAbsolutePath);
	}
	else
	{
		UE_LOG(LogEOSSDKManager, Warning, TEXT("EOSKit: XAudio2_9 DLL not found at %s, RTC disabled (voice chat will not work)"), *XAudioAbsolutePath);
		RtcOptions.PlatformSpecificOptions = nullptr;
		bRTCEnabled = false;
	}
#else
	RtcOptions.PlatformSpecificOptions = nullptr;
	bRTCEnabled = true; // Enable RTC on non-Windows platforms (they don't need XAudio)
#endif
	
	// Only set RTCOptions if RTC is properly configured
	PlatformOptions.RTCOptions = bRTCEnabled ? &RtcOptions : nullptr;

	// Log overlay configuration
	UE_LOG(LogEOSSDKManager, Log, TEXT("EOSKit: Overlay settings - Overlay: %s, Social Overlay: %s, Editor Overlay: %s, Flags: 0x%llX"), 
		bEnableOverlay ? TEXT("Enabled") : TEXT("Disabled"),
		bEnableSocialOverlay ? TEXT("Enabled") : TEXT("Disabled"),
		bEnableEditorOverlay ? TEXT("Enabled") : TEXT("Disabled"),
		static_cast<unsigned long long>(PlatformFlags));

	// Optional: validate encryption key length (EOS requires 64 hex chars if provided)
	if (ActiveArtifact.EncryptionKey.Len() > 0 && ActiveArtifact.EncryptionKey.Len() != 64)
	{
		UE_LOG(LogEOSSDKManager, Warning, TEXT("EOSKit: EncryptionKey length (%d) is unexpected; expected 64 hex characters."), ActiveArtifact.EncryptionKey.Len());
	}

	// Defensive: ensure pointers are not null (empty strings are allowed but should still yield non-null C strings)
	if (!PlatformOptions.ProductId || !PlatformOptions.SandboxId || !PlatformOptions.DeploymentId || !PlatformOptions.ClientCredentials.ClientId || !PlatformOptions.ClientCredentials.ClientSecret)
	{
		UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: One or more required UTF8 pointers are null after conversion. Aborting platform creation."));
		return false;
	}

	PlatformHandle = EOS_Platform_Create(&PlatformOptions);
	if (!PlatformHandle)
	{
		UE_LOG(LogEOSSDKManager, Error, TEXT("EOSKit: Failed to create EOS platform instance."));
		return false;
	}

	UE_LOG(LogEOSSDKManager, Log, TEXT("EOSKit: EOS Platform created successfully."));
	return true;
}

bool FEOSSDKManager::HandleTicker(float DeltaTime)
{
	Tick(DeltaTime);
	return true;
}

void FEOSSDKManager::Tick(float DeltaTime)
{
	if (PlatformHandle)
	{
		EOS_Platform_Tick(PlatformHandle);
	}
}

#endif // WITH_EOS_SDK

namespace EOSKitSDK
{

IEOSSDKManager* CreateSDKManager()
{
#if WITH_EOS_SDK
	if (!GEOSSDKManagerInstance.IsValid())
	{
		GEOSSDKManagerInstance = MakeUnique<FEOSSDKManager>();
		if (!GEOSSDKManagerInstance->IsInitialized())
		{
			GEOSSDKManagerInstance.Reset();
		}
	}
	return GEOSSDKManagerInstance.Get();
#else
	return nullptr;
#endif
}

void DestroySDKManager()
{
#if WITH_EOS_SDK
	GEOSSDKManagerInstance.Reset();
#endif
}

} // namespace EOSKitSDK
