// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IOnlineSubsystemEOSKit.h"
#include "OnlineSubsystemNames.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineTitleFileInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#if __has_include("Interfaces/OnlineStoreInterfaceV2.h")
#include "Interfaces/OnlineStoreInterfaceV2.h"
#elif __has_include("Interfaces/OnlineStoreV2Interface.h")
#include "Interfaces/OnlineStoreV2Interface.h"
#else
// Forward declare if header not available
class IOnlineStoreV2;
typedef TSharedPtr<class IOnlineStoreV2, ESPMode::ThreadSafe> IOnlineStoreV2Ptr;
#endif
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Interfaces/OnlineSharedCloudInterface.h"
#if __has_include("Interfaces/OnlineVoiceInterface.h")
#include "Interfaces/OnlineVoiceInterface.h"
#elif __has_include("Interfaces/VoiceInterface.h")
#include "Interfaces/VoiceInterface.h"
#else
// Forward declare if header not available
class IOnlineVoice;
typedef TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> IOnlineVoicePtr;
#endif
#include "EOSKitSharedTypes.h"

#if WITH_EOS_SDK
	#if defined(EOS_PLATFORM_BASE_FILE_NAME)
	#include EOS_PLATFORM_BASE_FILE_NAME
	#endif
	#include "eos_sdk.h"
	#include "eos_types.h"
#endif

DECLARE_STATS_GROUP(TEXT("EOSKit"), STATGROUP_EOSKIT, STATCAT_Advanced);

#if WITH_EOS_SDK

// Forward declarations - implemented interfaces
class FOnlineSessionEOSKit;
class FOnlineIdentityEOSKit;
class FOnlineFriendsEOSKit;

// Forward declarations - stub interfaces (to be implemented)
class FOnlinePresenceEOSKit;
class FOnlineStatsEOSKit;
class FOnlineLeaderboardsEOSKit;
class FOnlineAchievementsEOSKit;
class FOnlineUserEOSKit;
class FOnlineExternalUIEOSKit;
class FOnlineTitleFileEOSKit;
class FOnlineUserCloudEOSKit;
class FOnlineEntitlementsEOSKit;
class FOnlineStoreV2EOSKit;
class FOnlinePurchaseEOSKit;
class FOnlineSharedCloudEOSKit;
class FOnlineVoiceEOSKit;

typedef TSharedPtr<FOnlineSessionEOSKit, ESPMode::ThreadSafe> FOnlineSessionEOSKitPtr;
typedef TSharedPtr<FOnlineIdentityEOSKit, ESPMode::ThreadSafe> FOnlineIdentityEOSKitPtr;
typedef TSharedPtr<FOnlineFriendsEOSKit, ESPMode::ThreadSafe> FOnlineFriendsEOSKitPtr;
typedef TSharedPtr<FOnlinePresenceEOSKit, ESPMode::ThreadSafe> FOnlinePresenceEOSKitPtr;
typedef TSharedPtr<FOnlineStatsEOSKit, ESPMode::ThreadSafe> FOnlineStatsEOSKitPtr;
typedef TSharedPtr<FOnlineLeaderboardsEOSKit, ESPMode::ThreadSafe> FOnlineLeaderboardsEOSKitPtr;
typedef TSharedPtr<FOnlineAchievementsEOSKit, ESPMode::ThreadSafe> FOnlineAchievementsEOSKitPtr;
typedef TSharedPtr<FOnlineUserEOSKit, ESPMode::ThreadSafe> FOnlineUserEOSKitPtr;
typedef TSharedPtr<FOnlineExternalUIEOSKit, ESPMode::ThreadSafe> FOnlineExternalUIEOSKitPtr;
typedef TSharedPtr<FOnlineTitleFileEOSKit, ESPMode::ThreadSafe> FOnlineTitleFileEOSKitPtr;
typedef TSharedPtr<FOnlineUserCloudEOSKit, ESPMode::ThreadSafe> FOnlineUserCloudEOSKitPtr;
typedef TSharedPtr<FOnlineEntitlementsEOSKit, ESPMode::ThreadSafe> FOnlineEntitlementsEOSKitPtr;
typedef TSharedPtr<FOnlineStoreV2EOSKit, ESPMode::ThreadSafe> FOnlineStoreV2EOSKitPtr;
typedef TSharedPtr<FOnlinePurchaseEOSKit, ESPMode::ThreadSafe> FOnlinePurchaseEOSKitPtr;
typedef TSharedPtr<FOnlineSharedCloudEOSKit, ESPMode::ThreadSafe> FOnlineSharedCloudEOSKitPtr;
typedef TSharedPtr<FOnlineVoiceEOSKit, ESPMode::ThreadSafe> FOnlineVoiceEOSKitPtr;

/**
 *	OnlineSubsystemEOSKit - Implementation of the online subsystem for EOS services
 */
class ONLINESUBSYSTEMEOSKIT_API FOnlineSubsystemEOSKit : 
	public IOnlineSubsystemEOSKit
{
public:
	virtual ~FOnlineSubsystemEOSKit() = default;

	/** Used to be called before RHIInit() */
	static void ModuleInit();
	static void ModuleShutdown();

// IOnlineSubsystemEOSKit
#if WITH_EOS_SDK
	virtual IVoiceChatUser* GetVoiceChatUserInterface(const FUniqueNetId& LocalUserId) override;
	virtual IEOSKitPlatformHandlePtr GetEOSPlatformHandle() const override;
#endif

// IOnlineSubsystem
	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;	
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override;
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual FText GetOnlineServiceName() const override;
	virtual IOnlineStatsPtr GetStatsInterface() const override;
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void ReloadConfigs(const TSet<FString>& ConfigSections) override;

	virtual IOnlineGroupsPtr GetGroupsInterface() const override { return nullptr; }
	virtual IOnlinePartyPtr GetPartyInterface() const override { return nullptr; }
	virtual IOnlineTimePtr GetTimeInterface() const override { return nullptr; }
	virtual IOnlineEventsPtr GetEventsInterface() const override { return nullptr; }
	virtual IOnlineSharingPtr GetSharingInterface() const override { return nullptr; }
	virtual IOnlineMessagePtr GetMessageInterface() const override { return nullptr; }
	virtual IOnlineChatPtr GetChatInterface() const override { return nullptr; }
	virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override { return nullptr; }
	virtual IOnlineTournamentPtr GetTournamentInterface() const override { return nullptr; }
//~IOnlineSubsystem

	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;

// FTSTickerObjectBase
	virtual bool Tick(float DeltaTime) override;

	/** Only the factory makes instances */
	FOnlineSubsystemEOSKit() = delete;
	explicit FOnlineSubsystemEOSKit(FName InInstanceName);

	FString ProductId;

#if WITH_EOS_SDK
	/** EOS handles */
	IEOSKitPlatformHandlePtr EOSPlatformHandle;
	EOS_HAuth AuthHandle;
	EOS_HUI UIHandle;
	EOS_HFriends FriendsHandle;
	EOS_HUserInfo UserInfoHandle;
	EOS_HPresence PresenceHandle;
	EOS_HConnect ConnectHandle;
	EOS_HSessions SessionsHandle;
	EOS_HStats StatsHandle;
	EOS_HLeaderboards LeaderboardsHandle;
	EOS_HAchievements AchievementsHandle;
	EOS_HEcom EcomHandle;
	EOS_HTitleStorage TitleStorageHandle;
	EOS_HPlayerDataStorage PlayerDataStorageHandle;
#endif

	/** Interface pointers */
	FOnlineSessionEOSKitPtr SessionInterfacePtr;
	FOnlineIdentityEOSKitPtr IdentityInterfacePtr;
	FOnlineFriendsEOSKitPtr FriendsInterfacePtr;
	FOnlinePresenceEOSKitPtr PresenceInterfacePtr;
	FOnlineStatsEOSKitPtr StatsInterfacePtr;
	FOnlineLeaderboardsEOSKitPtr LeaderboardsInterfacePtr;
	FOnlineAchievementsEOSKitPtr AchievementsInterfacePtr;
	FOnlineUserEOSKitPtr UserInterfacePtr;
	FOnlineExternalUIEOSKitPtr ExternalUIInterfacePtr;
	FOnlineTitleFileEOSKitPtr TitleFileInterfacePtr;
	FOnlineUserCloudEOSKitPtr UserCloudInterfacePtr;
	FOnlineEntitlementsEOSKitPtr EntitlementsInterfacePtr;
	FOnlineStoreV2EOSKitPtr StoreV2InterfacePtr;
	FOnlinePurchaseEOSKitPtr PurchaseInterfacePtr;
	FOnlineSharedCloudEOSKitPtr SharedCloudInterfacePtr;
	FOnlineVoiceEOSKitPtr VoiceInterfacePtr;

	bool bWasLaunchedByEGS;
	bool bIsDefaultOSS;
	bool bIsPlatformOSS;

private:
	bool PlatformCreate();
};

typedef TSharedPtr<FOnlineSubsystemEOSKit, ESPMode::ThreadSafe> FOnlineSubsystemEOSKitPtr;

#else

class ONLINESUBSYSTEMEOSKIT_API FOnlineSubsystemEOSKit :
	public FOnlineSubsystemImpl
{
public:
	explicit FOnlineSubsystemEOSKit(FName InInstanceName) :
		FOnlineSubsystemImpl("EOSKit", InInstanceName)
	{
	}

	virtual ~FOnlineSubsystemEOSKit() = default;

	virtual IOnlineSessionPtr GetSessionInterface() const override { return nullptr; }
	virtual IOnlineFriendsPtr GetFriendsInterface() const override { return nullptr; }
	virtual IOnlineGroupsPtr GetGroupsInterface() const override { return nullptr; }
	virtual IOnlinePartyPtr GetPartyInterface() const override { return nullptr; }
	virtual IOnlineSharedCloudPtr GetSharedCloudInterface() const override { return nullptr; }
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override { return nullptr; }
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override { return nullptr; }
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override { return nullptr; }
	virtual IOnlineVoicePtr GetVoiceInterface() const override { return nullptr; }
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override { return nullptr; }
	virtual IOnlineIdentityPtr GetIdentityInterface() const override { return nullptr; }
	virtual IOnlineTitleFilePtr GetTitleFileInterface() const override { return nullptr; }
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override { return nullptr; }
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override { return nullptr; }
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override { return nullptr; }
	virtual IOnlineUserPtr GetUserInterface() const override { return nullptr; }
	virtual IOnlinePresencePtr GetPresenceInterface() const override { return nullptr; }
	virtual FText GetOnlineServiceName() const override { return FText::FromString(TEXT("EOSKit")); }
	virtual IOnlineStatsPtr GetStatsInterface() const override { return nullptr; }
	virtual IOnlineTimePtr GetTimeInterface() const override { return nullptr; }
	virtual IOnlineEventsPtr GetEventsInterface() const override { return nullptr; }
	virtual IOnlineSharingPtr GetSharingInterface() const override { return nullptr; }
	virtual IOnlineMessagePtr GetMessageInterface() const override { return nullptr; }
	virtual IOnlineChatPtr GetChatInterface() const override { return nullptr; }
	virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override { return nullptr; }
	virtual IOnlineTournamentPtr GetTournamentInterface() const override { return nullptr; }
	virtual bool Exec(class UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }
	virtual void ReloadConfigs(const TSet<FString>& ConfigSections) override {}

	virtual bool Init() override { return false; }
	virtual bool Shutdown() override { return false; }
	virtual FString GetAppId() const override { return FString(); }
	virtual bool Tick(float DeltaTime) override { return false; }
};

typedef TSharedPtr<FOnlineSubsystemEOSKit, ESPMode::ThreadSafe> FOnlineSubsystemEOSKitPtr;

#endif // WITH_EOS_SDK

