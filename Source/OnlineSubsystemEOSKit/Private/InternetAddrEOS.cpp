// Copyright (C) 2024, All Rights Reserved.

#include "InternetAddrEOS.h"

void FInternetAddrEOS::SetProductUserId(const FString& InPUID)
{
	ProductUserId = EOS_ProductUserId_FromString(TCHAR_TO_UTF8(*InPUID));
}

void FInternetAddrEOS::SetIp(uint32 InAddr)
{
	// Not used for EOS P2P
}

void FInternetAddrEOS::SetIp(const TCHAR* InAddr, bool& bIsValid)
{
	SetProductUserId(InAddr);
	bIsValid = (ProductUserId != nullptr);
}

void FInternetAddrEOS::GetIp(uint32& OutAddr) const
{
	// Not used for EOS P2P
	OutAddr = 0;
}

void FInternetAddrEOS::SetPort(int32 InPort)
{
	// Not used for EOS P2P
}

int32 FInternetAddrEOS::GetPort() const
{
	// Not used for EOS P2P
	return 0;
}

void FInternetAddrEOS::SetRawIp(const TArray<uint8>& RawAddr)
{
	// Not used for EOS P2P
}

TArray<uint8> FInternetAddrEOS::GetRawIp() const
{
	// Not used for EOS P2P
	return TArray<uint8>();
}

void FInternetAddrEOS::SetAnyAddress()
{
	// Not applicable for EOS P2P
	ProductUserId = nullptr;
}

void FInternetAddrEOS::SetBroadcastAddress()
{
	// Not applicable for EOS P2P
	ProductUserId = nullptr;
}

void FInternetAddrEOS::SetLoopbackAddress()
{
	// Set to local user ID (will be set by socket subsystem)
	ProductUserId = nullptr;
}

FString FInternetAddrEOS::ToString(bool bAppendPort) const
{
	if (ProductUserId)
	{
		char PUIDString[EOS_PRODUCTUSERID_MAX_LENGTH + 1];
		int32_t BufferSize = sizeof(PUIDString);
		if (EOS_ProductUserId_ToString(ProductUserId, PUIDString, &BufferSize) == EOS_EResult::EOS_Success)
		{
			return FString(UTF8_TO_TCHAR(PUIDString));
		}
	}
	return FString(TEXT("INVALID"));
}

bool FInternetAddrEOS::IsValid() const
{
	return ProductUserId != nullptr && EOS_ProductUserId_IsValid(ProductUserId) == EOS_TRUE;
}

TSharedRef<FInternetAddr> FInternetAddrEOS::Clone() const
{
	FInternetAddrEOS* NewAddr = new FInternetAddrEOS();
	NewAddr->ProductUserId = ProductUserId;
	return MakeShareable(NewAddr);
}

uint32 FInternetAddrEOS::GetTypeHash() const
{
	// Use ProductUserId pointer as hash
	return ::GetTypeHash((void*)ProductUserId);
}
