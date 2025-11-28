#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

// EOS SDK Includes
#if defined(EOS_PLATFORM_WINDOWS_32) || defined(EOS_PLATFORM_WINDOWS_64)
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#include "eos_sdk.h"
#include "eos_auth.h"
#include "eos_connect.h"
#if defined(EOS_PLATFORM_WINDOWS_32) || defined(EOS_PLATFORM_WINDOWS_64)
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "EOSKitSubsystem.generated.h"

UCLASS()
class EOSKIT_API UEOSKitSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return true; }

	// Raw Handle Accessor
	EOS_HPlatform GetPlatformHandle() const { return PlatformHandle; }
	
	// Get ProductUserId - now automatically updated when user logs in (via overlay or manual login)
	EOS_ProductUserId GetProductUserId() const;
	
	void SetProductUserId(EOS_ProductUserId InProductUserId) { ProductUserId = InProductUserId; }

private:
	EOS_HPlatform PlatformHandle;
	EOS_ProductUserId ProductUserId;
	EOS_NotificationId LoginNotificationId;
	
	void CreateEOSPlatform();
	void PerformAutoLogin();
	void StartAutoConnectLogin(EOS_EpicAccountId EpicAccountId);
	void RegisterForConnectLoginStatusChanges();
	
	static void EOS_CALL OnAutoLoginComplete(const EOS_Auth_LoginCallbackInfo* Data);
	static void EOS_CALL OnAutoConnectLoginComplete(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL OnAutoCreateUserComplete(const EOS_Connect_CreateUserCallbackInfo* Data);
	static void EOS_CALL OnConnectLoginStatusChanged(const EOS_Connect_LoginStatusChangedCallbackInfo* Data);
};
