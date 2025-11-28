# EOSKit Quick Reference Guide

## Table of Contents
- [Module Overview](#module-overview)
- [Common Operations](#common-operations)
- [Blueprint Node Reference](#blueprint-node-reference)
- [C++ API Quick Reference](#c-api-quick-reference)
- [Configuration Snippets](#configuration-snippets)

---

## Module Overview

| Module | Purpose | Key Classes |
|--------|---------|-------------|
| **EOSKit** | Core platform, subsystems | `UEOSKitSubsystem`, `UEOSKitEngineSubsystem` |
| **EOSKitAuth** | Login, authentication | `UEOSLoginAsync`, `UEOSConnectLoginAsync` |
| **EOSKitSessions** | Matchmaking, sessions | `UEOSCreateEOKSessionAsync`, `UEOSFindEOKSessionsAsync` |
| **EOSKitLobbies** | Lobby system | `UEOSCreateLobbyAsync`, `UEOSJoinLobbyAsync` |
| **EOSKitVoice** | Voice chat (RTC) | `UEOSKitVoiceSubsystem`, `FEOSVoiceChat` |
| **EOSKitAchievements** | Achievements | `UEOSQueryDefinitionsAsync`, `UEOSUnlockAchievementAsync` |
| **EOSKitStats** | Leaderboards, stats | `UEOSWritePlayerDataAsync`, `UEOSQueryLeaderboardsAsync` |
| **EOSKitTitleStorage** | Cloud files | `UEOSQueryFileListAsync`, `UEOSReadFileAsync` |
| **EOSKitSockets** | Networking | `USocketSubsystemEOS`, `UNetDriverEOS` |
| **EOSKitWeb** | Web services | Anti-cheat, Reports, Ecom |

---

## Common Operations

### Get Subsystems

```cpp
// Engine Subsystem (C++)
UEOSKitVoiceSubsystem* VoiceSys = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();

// Game Instance Subsystem (C++)
UEOSKitSubsystem* EOSSys = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
UEOSKitSessionsSubsystem* SessionSys = GetGameInstance()->GetSubsystem<UEOSKitSessionsSubsystem>();
```

### Get Product User ID

```cpp
// C++
UEOSKitSubsystem* EOSSys = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
EOS_ProductUserId PUID = EOSSys->GetProductUserId();

// Convert to string
FString PUIDStr = UTF8_TO_TCHAR(EOS_ProductUserId_ToString(PUID));
```

### Get Platform Handle

```cpp
// C++
UEOSKitSubsystem* EOSSys = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
EOS_HPlatform Platform = EOSSys->GetPlatformHandle();
```

---

## Blueprint Node Reference

### Authentication

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **EOS Login** | LoginType, Credentials | OnSuccess(EpicId, ProductUserId), OnFailure | Login with Epic account |
| **EOS Connect Login** | AuthToken, AuthType | OnSuccess(ProductUserId), OnFailure | Login with external account |
| **Create Device ID** | DeviceModel | OnSuccess, OnFailure | Create device-based account |
| **Link Account** | AuthType, AuthToken | OnSuccess, OnFailure | Link external account |

### Sessions

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Create EOK Session** | SessionName, MaxPlayers, Settings | OnSuccess, OnFailure | Create multiplayer session |
| **Find EOK Sessions** | MaxResults, Filters | OnSuccess(Sessions), OnFailure | Search for sessions |
| **Join EOK Session** | SearchResult | OnSuccess, OnFailure | Join found session |
| **Destroy EOK Session** | SessionName | OnSuccess, OnFailure | Delete session |
| **Send Invite** | TargetUserId, SessionName | OnSuccess, OnFailure | Invite player to session |

### Lobbies

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Create Lobby** | MaxMembers, Permission | OnSuccess(LobbyId), OnFailure | Create lobby |
| **Find Lobbies** | MaxResults, Filters | OnSuccess(Lobbies), OnFailure | Search lobbies |
| **Join Lobby** | LobbyId | OnSuccess, OnFailure | Join lobby |
| **Update Lobby** | LobbyId, Attributes | OnSuccess, OnFailure | Update lobby settings |
| **Kick Member** | LobbyId, UserId | OnSuccess, OnFailure | Remove member |

### Voice Chat

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Connect Voice** | UserId, RoomName | OnComplete(Result) | Join voice room |
| **Disconnect Voice** | UserId, RoomName | - | Leave voice room |
| **Set Microphone Muted** | bMuted | - | Mute/unmute mic |

### Achievements

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Query Definitions** | ProductUserId | OnSuccess(Definitions), OnFailure | Get achievements list |
| **Query Player Achievements** | UserId, TargetUserId | OnSuccess(Progress), OnFailure | Get player progress |
| **Unlock Achievement** | UserId, AchievementId | OnSuccess, OnFailure | Unlock achievement |

### Statistics

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Write Player Data** | UserId, Stats[] | OnSuccess, OnFailure | Update player stats |
| **Query Leaderboards** | Name, StartRank, MaxResults | OnSuccess(Entries), OnFailure | Get leaderboard |

### Title Storage

| Node Name | Inputs | Outputs | Description |
|-----------|--------|---------|-------------|
| **Query File List** | ProductUserId | OnSuccess(Files[]), OnFailure | List cloud files |
| **Read File** | UserId, FileName | OnSuccess(Data[]), OnFailure | Download file |

---

## C++ API Quick Reference

### Login

```cpp
#include "EOSLoginAsync.h"

// Epic Account Login
UEOSLoginAsync* Login = UEOSLoginAsync::EOSLogin(
    WorldContext,
    EEOSLoginType::AccountPortal,
    FEOSLoginCredentials()
);
Login->OnSuccess.AddDynamic(this, &AMyClass::OnLoginSuccess);
Login->Activate();
```

### Create Session

```cpp
#include "EOSCreateEOKSessionAsync.h"

TMap<FString, FString> Settings;
Settings.Add(TEXT("Map"), TEXT("Arena"));

UEOSCreateEOKSessionAsync* Create = UEOSCreateEOKSessionAsync::CreateEOKSession(
    this,
    TEXT("MySession"),
    4, // Max players
    false, false,
    Settings
);
Create->OnSuccess.AddDynamic(this, &AMyClass::OnCreated);
Create->Activate();
```

### Find Sessions

```cpp
#include "EOSFindEOKSessionsAsync.h"

UEOSFindEOKSessionsAsync* Find = UEOSFindEOKSessionsAsync::FindEOKSessions(
    this,
    10, // Max results
    TMap<FString, FString>() // Filters
);
Find->OnSuccess.AddDynamic(this, &AMyClass::OnFound);
Find->Activate();
```

### Create Lobby

```cpp
#include "EOSCreateLobbyAsync.h"

UEOSCreateLobbyAsync* CreateLobby = UEOSCreateLobbyAsync::CreateLobby(
    this,
    4, // Max members
    ELobbyPermissionLevel::PublicAdvertised,
    true // Allow invites
);
CreateLobby->OnSuccess.AddDynamic(this, &AMyClass::OnLobbyCreated);
CreateLobby->Activate();
```

### Voice Chat

```cpp
// Get subsystem
UEOSKitVoiceSubsystem* Voice = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();

// Bind events
Voice->OnVoiceConnectionComplete.AddDynamic(this, &AMyClass::OnVoiceConnected);
Voice->OnVoiceUserJoined.AddDynamic(this, &AMyClass::OnUserJoined);

// Connect
Voice->ConnectVoice(ProductUserId, TEXT("Room1"));

// Mute
Voice->SetMicrophoneMuted(true);
```

### Unlock Achievement

```cpp
#include "Functions/EOSUnlockAchievementAsync.h"

UEOSUnlockAchievementAsync* Unlock = UEOSUnlockAchievementAsync::UnlockAchievement(
    this,
    ProductUserId,
    TEXT("achievement_first_kill")
);
Unlock->OnSuccess.AddDynamic(this, &AMyClass::OnUnlocked);
Unlock->Activate();
```

### Write Stats

```cpp
#include "EOSWritePlayerDataAsync.h"

TArray<FEOSStat> Stats;
Stats.Add(FEOSStat{TEXT("Kills"), 10});
Stats.Add(FEOSStat{TEXT("Deaths"), 3});

UEOSWritePlayerDataAsync* Write = UEOSWritePlayerDataAsync::WritePlayerData(
    this,
    ProductUserId,
    Stats
);
Write->OnSuccess.AddDynamic(this, &AMyClass::OnWritten);
Write->Activate();
```

### Query Leaderboard

```cpp
#include "EOSQueryLeaderboardsAsync.h"

UEOSQueryLeaderboardsAsync* Query = UEOSQueryLeaderboardsAsync::QueryLeaderboards(
    this,
    ProductUserId,
    TEXT("HighScores"),
    0, // Start rank
    10 // Max results
);
Query->OnSuccess.AddDynamic(this, &AMyClass::OnLeaderboard);
Query->Activate();
```

---

## Configuration Snippets

### DefaultEngine.ini - Basic Setup

```ini
[/Script/EOSKit.EOSKitSettings]
bEnableAutoLogin=True
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
```

### DefaultEngine.ini - Voice Chat

```ini
[/Script/EOSKit.EOSKitSettings]
bEnableVoiceRTC=True
DefaultInputDevice=""
DefaultOutputDevice=""
```

### DefaultEngine.ini - NetDriver

```ini
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/EOSKitSockets.NetDriverEOS",DriverClassNameFallback="/Script/EOSKitSockets.NetDriverEOS")
```

### DefaultEngine.ini - Online Subsystem

```ini
[OnlineSubsystem]
DefaultPlatformService=EOSKit

[OnlineSubsystemEOSKit]
bEnabled=true
```

---

## Event Delegates

### Voice Chat Events

```cpp
// Declare in header
UFUNCTION()
void OnVoiceConnected(EEOSResult Result, const FString& RoomName);

UFUNCTION()
void OnUserJoined(const FString& UserId, const FString& RoomName);

UFUNCTION()
void OnUserTalking(const FString& UserId, bool bIsTalking);

// Bind in code
Voice->OnVoiceConnectionComplete.AddDynamic(this, &AMyClass::OnVoiceConnected);
Voice->OnVoiceUserJoined.AddDynamic(this, &AMyClass::OnUserJoined);
Voice->OnVoiceUserTalking.AddDynamic(this, &AMyClass::OnUserTalking);
```

### Lobby Events

```cpp
UFUNCTION()
void OnLobbyMemberJoined(const FString& LobbyId, const FString& UserId);

UFUNCTION()
void OnLobbyUpdated(const FString& LobbyId);

// Bind
UEOSKitLobbySubsystem* Lobby = GetGameInstance()->GetSubsystem<UEOSKitLobbySubsystem>();
Lobby->OnLobbyMemberJoined.AddDynamic(this, &AMyClass::OnMemberJoined);
```

---

## Common Code Patterns

### Safe Subsystem Access

```cpp
UEOSKitSubsystem* GetEOSSubsystem()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("No GameInstance"));
        return nullptr;
    }
    
    UEOSKitSubsystem* EOS = GI->GetSubsystem<UEOSKitSubsystem>();
    if (!EOS)
    {
        UE_LOG(LogTemp, Error, TEXT("EOSKit subsystem not found"));
        return nullptr;
    }
    
    return EOS;
}
```

### Async Node with Error Handling

```cpp
void AMyClass::DoSomething()
{
    UMyAsyncNode* Node = UMyAsyncNode::DoOperation(this, Params);
    
    Node->OnSuccess.AddDynamic(this, &AMyClass::OnSuccess);
    Node->OnFailure.AddDynamic(this, &AMyClass::OnFailure);
    
    Node->Activate();
}

void AMyClass::OnSuccess(/* success params */)
{
    UE_LOG(LogTemp, Log, TEXT("Operation succeeded"));
}

void AMyClass::OnFailure(/* error params */)
{
    UE_LOG(LogTemp, Error, TEXT("Operation failed"));
}
```

### Product User ID Validation

```cpp
bool ValidateProductUserId(EOS_ProductUserId PUID)
{
    if (!EOS_ProductUserId_IsValid(PUID))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ProductUserId"));
        return false;
    }
    return true;
}
```

### Session Attribute Helpers

```cpp
TMap<FString, FString> CreateSessionSettings()
{
    TMap<FString, FString> Settings;
    Settings.Add(TEXT("GameMode"), TEXT("Deathmatch"));
    Settings.Add(TEXT("Map"), TEXT("Arena01"));
    Settings.Add(TEXT("MaxKills"), TEXT("50"));
    Settings.Add(TEXT("TimeLimit"), TEXT("600"));
    return Settings;
}
```

---

## Debugging Tips

### Enable Verbose Logging

```cpp
// In code
UE_LOG(LogEOSKit, VeryVerbose, TEXT("Debug info: %s"), *Info);

// In DefaultEngine.ini
[Core.Log]
LogEOSKit=VeryVerbose
LogOnline=Verbose
LogNet=Verbose
```

### Check Platform Handle

```cpp
UEOSKitSubsystem* EOS = GetEOSSubsystem();
if (!EOS || !EOS->GetPlatformHandle())
{
    UE_LOG(LogTemp, Error, TEXT("EOS Platform not initialized"));
    return;
}
```

### Check Login State

```cpp
UEOSKitSubsystem* EOS = GetEOSSubsystem();
EOS_ProductUserId PUID = EOS->GetProductUserId();
if (!EOS_ProductUserId_IsValid(PUID))
{
    UE_LOG(LogTemp, Error, TEXT("User not logged in"));
    return;
}
```

### Print Session Info

```cpp
void PrintSessionInfo(const FOnlineSession& Session)
{
    UE_LOG(LogTemp, Log, TEXT("Session: %s"), *Session.SessionName.ToString());
    UE_LOG(LogTemp, Log, TEXT("  Players: %d / %d"), 
        Session.SessionSettings.NumPublicConnections - Session.NumOpenPublicConnections,
        Session.SessionSettings.NumPublicConnections);
    
    for (const auto& Setting : Session.SessionSettings.Settings)
    {
        FString Value;
        Setting.Value.Data.GetValue(Value);
        UE_LOG(LogTemp, Log, TEXT("  %s = %s"), *Setting.Key.ToString(), *Value);
    }
}
```

---

## Performance Tips

### 1. Cache Subsystem References
```cpp
// In BeginPlay
VoiceSubsystem = GEngine->GetEngineSubsystem<UEOSKitVoiceSubsystem>();
EOSSubsystem = GetGameInstance()->GetSubsystem<UEOSKitSubsystem>();
```

### 2. Limit Search Results
```cpp
// Don't request too many
UEOSFindEOKSessionsAsync::FindEOKSessions(this, 10, Filters); // Good
UEOSFindEOKSessionsAsync::FindEOKSessions(this, 1000, Filters); // Bad
```

### 3. Debounce Voice Events
```cpp
void OnUserTalking(const FString& UserId, bool bTalking)
{
    // Update UI sparingly
    if (bTalking)
    {
        GetWorldTimerManager().SetTimer(
            TalkingTimerHandle,
            FTimerDelegate::CreateUObject(this, &AMyClass::UpdateTalkingUI, UserId),
            0.5f, // Update every 0.5s max
            false
        );
    }
}
```

### 4. Async Node Cleanup
```cpp
// Nodes are automatically garbage collected
// But you can help by clearing strong references
void Cleanup()
{
    AsyncNodeRef = nullptr; // Let GC handle it
}
```

---

## Common Error Codes

| Error | Meaning | Solution |
|-------|---------|----------|
| `InvalidParameters` | Bad input values | Check all parameters are valid |
| `NotLoggedIn` | User not authenticated | Ensure login completes first |
| `InvalidState` | Operation not allowed now | Check state (e.g., already in session) |
| `NotFound` | Resource doesn't exist | Verify IDs are correct |
| `TimedOut` | Network timeout | Check internet connection |
| `AlreadyExists` | Duplicate resource | Use different name/ID |

---

## Module Dependencies

Add to `YourModule.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "EOSKit",           // Core
    "EOSKitShared",     // Shared types
    "EOSKitAuth",       // Authentication
    "EOSKitSessions",   // Sessions
    "EOSKitVoice",      // Voice chat
    "EOSSDK"            // EOS SDK
});
```

---

## Include Headers

Common includes needed:

```cpp
// Core
#include "EOSKitSubsystem.h"
#include "EOSKitSharedTypes.h"

// Auth
#include "EOSLoginAsync.h"
#include "Functions/Connect/EOSConnectLoginAsync.h"

// Sessions
#include "EOSCreateEOKSessionAsync.h"
#include "EOSFindEOKSessionsAsync.h"
#include "EOSJoinEOKSessionAsync.h"

// Voice
#include "Subsystem/EOSKitVoiceSubsystem.h"

// Lobbies
#include "EOSCreateLobbyAsync.h"
#include "EOSJoinLobbyAsync.h"

// Achievements
#include "Functions/EOSUnlockAchievementAsync.h"
#include "Functions/EOSQueryDefinitionsAsync.h"

// Stats
#include "EOSWritePlayerDataAsync.h"
#include "EOSQueryLeaderboardsAsync.h"
```

---

*For detailed information, see FEATURES_DOCUMENTATION.md*
