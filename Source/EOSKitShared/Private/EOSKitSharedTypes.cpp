// Copyright (C) 2024, All Rights Reserved.

#include "EOSKitSharedTypes.h"

#if WITH_EOS_SDK
#include "eos_common.h"
#include "eos_types.h"
#endif

// ========================================
// Helper Functions Implementation
// ========================================

#if WITH_EOS_SDK

EEOSResult ConvertEOSResultToEEOSResult(EOS_EResult EosResult)
{
	switch (EosResult)
	{
	case EOS_EResult::EOS_Success:
		return EEOSResult::EOS_Success;
	case EOS_EResult::EOS_NoConnection:
		return EEOSResult::EOS_NoConnection;
	case EOS_EResult::EOS_InvalidCredentials:
		return EEOSResult::EOS_InvalidCredentials;
	case EOS_EResult::EOS_InvalidUser:
		return EEOSResult::EOS_InvalidUser;
	case EOS_EResult::EOS_InvalidAuth:
		return EEOSResult::EOS_InvalidAuth;
	case EOS_EResult::EOS_AccessDenied:
		return EEOSResult::EOS_AccessDenied;
	case EOS_EResult::EOS_MissingPermissions:
		return EEOSResult::EOS_MissingPermissions;
	case EOS_EResult::EOS_Token_Not_Account:
		return EEOSResult::EOS_Token_Not_Account;
	case EOS_EResult::EOS_TooManyRequests:
		return EEOSResult::EOS_TooManyRequests;
	case EOS_EResult::EOS_AlreadyPending:
		return EEOSResult::EOS_AlreadyPending;
	case EOS_EResult::EOS_InvalidParameters:
		return EEOSResult::EOS_InvalidParameters;
	case EOS_EResult::EOS_InvalidRequest:
		return EEOSResult::EOS_InvalidRequest;
	case EOS_EResult::EOS_UnrecognizedResponse:
		return EEOSResult::EOS_UnrecognizedResponse;
	case EOS_EResult::EOS_IncompatibleVersion:
		return EEOSResult::EOS_IncompatibleVersion;
	case EOS_EResult::EOS_NotConfigured:
		return EEOSResult::EOS_NotConfigured;
	case EOS_EResult::EOS_AlreadyConfigured:
		return EEOSResult::EOS_AlreadyConfigured;
	case EOS_EResult::EOS_NotImplemented:
		return EEOSResult::EOS_NotImplemented;
	case EOS_EResult::EOS_Canceled:
		return EEOSResult::EOS_Canceled;
	case EOS_EResult::EOS_NotFound:
		return EEOSResult::EOS_NotFound;
	case EOS_EResult::EOS_OperationWillRetry:
		return EEOSResult::EOS_OperationWillRetry;
	case EOS_EResult::EOS_NoChange:
		return EEOSResult::EOS_NoChange;
	case EOS_EResult::EOS_VersionMismatch:
		return EEOSResult::EOS_VersionMismatch;
	case EOS_EResult::EOS_LimitExceeded:
		return EEOSResult::EOS_LimitExceeded;
	case EOS_EResult::EOS_Disabled:
		return EEOSResult::EOS_Disabled;
	case EOS_EResult::EOS_DuplicateNotAllowed:
		return EEOSResult::EOS_DuplicateNotAllowed;
	case EOS_EResult::EOS_InvalidSandboxId:
		return EEOSResult::EOS_InvalidSandboxId;
	case EOS_EResult::EOS_TimedOut:
		return EEOSResult::EOS_TimedOut;
	case EOS_EResult::EOS_PartialResult:
		return EEOSResult::EOS_PartialResult;
	case EOS_EResult::EOS_Missing_Role:
		return EEOSResult::EOS_Missing_Role;
	case EOS_EResult::EOS_Missing_Feature:
		return EEOSResult::EOS_Missing_Feature;
	case EOS_EResult::EOS_Invalid_Sandbox:
		return EEOSResult::EOS_Invalid_Sandbox;
	case EOS_EResult::EOS_Invalid_Deployment:
		return EEOSResult::EOS_Invalid_Deployment;
	case EOS_EResult::EOS_Invalid_Product:
		return EEOSResult::EOS_Invalid_Product;
	case EOS_EResult::EOS_Invalid_ProductUserID:
		return EEOSResult::EOS_Invalid_ProductUserID;
	case EOS_EResult::EOS_ServiceFailure:
		return EEOSResult::EOS_ServiceFailure;
	case EOS_EResult::EOS_CacheDirectoryMissing:
		return EEOSResult::EOS_CacheDirectoryMissing;
	case EOS_EResult::EOS_CacheDirectoryInvalid:
		return EEOSResult::EOS_CacheDirectoryInvalid;
	case EOS_EResult::EOS_InvalidState:
		return EEOSResult::EOS_InvalidState;
	case EOS_EResult::EOS_RequestInProgress:
		return EEOSResult::EOS_RequestInProgress;
	case EOS_EResult::EOS_ApplicationSuspended:
		return EEOSResult::EOS_ApplicationSuspended;
	case EOS_EResult::EOS_NetworkDisconnected:
		return EEOSResult::EOS_NetworkDisconnected;
	default:
		return EEOSResult::EOS_UnexpectedError;
	}
}

EOS_EResult ConvertEEOSResultToEOSResult(EEOSResult Result)
{
	switch (Result)
	{
	case EEOSResult::EOS_Success:
		return EOS_EResult::EOS_Success;
	case EEOSResult::EOS_NoConnection:
		return EOS_EResult::EOS_NoConnection;
	case EEOSResult::EOS_InvalidCredentials:
		return EOS_EResult::EOS_InvalidCredentials;
	case EEOSResult::EOS_InvalidUser:
		return EOS_EResult::EOS_InvalidUser;
	case EEOSResult::EOS_InvalidAuth:
		return EOS_EResult::EOS_InvalidAuth;
	case EEOSResult::EOS_AccessDenied:
		return EOS_EResult::EOS_AccessDenied;
	case EEOSResult::EOS_MissingPermissions:
		return EOS_EResult::EOS_MissingPermissions;
	case EEOSResult::EOS_Token_Not_Account:
		return EOS_EResult::EOS_Token_Not_Account;
	case EEOSResult::EOS_TooManyRequests:
		return EOS_EResult::EOS_TooManyRequests;
	case EEOSResult::EOS_AlreadyPending:
		return EOS_EResult::EOS_AlreadyPending;
	case EEOSResult::EOS_InvalidParameters:
		return EOS_EResult::EOS_InvalidParameters;
	case EEOSResult::EOS_InvalidRequest:
		return EOS_EResult::EOS_InvalidRequest;
	case EEOSResult::EOS_UnrecognizedResponse:
		return EOS_EResult::EOS_UnrecognizedResponse;
	case EEOSResult::EOS_IncompatibleVersion:
		return EOS_EResult::EOS_IncompatibleVersion;
	case EEOSResult::EOS_NotConfigured:
		return EOS_EResult::EOS_NotConfigured;
	case EEOSResult::EOS_AlreadyConfigured:
		return EOS_EResult::EOS_AlreadyConfigured;
	case EEOSResult::EOS_NotImplemented:
		return EOS_EResult::EOS_NotImplemented;
	case EEOSResult::EOS_Canceled:
		return EOS_EResult::EOS_Canceled;
	case EEOSResult::EOS_NotFound:
		return EOS_EResult::EOS_NotFound;
	case EEOSResult::EOS_OperationWillRetry:
		return EOS_EResult::EOS_OperationWillRetry;
	case EEOSResult::EOS_NoChange:
		return EOS_EResult::EOS_NoChange;
	case EEOSResult::EOS_VersionMismatch:
		return EOS_EResult::EOS_VersionMismatch;
	case EEOSResult::EOS_LimitExceeded:
		return EOS_EResult::EOS_LimitExceeded;
	case EEOSResult::EOS_Disabled:
		return EOS_EResult::EOS_Disabled;
	case EEOSResult::EOS_DuplicateNotAllowed:
		return EOS_EResult::EOS_DuplicateNotAllowed;
	case EEOSResult::EOS_InvalidSandboxId:
		return EOS_EResult::EOS_InvalidSandboxId;
	case EEOSResult::EOS_TimedOut:
		return EOS_EResult::EOS_TimedOut;
	case EEOSResult::EOS_PartialResult:
		return EOS_EResult::EOS_PartialResult;
	case EEOSResult::EOS_Missing_Role:
		return EOS_EResult::EOS_Missing_Role;
	case EEOSResult::EOS_Missing_Feature:
		return EOS_EResult::EOS_Missing_Feature;
	case EEOSResult::EOS_Invalid_Sandbox:
		return EOS_EResult::EOS_Invalid_Sandbox;
	case EEOSResult::EOS_Invalid_Deployment:
		return EOS_EResult::EOS_Invalid_Deployment;
	case EEOSResult::EOS_Invalid_Product:
		return EOS_EResult::EOS_Invalid_Product;
	case EEOSResult::EOS_Invalid_ProductUserID:
		return EOS_EResult::EOS_Invalid_ProductUserID;
	case EEOSResult::EOS_ServiceFailure:
		return EOS_EResult::EOS_ServiceFailure;
	case EEOSResult::EOS_CacheDirectoryMissing:
		return EOS_EResult::EOS_CacheDirectoryMissing;
	case EEOSResult::EOS_CacheDirectoryInvalid:
		return EOS_EResult::EOS_CacheDirectoryInvalid;
	case EEOSResult::EOS_InvalidState:
		return EOS_EResult::EOS_InvalidState;
	case EEOSResult::EOS_RequestInProgress:
		return EOS_EResult::EOS_RequestInProgress;
	case EEOSResult::EOS_ApplicationSuspended:
		return EOS_EResult::EOS_ApplicationSuspended;
	case EEOSResult::EOS_NetworkDisconnected:
		return EOS_EResult::EOS_NetworkDisconnected;
	default:
		return EOS_EResult::EOS_UnexpectedError;
	}
}

#endif // WITH_EOS_SDK

