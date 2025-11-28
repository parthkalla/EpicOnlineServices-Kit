# EOSKitVoice - Production Implementation Guide

## Current Status: HYBRID IMPLEMENTATION (Option C)

The EOSKitVoice module has been set up with **complete scaffolding and structure** but requires audio integration completion.

---

## ? COMPLETED COMPONENTS

### 1. Audio Device Management
**File**: `EOSAudioDevicePool.h/.cpp`  
**Status**: ? COMPLETE

```cpp
// Fully implemented:
- Device enumeration (input/output)
- Default device detection
- Device info caching
- Thread-safe access
```

### 2. Module Structure
**Files**: `EOSKitVoice.Build.cs`, Module boilerplate  
**Status**: ? COMPLETE

- VoiceChat module dependency added
- Conditional compilation (#if WITH_EOS_RTC)
- All required dependencies configured

### 3. Basic Subsystem
**File**: `EOSKitVoiceSubsystem.h/.cpp`  
**Status**: ? COMPLETE (Simplified)

```cpp
// Working features:
- ConnectVoice/DisconnectVoice
- Participant tracking (OnVoiceUserJoined/Left)
- Muting placeholder
```

### 4. Synth Component
**File**: `EOSVoiceChatSynthComponent.h/.cpp`  
**Status**: ? COMPLETE (Ready for audio)

```cpp
// Implemented:
- Audio buffer management (thread-safe)
- AddAudioBuffer() - receives audio data
- OnGenerateAudio() - outputs to Unreal audio
- Volume control per voice
```

---

## ? TODO: Audio Integration

### Step 1: Implement FEOSVoiceChatUser Class

**File**: `Plugins/EOSKit/Source/EOSKitVoice/Public/EOSVoiceChatUser.h`

This is the **CORE** component. Create it with this structure:

```cpp
#pragma once

#include "CoreMinimal.h"
#if WITH_EOS_RTC
#include "VoiceChat.h"

class FEOSVoiceChat;

class EOSKITVOICE_API FEOSVoiceChatUser : public IVoiceChatUser
{
public:
    FEOSVoiceChatUser(FEOSVoiceChat& InVoiceChat);
    virtual ~FEOSVoiceChatUser();

    // Implement ALL IVoiceChatUser methods
    // See VoiceChat.h for complete list
    
    // Key methods to implement:
    virtual void Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate) override;
    virtual void JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties) override;
    // ... (60+ methods total)

protected:
    FEOSVoiceChat& VoiceChat;
    
    // Login state
    enum class ELoginState { LoggedOut, LoggingIn, LoggedIn, LoggingOut };
    struct FLoginSession {
        ELoginState State = ELoginState::LoggedOut;
        FString PlayerName;
        EOS_ProductUserId ProductUserId = nullptr;
    } LoginSession;
    
    // Channel management
    struct FChannelSession {
        FString ChannelName;
        EVoiceChatChannelType ChannelType;
        TSet<FString> PlayerNames;
    };
    TMap<FString, FChannelSession> ChannelSessions;
    
    // Audio settings
    float AudioInputVolume = 1.0f;
    float AudioOutputVolume = 1.0f;
    bool bAudioInputMuted = false;
    bool bAudioOutputMuted = false;
    
    friend class FEOSVoiceChat;
};

#endif // WITH_EOS_RTC
```

**Implementation File**: `EOSVoiceChatUser.cpp`

Key implementation points:

```cpp
void FEOSVoiceChatUser::Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate)
{
    // TODO: Parse Credentials (should contain EOS ProductUserId token)
    // TODO: Convert to EOS_ProductUserId
    // TODO: Store in LoginSession
    
    LoginSession.State = ELoginState::LoggedIn;
    LoginSession.PlayerName = PlayerName;
    // LoginSession.ProductUserId = ParsedFromCredentials;
    
    Delegate.ExecuteIfBound(PlayerName, FVoiceChatResult::CreateSuccess());
    OnVoiceChatLoggedInDelegate.Broadcast(PlayerName);
}

void FEOSVoiceChatUser::JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties)
{
    // TODO: Parse ChannelCredentials (contains LobbyId)
    // TODO: Call EOS_RTC_JoinRoom
    // TODO: Setup audio callbacks
    
    FChannelSession Session;
    Session.ChannelName = ChannelName;
    Session.ChannelType = ChannelType;
    ChannelSessions.Add(ChannelName, Session);
    
    Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult::CreateSuccess());
    OnVoiceChatChannelJoinedDelegate.Broadcast(ChannelName);
}
```

---

### Step 2: Update FEOSVoiceChat to Implement IVoiceChat

**File**: Update `Plugins/EOSKit/Source/EOSKitVoice/Public/EOSVoiceChat.h`

Replace the current UObject-based class with:

```cpp
#pragma once

#if WITH_EOS_RTC

#include "CoreMinimal.h"
#include "VoiceChat.h"
#include "IEOSSDKManager.h"

class FEOSAudioDevicePool;
class FEOSVoiceChatUser;

typedef TSharedPtr<class FEOSVoiceChat, ESPMode::ThreadSafe> FEOSVoiceChatPtr;
typedef TSharedPtr<class FEOSVoiceChatUser, ESPMode::ThreadSafe> FEOSVoiceChatUserPtr;
typedef TSharedRef<class FEOSVoiceChatUser, ESPMode::ThreadSafe> FEOSVoiceChatUserRef;

class EOSKITVOICE_API FEOSVoiceChat : public TSharedFromThis<FEOSVoiceChat, ESPMode::ThreadSafe>, public IVoiceChat
{
public:
    FEOSVoiceChat(IEOSSDKManager& InSDKManager);
    virtual ~FEOSVoiceChat();

    // ~Begin IVoiceChat Interface
    virtual bool Initialize() override;
    virtual void Initialize(const FOnVoiceChatInitializeCompleteDelegate& Delegate) override;
    virtual bool Uninitialize() override;
    virtual void Uninitialize(const FOnVoiceChatUninitializeCompleteDelegate& Delegate) override;
    virtual bool IsInitialized() const override;
    virtual void Connect(const FOnVoiceChatConnectCompleteDelegate& Delegate) override;
    virtual void Disconnect(const FOnVoiceChatDisconnectCompleteDelegate& Delegate) override;
    virtual bool IsConnecting() const override;
    virtual bool IsConnected() const override;
    virtual FOnVoiceChatConnectedDelegate& OnVoiceChatConnected() override;
    virtual FOnVoiceChatDisconnectedDelegate& OnVoiceChatDisconnected() override;
    virtual FOnVoiceChatReconnectedDelegate& OnVoiceChatReconnected() override;
    virtual IVoiceChatUser* CreateUser() override;
    virtual void ReleaseUser(IVoiceChatUser* VoiceChatUser) override;
    // ~End IVoiceChat Interface

    // Get EOS handles
    EOS_HRTC GetRtcInterface() const { return InitSession.EosRtcInterface; }

protected:
    friend class FEOSVoiceChatUser;

    enum class EInitializationState {
        Uninitialized,
        Initializing,
        Initialized,
        Uninitializing
    };

    struct FInitSession {
        EInitializationState State = EInitializationState::Uninitialized;
        EOS_HRTC EosRtcInterface = nullptr;
        TSharedRef<FEOSAudioDevicePool> EosAudioDevicePool;
        
        FInitSession(EOS_HRTC InRtcInterface) 
            : EosAudioDevicePool(MakeShared<FEOSAudioDevicePool>(InRtcInterface)) 
        {}
    };
    FInitSession InitSession;

    enum class EConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };
    EConnectionState ConnectionState = EConnectionState::Disconnected;

    TArray<FEOSVoiceChatUserRef> VoiceChatUsers;
    
    // Delegates
    FOnVoiceChatConnectedDelegate OnVoiceChatConnectedDelegate;
    FOnVoiceChatDisconnectedDelegate OnVoiceChatDisconnectedDelegate;
    FOnVoiceChatReconnectedDelegate OnVoiceChatReconnectedDelegate;

    IEOSSDKManager& SDKManager;
};

#endif // WITH_EOS_RTC
```

---

### Step 3: Implement Audio Callbacks

**File**: `EOSVoiceChat.cpp`

Add these callback registrations:

```cpp
void FEOSVoiceChat::Initialize(const FOnVoiceChatInitializeCompleteDelegate& Delegate)
{
    // Get EOS Platform Handle from EOSKitSubsystem
    UEOSKitSubsystem* EOSSubsystem = /* Get from Game Instance */;
    EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();
    
    InitSession.EosRtcInterface = EOS_Platform_GetRTCInterface(PlatformHandle);
    
    if (InitSession.EosRtcInterface)
    {
        RegisterAudioCallbacks();
        InitSession.State = EInitializationState::Initialized;
        Delegate.ExecuteIfBound(FVoiceChatResult::CreateSuccess());
    }
    else
    {
        Delegate.ExecuteIfBound(VoiceChat::Errors::NotInitialized());
    }
}

void FEOSVoiceChat::RegisterAudioCallbacks()
{
    // TODO: Register participant status changed callback
    EOS_RTC_AddNotifyParticipantStatusChangedOptions ParticipantOptions = {};
    ParticipantOptions.ApiVersion = EOS_RTC_ADDNOTIFYPARTICIPANTSTATUSCHANGED_API_LATEST;
    
    EOS_RTC_AddNotifyParticipantStatusChanged(
        InitSession.EosRtcInterface,
        &ParticipantOptions,
        this,
        &FEOSVoiceChat::OnParticipantStatusChangedStatic
    );
    
    // TODO: Register audio device changed callback
    // TODO: Register audio before send callback (for voice processing)
    // TODO: Register audio before render callback (for incoming audio)
}

void EOS_CALL FEOSVoiceChat::OnParticipantStatusChangedStatic(const EOS_RTC_ParticipantStatusChangedCallbackInfo* Data)
{
    if (!Data || !Data->ClientData) return;
    
    FEOSVoiceChat* This = static_cast<FEOSVoiceChat*>(Data->ClientData);
    
    // TODO: Notify user about participant joined/left
    // TODO: Create/destroy audio synth components
}
```

---

## ?? REFERENCE IMPLEMENTATION

You have the **complete production implementation** in these files:
- `EIKVoiceChat.h` (your reference)
- `EIKVoiceChat.cpp` (your reference)

### Key Sections to Copy/Adapt:

1. **Login Implementation** (lines ~200-300 in reference)
   - Token parsing
   - ProductUserId conversion
   
2. **Channel Join** (lines ~400-600 in reference)
   - Lobby integration
   - RTC room setup
   
3. **Audio Callbacks** (lines ~800-1200 in reference)
   - Participant status
   - Audio before render
   - Audio before send

4. **Device Management** (lines ~100-200 in reference)
   - Device enumeration
   - Device selection

---

## ?? QUICK START: Minimal Working Implementation

If you want voice chat working ASAP, implement just these 3 methods:

###1. Login
```cpp
void FEOSVoiceChatUser::Login(...)
{
    // Store credentials
    LoginSession.State = ELoginState::LoggedIn;
    LoginSession.PlayerName = PlayerName;
    Delegate.ExecuteIfBound(PlayerName, FVoiceChatResult::CreateSuccess());
}
```

### 2. JoinChannel
```cpp
void FEOSVoiceChatUser::JoinChannel(...)
{
    // Call existing UEOSKitVoiceSubsystem::ConnectVoice
    // Store channel session
    Delegate.ExecuteIfBound(ChannelName, FVoiceChatResult::CreateSuccess());
}
```

### 3. Audio Routing
```cpp
// In OnParticipantStatusChanged callback:
void FEOSVoiceChat::OnParticipantJoined(const FString& UserId)
{
    // Find the UEOSVoiceChatSynthComponent for this user
    UEOSVoiceChatSynthComponent* SynthComp = GetOrCreateSynthComponent(UserId);
    
    // When audio arrives (in audio callback):
    // SynthComp->AddAudioBuffer(AudioData, NumSamples, SampleRate, NumChannels);
}
```

---

## ?? INTEGRATION WITH YOUR EXISTING CODE

You already have:
- ? `UEOSKitVoiceSubsystem` (room join/leave)
- ? `UEOSVoiceChatSynthComponent` (audio playback)
- ? `FEOSAudioDevicePool` (device management)

**Bridge them together:**

```cpp
// In FEOSVoiceChatUser::JoinChannel:
UEOSKitVoiceSubsystem* VoiceSubsystem = GetVoiceSubsystem();
VoiceSubsystem->ConnectVoice(LoginSession.PlayerName, ChannelName);

// When participant joins:
UEOSVoiceChatSynthComponent* SynthComp = VoiceSubsystem->GetOrCreateSynthComponent(UserId);

// Route audio:
SynthComp->AddAudioBuffer(AudioBuffer, NumSamples, 48000, 2);
```

---

## ?? BUILD CONFIGURATION

Ensure these are in your `.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "VoiceChat",      // IVoiceChat interface
    "AudioMixer",     // Audio processing
    "SignalProcessing" // DSP
});

PublicDefinitions.Add("WITH_EOS_RTC=1");
```

---

## ?? NEXT STEPS

1. **Copy reference implementation structure** from EIKVoiceChat
2. **Implement FEOSVoiceChatUser** (300-500 lines)
3. **Update FEOSVoiceChat** to use IVoiceChat (200-300 lines)
4. **Bridge to existing UEOSKitVoiceSubsystem** (100 lines)
5. **Test with 2 players** in PIE

**Estimated Time**: 1-2 days for working implementation  
**Full Production**: 1-2 weeks with all features

---

## ?? RESOURCES

- **VoiceChat.h**: `Engine/Source/Runtime/VoiceChat/Public/VoiceChat.h`
- **EOS RTC Docs**: Epic Online Services Documentation
- **Your Reference**: The EIKVoiceChat files you provided

---

**Current Status**: Structure complete, audio integration pending  
**Recommended**: Start with minimal 3-method implementation above, then expand
