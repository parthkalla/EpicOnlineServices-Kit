#pragma once

#include "CoreMinimal.h"
#include "EOSKitTypes.generated.h"

// Enum Exposed to Blueprints for Login Types
UENUM(BlueprintType)
enum class EEOSLoginMethod : uint8
{
	AccountPortal       UMETA(DisplayName = "Account Portal (Web)"),
	Developer           UMETA(DisplayName = "Developer Tool (Local)"),
	ExchangeCode        UMETA(DisplayName = "Exchange Code (Launcher)"),
	DeviceCode          UMETA(DisplayName = "Device Code")
};

/** Login flags to control EOS Auth Scopes */
UENUM(BlueprintType)
enum class EEOSLoginFlags : uint8
{
	BasicProfile    UMETA(DisplayName = "Basic Profile"),
	FriendsList     UMETA(DisplayName = "Friends List"),
	Presence        UMETA(DisplayName = "Presence")
};

// Generic failure/success struct
USTRUCT(BlueprintType)
struct FEOSResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;
};

UENUM(BlueprintType)
enum class EEOSLobbyPermission : uint8
{
	Public UMETA(DisplayName = "Public"),
	JoinViaPresence UMETA(DisplayName = "Join Via Presence"),
	InviteOnly UMETA(DisplayName = "Invite Only")
};

USTRUCT(BlueprintType)
struct FEOSLobbyInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString LobbyId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	bool bIsPublic;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString BucketId;
};

USTRUCT(BlueprintType)
struct FEOSLeaderboardDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString LeaderboardId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString StatName;
};

USTRUCT(BlueprintType)
struct FEOSLeaderboardRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString UserId;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	int32 Rank;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	int32 Score;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit")
	FString UserDisplayName;
};
