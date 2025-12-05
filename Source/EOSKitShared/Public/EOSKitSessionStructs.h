#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "EOSKitSharedTypes.h"
#include "EOSKitSessionStructs.generated.h"

/**
 * Enum for session match types
 */
UENUM(BlueprintType)
enum class EEOSKitMatchType : uint8
{
	MatchmakingSession UMETA(DisplayName = "Matchmaking Session"),
	CustomSession UMETA(DisplayName = "Custom Session"),
	LobbySession UMETA(DisplayName = "Lobby Session")
};

/**
 * Enum for region selection
 */
UENUM(BlueprintType)
enum class EEOSKitRegion : uint8
{
	NoSelection UMETA(DisplayName = "No Selection"),
	NAEast UMETA(DisplayName = "North America - East"),
	NAWest UMETA(DisplayName = "North America - West"),
	Europe UMETA(DisplayName = "Europe"),
	Asia UMETA(DisplayName = "Asia"),
	Oceania UMETA(DisplayName = "Oceania"),
	SouthAmerica UMETA(DisplayName = "South America")
};

/**
 * Enum for external account types (Steam, PSN, Xbox, etc.)
 * Used for cross-platform account mapping
 */
UENUM(BlueprintType)
enum class EEOSKitExternalAccountType : uint8
{
	EKAT_Epic UMETA(DisplayName = "Epic Games"),
	EKAT_Steam UMETA(DisplayName = "Steam"),
	EKAT_PSN UMETA(DisplayName = "PlayStation Network"),
	EKAT_XBL UMETA(DisplayName = "Xbox Live"),
	EKAT_Discord UMETA(DisplayName = "Discord"),
	EKAT_GOG UMETA(DisplayName = "GOG"),
	EKAT_Nintendo UMETA(DisplayName = "Nintendo"),
	EKAT_Apple UMETA(DisplayName = "Apple"),
	EKAT_Google UMETA(DisplayName = "Google"),
	EKAT_Oculus UMETA(DisplayName = "Oculus"),
	EKAT_ItchIO UMETA(DisplayName = "Itch.io"),
	EKAT_Amazon UMETA(DisplayName = "Amazon")
};

/**
 * Connect ID Token structure for verification
 */
USTRUCT(BlueprintType)
struct FEOSKitConnectIdToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Connect")
	FString JsonWebToken;

	FEOSKitConnectIdToken()
		: JsonWebToken(TEXT(""))
	{
	}

	FEOSKitConnectIdToken(const FString& InToken)
		: JsonWebToken(InToken)
	{
	}
};

// FEOSKitContinuanceToken is now defined in EOSKitSharedTypes.h

/**
 * Struct for EOS attribute (key-value pair for session attributes)
 */
USTRUCT(BlueprintType)
struct FEOSKitAttribute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	float FloatValue = 0.0f;

	FEOSKitAttribute()
		: Key(TEXT(""))
		, StringValue(TEXT(""))
		, BoolValue(false)
		, IntValue(0)
		, FloatValue(0.0f)
	{
	}
};

/**
 * Settings for creating a session
 */
USTRUCT(BlueprintType)
struct FEOSKitCreateSessionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	int32 NumberOfPublicConnections = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	int32 NumberOfPrivateConnections = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bIsLanMatch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bShouldAdvertise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bAllowJoinInProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bAllowJoinViaPresence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bAllowJoinViaPresenceFriendsOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bUsePresence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bUsesStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bIsDedicatedServer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bEnforceSanctions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	EEOSKitRegion Region = EEOSKitRegion::NoSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	FString BucketId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	TMap<FString, FEOSKitAttribute> ExtraSettings;

	FEOSKitCreateSessionSettings()
		: SessionName(TEXT(""))
		, NumberOfPublicConnections(5)
		, NumberOfPrivateConnections(0)
		, bIsLanMatch(false)
		, bShouldAdvertise(true)
		, bAllowJoinInProgress(true)
		, bAllowJoinViaPresence(true)
		, bAllowJoinViaPresenceFriendsOnly(false)
		, bUsePresence(true)
		, bUsesStats(false)
		, bIsDedicatedServer(false)
		, bEnforceSanctions(false)
		, Region(EEOSKitRegion::NoSelection)
		, BucketId(TEXT("DefaultBucket"))
	{
	}
};

/**
 * Settings for creating a lobby
 */
USTRUCT(BlueprintType)
struct FEOSKitCreateLobbySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	int32 NumberOfPublicConnections = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	int32 NumberOfPrivateConnections = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bIsLanMatch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bAllowInvites = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bShouldAdvertise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bAllowJoinInProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bUseVoiceChat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bUsePresence = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	EEOSKitRegion Region = EEOSKitRegion::NoSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	FString BucketId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bSupportHostMigration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bEnableJoinViaId = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	FString LobbyIdOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	TMap<FString, FEOSKitAttribute> ExtraSettings;

	FEOSKitCreateLobbySettings()
		: SessionName(TEXT(""))
		, NumberOfPublicConnections(4)
		, NumberOfPrivateConnections(0)
		, bIsLanMatch(false)
		, bAllowInvites(true)
		, bShouldAdvertise(true)
		, bAllowJoinInProgress(true)
		, bUseVoiceChat(false)
		, bUsePresence(true)
		, Region(EEOSKitRegion::NoSelection)
		, BucketId(TEXT("DefaultBucket"))
		, bSupportHostMigration(false)
		, bEnableJoinViaId(true)
		, LobbyIdOverride(TEXT(""))
	{
	}
};

/**
 * Settings for finding sessions
 */
USTRUCT(BlueprintType)
struct FEOSKitSessionSearchSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	EEOSKitMatchType MatchType = EEOSKitMatchType::MatchmakingSession;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	int32 MaxResults = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	EEOSKitRegion RegionToSearch = EEOSKitRegion::NoSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bLanSearch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bIncludePartySessions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	TMap<FString, FEOSKitAttribute> SearchFilters;

	FEOSKitSessionSearchSettings()
		: MatchType(EEOSKitMatchType::MatchmakingSession)
		, MaxResults(15)
		, RegionToSearch(EEOSKitRegion::NoSelection)
		, bLanSearch(false)
		, bIncludePartySessions(false)
	{
	}
};

/**
 * Information about a found session
 */
USTRUCT(BlueprintType)
struct FEOSKitSessionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FString SessionName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FString SessionId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FString OwnerName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 CurrentNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 MaxNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	bool bIsDedicatedServer = false;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 Ping = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	bool bIsLanMatch = false;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	bool bAllowJoinInProgress = true;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	TMap<FString, FEOSKitAttribute> SessionSettings;

	FEOSKitSessionResult()
		: SessionName(TEXT(""))
		, SessionId(TEXT(""))
		, OwnerName(TEXT(""))
		, CurrentNumberOfPlayers(0)
		, MaxNumberOfPlayers(0)
		, bIsDedicatedServer(false)
		, Ping(0)
		, bIsLanMatch(false)
		, bAllowJoinInProgress(true)
	{
	}
};

/**
 * Dedicated server settings for session creation
 */
USTRUCT(BlueprintType)
struct FEOSKitDedicatedServerSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	bool bUseDedicatedServer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	FString DedicatedServerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Session")
	int32 ServerPort = 7777;

	FEOSKitDedicatedServerSettings()
		: bUseDedicatedServer(false)
		, DedicatedServerName(TEXT(""))
		, ServerPort(7777)
	{
	}
};

/**
 * Member settings for lobby
 */
USTRUCT(BlueprintType)
struct FEOSKitLobbyMemberSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	bool bAllowInvites = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit|Lobby")
	TMap<FString, FEOSKitAttribute> MemberAttributes;

	FEOSKitLobbyMemberSettings()
		: bAllowInvites(true)
	{
	}
};

/**
 * Result from finding a session by ID
 */
USTRUCT(BlueprintType)
struct FEOSKitSessionFindResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FString SessionName;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FString SessionId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 CurrentNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 MaxNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	int32 Ping = 0;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	bool bIsDedicatedServer = false;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	TMap<FString, FEOSKitAttribute> SessionSettings;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|Session")
	FBlueprintSessionResult SessionResult;
	
	// Store the actual OnlineSessionSearchResult for joining
	FOnlineSessionSearchResult OnlineResult;

	FEOSKitSessionFindResult()
		: SessionName(TEXT(""))
		, SessionId(TEXT(""))
		, CurrentNumberOfPlayers(0)
		, MaxNumberOfPlayers(0)
		, Ping(0)
		, bIsDedicatedServer(false)
	{
	}
};

/**
 * Member-specific attributes for updating sessions
 */
USTRUCT(BlueprintType)
struct FEOSKitMemberSpecificAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Session")
	FString MemberId;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit|Session")
	TMap<FString, FEOSKitAttribute> Attributes;

	FEOSKitMemberSpecificAttribute()
		: MemberId(TEXT(""))
	{
	}
};
