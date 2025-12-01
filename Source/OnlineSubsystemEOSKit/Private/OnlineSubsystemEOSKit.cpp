// Copyright (C) 2024, All Rights Reserved.

#include "OnlineSubsystemEOSKit.h"
#include "OnlineSessionEOSKit.h"
#include "OnlineIdentityEOSKit.h"
#include "OnlineFriendsEOSKit.h"
#include "OnlinePresenceEOSKit.h"
#include "OnlineStatsEOSKit.h"
#include "OnlineLeaderboardsEOSKit.h"
#include "OnlineAchievementsEOSKit.h"
#include "OnlineUserEOSKit.h"
#include "OnlineExternalUIEOSKit.h"
#include "OnlineTitleFileEOSKit.h"
#include "OnlineUserCloudEOSKit.h"
#include "OnlineEntitlementsEOSKit.h"
#include "OnlineStoreV2EOSKit.h"
#include "OnlinePurchaseEOSKit.h"
#include "OnlineSharedCloudEOSKit.h"
#include "OnlineVoiceEOSKit.h"
#include "EOSKitSettings.h"
#include "EOSKitSubsystem.h"
#include "IEOSSDKManager.h"
#include "IEOSKitPlatformHandle.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/NetworkVersion.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK
	#include "eos_auth.h"
	#include "eos_ui.h"
	#include "eos_friends.h"
	#include "eos_userinfo.h"
	#include "eos_presence.h"
	#include "eos_connect.h"
	#include "eos_sessions.h"
	#include "eos_stats.h"
	#include "eos_leaderboards.h"
	#include "eos_achievements.h"
	#include "eos_ecom.h"
	#include "eos_titlestorage.h"
	#include "eos_playerdatastorage.h"
#endif

#if WITH_EOS_SDK

void FOnlineSubsystemEOSKit::ModuleInit()
{
	// Module initialization - can be used for early setup
	UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit::ModuleInit()"));
}

void FOnlineSubsystemEOSKit::ModuleShutdown()
{
	// Module shutdown - cleanup
	UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit::ModuleShutdown()"));
}

FOnlineSubsystemEOSKit::FOnlineSubsystemEOSKit(FName InInstanceName)
	: IOnlineSubsystemEOSKit("EOSKit", InInstanceName)
	, ProductId(TEXT(""))
#if WITH_EOS_SDK
	, EOSPlatformHandle()
	, AuthHandle(nullptr)
	, UIHandle(nullptr)
	, FriendsHandle(nullptr)
	, UserInfoHandle(nullptr)
	, PresenceHandle(nullptr)
	, ConnectHandle(nullptr)
	, SessionsHandle(nullptr)
	, StatsHandle(nullptr)
	, LeaderboardsHandle(nullptr)
	, AchievementsHandle(nullptr)
	, EcomHandle(nullptr)
	, TitleStorageHandle(nullptr)
	, PlayerDataStorageHandle(nullptr)
#endif
	, bWasLaunchedByEGS(false)
	, bIsDefaultOSS(false)
	, bIsPlatformOSS(false)
{
}

bool FOnlineSubsystemEOSKit::Init()
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit::Init()"));

	// Get settings
	UEOSKitSettings* Settings = GetMutableDefault<UEOSKitSettings>();
	if (!Settings)
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSubsystemEOSKit: Failed to get EOSKitSettings"));
		return false;
	}

	// Get active artifact
	FEOSArtifact ActiveArtifact = Settings->GetActiveArtifact();
	if (ActiveArtifact.ProductId.IsEmpty())
	{
		UE_LOG_ONLINE(Error, TEXT("FOnlineSubsystemEOSKit: ProductId is empty in settings"));
		return false;
	}

	ProductId = ActiveArtifact.ProductId;

#if WITH_EOS_SDK
	// Get platform handle from IEOSSDKManager (shared with UEOSKitSubsystem)
	IEOSSDKManager* SDKManager = IEOSSDKManager::Get();
	if (SDKManager && SDKManager->IsInitialized())
	{
		EOS_HPlatform RawPlatformHandle = static_cast<EOS_HPlatform>(SDKManager->GetPlatformHandle());
		
		if (RawPlatformHandle)
		{
			// Create platform handle wrapper
			EOSPlatformHandle = MakeShared<IEOSKitPlatformHandle, ESPMode::ThreadSafe>(RawPlatformHandle);
			
			// Get all EOS interface handles
			AuthHandle = EOS_Platform_GetAuthInterface(RawPlatformHandle);
			UIHandle = EOS_Platform_GetUIInterface(RawPlatformHandle);
			FriendsHandle = EOS_Platform_GetFriendsInterface(RawPlatformHandle);
			UserInfoHandle = EOS_Platform_GetUserInfoInterface(RawPlatformHandle);
			PresenceHandle = EOS_Platform_GetPresenceInterface(RawPlatformHandle);
			ConnectHandle = EOS_Platform_GetConnectInterface(RawPlatformHandle);
			SessionsHandle = EOS_Platform_GetSessionsInterface(RawPlatformHandle);
			StatsHandle = EOS_Platform_GetStatsInterface(RawPlatformHandle);
			LeaderboardsHandle = EOS_Platform_GetLeaderboardsInterface(RawPlatformHandle);
			AchievementsHandle = EOS_Platform_GetAchievementsInterface(RawPlatformHandle);
			EcomHandle = EOS_Platform_GetEcomInterface(RawPlatformHandle);
			TitleStorageHandle = EOS_Platform_GetTitleStorageInterface(RawPlatformHandle);
			PlayerDataStorageHandle = EOS_Platform_GetPlayerDataStorageInterface(RawPlatformHandle);
			
			// Create interface implementations
			SessionInterfacePtr = MakeShared<FOnlineSessionEOSKit, ESPMode::ThreadSafe>(this);
			IdentityInterfacePtr = MakeShared<FOnlineIdentityEOSKit, ESPMode::ThreadSafe>(this);
			FriendsInterfacePtr = MakeShared<FOnlineFriendsEOSKit, ESPMode::ThreadSafe>(this);
			PresenceInterfacePtr = MakeShared<FOnlinePresenceEOSKit, ESPMode::ThreadSafe>(this);
			StatsInterfacePtr = MakeShared<FOnlineStatsEOSKit, ESPMode::ThreadSafe>(this);
			LeaderboardsInterfacePtr = MakeShared<FOnlineLeaderboardsEOSKit, ESPMode::ThreadSafe>(this);
			AchievementsInterfacePtr = MakeShared<FOnlineAchievementsEOSKit, ESPMode::ThreadSafe>(this);
			UserInterfacePtr = MakeShared<FOnlineUserEOSKit, ESPMode::ThreadSafe>(this);
			ExternalUIInterfacePtr = MakeShared<FOnlineExternalUIEOSKit, ESPMode::ThreadSafe>(this);
			TitleFileInterfacePtr = MakeShared<FOnlineTitleFileEOSKit, ESPMode::ThreadSafe>(this);
			UserCloudInterfacePtr = MakeShared<FOnlineUserCloudEOSKit, ESPMode::ThreadSafe>(this);
			EntitlementsInterfacePtr = MakeShared<FOnlineEntitlementsEOSKit, ESPMode::ThreadSafe>(this);
			StoreV2InterfacePtr = MakeShared<FOnlineStoreV2EOSKit, ESPMode::ThreadSafe>(this);
			PurchaseInterfacePtr = MakeShared<FOnlinePurchaseEOSKit, ESPMode::ThreadSafe>(this);
			SharedCloudInterfacePtr = MakeShared<FOnlineSharedCloudEOSKit, ESPMode::ThreadSafe>(this);
			VoiceInterfacePtr = MakeShared<FOnlineVoiceEOSKit, ESPMode::ThreadSafe>(this);
			
			UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit: Successfully retrieved EOS handles from SDK Manager and created all interfaces"));
		}
		else
		{
			UE_LOG_ONLINE(Warning, TEXT("FOnlineSubsystemEOSKit: SDK Manager is initialized but platform handle is null"));
		}
	}
	else
	{
		UE_LOG_ONLINE(Warning, TEXT("FOnlineSubsystemEOSKit: SDK Manager not available or not initialized. EOSKitSubsystem may need to initialize first."));
	}
#endif

	bIsDefaultOSS = true;
	
	UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit: Initialized successfully with ProductId: %s"), *ProductId);
	return true;
}

bool FOnlineSubsystemEOSKit::Shutdown()
{
	UE_LOG_ONLINE(Log, TEXT("FOnlineSubsystemEOSKit::Shutdown()"));

	// Cleanup interfaces
	SessionInterfacePtr.Reset();
	IdentityInterfacePtr.Reset();
	FriendsInterfacePtr.Reset();
	PresenceInterfacePtr.Reset();
	StatsInterfacePtr.Reset();
	LeaderboardsInterfacePtr.Reset();
	AchievementsInterfacePtr.Reset();
	UserInterfacePtr.Reset();
	ExternalUIInterfacePtr.Reset();
	TitleFileInterfacePtr.Reset();
	UserCloudInterfacePtr.Reset();
	EntitlementsInterfacePtr.Reset();
	StoreV2InterfacePtr.Reset();
	PurchaseInterfacePtr.Reset();
	SharedCloudInterfacePtr.Reset();
	VoiceInterfacePtr.Reset();

#if WITH_EOS_SDK
	EOSPlatformHandle.Reset();
	AuthHandle = nullptr;
	UIHandle = nullptr;
	FriendsHandle = nullptr;
	UserInfoHandle = nullptr;
	PresenceHandle = nullptr;
	ConnectHandle = nullptr;
	SessionsHandle = nullptr;
	StatsHandle = nullptr;
	LeaderboardsHandle = nullptr;
	AchievementsHandle = nullptr;
	EcomHandle = nullptr;
	TitleStorageHandle = nullptr;
	PlayerDataStorageHandle = nullptr;
#endif

	return true;
}

FString FOnlineSubsystemEOSKit::GetAppId() const
{
	return ProductId;
}

bool FOnlineSubsystemEOSKit::Tick(float DeltaTime)
{
	// Tick the EOS platform if available
	// The SDK Manager handles ticking, but we can also tick here if needed
#if WITH_EOS_SDK
	if (EOSPlatformHandle.IsValid())
	{
		EOSPlatformHandle->Tick();
	}
#endif
	return true;
}

#if WITH_EOS_SDK
IVoiceChatUser* FOnlineSubsystemEOSKit::GetVoiceChatUserInterface(const FUniqueNetId& LocalUserId)
{
	// Voice chat is handled by EOSKitVoice module
	// Return nullptr for now - can be implemented later if needed
	return nullptr;
}

IEOSKitPlatformHandlePtr FOnlineSubsystemEOSKit::GetEOSPlatformHandle() const
{
	return EOSPlatformHandle;
}
#endif

bool FOnlineSubsystemEOSKit::Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FOnlineSubsystemEOSKit::ReloadConfigs(const TSet<FString>& ConfigSections)
{
	// Reload configuration
}

bool FOnlineSubsystemEOSKit::PlatformCreate()
{
	// Platform creation - minimal implementation
	// Full implementation would create EOS platform handle here
	return true;
}

// IOnlineSubsystem interface implementations
IOnlineSessionPtr FOnlineSubsystemEOSKit::GetSessionInterface() const
{
	return SessionInterfacePtr;
}

IOnlineFriendsPtr FOnlineSubsystemEOSKit::GetFriendsInterface() const
{
	return FriendsInterfacePtr;
}

IOnlineSharedCloudPtr FOnlineSubsystemEOSKit::GetSharedCloudInterface() const
{
	return SharedCloudInterfacePtr;
}

IOnlineUserCloudPtr FOnlineSubsystemEOSKit::GetUserCloudInterface() const
{
	return UserCloudInterfacePtr;
}

IOnlineEntitlementsPtr FOnlineSubsystemEOSKit::GetEntitlementsInterface() const
{
	return EntitlementsInterfacePtr;
}

IOnlineLeaderboardsPtr FOnlineSubsystemEOSKit::GetLeaderboardsInterface() const
{
	return LeaderboardsInterfacePtr;
}

IOnlineVoicePtr FOnlineSubsystemEOSKit::GetVoiceInterface() const
{
	return VoiceInterfacePtr;
}

IOnlineExternalUIPtr FOnlineSubsystemEOSKit::GetExternalUIInterface() const
{
	return ExternalUIInterfacePtr;
}

IOnlineIdentityPtr FOnlineSubsystemEOSKit::GetIdentityInterface() const
{
	return IdentityInterfacePtr;
}

IOnlineTitleFilePtr FOnlineSubsystemEOSKit::GetTitleFileInterface() const
{
	return TitleFileInterfacePtr;
}

IOnlineStoreV2Ptr FOnlineSubsystemEOSKit::GetStoreV2Interface() const
{
	return StoreV2InterfacePtr;
}

IOnlinePurchasePtr FOnlineSubsystemEOSKit::GetPurchaseInterface() const
{
	return PurchaseInterfacePtr;
}

IOnlineAchievementsPtr FOnlineSubsystemEOSKit::GetAchievementsInterface() const
{
	return AchievementsInterfacePtr;
}

IOnlineUserPtr FOnlineSubsystemEOSKit::GetUserInterface() const
{
	return UserInterfacePtr;
}

IOnlinePresencePtr FOnlineSubsystemEOSKit::GetPresenceInterface() const
{
	return PresenceInterfacePtr;
}

FText FOnlineSubsystemEOSKit::GetOnlineServiceName() const
{
	return FText::FromString(TEXT("EOSKit"));
}

IOnlineStatsPtr FOnlineSubsystemEOSKit::GetStatsInterface() const
{
	return StatsInterfacePtr;
}

#endif // WITH_EOS_SDK

