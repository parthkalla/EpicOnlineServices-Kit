# EOSKit Plugin - Complete Features Documentation

## Table of Contents
1. [Overview](#overview)
2. [Authentication & Identity](#authentication--identity)
3. [Sessions & Matchmaking](#sessions--matchmaking)
4. [Lobbies](#lobbies)
5. [Voice Chat (RTC)](#voice-chat-rtc)
6. [Achievements](#achievements)
7. [Statistics & Leaderboards](#statistics--leaderboards)
8. [Title Storage](#title-storage)
9. [Networking & Sockets](#networking--sockets)
10. [Web Services](#web-services)
11. [Configuration](#configuration)
12. [Advanced Features](#advanced-features)

---

## Overview

**EOSKit** is a comprehensive Unreal Engine plugin that integrates Epic Online Services (EOS) SDK, providing Blueprint and C++ support for multiplayer, social, and online features.

### Supported Unreal Engine Versions
- Unreal Engine 5.3+
- Unreal Engine 5.4
- Unreal Engine 5.5

### Module Structure
```
EOSKit/
??? EOSKit (Core)              - Main subsystem, platform initialization
??? EOSKitShared               - Shared types, interfaces, utilities
??? EOSKitAuth                 - Authentication and login
??? EOSKitSessions             - Session management and matchmaking
??? EOSKitLobbies              - Lobby system
??? EOSKitVoice                - Voice chat (RTC)
??? EOSKitAchievements         - Achievements system
??? EOSKitStats                - Statistics and leaderboards
??? EOSKitTitleStorage         - Cloud file storage
??? EOSKitSockets              - Low-level networking
??? EOSKitWeb                  - Web API integrations
```

---

## Authentication & Identity

### Module: `EOSKitAuth`

#### Features

##### 1. **Auto Login**
Automatically logs in users when the game starts.

**Blueprint Functions:**
- Enabled via `DefaultEngine.ini` configuration
- Triggers `OnAutoLoginComplete` event

**C++ Usage:**
```cpp
// Configured in EOSKitSubsystem
void PerformAutoLogin();
```

**Configuration:**
```ini
[/Script/EOSKit.EOSKitSettings]
bEnableAutoLogin=True
```

##### 2. **Manual Login (Epic Account)**
Login using Epic Games account.

**Blueprint Node:** `EOS Login`
- **Inputs:**
  - Login Type (Account Portal, Developer, Exchange Code, etc.)
  - Credentials (optional based on login type)
- **Outputs:**
  - On Success (EpicAccountId, ProductUserId)
  - On Failure (Error Code, Error Message)

**C++ Usage:**
```cpp
#include "EOSLoginAsync.h"

UEOSLoginAsync* LoginNode = UEOSLoginAsync::EOSLogin(
    WorldContextObject,
    EEOSLoginType::AccountPortal,
    FEOSLoginCredentials()
);

LoginNode->OnSuccess.AddDynamic(this, &AMyClass::OnLoginSuccess);
LoginNode->Activate();
```

##### 3. **Connect Login**
Login using EOS Connect interface (for non-Epic accounts).

**Blueprint Node:** `EOS Connect Login`
- **Inputs:**
  - External Auth Token
  - External Auth Type
- **Outputs:**
  - On Success (ProductUserId)
  - On Failure

**C++ Usage:**
```cpp
#include "Functions/Connect/EOSConnectLoginAsync.h"

UEOSConnectLoginAsync* ConnectLogin = UEOSConnectLoginAsync::EOSConnectLogin(
    WorldContextObject,
    ExternalAuthToken,
    EExternalAuthType::Steam
);
```

##### 4. **Device ID Authentication**
Create and use device-based authentication.

**Blueprint Nodes:**
- `Create Device ID` - Creates a device-based account
- `Transfer Device ID Account` - Transfers device account to full account
- `Link Account` - Links external account to existing account
- `Unlink Account` - Unlinks external account

**Use Cases:**
- Guest accounts
- Device-specific login
- Account migration

##### 5. **Account Linking**
Link multiple authentication methods to a single account.

**Blueprint Node:** `Link Account`
```cpp
UEOSLinkAccountAsync::EOSLinkAccount(
    WorldContextObject,
    ExternalAuthType,
    ExternalAuthToken
);
```

---

## Sessions & Matchmaking

### Module: `EOSKitSessions`

#### Features

##### 1. **Session Creation**

**Blueprint Node:** `Create EOK Session`
- **Parameters:**
  - Session Name
  - Max Players
  - Is LAN
  - Is Dedicated
  - Allow Join In Progress
  - Custom Settings (TMap<FString, FString>)

**C++ Usage:**
```cpp
#include "EOSCreateEOKSessionAsync.h"

UEOSCreateEOKSessionAsync* CreateSession = UEOSCreateEOKSessionAsync::CreateEOKSession(
    WorldContextObject,
    SessionName,
    MaxPlayers,
    bIsLAN,
    bIsDedicated,
    CustomSettings
);
```

**Features:**
- Custom session attributes
- Presence settings
- Invite permissions
- Join-in-progress support

##### 2. **Session Search & Discovery**

**Blueprint Node:** `Find EOK Sessions`
- **Search Filters:**
  - Max Results
  - Custom Attribute Filters
  - Presence Filter

**C++ Usage:**
```cpp
UEOSFindEOKSessionsAsync* FindSessions = UEOSFindEOKSessionsAsync::FindEOKSessions(
    WorldContextObject,
    MaxSearchResults,
    SearchFilters
);
```

**Search by ID:**
```cpp
UEOSFindEOKSessionByIDAsync::FindEOKSessionByID(WorldContextObject, SessionId);
```

##### 3. **Joining Sessions**

**Blueprint Node:** `Join EOK Session`
- Inputs: Session Search Result
- Handles automatic connection

**Features:**
- Auto-connection to game server
- Session state synchronization
- Player registration

##### 4. **Session Management**

**Available Operations:**
- `Update EOK Session` - Modify session settings
- `Start Session` - Mark session as in-progress
- `End Session` - Mark session as ended
- `Destroy EOK Session` - Remove session
- `Register Players` - Add players to session
- `Unregister Players` - Remove players from session

##### 5. **Session Invites**

**Blueprint Nodes:**
- `Send Invite` - Send session invite to player
- `Query Invites` - Get pending invites
- `Accept Invite` - Accept session invite
- `Reject Invite` - Decline session invite

##### 6. **Session Attributes**

**Blueprint Node:** `Get Session Attribute`
- Get custom session data
- Real-time attribute updates

---

## Lobbies

### Module: `EOSKitLobbies`

#### Features

##### 1. **Lobby Creation**

**Blueprint Node:** `Create Lobby`
- **Parameters:**
  - Lobby Name
  - Max Members
  - Permission Level (Public/Private/Friends)
  - Allow Invites
  - Bucket ID (for matchmaking)

**C++ Usage:**
```cpp
#include "EOSCreateLobbyAsync.h"

UEOSCreateLobbyAsync* CreateLobby = UEOSCreateLobbyAsync::CreateLobby(
    WorldContextObject,
    MaxLobbyMembers,
    PermissionLevel,
    bAllowInvites
);
```

##### 2. **Lobby Search**

**Blueprint Node:** `Find Lobbies`
- Search by attributes
- Filter by bucket
- Maximum results limit

**Search Object:**
```cpp
UEOSLobbySearchFindAsync* Search = UEOSLobbySearchFindAsync::FindLobbies(
    WorldContextObject,
    MaxResults,
    SearchFilters
);
```

##### 3. **Joining Lobbies**

**Blueprint Node:** `Join Lobby`
- Join via search result
- Join via invite
- Join via lobby ID

##### 4. **Lobby Management**

**Available Operations:**
- `Update Lobby` - Modify lobby settings/attributes
- `Leave Lobby` - Leave current lobby
- `Destroy Lobby` - Remove lobby (owner only)
- `Kick Member` - Remove member (owner only)
- `Promote Member` - Transfer ownership

##### 5. **Lobby Invites**

**Blueprint Nodes:**
- `Send Lobby Invite` - Invite player to lobby
- `Query Lobby Invites` - Get pending lobby invites
- `Reject Lobby Invite` - Decline lobby invite

##### 6. **Real-time Updates**

**Subsystem Events:**
- OnLobbyMemberJoined
- OnLobbyMemberLeft
- OnLobbyUpdated
- OnLobbyOwnerChanged

**Blueprint Usage:**
```cpp
UEOSKitLobbySubsystem* LobbySubsystem = GetGameInstance()->GetSubsystem<UEOSKitLobbySubsystem>();
LobbySubsystem->OnLobbyMemberJoined.AddDynamic(this, &AMyActor::OnMemberJoined);
```

---

## Voice Chat (RTC)

### Module: `EOSKitVoice`

#### Features

##### 1. **Voice Room Management**

**Blueprint Functions:**
```cpp
// Get the voice subsystem
UEOSKitVoiceSubsystem* VoiceSubsystem = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();

// Connect to voice room
VoiceSubsystem->ConnectVoice(ProductUserId, RoomName);

// Disconnect from voice room
VoiceSubsystem->DisconnectVoice(ProductUserId, RoomName);
```

**Events:**
- `OnVoiceConnectionComplete` - Room join result
- `OnVoiceUserJoined` - Player joined voice
- `OnVoiceUserLeft` - Player left voice
- `OnVoiceUserTalking` - Player talking state changed

##### 2. **Microphone Control**

**Blueprint Function:**
```cpp
VoiceSubsystem->SetMicrophoneMuted(bIsMuted);
```

**Features:**
- Mute/unmute local microphone
- Per-player mute controls
- Volume adjustment

##### 3. **Audio Device Management**

**C++ Interface:**
```cpp
// Get available devices
TArray<FVoiceChatDeviceInfo> InputDevices = VoiceUser->GetAvailableInputDeviceInfos();
TArray<FVoiceChatDeviceInfo> OutputDevices = VoiceUser->GetAvailableOutputDeviceInfos();

// Set devices
VoiceUser->SetInputDeviceId(DeviceId);
VoiceUser->SetOutputDeviceId(DeviceId);
```

##### 4. **IVoiceChat Interface Integration**

Fully implements Unreal's standard `IVoiceChat` interface for compatibility with engine features.

**Features:**
- Login/Logout
- Channel join/leave
- Device management
- Mute controls
- Volume controls
- 3D positional audio support

##### 5. **Audio Playback**

**Automatic Features:**
- Real-time audio synthesis
- Per-user audio components
- Automatic cleanup
- Buffer management

**Implementation:**
```cpp
// Synth component per remote user
UEOSVoiceChatSynthComponent - AudioMixer-based playback
```

##### 6. **Voice Events**

**Blueprint Delegates:**
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceConnectionComplete, EEOSResult, Result, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserJoined, const FString&, UserId, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserLeft, const FString&, UserId, const FString&, RoomName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceUserTalking, const FString&, UserId, bool, bIsTalking);
```

---

## Achievements

### Module: `EOSKitAchievements`

#### Features

##### 1. **Query Achievement Definitions**

**Blueprint Node:** `Query Definitions`
- Retrieves all available achievements
- Includes unlock requirements
- Returns achievement metadata

**C++ Usage:**
```cpp
#include "Functions/EOSQueryDefinitionsAsync.h"

UEOSQueryDefinitionsAsync* QueryDefs = UEOSQueryDefinitionsAsync::QueryDefinitions(
    WorldContextObject,
    ProductUserId
);
```

##### 2. **Query Player Achievements**

**Blueprint Node:** `Query Player Achievements`
- Get player's achievement progress
- Unlock status
- Unlock time

**C++ Usage:**
```cpp
UEOSQueryPlayerAchievementsAsync* QueryProgress = 
    UEOSQueryPlayerAchievementsAsync::QueryPlayerAchievements(
        WorldContextObject,
        ProductUserId,
        TargetUserId
    );
```

##### 3. **Unlock Achievements**

**Blueprint Node:** `Unlock Achievement`
- Unlock specific achievement
- Automatic progress tracking

**C++ Usage:**
```cpp
UEOSUnlockAchievementAsync* UnlockAchv = 
    UEOSUnlockAchievementAsync::UnlockAchievement(
        WorldContextObject,
        ProductUserId,
        AchievementId
    );
```

**Features:**
- Async operation
- Success/failure callbacks
- Automatic synchronization with EOS

---

## Statistics & Leaderboards

### Module: `EOSKitStats`

#### Features

##### 1. **Write Player Stats**

**Blueprint Node:** `Write Player Data`
- **Parameters:**
  - Product User ID
  - Stats Array (TArray<FEOSStat>)

**Stat Structure:**
```cpp
USTRUCT(BlueprintType)
struct FEOSStat
{
    UPROPERTY(BlueprintReadWrite)
    FString StatName;
    
    UPROPERTY(BlueprintReadWrite)
    int32 Value;
};
```

**C++ Usage:**
```cpp
#include "EOSWritePlayerDataAsync.h"

TArray<FEOSStat> Stats;
Stats.Add(FEOSStat{TEXT("Kills"), 10});
Stats.Add(FEOSStat{TEXT("Deaths"), 5});

UEOSWritePlayerDataAsync* WriteStats = 
    UEOSWritePlayerDataAsync::WritePlayerData(
        WorldContextObject,
        ProductUserId,
        Stats
    );
```

##### 2. **Query Leaderboards**

**Blueprint Node:** `Query Leaderboards`
- **Parameters:**
  - Leaderboard Name
  - Start Rank
  - Max Results

**C++ Usage:**
```cpp
#include "EOSQueryLeaderboardsAsync.h"

UEOSQueryLeaderboardsAsync* QueryLeaderboard = 
    UEOSQueryLeaderboardsAsync::QueryLeaderboards(
        WorldContextObject,
        ProductUserId,
        LeaderboardName,
        StartRank,
        MaxResults
    );
```

**Returns:**
- Player ranks
- Player scores
- Player user IDs

---

## Title Storage

### Module: `EOSKitTitleStorage`

#### Features

##### 1. **Query File List**

**Blueprint Node:** `Query File List`
- Lists all available cloud files
- Returns file metadata

**C++ Usage:**
```cpp
#include "Functions/EOSQueryFileListAsync.h"

UEOSQueryFileListAsync* QueryFiles = 
    UEOSQueryFileListAsync::QueryFileList(
        WorldContextObject,
        ProductUserId
    );
```

**Returns:**
```cpp
USTRUCT(BlueprintType)
struct FEOSTitleFile
{
    UPROPERTY(BlueprintReadOnly)
    FString FileName;
    
    UPROPERTY(BlueprintReadOnly)
    int32 FileSize;
    
    UPROPERTY(BlueprintReadOnly)
    FString MD5Hash;
};
```

##### 2. **Read/Download Files**

**Blueprint Node:** `Read File`
- Downloads file from cloud storage
- Returns file data as byte array

**C++ Usage:**
```cpp
#include "Functions/EOSReadFileAsync.h"

UEOSReadFileAsync* ReadFile = 
    UEOSReadFileAsync::ReadFile(
        WorldContextObject,
        ProductUserId,
        FileName
    );
```

**Use Cases:**
- Configuration files
- Game data
- Dynamic content
- Updates and patches

---

## Networking & Sockets

### Module: `EOSKitSockets`

#### Features

##### 1. **EOS Socket Subsystem**

**Implementation:**
- `USocketSubsystemEOS` - EOS-based socket implementation
- NAT traversal support
- P2P connections
- Low-latency networking

##### 2. **EOS NetDriver**

**Classes:**
- `UNetDriverEOS` - Network driver for EOS sockets
- `UNetDriverEOSKit` - Extended net driver with additional features
- `UNetDriverEOSKitBase` - Base class for custom implementations

**Configuration:**
```ini
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/EOSKitSockets.NetDriverEOS",DriverClassNameFallback="/Script/EOSKitSockets.NetDriverEOS")
```

##### 3. **Internet Address (EOS)**

**Class:** `FInternetAddrEOS`
- EOS-specific addressing
- Product User ID based addressing
- Socket name support

##### 4. **EOS Socket Implementation**

**Class:** `FSocketEOS`
- Implements FSocket interface
- Send/Receive operations
- Connection management
- Async operations

**Use Cases:**
- Peer-to-peer multiplayer
- Client-server architecture
- Custom networking protocols
- Voice chat transport

---

## Web Services

### Module: `EOSKitWeb`

#### Features

##### 1. **Anti-Cheat**

**Blueprint Node:** `Begin Anti-Cheat Session`
- Initializes anti-cheat for session
- Server-side validation

**C++ Usage:**
```cpp
#include "Functions/AntiCheat/EOSAntiCheatBeginSessionAsync.h"

UEOSAntiCheatBeginSessionAsync* BeginAC = 
    UEOSAntiCheatBeginSessionAsync::BeginAntiCheatSession(
        WorldContextObject,
        SessionName,
        RegisterTimeout
    );
```

##### 2. **Player Reports**

**Blueprint Node:** `Report Player`
- Report player for violations
- Specify reason and category

**C++ Usage:**
```cpp
#include "Functions/PlayerReports/EOSReportPlayerAsync.h"

UEOSReportPlayerAsync* Report = 
    UEOSReportPlayerAsync::ReportPlayer(
        WorldContextObject,
        ReportedUserId,
        ReportCategory,
        ReasonText
    );
```

##### 3. **Sanctions**

**Blueprint Node:** `Query Sanctions`
- Check player sanctions/bans
- Get active punishments

**C++ Usage:**
```cpp
#include "Functions/Sanctions/EOSQuerySanctionsAsync.h"

UEOSQuerySanctionsAsync* QuerySanctions = 
    UEOSQuerySanctionsAsync::QuerySanctions(
        WorldContextObject,
        TargetUserId
    );
```

##### 4. **E-Commerce**

**Blueprint Node:** `Ecom Query Offers`
- Query available in-game purchases
- Get catalog items

**C++ Usage:**
```cpp
#include "Functions/Ecom/EOSEcomQueryOffersAsync.h"

UEOSEcomQueryOffersAsync* QueryOffers = 
    UEOSEcomQueryOffersAsync::QueryOffers(
        WorldContextObject,
        ProductUserId
    );
```

##### 5. **External Account Mapping**

**Blueprint Node:** `Query External Account Mappings`
- Map external platform accounts to EOS accounts
- Cross-platform account discovery

---

## Configuration

### DefaultEngine.ini Settings

```ini
[/Script/EOSKit.EOSKitSettings]
; Authentication
bEnableAutoLogin=True
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET

; Networking
bEnableP2P=True
bEnableVoiceChat=True

; Sessions
bEnablePresence=True
bEnableSessions=True

; Voice Chat
bEnableVoiceRTC=True
DefaultInputDevice=""
DefaultOutputDevice=""

; NetDriver
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/EOSKitSockets.NetDriverEOS",DriverClassNameFallback="/Script/EOSKitSockets.NetDriverEOS")

[OnlineSubsystem]
DefaultPlatformService=EOSKit

[OnlineSubsystemEOSKit]
bEnabled=true
```

---

## Advanced Features

### 1. **Subsystem Architecture**

**Engine Subsystems:**
- `UEOSKitEngineSubsystem` - Engine-level management
- `UEOSKitVoiceSubsystem` - Voice chat management

**Game Instance Subsystems:**
- `UEOSKitSubsystem` - Main EOS platform interface
- `UEOSKitGameInstanceSubsystem` - Game instance specific features
- `UEOSKitSessionsSubsystem` - Session management
- `UEOSKitLobbySubsystem` - Lobby management
- `UEOSKitConnectSubsystem` - Connect interface management

**Access Pattern:**
```cpp
// Engine Subsystem
UEOSKitVoiceSubsystem* VoiceSys = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();

// Game Instance Subsystem
UEOSKitSubsystem* EOSSys = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
```

### 2. **Event-Driven Architecture**

**Multicast Delegates:**
```cpp
// Voice Events
UPROPERTY(BlueprintAssignable)
FOnVoiceConnectionComplete OnVoiceConnectionComplete;

// Lobby Events
UPROPERTY(BlueprintAssignable)
FOnLobbyMemberJoined OnLobbyMemberJoined;

// Session Events
UPROPERTY(BlueprintAssignable)
FOnSessionCreated OnSessionCreated;
```

**Blueprint Usage:**
```cpp
// Bind to events
Event OnVoiceConnectionComplete -> Custom Logic
```

### 3. **Async Node Pattern**

All async operations follow a consistent pattern:

**Blueprint:**
- Node name describes operation
- Outputs: OnSuccess, OnFailure
- Automatic activation

**C++ Creation:**
```cpp
static UMyAsyncNode* MyOperation(
    UObject* WorldContextObject,
    FOperationParams Params
)
{
    UMyAsyncNode* Node = NewObject<UMyAsyncNode>();
    Node->Params = Params;
    return Node;
}
```

### 4. **Error Handling**

**Shared Error Types:**
```cpp
UENUM(BlueprintType)
enum class EEOSResult : uint8
{
    Success,
    Failed,
    InvalidParameters,
    InvalidState,
    InvalidUser,
    NotLoggedIn,
    NotFound,
    AlreadyExists,
    TimedOut,
    Cancelled
};
```

### 5. **Platform Handle Access**

**Direct EOS SDK Access:**
```cpp
UEOSKitSubsystem* EOSSubsystem = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
EOS_HPlatform PlatformHandle = EOSSubsystem->GetPlatformHandle();

// Use with any EOS SDK function
EOS_Auth_Login(AuthHandle, &LoginOptions, nullptr, &OnLoginComplete);
```

### 6. **Product User ID Management**

**Getting Product User ID:**
```cpp
UEOSKitSubsystem* EOSSubsystem = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
EOS_ProductUserId PUID = EOSSubsystem->GetProductUserId();

// Convert to string
FString PUIDString = UTF8_TO_TCHAR(EOS_ProductUserId_ToString(PUID));
```

### 7. **Online Subsystem Integration**

**EOSKit integrates with Unreal's Online Subsystem:**
```cpp
IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(TEXT("EOSKit"));
IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
```

---

## Quick Start Examples

### Example 1: Complete Login Flow

**Blueprint:**
1. Add "EOS Login" node
2. Set Login Type to "Account Portal"
3. Connect OnSuccess to "Connect Voice" node
4. Connect to your game logic

**C++:**
```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Start login
    UEOSLoginAsync* Login = UEOSLoginAsync::EOSLogin(
        this, 
        EEOSLoginType::AccountPortal,
        FEOSLoginCredentials()
    );
    
    Login->OnSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    Login->Activate();
}

void AMyGameMode::OnLoginSuccess(FString EpicAccountId, FString ProductUserId)
{
    UE_LOG(LogTemp, Log, TEXT("Logged in as: %s"), *ProductUserId);
    
    // Store for later use
    this->LocalProductUserId = ProductUserId;
}
```

### Example 2: Create and Join Session

**Blueprint:**
1. "Create EOK Session" node
2. Set Max Players, Session Name
3. OnSuccess -> "Find EOK Sessions"
4. "Join EOK Session" with search result

**C++:**
```cpp
// Create session
void AMyGameMode::CreateMultiplayerSession()
{
    TMap<FString, FString> Settings;
    Settings.Add(TEXT("GameMode"), TEXT("Deathmatch"));
    Settings.Add(TEXT("MapName"), TEXT("Arena01"));
    
    UEOSCreateEOKSessionAsync* CreateSession = 
        UEOSCreateEOKSessionAsync::CreateEOKSession(
            this,
            TEXT("MySession"),
            4, // Max players
            false, // Not LAN
            false, // Not dedicated
            Settings
        );
    
    CreateSession->OnSuccess.AddDynamic(this, &AMyGameMode::OnSessionCreated);
    CreateSession->Activate();
}

// Find sessions
void AMyGameMode::FindSessions()
{
    UEOSFindEOKSessionsAsync* FindSessions = 
        UEOSFindEOKSessionsAsync::FindEOKSessions(
            this,
            10, // Max results
            TMap<FString, FString>() // No filters
        );
    
    FindSessions->OnSuccess.AddDynamic(this, &AMyGameMode::OnSessionsFound);
    FindSessions->Activate();
}
```

### Example 3: Voice Chat Integration

**Blueprint:**
1. Get EOSKitVoiceSubsystem
2. "Connect Voice" with ProductUserId and RoomName
3. Bind to OnVoiceUserJoined event
4. "Set Microphone Muted" to control mic

**C++:**
```cpp
void AMyPlayerController::EnableVoiceChat()
{
    UEOSKitVoiceSubsystem* VoiceSys = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
    
    // Bind to events
    VoiceSys->OnVoiceConnectionComplete.AddDynamic(this, &AMyPlayerController::OnVoiceConnected);
    VoiceSys->OnVoiceUserJoined.AddDynamic(this, &AMyPlayerController::OnUserJoinedVoice);
    VoiceSys->OnVoiceUserTalking.AddDynamic(this, &AMyPlayerController::OnUserTalking);
    
    // Connect to voice room
    FString RoomName = TEXT("Game_Room_1");
    VoiceSys->ConnectVoice(LocalProductUserId, RoomName);
}

void AMyPlayerController::ToggleMicrophone(bool bMute)
{
    UEOSKitVoiceSubsystem* VoiceSys = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
    VoiceSys->SetMicrophoneMuted(bMute);
}
```

---

## Troubleshooting

### Common Issues

#### 1. **ProductUserId is Null**
**Problem:** Can't use features because ProductUserId is invalid.

**Solution:**
- Ensure auto-login is configured correctly
- Check `OnAutoLoginComplete` fires
- Verify DevPortal configuration
- Use manual login if auto-login fails

#### 2. **Voice Chat No Audio**
**Problem:** Can connect but no audio heard.

**Solution:**
- Check microphone permissions
- Verify audio devices in Unreal settings
- Check if `WITH_EOS_RTC=1` is defined
- Ensure voice subsystem initialized

#### 3. **Session Not Found**
**Problem:** Created session but can't find it in search.

**Solution:**
- Wait a few seconds after creation
- Check session attributes match search filters
- Verify presence settings enabled
- Check bucket ID configuration

#### 4. **Build Errors**
**Problem:** Plugin won't compile.

**Solution:**
- Verify EOS SDK properly installed
- Check module dependencies in `.Build.cs`
- Ensure correct engine version (5.3+)
- Regenerate project files

---

## Best Practices

### 1. **Always Check Login State**
```cpp
UEOSKitSubsystem* EOSSys = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
if (!EOSSys || !EOSSys->GetProductUserId())
{
    // Handle not logged in
    return;
}
```

### 2. **Use Subsystem Events**
Prefer binding to subsystem events over polling:
```cpp
VoiceSubsystem->OnVoiceUserJoined.AddDynamic(this, &AMyClass::OnUserJoined);
```

### 3. **Handle Async Node Failures**
Always implement OnFailure handlers:
```cpp
AsyncNode->OnSuccess.AddDynamic(this, &AMyClass::OnSuccess);
AsyncNode->OnFailure.AddDynamic(this, &AMyClass::OnFailure);
```

### 4. **Clean Up Resources**
```cpp
void AMyClass::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    // Disconnect voice
    if (VoiceSubsystem)
    {
        VoiceSubsystem->DisconnectVoice(ProductUserId, RoomName);
    }
    
    // Leave session
    if (SessionInterface.IsValid())
    {
        SessionInterface->DestroySession(SessionName);
    }
}
```

### 5. **Error Logging**
```cpp
UE_LOG(LogEOSKit, Error, TEXT("Operation failed: %s"), *ErrorMessage);
```

---

## Support & Resources

### Documentation Files
- `/Plugins/EOSKit/README.md` - Main plugin documentation
- `/Plugins/EOSKit/Documentation/QuickStart.md` - Getting started guide
- `/Plugins/EOSKit/Documentation/API_Reference.md` - Detailed API reference
- `/Plugins/EOSKit/Source/EOSKitVoice/README_VOICE_MODULE.md` - Voice chat guide

### EOS Developer Portal
- [EOS Documentation](https://dev.epicgames.com/docs/services)
- [Dev Portal](https://dev.epicgames.com/portal)

### Community
- [EOS Discord](https://discord.gg/epic-games)
- [Unreal Engine Forums](https://forums.unrealengine.com/)

---

## Version Information

**Plugin Version:** 1.0.0
**Supported EOS SDK:** 1.16+
**Unreal Engine:** 5.3, 5.4, 5.5
**Last Updated:** 2024

---

## License

Copyright (C) 2024, All Rights Reserved.

This plugin integrates Epic Online Services SDK.
See EOS licensing terms at: https://dev.epicgames.com/docs/services/en-US/eos-sdk-download-types.html

---

## Changelog

### Version 1.0.0
- Initial release
- Full authentication support
- Sessions and matchmaking
- Lobbies system
- Voice chat (RTC)
- Achievements
- Statistics
- Title storage
- Networking sockets
- Web services integration

---

*For detailed implementation guides, see individual module documentation files.*
