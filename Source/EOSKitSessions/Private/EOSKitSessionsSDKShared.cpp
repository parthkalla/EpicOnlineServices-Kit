#include "EOSKitSessionsSDKShared.h"

#if WITH_EOS_SDK

EEOSKitResult ConvertEOSResult(EOS_EResult Result)
{
    if (Result == EOS_EResult::EOS_Success)
    {
        return EEOSKitResult::Success;
    }
    else if (Result == EOS_EResult::EOS_InvalidParameters)
    {
        return EEOSKitResult::InvalidParameters;
    }
    else if (Result == EOS_EResult::EOS_InvalidState)
    {
        return EEOSKitResult::InvalidState;
    }
    else if (Result == EOS_EResult::EOS_InvalidUser)
    {
        return EEOSKitResult::InvalidUser;
    }
    else if (Result == EOS_EResult::EOS_NotFound)
    {
        return EEOSKitResult::NotFound;
    }
    else if (Result == EOS_EResult::EOS_DuplicateNotAllowed)
    {
        return EEOSKitResult::DuplicateNotAllowed;
    }
    else if (Result == EOS_EResult::EOS_TimedOut)
    {
        return EEOSKitResult::TimedOut;
    }
    else if (Result == EOS_EResult::EOS_Canceled)
    {
        return EEOSKitResult::Canceled;
    }
    else if (Result == EOS_EResult::EOS_NoConnection)
    {
        return EEOSKitResult::NoConnection;
    }
    else if (Result == EOS_EResult::EOS_LimitExceeded)
    {
        return EEOSKitResult::LimitExceeded;
    }
    else if (Result == EOS_EResult::EOS_Sessions_PlayerSanctioned)
    {
        return EEOSKitResult::PlayerSanctioned;
    }
    else
    {
        return EEOSKitResult::Other;
    }
}

#else

EEOSKitResult ConvertEOSResult(int32 Result)
{
    return EEOSKitResult::Other;
}

#endif
