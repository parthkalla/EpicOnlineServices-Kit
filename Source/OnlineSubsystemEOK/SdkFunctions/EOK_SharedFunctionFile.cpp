// Copyright (c) 2023 Betide Studio. All Rights Reserved.


#include "EOK_SharedFunctionFile.h"

FString UEOK_SharedFunctionFile::ConvertOculusUserIdAndNonceToEosFormat(FString OculusUserId, FString OculusNonce)
{
	return OculusUserId + TEXT("|") + OculusNonce;
}
DEFINE_LOG_CATEGORY(LogEOK);
