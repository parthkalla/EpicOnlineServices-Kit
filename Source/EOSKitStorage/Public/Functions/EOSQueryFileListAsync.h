// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EOSQueryFileListAsync.generated.h"

USTRUCT(BlueprintType)
struct FEOSTitleStorageFileInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|TitleStorage")
	FString Filename;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|TitleStorage")
	int32 FileSizeBytes;

	UPROPERTY(BlueprintReadOnly, Category = "EOSKit|TitleStorage")
	FString MD5Hash;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryFileListComplete, const TArray<FEOSTitleStorageFileInfo>&, Files);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueryFileListFail, const FString&, ErrorMessage);

/**
 * Query available files from EOS Title Storage
 */
UCLASS()
class EOSKITSTORAGE_API UEOSQueryFileListAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnQueryFileListComplete OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnQueryFileListFail OnFail;

	/**
	 * Query file list from EOS Title Storage
	 * 
	 * @param WorldContextObject World context
	 * @param LocalUserId The Product User ID of the local player
	 * @param Tags Optional array of tags to filter files
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "EOSKit|TitleStorage")
	static UEOSQueryFileListAsync* QueryFileList(
		UObject* WorldContextObject,
		const FString& LocalUserId,
		const TArray<FString>& Tags
	);

	virtual void Activate() override;

	void PerformQuery();

	UObject* WorldContextObject;
	FString UserId;
	TArray<FString> FileTags;
};
