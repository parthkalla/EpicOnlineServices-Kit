// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include <cstdint>

#ifndef EOSKITSHARED_API
#define EOSKITSHARED_API
#endif

/**
 * Pure virtual interface for EOS SDK management
 * Allows different modules to interact with the SDK without tight coupling
 */
class EOSKITSHARED_API IEOSSDKManager
{
public:
	virtual ~IEOSSDKManager() = default;

	/**
	 * Tick the SDK to process callbacks and events
	 * @param DeltaTime Time since last tick in seconds
	 */
	virtual void Tick(float DeltaTime) = 0;

	/**
	 * Get the platform handle
	 * @return EOS_HPlatform handle or nullptr if not initialized
	 */
	virtual void* GetPlatformHandle() const = 0;

	/**
	 * Check if the SDK is initialized
	 * @return true if initialized and ready to use
	 */
	virtual bool IsInitialized() const = 0;

	/**
	 * Get the tick budget in milliseconds
	 * @return Tick budget for EOS platform updates
	 */
	virtual std::int32_t GetTickBudgetMs() const = 0;

	/**
	 * Get the singleton instance of the SDK manager
	 * @return Pointer to the SDK manager or nullptr if not available
	 */
	static IEOSSDKManager* Get()
	{
		return Instance;
	}

	/**
	 * Set the singleton instance
	 * @param InInstance The instance to set as singleton
	 */
	static void Set(IEOSSDKManager* InInstance)
	{
		Instance = InInstance;
	}

protected:
	static IEOSSDKManager* Instance;
};

namespace EOSKitSDK
{
	EOSKITSHARED_API IEOSSDKManager* CreateSDKManager();
	EOSKITSHARED_API void DestroySDKManager();
}