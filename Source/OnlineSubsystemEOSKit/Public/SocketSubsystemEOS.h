// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SocketSubsystem.h"
#include "InternetAddrEOS.h"
#include "eos_p2p.h"

/**
 * Socket subsystem implementation for EOS P2P
 */
class FSocketSubsystemEOS : public ISocketSubsystem
{
public:
	/** 
	 * Singleton interface for this subsystem 
	 */
	static FSocketSubsystemEOS* Create();
	
	/**
	 * Performs EOS specific socket clean up
	 */
	static void Destroy();

	//~ Begin ISocketSubsystem Interface
	virtual bool Init(FString& Error) override;
	virtual void Shutdown() override;
	virtual class FSocket* CreateSocket(const FName& SocketType, const FString& SocketDescription, const FName& ProtocolType) override;
	virtual void DestroySocket(class FSocket* Socket) override;
	virtual FAddressInfoResult GetAddressInfo(const TCHAR* HostName, const TCHAR* ServiceName = nullptr,
		EAddressInfoFlags QueryFlags = EAddressInfoFlags::Default,
		const FName ProtocolTypeName = NAME_None,
		ESocketType SocketType = ESocketType::SOCKTYPE_Unknown) override;
	virtual TSharedPtr<FInternetAddr> GetAddressFromString(const FString& InAddress) override;
	virtual bool GetHostName(FString& HostName) override;
	virtual TSharedRef<FInternetAddr> CreateInternetAddr() override;
	virtual bool HasNetworkDevice() override;
	virtual const TCHAR* GetSocketAPIName() const override;
	virtual ESocketErrors GetLastErrorCode() override;
	virtual ESocketErrors TranslateErrorCode(int32 Code) override;
	virtual bool GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>>& OutAddresses) override;
	virtual TArray<TSharedRef<FInternetAddr>> GetLocalBindAddresses() override;
	virtual TSharedRef<FInternetAddr> GetLocalBindAddr(FOutputDevice& Out) override;
	virtual bool RequiresChatDataBeSeparate() override;
	virtual bool RequiresEncryptedPackets() override;
	virtual bool IsSocketWaitSupported() const override;
	//~ End ISocketSubsystem Interface

	/**
	 * Get the EOS P2P interface handle
	 */
	EOS_HP2P GetP2PHandle() const { return P2PHandle; }

	/**
	 * Get the local Product User ID
	 */
	EOS_ProductUserId GetLocalProductUserId() const { return LocalProductUserId; }

protected:
	/** Hidden on purpose */
	FSocketSubsystemEOS();

private:
	/** Single instantiation of this subsystem */
	static FSocketSubsystemEOS* SocketSingleton;

	/** EOS P2P interface handle */
	EOS_HP2P P2PHandle;

	/** Local user's Product User ID */
	EOS_ProductUserId LocalProductUserId;

	/** Last socket error */
	ESocketErrors LastSocketError;
};
