// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EOS_SDK
#include "eos_sdk.h"
#include "eos_common.h"
#endif

/** Class to handle all EOS callbacks generically using a lambda to process callback results */
#if ENGINE_MAJOR_VERSION == 5
template<typename CallbackFuncType, typename CallbackType, typename OwningType>
#else
template<typename CallbackFuncType, typename CallbackType>
#endif
class TEOSCallback :
	public FCallbackBase
{
public:
	TFunction<void(const CallbackType*)> CallbackLambda;

#if ENGINE_MAJOR_VERSION == 5
	TEOSCallback(TWeakPtr<OwningType> InOwner)
	: FCallbackBase()
	, Owner(InOwner)
	{
	}
	TEOSCallback(TWeakPtr<const OwningType> InOwner)
		: FCallbackBase()
		, Owner(InOwner)
	{
	}
#else
	TEOSCallback()
	{
	}
#endif
	
	virtual ~TEOSCallback() = default;

	CallbackFuncType GetCallbackPtr()
	{
		return &CallbackImpl;
	}

#if ENGINE_MAJOR_VERSION == 5
protected:
	/** The object that needs to be checked for lifetime before calling the callback */
	TWeakPtr<const OwningType> Owner;
#endif

private:
	static void EOS_CALL CallbackImpl(const CallbackType* Data)
	{
#if WITH_EOS_SDK
		if (EOS_EResult_IsOperationComplete(Data->ResultCode) == EOS_FALSE)
		{
			// Ignore - operation will retry
			return;
		}
#endif
		if (!Data->ClientData)
		{
			UE_LOG(LogTemp, Error, TEXT("TEOSCallback: ClientData is null in CallbackImpl"));
			return;
		}
		check(IsInGameThread());

		TEOSCallback* CallbackThis = (TEOSCallback*)Data->ClientData;
		if (!CallbackThis)
		{
			UE_LOG(LogTemp, Error, TEXT("TEOSCallback: CallbackThis is null in CallbackImpl"));
			return;
		}
		check(CallbackThis);

#if ENGINE_MAJOR_VERSION == 5
		if (CallbackThis->Owner.IsValid() && CallbackThis->CallbackLambda)
		{
			check(CallbackThis->CallbackLambda);
			CallbackThis->CallbackLambda(Data);
		}
#else
		check(CallbackThis->CallbackLambda);
		CallbackThis->CallbackLambda(Data);
#endif
		delete CallbackThis;
	}
};

