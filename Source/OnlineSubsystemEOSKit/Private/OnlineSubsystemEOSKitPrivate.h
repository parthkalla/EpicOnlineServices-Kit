// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <type_traits>

#if WITH_EOS_SDK
#include "eos_sdk.h"
#include "eos_common.h"
#endif

// EOS string buffer length constant (from EIK)
#define EOS_OSS_STRING_BUFFER_LENGTH 256 + 1

// Helper trait to check if a callback type has ResultCode member
template<typename T>
struct THasResultCode
{
private:
	template<typename U>
	static auto Test(int) -> decltype(std::declval<U>().ResultCode, std::true_type{});
	template<typename>
	static std::false_type Test(...);
public:
	static constexpr bool value = decltype(Test<T>(0))::value;
};

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
		// Only check ResultCode if the callback type has it (notification callbacks don't have ResultCode)
		if constexpr (THasResultCode<CallbackType>::value)
		{
			if (EOS_EResult_IsOperationComplete(Data->ResultCode) == EOS_FALSE)
			{
				// Ignore - operation will retry
				return;
			}
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

/** Class to handle global EOS callbacks (notifications) that don't delete themselves */
#if ENGINE_MAJOR_VERSION == 5
template<typename CallbackFuncType, typename CallbackType, typename OwningType>
#else
template<typename CallbackFuncType, typename CallbackType>
#endif
class TEIKGlobalCallback :
	public FCallbackBase
{
public:
	TFunction<void(const CallbackType*)> CallbackLambda;

#if ENGINE_MAJOR_VERSION == 5
	TEIKGlobalCallback(TWeakPtr<OwningType> InOwner)
		: FCallbackBase()
		, Owner(InOwner)
	{
	}
	TEIKGlobalCallback(TWeakPtr<const OwningType> InOwner)
		: FCallbackBase()
		, Owner(InOwner)
	{
	}
#else
	TEIKGlobalCallback() = default;
#endif

	virtual ~TEIKGlobalCallback() = default;

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
		// Only check ResultCode if the callback type has it (notification callbacks don't have ResultCode)
		if constexpr (THasResultCode<CallbackType>::value)
		{
			if (EOS_EResult_IsOperationComplete(Data->ResultCode) == EOS_FALSE)
			{
				// Ignore - operation will retry
				return;
			}
		}
#endif
		if (!Data->ClientData)
		{
			UE_LOG(LogTemp, Error, TEXT("TEIKGlobalCallback: ClientData is null in CallbackImpl"));
			return;
		}
		check(IsInGameThread());

	TEIKGlobalCallback* CallbackThis = (TEIKGlobalCallback*)Data->ClientData;
	if (!CallbackThis)
	{
		UE_LOG(LogTemp, Error, TEXT("TEIKGlobalCallback: CallbackThis is null in CallbackImpl"));
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
	// NOTE: Don't delete - this is a global callback that persists
	}
};

