// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

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
	virtual int32 GetTickBudgetMs() const = 0;

	/**
	 * Register a ticker for periodic updates
	 * @param TickDelegate The delegate to call on tick
	 * @param TickInterval Interval in seconds between ticks
	 * @return Handle to the registered ticker
	 */
	virtual FTSTicker::FDelegateHandle RegisterTicker(FTickerDelegate TickDelegate, float TickInterval) = 0;

	/**
	 * Unregister a previously registered ticker
	 * @param Handle The handle returned from RegisterTicker
	 */
	virtual void UnregisterTicker(FTSTicker::FDelegateHandle Handle) = 0;

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
