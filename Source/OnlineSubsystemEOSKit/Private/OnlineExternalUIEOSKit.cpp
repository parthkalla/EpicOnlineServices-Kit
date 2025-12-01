// Copyright (C) 2024, All Rights Reserved.

#include "OnlineExternalUIEOSKit.h"
#include "OnlineSubsystemEOSKit.h"
#include "EOSKitUiSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#if WITH_EOS_SDK

FOnlineExternalUIEOSKit::FOnlineExternalUIEOSKit(FOnlineSubsystemEOSKit* InSubsystem)
	: EOSKitSubsystem(InSubsystem)
	, UIHandle(nullptr)
{
	if (EOSKitSubsystem)
	{
		UIHandle = EOSKitSubsystem->UIHandle;
	}
}

FOnlineExternalUIEOSKit::~FOnlineExternalUIEOSKit()
{
}

bool FOnlineExternalUIEOSKit::ShowLoginUI(const int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton, const FOnLoginUIClosedDelegate& Delegate)
{
	// EOS doesn't have a login UI - login is handled through Auth interface
	Delegate.ExecuteIfBound(TSharedPtr<const FUniqueNetId>(), ControllerIndex, FOnlineError::Success());
	return false;
}

bool FOnlineExternalUIEOSKit::ShowFriendsUI(int32 LocalUserNum)
{
	// Delegate to EOSKitUiSubsystem
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->GetGameInstance())
			{
				UEOSKitUiSubsystem* UiSubsystem = Context.World()->GetGameInstance()->GetSubsystem<UEOSKitUiSubsystem>();
				if (UiSubsystem)
				{
					UE_LOG_ONLINE(Log, TEXT("FOnlineExternalUIEOSKit::ShowFriendsUI: Delegating to UEOSKitUiSubsystem"));
					// UiSubsystem->ShowFriends(...);
					return true;
				}
			}
		}
	}
	return false;
}

bool FOnlineExternalUIEOSKit::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
	return false;
}

bool FOnlineExternalUIEOSKit::ShowAchievementsUI(int32 LocalUserNum)
{
	return false;
}

bool FOnlineExternalUIEOSKit::ShowLeaderboardUI(const FString& LeaderboardName)
{
	return false;
}

bool FOnlineExternalUIEOSKit::ShowWebURL(const FString& WebURL, const FShowWebUrlParams& ShowParams, const FOnShowWebUrlClosedDelegate& Delegate)
{
	// Not implemented
	if (Delegate.IsBound())
	{
		// In UE 5.5, FOnShowWebUrlClosedDelegate takes (const FString&) for error message
		Delegate.ExecuteIfBound(TEXT("Not implemented"));
	}
	return false;
}

bool FOnlineExternalUIEOSKit::CloseWebURL()
{
	return false;
}

bool FOnlineExternalUIEOSKit::ShowProfileUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate& Delegate)
{
	// Not implemented
	if (Delegate.IsBound())
	{
		Delegate.ExecuteIfBound();
	}
	return false;
}

bool FOnlineExternalUIEOSKit::ShowAccountUpgradeUI(const FUniqueNetId& UniqueId)
{
	return false;
}

bool FOnlineExternalUIEOSKit::ShowStoreUI(int32 LocalUserNum, const FShowStoreParams& ShowParams, const FOnShowStoreUIClosedDelegate& Delegate)
{
	// In UE 5.5, FOnShowStoreUIClosedDelegate takes (bool) - (bWasSuccessful)
	Delegate.ExecuteIfBound(false);
	return false;
}

bool FOnlineExternalUIEOSKit::ShowSendMessageUI(int32 LocalUserNum, const FShowSendMessageParams& ShowParams, const FOnShowSendMessageUIClosedDelegate& Delegate)
{
	// Not implemented
	if (Delegate.IsBound())
	{
		// In UE 5.5, FOnShowSendMessageUIClosedDelegate takes (bool) for success
		Delegate.ExecuteIfBound(false);
	}
	return false;
}

bool FOnlineExternalUIEOSKit::ShowAccountCreationUI(const int ControllerIndex, const FOnAccountCreationUIClosedDelegate& Delegate)
{
	// Show account creation UI - not implemented yet
	// In UE 5.5, FOnAccountCreationUIClosedDelegate takes (int, const FOnlineAccountCredentials&, const FOnlineError&)
	FOnlineAccountCredentials EmptyCredentials;
	Delegate.ExecuteIfBound(ControllerIndex, EmptyCredentials, FOnlineError(TEXT("Not implemented")));
	return false;
}

#endif // WITH_EOS_SDK

