// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPAddress.h"
#include "eos_common.h"

/**
 * Represents a network address using an EOS Product User ID (PUID)
 */
class FInternetAddrEOS : public FInternetAddr
{
public:
	FInternetAddrEOS()
		: ProductUserId(nullptr)
	{
	}

	/**
	 * Sets the Product User ID from a string
	 * @param InPUID The Product User ID as a string
	 */
	void SetProductUserId(const FString& InPUID);

	/**
	 * Gets the Product User ID handle
	 */
	EOS_ProductUserId GetProductUserId() const { return ProductUserId; }

	//~ Begin FInternetAddr Interface
	virtual void SetIp(uint32 InAddr) override;
	virtual void SetIp(const TCHAR* InAddr, bool& bIsValid) override;
	virtual void GetIp(uint32& OutAddr) const override;
	virtual void SetPort(int32 InPort) override;
	virtual int32 GetPort() const override;
	virtual void SetRawIp(const TArray<uint8>& RawAddr) override;
	virtual TArray<uint8> GetRawIp() const override;
	virtual void SetAnyAddress() override;
	virtual void SetBroadcastAddress() override;
	virtual void SetLoopbackAddress() override;
	virtual FString ToString(bool bAppendPort) const override;
	virtual bool IsValid() const override;
	virtual TSharedRef<FInternetAddr> Clone() const override;
	virtual uint32 GetTypeHash() const override;
	//~ End FInternetAddr Interface

private:
	EOS_ProductUserId ProductUserId;
};
