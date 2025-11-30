#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "OnlineSubsystem.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/OnlineSessionNames.h"
#endif
#include "EOSKitSessionStructs.h"
#include "EOSKitSharedTypes.h"

#include "EOSKitSubsystem.generated.h"

#if WITH_EOS_SDK
// EOS types are now available from EOSKitSharedTypes.h
// Forward declare EOS_HPlatform since it's not in the shared types
struct EOS_PlatformHandle;
typedef struct EOS_PlatformHandle* EOS_HPlatform;
#endif

// ========================================
// Enums and Structs (matching EIK)
// ========================================

UENUM(BlueprintType)
enum class EEOSKitAttributeType : uint8
{
	String,
	Bool,
	Integer
};

USTRUCT(BlueprintType)
struct FEOSKitAttributeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	TEnumAsByte<EEOSKitAttributeType> AttributeType = EEOSKitAttributeType::String;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FString StringValue = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	int32 IntValue = 0;

	FEOSKitAttributeData()
	{
		AttributeType = EEOSKitAttributeType::String;
		StringValue = "";
		BoolValue = false;
		IntValue = 0;
	}

	FVariantData GetVariantData() const
	{
		FVariantData VariantData;
		switch (AttributeType)
		{
		case EEOSKitAttributeType::String:
			VariantData.SetValue(StringValue);
			break;
		case EEOSKitAttributeType::Bool:
			VariantData.SetValue(BoolValue);
			break;
		case EEOSKitAttributeType::Integer:
			VariantData.SetValue(IntValue);
			break;
		default:
			VariantData.SetValue(StringValue);
			break;
		}
		return VariantData;
	}

	FEOSKitAttributeData(FVariantData VariantData)
	{
		switch (VariantData.GetType())
		{
		case EOnlineKeyValuePairDataType::String:
			AttributeType = EEOSKitAttributeType::String;
			VariantData.GetValue(StringValue);
			break;
		case EOnlineKeyValuePairDataType::Bool:
			AttributeType = EEOSKitAttributeType::Bool;
			VariantData.GetValue(BoolValue);
			break;
		case EOnlineKeyValuePairDataType::Int32:
			AttributeType = EEOSKitAttributeType::Integer;
			VariantData.GetValue(IntValue);
			break;
		case EOnlineKeyValuePairDataType::Int64:
			AttributeType = EEOSKitAttributeType::Integer;
			int64 Int64Value;
			VariantData.GetValue(Int64Value);
			IntValue = Int64Value;
			break;
		default:
			AttributeType = EEOSKitAttributeType::String;
			VariantData.GetValue(StringValue);
			break;
		}
	}
};

USTRUCT(BlueprintType)
struct FEOSKitSessionFindStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FBlueprintSessionResult SessionResult = FBlueprintSessionResult();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	TMap<FString, FEOSKitAttributeData> SessionSettings = TMap<FString, FEOSKitAttributeData>();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString SessionName = FString();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 CurrentNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 MaxNumberOfPlayers = 0;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	bool bIsDedicatedServer = false;
};

USTRUCT(BlueprintType)
struct FEOSKitStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString StatsName;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString StatsValue;
};

// ========================================
// Delegates (matching EIK)
// ========================================

DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_Login_Callback, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_Logout_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_CreateSession_Callback, bool, bWasSuccess, const FName&, SessionName);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_CreateLobby_Callback, bool, bWasSuccess, const FName&, SessionName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_DestroySession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_JoinSession_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_FindSession_Callback, bool, bWasSuccess, const TArray<FEOSKitSessionFindStruct>&, SessionResults);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_UpdateStat_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_GetStats_Callback, bool, bWasSuccess, const TArray<FEOSKitStats>&, Stats);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_WriteFile_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_GetFile_Callback, bool, bWasSuccess, USaveGame*, SaveGame);
// Structs used in delegates - must be defined before delegate declarations
USTRUCT(BlueprintType)
struct FFileListStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString Hash = FString();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FName HashType = FName();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString DLName = FString();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString FileName = FString();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 FileSize = 0;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	FString URL = FString();

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	int32 iChunkID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "EOSKit")
	TMap<FString, FString> ExternalStorageIds = TMap<FString, FString>();
};

USTRUCT(BlueprintType)
struct FOffersStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FText LongDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FText RegularPriceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	int64 RegularPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FText PriceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	int64 NumericPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FDateTime ReleaseDate = FDateTime::MinValue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EOSKit")
	FDateTime ExpirationDate = FDateTime::MinValue();
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_TitleFileList_Callback, bool, bWasSuccess, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_GetTitleFile_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FBP_EOSKit_PurchaseOffer_Callback, bool, bWasSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_GetOffers_Callback, bool, bWasSuccess, const TArray<FOffersStruct>&, Offers);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FBP_EOSKit_GetOwnedItems_Callback, bool, bWasSuccess, const TArray<FString>&, OwnedItemNames);

// ========================================
// EOSKit Subsystem (matching EIK_Subsystem)
// ========================================

UCLASS()
class EOSKIT_API UEOSKitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEOSKitSubsystem();

	// ========================================
	// Login Functions
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithDeviceID(int32 LocalUserNum, const FString& DisplayName, const FString& DeviceName, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithAccountPortal(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithSteam(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithPersistantAuth(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithDeveloperTool(int32 LocalUserNum, const FString& LocalIP, const FString& Credential, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void LoginWithEpicLauncher(int32 LocalUserNum, const FBP_EOSKit_Login_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Login")
	void Logout(int32 LocalUserNum, const FBP_EOSKit_Logout_Callback& Result);

	// ========================================
	// User Information
	// ========================================

	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	static FString GetPlayerNickname(const int32 LocalUserNum);

	UFUNCTION(BlueprintPure, Category = "EOSKit|User")
	static bool GetLoginStatus(const int32 LocalUserNum);

	// ========================================
	// Session Functions
	// ========================================

	UFUNCTION(BlueprintCallable, DisplayName = "Create EOS Session", Category = "EOSKit|Sessions", meta = (AutoCreateRefTerm = "Custom_Settings"))
	void CreateEOSSession(
		const FBP_EOSKit_CreateSession_Callback& Result,
		TMap<FString, FString> Custom_Settings,
		FString SessionName = TEXT("Modified_EOS_Session"),
		bool bIsDedicatedServer = false,
		bool bIsLan = false,
		int32 NumberOfPublicConnections = 4,
		EEOSKitRegion Region = EEOSKitRegion::NoSelection
	);

	UFUNCTION(BlueprintCallable, DisplayName = "Create EOS Lobby", Category = "EOSKit|Sessions", meta = (AutoCreateRefTerm = "Custom_Settings"))
	void CreateEOSLobby(
		const FBP_EOSKit_CreateLobby_Callback& Result,
		TMap<FString, FString> Custom_Settings,
		FString SessionName = TEXT("Modified_EOS_Lobby"),
		bool bUseVoiceChat = true,
		bool bUsePresence = true,
		bool bAllowInvites = true,
		bool bAdvertise = true,
		bool bAllowJoinInProgress = true,
		bool bIsLan = false,
		int32 NumberOfPublicConnections = 4,
		int32 NumberOfPrivateConnections = 4
	);

	UFUNCTION(BlueprintCallable, DisplayName = "Find EOS Session", Category = "EOSKit|Sessions", meta = (AutoCreateRefTerm = "Search_Settings"))
	void FindEOSSession(
		const FBP_EOSKit_FindSession_Callback& Result,
		TMap<FString, FString> Search_Settings,
		EEOSKitMatchType MatchType = EEOSKitMatchType::LobbySession,
		EEOSKitRegion RegionToSearch = EEOSKitRegion::NoSelection
	);

	UFUNCTION(BlueprintCallable, DisplayName = "Destroy EOS Session", Category = "EOSKit|Sessions")
	void DestroyEosSession(const FBP_EOSKit_DestroySession_Callback& Result, FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName = "Join EOS Session", Category = "EOSKit|Sessions")
	void JoinEosSession(
		const FBP_EOSKit_JoinSession_Callback& Result,
		FName SessionName,
		bool bIsDedicatedServerSession,
		FBlueprintSessionResult SessionResult
	);

	// ========================================
	// Session Management
	// ========================================

	UFUNCTION(BlueprintCallable, DisplayName = "Unregister Players", Category = "EOSKit|Sessions")
	void UnRegisterPlayer(FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName = "Register Players", Category = "EOSKit|Sessions")
	void RegisterPlayer(FName SessionName, bool bWasInvited);

	UFUNCTION(BlueprintCallable, DisplayName = "Start EOS Session", Category = "EOSKit|Sessions")
	void StartSession(FName SessionName);

	UFUNCTION(BlueprintCallable, DisplayName = "End EOS Session", Category = "EOSKit|Sessions")
	void EndSession(FName SessionName);

	// ========================================
	// Social Features
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Friend")
	bool ShowFriendUserInterface();

	// ========================================
	// Statistics
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Statistics")
	void UpdateStats(const FBP_EOSKit_UpdateStat_Callback& Result, const FString& StatName, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Statistics")
	void GetStats(const FBP_EOSKit_GetStats_Callback& Result, const TArray<FString>& StatName);

	// ========================================
	// Player Data
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	void SetPlayerData(const FBP_EOSKit_WriteFile_Callback& Result, const FString& FileName, USaveGame* SavedGame);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	void GetPlayerData(const FBP_EOSKit_GetFile_Callback& Result, const FString& FileName);

	// ========================================
	// Title Files
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	void EnumerateTitleFiles(const FBP_EOSKit_TitleFileList_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	TArray<FFileListStruct> GetTitleFileList();

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	void GetTitleFile(const FBP_EOSKit_GetTitleFile_Callback& Result, const FString& FileName);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Data")
	TArray<uint8> GetTitleFileContent(const FString& FileName);

	// ========================================
	// Leaderboard
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Leaderboard")
	void GetLeaderboard(const FBP_EOSKit_GetFile_Callback& Result, FName LeaderboardName, int32 Rank, int32 Range);

	// ========================================
	// Store
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Store")
	void PurchaseItem(const FBP_EOSKit_PurchaseOffer_Callback& Result, const FString& ItemID);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Store")
	void QueryOffers(const FBP_EOSKit_GetOffers_Callback& Result);

	UFUNCTION(BlueprintCallable, Category = "EOSKit|Store")
	void GetOwnedItems(const FBP_EOSKit_GetOwnedItems_Callback& Result);

	// ========================================
	// Utility
	// ========================================

	UFUNCTION(BlueprintPure, Category = "EOSKit|Extra")
	FString GenerateSessionCode(int32 CodeLength = 9) const;

	// ========================================
	// EOS SDK Access
	// ========================================

	/**
	 * Get the EOS Platform Handle
	 * @return EOS_HPlatform handle or nullptr if not available
	 */
	EOS_HPlatform GetPlatformHandle() const;

	/**
	 * Get the Product User ID for the logged in user
	 * @param LocalUserNum - Local user index (default 0)
	 * @return EOS_ProductUserId or nullptr if not logged in
	 */
	EOS_ProductUserId GetProductUserId(int32 LocalUserNum = 0) const;

	// ========================================
	// Callback Functions (Internal)
	// ========================================

	void LoginCallback(int32 LocalUserNum, bool bWasSuccess, const FUniqueNetId& UserId, const FString& Error) const;
	void LogoutCallback(int32 LocalUserNum, bool bWasSuccess) const;
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful) const;
	void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful) const;
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionCompleted(bool bWasSuccess) const;
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccess) const;
	void OnUpdateStatsCompleted(const FOnlineError& Result) const;
	void OnGetStatsCompleted(const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult) const;
	void OnWriteFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const;
	void OnGetFileComplete(bool bSuccess, const FUniqueNetId& UserID, const FString& FileName) const;
	void OnTitleFileListComplete(bool bSuccess, const FString& Error) const;
	void OnTitleFileComplete(bool bSuccess, const FString& FileName) const;
	void OnLeaderboardListCompleted(bool bWasSuccess) const;

	// ========================================
	// Internal Login Function
	// ========================================

	void Login(int32 LocalUserNum, const FString& ID, const FString& Token, const FString& Type, const FBP_EOSKit_Login_Callback& Result);

private:
	// Callback delegates
	FBP_EOSKit_Login_Callback LoginCallBackBP;
	FBP_EOSKit_Logout_Callback LogoutCallbackBP;
	FBP_EOSKit_CreateSession_Callback CreateSession_CallbackBP;
	FBP_EOSKit_CreateLobby_Callback CreateLobby_CallbackBP;
	FBP_EOSKit_JoinSession_Callback JoinSession_CallbackBP;
	FBP_EOSKit_FindSession_Callback FindSession_CallbackBP;
	FBP_EOSKit_DestroySession_Callback DestroySession_CallbackBP;
	FBP_EOSKit_UpdateStat_Callback UpdateStat_CallbackBP;
	FBP_EOSKit_GetStats_Callback GetStats_CallbackBP;
	FBP_EOSKit_GetFile_Callback GetFile_CallbackBP;
	FBP_EOSKit_GetOwnedItems_Callback GetOwnedItems_CallbackBP;
	FBP_EOSKit_PurchaseOffer_Callback PurchaseOffer_CallbackBP;
	FBP_EOSKit_GetOffers_Callback GetOffers_CallbackBP;
	FBP_EOSKit_WriteFile_Callback WriteFile_CallbackBP;
	FBP_EOSKit_TitleFileList_Callback TitleFileList_CallbackBP;
	FBP_EOSKit_GetTitleFile_Callback GetTitleFile_CallbackBP;

	// Session search
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// Local variables
	FString LocalPortInfo;
	bool Local_bIsDedicatedServerSession = false;

	// Leaderboard
	TSharedRef<FOnlineLeaderboardRead> ReadRef;
};
