// Copyright (c) 2025 Asrock Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_GoogleSignIn.generated.h"

UCLASS()
class EOKLoginMethods_API UBFL_GoogleSignIn : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UBFL_GoogleSignIn();

	UFUNCTION(BlueprintCallable, meta = (DisplayName="GoogleSubsystem_GetUserDisplayName"), Category = "Epic Online Services-Kit V2|GoogleSubsystem")
	static FString GetUserDisplayName();

	UFUNCTION(BlueprintCallable, meta = (DisplayName="GoogleSubsystem_GetUserFirstName"), Category = "Epic Online Services-Kit V2|GoogleSubsystem")
	static FString GetUserFirstName();

	UFUNCTION(BlueprintCallable, meta = (DisplayName="GoogleSubsystem_GetUserLastName"), Category = "Epic Online Services-Kit V2|GoogleSubsystem")
	static FString GetUserLastName();

	UFUNCTION(BlueprintCallable, meta = (DisplayName="GoogleSubsystem_GetUserProfilePictureUrl"), Category = "Epic Online Services-Kit V2|GoogleSubsystem")
	static FString GetUserProfilePictureUrl();

	UFUNCTION(BlueprintCallable, meta = (DisplayName="GoogleSubsystem_IsUserLoggedIn"), Category = "Epic Online Services-Kit V2|GoogleSubsystem")
	static bool IsUserLoggedIn();
	
};
