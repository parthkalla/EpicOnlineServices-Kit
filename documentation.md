# 🎮 Extreme Multiplayer Lobby System Documentation

## Table of Contents
1. [Overview](#overview)
2. [Getting Started](#getting-started)
3. [Lobby Lifecycle](#lobby-lifecycle)
4. [Lobby Search & Matchmaking](#lobby-search--matchmaking)
5. [Lobby Invitations](#lobby-invitations)
6. [Lobby Notifications & Events](#lobby-notifications--events)
7. [Member Management](#member-management)
8. [Lobby Attributes & Customization](#lobby-attributes--customization)
9. [Blueprint Examples](#blueprint-examples)
10. [C++ Examples](#c-examples)
11. [Best Practices](#best-practices)
12. [Troubleshooting](#troubleshooting)

---

## Overview

The **Extreme Multiplayer Lobby System** is a comprehensive, production-ready lobby implementation built on Epic Online Services (EOS) SDK. It provides everything you need to create scalable, feature-rich multiplayer lobbies for your game.

### Key Features

✅ **Complete Lobby Lifecycle Management**
- Create public or private lobbies with custom settings
- Join lobbies via search, invite, or direct ID
- Update lobby settings in real-time
- Leave or destroy lobbies cleanly

✅ **Advanced Search & Matchmaking**
- Search lobbies with custom filters
- Support for bucketed matchmaking
- Region-based lobby discovery
- Attribute-based filtering

✅ **Rich Invitation System**
- Send invitations to friends
- Accept or reject invitations
- Query pending invites
- Overlay integration for seamless UX

✅ **Real-Time Notifications**
- Join/leave notifications
- Lobby update events
- Member status changes
- Invitation events

✅ **Powerful Member Management**
- Kick members (owner only)
- Promote members to owner
- Per-member attributes
- Member status tracking

✅ **Voice Chat Integration**
- Optional voice chat support
- Automatic RTC room creation
- Per-lobby voice configuration

✅ **Host Migration Support**
- Graceful host migration
- Automatic ownership transfer
- Maintain lobby state during migration

---

## Getting Started

### Prerequisites

Before using the lobby system, ensure you have:

1. **EOSKit Plugin Installed** - See main README.md for installation
2. **EOS Credentials Configured** - Set up in `DefaultEngine.ini`
3. **User Logged In** - User must be authenticated via EOS Auth
4. **Online Subsystem** - EOSKit Online Subsystem enabled

### Basic Configuration

Add to your `DefaultEngine.ini`:

```ini
[/Script/EOSKit.EOSKitSettings]
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
bEnableAutoLogin=True

[OnlineSubsystem]
DefaultPlatformService=EOSKit

[OnlineSubsystemEOSKit]
bEnabled=true
```

### Quick Start Example

Here's a minimal example to create and join a lobby:

**Blueprint:**
1. Create Lobby node → Set Max Players to 4
2. On Success → Display Lobby ID
3. Find Lobbies node → Search for available lobbies
4. Join Lobby node → Join selected lobby

**C++:**
```cpp
// Create a lobby
UEOSCreateLobbyAsync* CreateLobby = UEOSCreateLobbyAsync::CreateLobby(
    this, 
    4,              // Max players
    "DefaultBucket", // Bucket ID
    true            // Is public
);
CreateLobby->OnSuccess.AddDynamic(this, &AMyClass::OnLobbyCreated);
CreateLobby->Activate();
```

---

## Lobby Lifecycle

### Creating a Lobby

Lobbies are created using `UEOSCreateLobbyAsync` or `UEOSCreateEOKLobbyAsync` (for advanced settings).

#### Basic Lobby Creation

**Blueprint:**
- Use the **Create Lobby** node
- Set **Max Players** (2-100)
- Set **Bucket Id** for matchmaking
- Set **Is Public** for visibility

**C++:**
```cpp
UEOSCreateLobbyAsync* CreateLobby = UEOSCreateLobbyAsync::CreateLobby(
    WorldContextObject,
    8,                  // Max players
    TEXT("RankedMatch"), // Bucket ID
    true                // Public lobby
);

CreateLobby->OnSuccess.AddDynamic(this, &AGameMode::OnLobbyCreated);
CreateLobby->OnFailure.AddDynamic(this, &AGameMode::OnLobbyCreateFailed);
CreateLobby->Activate();

void AGameMode::OnLobbyCreated(const FString& LobbyId)
{
    UE_LOG(LogTemp, Log, TEXT("Lobby created: %s"), *LobbyId);
    // Store lobby ID for future operations
}
```

#### Advanced Lobby Creation with Settings

For more control, use `UEOSCreateEOKLobbyAsync` with `FEOSKitCreateLobbySettings`:

**C++:**
```cpp
FEOSKitCreateLobbySettings Settings;
Settings.SessionName = TEXT("MyAwesomeLobby");
Settings.NumberOfPublicConnections = 4;
Settings.NumberOfPrivateConnections = 0;
Settings.bAllowInvites = true;
Settings.bShouldAdvertise = true;
Settings.bAllowJoinInProgress = true;
Settings.bUseVoiceChat = true;  // Enable voice chat
Settings.bUsePresence = true;
Settings.Region = EEOSKitRegion::NAEast;
Settings.BucketId = TEXT("CompetitiveMatch");
Settings.bSupportHostMigration = true;
Settings.bEnableJoinViaId = true;

// Add custom attributes
FEOSKitAttribute GameMode;
GameMode.Key = TEXT("GameMode");
GameMode.StringValue = TEXT("CaptureTheFlag");
Settings.ExtraSettings.Add(TEXT("GameMode"), GameMode);

FEOSKitAttribute DifficultyLevel;
DifficultyLevel.Key = TEXT("Difficulty");
DifficultyLevel.IntValue = 3;
Settings.ExtraSettings.Add(TEXT("Difficulty"), DifficultyLevel);

UEOSCreateEOKLobbyAsync* CreateLobby = UEOSCreateEOKLobbyAsync::CreateEOKLobby(
    WorldContextObject,
    Settings
);

CreateLobby->OnSuccess.AddDynamic(this, &AGameMode::OnAdvancedLobbyCreated);
CreateLobby->Activate();
```

### Joining a Lobby

Players can join lobbies through search results, invitations, or direct lobby ID.

#### Join via Search Result

**C++:**
```cpp
// After finding lobbies (see Search section)
void AGameMode::OnLobbiesFound(const TArray<FEOSLobbyInfo>& Lobbies)
{
    if (Lobbies.Num() > 0)
    {
        // Join the first available lobby
        FBlueprintSessionResult SessionResult = ConvertToSessionResult(Lobbies[0]);
        
        UEOSJoinLobbyAsync* JoinLobby = UEOSJoinLobbyAsync::JoinEOSKitSession(
            WorldContextObject,
            TEXT("GameSession"),
            SessionResult,
            false  // Not LAN
        );
        
        JoinLobby->Success.AddDynamic(this, &AGameMode::OnJoinSuccess);
        JoinLobby->Failure.AddDynamic(this, &AGameMode::OnJoinFailure);
        JoinLobby->Activate();
    }
}

void AGameMode::OnJoinSuccess(EEOSKitJoinResult Result, FString SessionJoinAddress)
{
    if (Result == EEOSKitJoinResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully joined lobby at: %s"), *SessionJoinAddress);
        // Optionally travel to the session
        GetWorld()->GetFirstPlayerController()->ClientTravel(SessionJoinAddress, TRAVEL_Absolute);
    }
}
```

### Updating a Lobby

Only the lobby owner can update lobby settings.

**C++:**
```cpp
// Update lobby to change max players or other settings
void AGameMode::UpdateLobbyMaxPlayers(const FString& LobbyId, int32 NewMaxPlayers)
{
    // Note: This is a low-level example. In practice, you'd use EOS_Lobby_UpdateLobbyModification
    // and EOS_LobbyModification APIs to create a modification handle first
    
    // 1. Create lobby modification handle
    // 2. Set desired changes (max players, attributes, etc.)
    // 3. Apply changes with UEOSUpdateLobbyAsync
    
    // Example assumes you have a modification handle
    UEOSUpdateLobbyAsync* UpdateLobby = UEOSUpdateLobbyAsync::UpdateLobby(ModificationHandle);
    UpdateLobby->OnComplete.AddDynamic(this, &AGameMode::OnLobbyUpdated);
    UpdateLobby->Activate();
}

void AGameMode::OnLobbyUpdated(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Lobby updated successfully: %s"), *LobbyId);
    }
}
```

### Leaving a Lobby

Members can leave a lobby at any time.

**C++:**
```cpp
UEOSLeaveLobbyAsync* LeaveLobby = UEOSLeaveLobbyAsync::LeaveLobby(
    WorldContextObject,
    LobbyId
);

LeaveLobby->OnComplete.AddDynamic(this, &AGameMode::OnLobbyLeft);
LeaveLobby->Activate();

void AGameMode::OnLobbyLeft(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully left lobby: %s"), *LobbyId);
        // Clean up local lobby state
    }
}
```

### Destroying a Lobby

Only the lobby owner can destroy a lobby. This kicks all members and removes the lobby.

**C++:**
```cpp
UEOSDestroyLobbyAsync* DestroyLobby = UEOSDestroyLobbyAsync::DestroyLobby(
    WorldContextObject,
    LobbyId
);

DestroyLobby->OnComplete.AddDynamic(this, &AGameMode::OnLobbyDestroyed);
DestroyLobby->Activate();

void AGameMode::OnLobbyDestroyed(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Lobby destroyed: %s"), *LobbyId);
    }
}
```

---

## Lobby Search & Matchmaking

### Finding Public Lobbies

Search for available lobbies based on filters and criteria.

**C++:**
```cpp
// Simple search for all lobbies
UEOSFindLobbiesAsync* FindLobbies = UEOSFindLobbiesAsync::FindLobbies(WorldContextObject);
FindLobbies->OnSuccess.AddDynamic(this, &AGameMode::OnLobbiesFound);
FindLobbies->OnFailure.AddDynamic(this, &AGameMode::OnLobbySearchFailed);
FindLobbies->Activate();

void AGameMode::OnLobbiesFound(const TArray<FEOSLobbyInfo>& Lobbies)
{
    UE_LOG(LogTemp, Log, TEXT("Found %d lobbies"), Lobbies.Num());
    
    for (const FEOSLobbyInfo& Lobby : Lobbies)
    {
        UE_LOG(LogTemp, Log, TEXT("Lobby: %s - Players: %d/%d"), 
            *Lobby.LobbyId, 
            Lobby.CurrentPlayers, 
            Lobby.MaxPlayers);
    }
}
```

### Advanced Search with Filters

Use `FEOSKitSessionSearchSettings` for filtered searches:

**C++:**
```cpp
FEOSKitSessionSearchSettings SearchSettings;
SearchSettings.MatchType = EEOSKitMatchType::LobbySession;
SearchSettings.MaxResults = 25;
SearchSettings.RegionToSearch = EEOSKitRegion::NAEast;
SearchSettings.bLanSearch = false;

// Add custom attribute filters
FEOSKitAttribute GameModeFilter;
GameModeFilter.Key = TEXT("GameMode");
GameModeFilter.StringValue = TEXT("CaptureTheFlag");
SearchSettings.SearchFilters.Add(TEXT("GameMode"), GameModeFilter);

FEOSKitAttribute DifficultyFilter;
DifficultyFilter.Key = TEXT("Difficulty");
DifficultyFilter.IntValue = 3;
SearchSettings.SearchFilters.Add(TEXT("Difficulty"), DifficultyFilter);

// Use with advanced find session methods
// (Specific implementation depends on your search async node)
```

### Bucketed Matchmaking

Buckets allow you to group lobbies by type, mode, or skill level:

```cpp
// When creating lobby
Settings.BucketId = TEXT("Ranked_Gold");  // Skill-based bucket

// When searching
SearchSettings.SearchFilters.Add(TEXT("BucketId"), BucketAttribute);
```

Common bucket strategies:
- **Skill-based**: `Ranked_Bronze`, `Ranked_Silver`, `Ranked_Gold`
- **Mode-based**: `Casual`, `Competitive`, `Tournament`
- **Region-based**: `US_East`, `EU_West`, `Asia_Pacific`
- **Custom**: Any game-specific categorization

---

## Lobby Invitations

### Sending Invitations

**C++:**
```cpp
UEOSSendLobbyInviteAsync* SendInvite = UEOSSendLobbyInviteAsync::SendLobbyInvite(
    WorldContextObject,
    LobbyId,
    FriendProductUserId  // EOS Product User ID of friend
);

SendInvite->OnComplete.AddDynamic(this, &AGameMode::OnInviteSent);
SendInvite->Activate();

void AGameMode::OnInviteSent(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Invitation sent for lobby: %s"), *LobbyId);
    }
}
```

### Querying Pending Invitations

**C++:**
```cpp
UEOSQueryLobbyInvitesAsync* QueryInvites = UEOSQueryLobbyInvitesAsync::QueryLobbyInvites(
    WorldContextObject
);

QueryInvites->OnSuccess.AddDynamic(this, &AGameMode::OnInvitesQueried);
QueryInvites->Activate();

void AGameMode::OnInvitesQueried(const TArray<FEOSLobbyInvite>& Invites)
{
    UE_LOG(LogTemp, Log, TEXT("Pending invitations: %d"), Invites.Num());
    
    // Display invites in UI for player to accept/reject
    for (const FEOSLobbyInvite& Invite : Invites)
    {
        // Show invite details
    }
}
```

### Accepting Invitations

When a player accepts an invitation through the overlay or your UI:

**C++:**
```cpp
// Invitation is typically accepted via EOS overlay
// Your game receives notification through the LobbySubsystem
// Then you can join the lobby using the invitation details
```

### Rejecting Invitations

**C++:**
```cpp
UEOSRejectLobbyInviteAsync* RejectInvite = UEOSRejectLobbyInviteAsync::RejectLobbyInvite(
    WorldContextObject,
    InviteId
);

RejectInvite->OnComplete.AddDynamic(this, &AGameMode::OnInviteRejected);
RejectInvite->Activate();
```

---

## Lobby Notifications & Events

The `UEOSKitLobbySubsystem` provides real-time notifications for lobby events.

### Accessing the Lobby Subsystem

**C++:**
```cpp
UGameInstance* GameInstance = GetWorld()->GetGameInstance();
UEOSKitLobbySubsystem* LobbySubsystem = GameInstance->GetSubsystem<UEOSKitLobbySubsystem>();
```

### Join Lobby Accepted Notification

Triggered when a user joins a lobby via the overlay.

**C++:**
```cpp
void AGameMode::RegisterLobbyNotifications()
{
    UEOSKitLobbySubsystem* LobbySubsystem = GetLobbySubsystem();
    
    FEOSKitOnJoinLobbyAccepted JoinDelegate;
    JoinDelegate.BindDynamic(this, &AGameMode::OnJoinLobbyAccepted);
    LobbySubsystem->RegisterJoinLobbyAcceptedNotification(JoinDelegate);
}

void AGameMode::OnJoinLobbyAccepted(FString ProductUserId, FString UiEventId)
{
    UE_LOG(LogTemp, Log, TEXT("User accepted join lobby request"));
    // Handle the join lobby flow
}
```

### Leave Lobby Requested Notification

Triggered when a user requests to leave via overlay. **Important**: SDK does not automatically leave - your game must call `LeaveLobby`.

**C++:**
```cpp
FEOSKitOnLeaveLobbyRequested LeaveDelegate;
LeaveDelegate.BindDynamic(this, &AGameMode::OnLeaveLobbyRequested);
LobbySubsystem->RegisterLeaveLobbyRequestedNotification(LeaveDelegate);

void AGameMode::OnLeaveLobbyRequested(FString ProductUserId, FString LobbyId)
{
    UE_LOG(LogTemp, Log, TEXT("User requested to leave lobby: %s"), *LobbyId);
    
    // Call LeaveLobby to complete the action
    UEOSLeaveLobbyAsync* LeaveLobby = UEOSLeaveLobbyAsync::LeaveLobby(this, LobbyId);
    LeaveLobby->Activate();
}
```

### Lobby Invite Received Notification

**C++:**
```cpp
FEOSKitOnLobbyInviteReceived InviteDelegate;
InviteDelegate.BindDynamic(this, &AGameMode::OnLobbyInviteReceived);
LobbySubsystem->RegisterLobbyInviteReceivedNotification(InviteDelegate);

void AGameMode::OnLobbyInviteReceived(FString LocalUserId, FString TargetUserId, FString InviteId)
{
    UE_LOG(LogTemp, Log, TEXT("Received lobby invitation: %s"), *InviteId);
    // Show invitation notification in UI
}
```

### Lobby Update Notification

Triggered when lobby settings change.

**C++:**
```cpp
FEOSKitOnLobbyUpdateReceived UpdateDelegate;
UpdateDelegate.BindDynamic(this, &AGameMode::OnLobbyUpdateReceived);
LobbySubsystem->RegisterLobbyUpdateReceivedNotification(UpdateDelegate);

void AGameMode::OnLobbyUpdateReceived(FString LobbyId)
{
    UE_LOG(LogTemp, Log, TEXT("Lobby updated: %s"), *LobbyId);
    // Refresh lobby details in UI
}
```

### Lobby Member Update Notification

Triggered when a member updates their attributes.

**C++:**
```cpp
FEOSKitOnLobbyMemberUpdateReceived MemberUpdateDelegate;
MemberUpdateDelegate.BindDynamic(this, &AGameMode::OnLobbyMemberUpdateReceived);
LobbySubsystem->RegisterLobbyMemberUpdateReceivedNotification(MemberUpdateDelegate);

void AGameMode::OnLobbyMemberUpdateReceived(FString LobbyId, FString TargetUserId)
{
    UE_LOG(LogTemp, Log, TEXT("Member updated in lobby %s: %s"), *LobbyId, *TargetUserId);
    // Refresh member info in UI
}
```

### Lobby Member Status Notification

Tracks member join/leave/disconnect status.

**C++:**
```cpp
FEOSKitOnLobbyMemberStatusReceived StatusDelegate;
StatusDelegate.BindDynamic(this, &AGameMode::OnLobbyMemberStatusReceived);
LobbySubsystem->RegisterLobbyMemberStatusReceivedNotification(StatusDelegate);

void AGameMode::OnLobbyMemberStatusReceived(FString TargetUserId, FString LobbyId, int32 CurrentStatus)
{
    // Status codes from EOS_ELobbyMemberStatus
    // 0 = Joined, 1 = Left, 2 = Disconnected, 3 = Kicked, 4 = Promoted
    UE_LOG(LogTemp, Log, TEXT("Member status changed: User=%s Status=%d"), *TargetUserId, CurrentStatus);
}
```

### Unregistering Notifications

Always clean up notifications when no longer needed:

**C++:**
```cpp
void AGameMode::UnregisterAllLobbyNotifications()
{
    UEOSKitLobbySubsystem* LobbySubsystem = GetLobbySubsystem();
    LobbySubsystem->UnregisterAllNotifications();
}

// Or unregister specific notifications
LobbySubsystem->UnregisterJoinLobbyAcceptedNotification();
LobbySubsystem->UnregisterLobbyUpdateReceivedNotification();
// etc.
```

---

## Member Management

### Kicking Members

Only the lobby owner can kick members.

**C++:**
```cpp
UEOSKickMemberAsync* KickMember = UEOSKickMemberAsync::KickMember(
    WorldContextObject,
    LobbyId,
    LocalUserProductId,  // Must be owner
    TargetUserProductId   // User to kick
);

KickMember->OnComplete.AddDynamic(this, &AGameMode::OnMemberKicked);
KickMember->Activate();

void AGameMode::OnMemberKicked(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Member kicked from lobby: %s"), *LobbyId);
    }
}
```

### Promoting Members to Owner

Transfer lobby ownership to another member.

**C++:**
```cpp
UEOSPromoteMemberAsync* PromoteMember = UEOSPromoteMemberAsync::PromoteMember(
    WorldContextObject,
    LobbyId,
    LocalUserProductId,  // Current owner
    NewOwnerProductId    // New owner
);

PromoteMember->OnComplete.AddDynamic(this, &AGameMode::OnMemberPromoted);
PromoteMember->Activate();

void AGameMode::OnMemberPromoted(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Ownership transferred in lobby: %s"), *LobbyId);
    }
}
```

### Managing Member Attributes

Members can have custom attributes like ready status, character selection, etc.

**C++:**
```cpp
// When updating lobby or member settings
FEOSKitLobbyMemberSettings MemberSettings;
MemberSettings.bAllowInvites = true;

// Add custom member attributes
FEOSKitAttribute ReadyStatus;
ReadyStatus.Key = TEXT("IsReady");
ReadyStatus.BoolValue = true;
MemberSettings.MemberAttributes.Add(TEXT("IsReady"), ReadyStatus);

FEOSKitAttribute SelectedCharacter;
SelectedCharacter.Key = TEXT("Character");
SelectedCharacter.StringValue = TEXT("Warrior");
MemberSettings.MemberAttributes.Add(TEXT("Character"), SelectedCharacter);

// Apply member settings (implementation depends on your update method)
```

---

## Lobby Attributes & Customization

Lobby attributes allow you to store custom data for filtering and display.

### Supported Attribute Types

**C++:**
```cpp
// String attributes
FEOSKitAttribute StringAttr;
StringAttr.Key = TEXT("MapName");
StringAttr.StringValue = TEXT("Forest Arena");

// Integer attributes
FEOSKitAttribute IntAttr;
IntAttr.Key = TEXT("MinLevel");
IntAttr.IntValue = 10;

// Float attributes
FEOSKitAttribute FloatAttr;
FloatAttr.Key = TEXT("DifficultyMultiplier");
FloatAttr.FloatValue = 1.5f;

// Boolean attributes
FEOSKitAttribute BoolAttr;
BoolAttr.Key = TEXT("FriendlyFire");
BoolAttr.BoolValue = true;
```

### Common Lobby Attributes

**Game Mode Information:**
```cpp
Settings.ExtraSettings.Add(TEXT("GameMode"), GameModeAttr);
Settings.ExtraSettings.Add(TEXT("MapName"), MapAttr);
Settings.ExtraSettings.Add(TEXT("GameType"), GameTypeAttr);
```

**Gameplay Settings:**
```cpp
Settings.ExtraSettings.Add(TEXT("RoundTime"), RoundTimeAttr);
Settings.ExtraSettings.Add(TEXT("ScoreLimit"), ScoreLimitAttr);
Settings.ExtraSettings.Add(TEXT("FriendlyFire"), FriendlyFireAttr);
```

**Matchmaking Criteria:**
```cpp
Settings.ExtraSettings.Add(TEXT("SkillRating"), SkillAttr);
Settings.ExtraSettings.Add(TEXT("MinLevel"), MinLevelAttr);
Settings.ExtraSettings.Add(TEXT("MaxLevel"), MaxLevelAttr);
```

**Custom Modifiers:**
```cpp
Settings.ExtraSettings.Add(TEXT("CustomRule1"), Rule1Attr);
Settings.ExtraSettings.Add(TEXT("CustomRule2"), Rule2Attr);
```

### Attribute Best Practices

1. **Use Consistent Keys**: Define attribute keys as constants
2. **Document Types**: Clearly document which attributes use which types
3. **Validate Values**: Check attribute ranges before applying
4. **Search Optimization**: Index frequently searched attributes
5. **Limit Count**: Use only necessary attributes to reduce bandwidth

---

## Blueprint Examples

### Example 1: Simple Lobby Browser

1. **Create UI with Lobby List**
   - Add a List View widget
   - Create lobby entry widget with Join button

2. **Find Lobbies on UI Open**
   - Event Construct → Find Lobbies node
   - On Success → Populate List View

3. **Join Selected Lobby**
   - On Join Button Clicked → Join Lobby node
   - Pass selected lobby session result
   - On Success → Travel to session

### Example 2: Lobby Ready System

1. **Create Ready Check UI**
   - Add checkbox or button for "Ready"
   - Display ready status for all members

2. **Update Member Ready Status**
   - On Ready Button Clicked → Update Member Attributes
   - Set "IsReady" attribute to true
   - All members see update via notification

3. **Start Game When All Ready**
   - Register for Member Update notifications
   - Check if all members ready
   - Owner starts game when conditions met

### Example 3: Invitation Flow

1. **Send Invite from Friends List**
   - Display friends list in UI
   - Send Lobby Invite node when friend selected
   - Show confirmation message

2. **Receive and Display Invite**
   - Register Invite Received notification
   - Show popup with invite details
   - Provide Accept/Reject buttons

3. **Accept Invite**
   - On Accept → Join Lobby with invite ID
   - On Success → Enter lobby

---

## C++ Examples

### Complete Lobby Manager Class

```cpp
// LobbyManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EOSKitSessionStructs.h"
#include "LobbyManager.generated.h"

UCLASS()
class YOURGAME_API ALobbyManager : public AActor
{
    GENERATED_BODY()
    
public:
    ALobbyManager();
    
    // Create a new lobby
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void CreateGameLobby(int32 MaxPlayers, const FString& GameMode);
    
    // Find available lobbies
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void FindAvailableLobbies();
    
    // Join a specific lobby
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void JoinLobby(const FBlueprintSessionResult& SessionResult);
    
    // Leave current lobby
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void LeaveCurrentLobby();
    
    // Send invitation to friend
    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void InviteFriendToLobby(const FString& FriendId);
    
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    // Callbacks
    UFUNCTION()
    void OnLobbyCreated(const FString& LobbyId);
    
    UFUNCTION()
    void OnLobbyCreateFailed(const FString& Error);
    
    UFUNCTION()
    void OnLobbiesFound(const TArray<FEOSLobbyInfo>& Lobbies);
    
    UFUNCTION()
    void OnLobbySearchFailed(const TArray<FEOSLobbyInfo>& Empty);
    
    UFUNCTION()
    void OnJoinSuccess(EEOSKitJoinResult Result, FString SessionJoinAddress);
    
    UFUNCTION()
    void OnJoinFailure(EEOSKitJoinResult Result, FString Error);
    
    UFUNCTION()
    void OnLobbyLeft(bool bSuccess, FString LobbyId);
    
    UFUNCTION()
    void OnInviteSent(bool bSuccess, FString LobbyId);
    
    // Notification handlers
    UFUNCTION()
    void OnLobbyUpdateReceived(FString LobbyId);
    
    UFUNCTION()
    void OnMemberStatusReceived(FString TargetUserId, FString LobbyId, int32 Status);
    
    // Setup notifications
    void RegisterLobbyNotifications();
    void UnregisterLobbyNotifications();
    
    // Current lobby state
    FString CurrentLobbyId;
    TArray<FEOSLobbyInfo> AvailableLobbies;
    
    // Reference to lobby subsystem
    UPROPERTY()
    class UEOSKitLobbySubsystem* LobbySubsystem;
};
```

```cpp
// LobbyManager.cpp
#include "LobbyManager.h"
#include "EOSKitLobbySubsystem.h"
#include "EOSCreateLobbyAsync.h"
#include "EOSFindLobbiesAsync.h"
#include "EOSJoinLobbyAsync.h"
#include "EOSLeaveLobbyAsync.h"
#include "EOSSendLobbyInviteAsync.h"

ALobbyManager::ALobbyManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALobbyManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get lobby subsystem
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    LobbySubsystem = GameInstance->GetSubsystem<UEOSKitLobbySubsystem>();
    
    RegisterLobbyNotifications();
}

void ALobbyManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterLobbyNotifications();
    Super::EndPlay(EndPlayReason);
}

void ALobbyManager::CreateGameLobby(int32 MaxPlayers, const FString& GameMode)
{
    UEOSCreateLobbyAsync* CreateLobby = UEOSCreateLobbyAsync::CreateLobby(
        this,
        MaxPlayers,
        TEXT("DefaultBucket"),
        true
    );
    
    CreateLobby->OnSuccess.AddDynamic(this, &ALobbyManager::OnLobbyCreated);
    CreateLobby->OnFailure.AddDynamic(this, &ALobbyManager::OnLobbyCreateFailed);
    CreateLobby->Activate();
}

void ALobbyManager::OnLobbyCreated(const FString& LobbyId)
{
    CurrentLobbyId = LobbyId;
    UE_LOG(LogTemp, Log, TEXT("Successfully created lobby: %s"), *LobbyId);
    
    // Notify UI or other systems
}

void ALobbyManager::OnLobbyCreateFailed(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to create lobby: %s"), *Error);
}

void ALobbyManager::FindAvailableLobbies()
{
    UEOSFindLobbiesAsync* FindLobbies = UEOSFindLobbiesAsync::FindLobbies(this);
    FindLobbies->OnSuccess.AddDynamic(this, &ALobbyManager::OnLobbiesFound);
    FindLobbies->OnFailure.AddDynamic(this, &ALobbyManager::OnLobbySearchFailed);
    FindLobbies->Activate();
}

void ALobbyManager::OnLobbiesFound(const TArray<FEOSLobbyInfo>& Lobbies)
{
    AvailableLobbies = Lobbies;
    UE_LOG(LogTemp, Log, TEXT("Found %d lobbies"), Lobbies.Num());
    
    // Update UI with lobby list
}

void ALobbyManager::OnLobbySearchFailed(const TArray<FEOSLobbyInfo>& Empty)
{
    UE_LOG(LogTemp, Warning, TEXT("Lobby search failed"));
}

void ALobbyManager::JoinLobby(const FBlueprintSessionResult& SessionResult)
{
    UEOSJoinLobbyAsync* JoinLobby = UEOSJoinLobbyAsync::JoinEOSKitSession(
        this,
        TEXT("GameSession"),
        SessionResult,
        false
    );
    
    JoinLobby->Success.AddDynamic(this, &ALobbyManager::OnJoinSuccess);
    JoinLobby->Failure.AddDynamic(this, &ALobbyManager::OnJoinFailure);
    JoinLobby->Activate();
}

void ALobbyManager::OnJoinSuccess(EEOSKitJoinResult Result, FString SessionJoinAddress)
{
    if (Result == EEOSKitJoinResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully joined lobby"));
        // Optionally travel to session
        // GetWorld()->GetFirstPlayerController()->ClientTravel(SessionJoinAddress, TRAVEL_Absolute);
    }
}

void ALobbyManager::OnJoinFailure(EEOSKitJoinResult Result, FString Error)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to join lobby: %d"), (int32)Result);
}

void ALobbyManager::LeaveCurrentLobby()
{
    if (CurrentLobbyId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("No lobby to leave"));
        return;
    }
    
    UEOSLeaveLobbyAsync* LeaveLobby = UEOSLeaveLobbyAsync::LeaveLobby(this, CurrentLobbyId);
    LeaveLobby->OnComplete.AddDynamic(this, &ALobbyManager::OnLobbyLeft);
    LeaveLobby->Activate();
}

void ALobbyManager::OnLobbyLeft(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        CurrentLobbyId.Empty();
        UE_LOG(LogTemp, Log, TEXT("Successfully left lobby"));
    }
}

void ALobbyManager::InviteFriendToLobby(const FString& FriendId)
{
    if (CurrentLobbyId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Not in a lobby"));
        return;
    }
    
    UEOSSendLobbyInviteAsync* SendInvite = UEOSSendLobbyInviteAsync::SendLobbyInvite(
        this,
        CurrentLobbyId,
        FriendId
    );
    
    SendInvite->OnComplete.AddDynamic(this, &ALobbyManager::OnInviteSent);
    SendInvite->Activate();
}

void ALobbyManager::OnInviteSent(bool bSuccess, FString LobbyId)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Invitation sent"));
    }
}

void ALobbyManager::RegisterLobbyNotifications()
{
    if (!LobbySubsystem) return;
    
    FEOSKitOnLobbyUpdateReceived UpdateDelegate;
    UpdateDelegate.BindDynamic(this, &ALobbyManager::OnLobbyUpdateReceived);
    LobbySubsystem->RegisterLobbyUpdateReceivedNotification(UpdateDelegate);
    
    FEOSKitOnLobbyMemberStatusReceived StatusDelegate;
    StatusDelegate.BindDynamic(this, &ALobbyManager::OnMemberStatusReceived);
    LobbySubsystem->RegisterLobbyMemberStatusReceivedNotification(StatusDelegate);
}

void ALobbyManager::UnregisterLobbyNotifications()
{
    if (LobbySubsystem)
    {
        LobbySubsystem->UnregisterAllNotifications();
    }
}

void ALobbyManager::OnLobbyUpdateReceived(FString LobbyId)
{
    UE_LOG(LogTemp, Log, TEXT("Lobby updated: %s"), *LobbyId);
    // Refresh lobby data
}

void ALobbyManager::OnMemberStatusReceived(FString TargetUserId, FString LobbyId, int32 Status)
{
    UE_LOG(LogTemp, Log, TEXT("Member status change: User=%s Status=%d"), *TargetUserId, Status);
    // Update member list UI
}
```

---

## Best Practices

### 1. Lobby Lifecycle Management

✅ **Always unregister notifications** when leaving or destroying lobbies
```cpp
LobbySubsystem->UnregisterAllNotifications();
```

✅ **Clean up lobby state** on leave/destroy
```cpp
CurrentLobbyId.Empty();
LobbyMembers.Empty();
```

✅ **Handle host migration** gracefully
```cpp
Settings.bSupportHostMigration = true;
```

### 2. Search & Performance

✅ **Limit search results** to reduce bandwidth
```cpp
SearchSettings.MaxResults = 25;  // Not 1000!
```

✅ **Use bucket IDs** for efficient matchmaking
```cpp
Settings.BucketId = TEXT("RankedGold");
```

✅ **Cache search results** to avoid repeated searches
```cpp
TArray<FEOSLobbyInfo> CachedLobbies;
```

### 3. Attributes & Data

✅ **Use appropriate attribute types**
- String for text: game mode, map name
- Int for counts: level, score
- Float for ratios: difficulty multiplier
- Bool for flags: friendly fire, voice chat

✅ **Keep attributes minimal** - only what you need to search/display

✅ **Validate attribute values** before applying
```cpp
if (MaxPlayers >= 2 && MaxPlayers <= 100)
{
    // Apply setting
}
```

### 4. Error Handling

✅ **Always handle failure cases**
```cpp
AsyncNode->OnSuccess.AddDynamic(this, &AClass::OnSuccess);
AsyncNode->OnFailure.AddDynamic(this, &AClass::OnFailure);  // Don't forget!
```

✅ **Log detailed error information**
```cpp
UE_LOG(LogTemp, Error, TEXT("Join failed: Result=%d, Lobby=%s"), (int32)Result, *LobbyId);
```

✅ **Provide user feedback** for all operations

### 5. Networking & Latency

✅ **Use regional buckets** to reduce ping
```cpp
Settings.Region = EEOSKitRegion::NAEast;
```

✅ **Show lobby ping** in search results

✅ **Implement timeout handling** for async operations

### 6. Security & Validation

✅ **Validate lobby owner** before admin operations
```cpp
if (LocalUserId == LobbyOwnerId)
{
    // Allow kick/promote/update
}
```

✅ **Sanitize user input** for lobby names and attributes

✅ **Use sanctions** if available
```cpp
Settings.bEnforceSanctions = true;
```

### 7. Voice Chat Integration

✅ **Enable voice only when needed**
```cpp
Settings.bUseVoiceChat = true;  // Only for lobbies that need it
```

✅ **Provide mute controls** for players

✅ **Test voice quality** across platforms

### 8. UI/UX Best Practices

✅ **Show lobby status** clearly: Open, Full, In Progress

✅ **Display member count**: "3/4 players"

✅ **Indicate lobby owner** with icon/badge

✅ **Show ready status** for all members

✅ **Provide clear feedback** for all actions

---

## Troubleshooting

### Common Issues

#### "Lobby creation fails silently"

**Cause**: EOS not initialized or user not logged in

**Solution**:
```cpp
// Ensure user is logged in before creating lobby
if (UEOSKitSubsystem* EOSSubsystem = GetEOSKitSubsystem())
{
    if (EOSSubsystem->IsLoggedIn())
    {
        // Create lobby
    }
}
```

#### "Can't find any lobbies"

**Causes**:
1. Wrong bucket ID
2. Region mismatch
3. No public lobbies exist
4. Attribute filters too strict

**Solution**:
```cpp
// Try broader search first
SearchSettings.BucketId.Empty();  // Search all buckets
SearchSettings.RegionToSearch = EEOSKitRegion::NoSelection;  // All regions
SearchSettings.SearchFilters.Empty();  // No filters
```

#### "Join fails with 'Session Does Not Exist'"

**Causes**:
1. Lobby was destroyed
2. Search results are stale
3. Lobby ID is invalid

**Solution**:
```cpp
// Refresh search before joining
// Add timestamp to lobby search results
// Verify lobby still exists before join
```

#### "Voice chat not working"

**Checklist**:
- [ ] `bUseVoiceChat = true` in lobby settings
- [ ] EOSKitRTC module enabled
- [ ] User has microphone permissions
- [ ] RTC subsystem initialized
- [ ] Voice participants added to room

#### "Notifications not firing"

**Causes**:
1. Not registered before lobby created
2. Subsystem not initialized
3. Delegates not bound correctly

**Solution**:
```cpp
// Register notifications early
void AGameMode::BeginPlay()
{
    Super::BeginPlay();
    RegisterLobbyNotifications();  // Do this first
}

// Verify subsystem exists
if (LobbySubsystem)
{
    // Register notifications
}
```

#### "Lobby updates not syncing"

**Solution**:
```cpp
// Ensure all members are registered for updates
FEOSKitOnLobbyUpdateReceived UpdateDelegate;
UpdateDelegate.BindDynamic(this, &AClass::OnLobbyUpdate);
LobbySubsystem->RegisterLobbyUpdateReceivedNotification(UpdateDelegate);

// Manually refresh after update
void OnLobbyUpdate(FString LobbyId)
{
    // Query lobby details again
    RefreshLobbyData(LobbyId);
}
```

#### "Can't kick members"

**Cause**: Not lobby owner

**Solution**:
```cpp
// Verify ownership before kick
if (IsLobbyOwner())
{
    KickMember(TargetUserId);
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Only lobby owner can kick members"));
}
```

### Debug Tips

**Enable verbose logging:**
```cpp
// In DefaultEngine.ini
[Core.Log]
LogEOSKit=Verbose
LogOnline=Verbose
LogOnlineSession=Verbose
```

**Test with EOS Developer Tool** for real-time lobby monitoring

**Use EOS Sandbox** for testing before production

**Monitor network conditions** - high latency affects lobby operations

---

## Additional Resources

### Related Documentation

- **Main README**: `/README.md` - Plugin overview and installation
- **Quick Start Guide**: `Documentation/QuickStart.md` - Getting started
- **Voice Module**: `Source/EOSKitRTC/README_VOICE_MODULE.md` - Voice chat integration
- **API Reference**: `Documentation/API_Reference.md` - Complete API docs

### EOS Documentation

- [Epic Online Services Documentation](https://dev.epicgames.com/docs/services)
- [Lobbies Interface](https://dev.epicgames.com/docs/game-services/lobbies)
- [Sessions Interface](https://dev.epicgames.com/docs/game-services/sessions)

### Support

- **Email**: support@eoskit.dev
- **Issues**: [GitHub Issues](https://github.com/parthkalla/EpicOnlineServices-Kit/issues)
- **Community**: Discord (link in README)

---

## Conclusion

The Extreme Multiplayer Lobby System provides everything you need to build production-ready multiplayer lobbies with EOS. From simple matchmaking to complex tournament systems, the flexible architecture supports any use case.

**Key Takeaways:**
- Use the lobby subsystem for real-time notifications
- Implement proper error handling for all operations
- Leverage attributes for flexible matchmaking
- Follow best practices for performance and security
- Test thoroughly across platforms

Happy coding! 🎮

---

*Last Updated: December 2024*
*Plugin Version: 1.0*
*EOS SDK Version: 1.16+*
