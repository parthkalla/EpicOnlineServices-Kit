// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EOS_SDK
	#include "eos_sdk.h"
	#include "eos_types.h"

	/**
	 * Simple wrapper for EOS platform handle
	 */
	class IEOSKitPlatformHandle
	{
	public:
		IEOSKitPlatformHandle(EOS_HPlatform InPlatformHandle) 
			: PlatformHandle(InPlatformHandle) 
		{}
		
		virtual ~IEOSKitPlatformHandle() = default;

		virtual void Tick() {}

		operator EOS_HPlatform() const { return PlatformHandle; }

		EOS_HPlatform PlatformHandle;
	};

	using IEOSKitPlatformHandlePtr = TSharedPtr<IEOSKitPlatformHandle, ESPMode::ThreadSafe>;
	using IEOSKitPlatformHandleWeakPtr = TWeakPtr<IEOSKitPlatformHandle, ESPMode::ThreadSafe>;
#else
	using IEOSKitPlatformHandlePtr = TSharedPtr<void, ESPMode::ThreadSafe>;
	using IEOSKitPlatformHandleWeakPtr = TWeakPtr<void, ESPMode::ThreadSafe>;
#endif

