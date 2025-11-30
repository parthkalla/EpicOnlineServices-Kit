# How to Create a Session After Login

This guide shows you how to create an EOS session after successfully logging in. There are two approaches:

## Prerequisites

1. **User must be logged in** - You need a valid `ProductUserId` before creating a session
2. **EOSKit must be initialized** - The EOSKit subsystem should be ready

---

## Method 1: High-Level Approach (Recommended for Beginners)

Use the `Create EOK Session` async node. This is the simplest method.

### Blueprint Example

1. **After Login Success:**
   - Connect to `On Success` output of your login node
   
2. **Create Session Node:**
   - Add `Create EOK Session` node
   - **Inputs:**
     - `World Context Object`: Self (or your world context)
     - `Session Name`: e.g., "MainSession"
     - `Number Of Public Connections`: e.g., 4 (max players)
     - `Session Settings`: (Optional) Map of custom attributes
     - `Dedicated Server Settings`: (Optional) Leave default if not using dedicated server
     - `Extra Settings`: (Optional) Additional session configuration
   
3. **Handle Results:**
   - Connect `On Success` → Print Session ID
   - Connect `On Fail` → Show error message

### C++ Example

```cpp
#include "EOSCreateEOKSessionAsync.h"
#include "EOSKitSessionStructs.h"

void AMyGameMode::CreateSessionAfterLogin()
{
    // Get world context
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Prepare session settings (optional)
    TMap<FString, FEOSKitAttribute> SessionSettings;
    
    // Add custom attributes if needed
    FEOSKitAttribute GameModeAttr;
    GameModeAttr.AttributeType = EEOSKitAttributeType::String;
    GameModeAttr.StringValue = TEXT("Deathmatch");
    SessionSettings.Add(TEXT("GAMEMODE"), GameModeAttr);
    
    // Create session
    UEOSCreateEOKSessionAsync* CreateSessionNode = UEOSCreateEOKSessionAsync::CreateEOKSession(
        World,
        SessionSettings,
        FName(TEXT("MainSession")),
        4, // Max 4 players
        FEOSKitDedicatedServerSettings(), // Default (not dedicated server)
        FEOSKitCreateSessionSettings() // Default extra settings
    );
    
    // Bind callbacks
    CreateSessionNode->OnSuccess.AddDynamic(this, &AMyGameMode::OnSessionCreated);
    CreateSessionNode->OnFail.AddDynamic(this, &AMyGameMode::OnSessionCreateFailed);
    
    // Activate the node
    CreateSessionNode->Activate();
}

void AMyGameMode::OnSessionCreated(const FString& SessionID)
{
    UE_LOG(LogTemp, Log, TEXT("Session created successfully! Session ID: %s"), *SessionID);
    // Your session is now created and ready
}

void AMyGameMode::OnSessionCreateFailed(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to create session: %s"), *ErrorMessage);
}
```

---

## Method 2: SDK-Level Approach (More Control)

Use the SDK functions directly for more control over session creation.

### Step-by-Step Process

1. **Create Session Modification Handle**
2. **Configure Session Properties** (max players, bucket ID, etc.)
3. **Add Attributes** (optional)
4. **Update Session** (this creates the session)
5. **Start Session** (optional - makes session active for matchmaking)

### Blueprint Example

1. **Create Session Modification:**
   - Use `EOS_Sessions_CreateSessionModification` (from `EOS_SessionsSubsystem`)
   - **Inputs:**
     - `Session Name`: e.g., "MainSession"
     - `Bucket Id`: e.g., "GameBucket" (used for matchmaking)
     - `Max Players`: e.g., 4
     - `Local User Id`: Get from `EOSKitSubsystem` → `Get Product User ID`
     - `Presence Enabled`: true/false
     - `Session Id`: Leave empty for new session
   
2. **Configure Session Modification:**
   - Use `EOS_SessionModification_SetMaxPlayers` (if needed)
   - Use `EOS_SessionModification_SetPermissionLevel` (PublicAdvertised, JoinViaPresence, or InviteOnly)
   - Use `EOS_SessionModification_AddAttribute` to add custom attributes
   
3. **Update Session:**
   - Use `EOS_Sessions_UpdateSession` async node
   - Pass the `Session Modification Handle` from step 1
   - **On Success:** You'll get `Session Name` and `Session Id`
   
4. **Start Session (Optional):**
   - Use `EOS_Sessions_StartSession` async node
   - Pass the `Session Name` from step 3
   - This makes the session active for matchmaking

### C++ Example

```cpp
#include "EOS_SessionsSubsystem.h"
#include "EOS_Sessions_UpdateSession.h"
#include "EOS_Sessions_StartSession.h"
#include "EOSKitSubsystem.h"
#include "Kismet/GameplayStatics.h"

void AMyGameMode::CreateSessionSDKLevel()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get subsystems
    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(World);
    UEOSKitSubsystem* EOSKitSubsystem = GameInstance->GetSubsystem<UEOSKitSubsystem>();
    
    if (!EOSKitSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("EOSKit Subsystem not found!"));
        return;
    }
    
    // Get Product User ID (must be logged in)
    FEOSKitProductUserId LocalUserId;
    LocalUserId.ProductUserIdValue = EOSKitSubsystem->GetProductUserId();
    
    if (!LocalUserId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("User not logged in!"));
        return;
    }
    
    // Step 1: Create Session Modification Options
    FEOSKit_Sessions_CreateSessionModificationOptions CreateOptions;
    CreateOptions.SessionName = TEXT("MainSession");
    CreateOptions.BucketId = TEXT("GameBucket");
    CreateOptions.MaxPlayers = 4;
    CreateOptions.LocalUserId = LocalUserId;
    CreateOptions.bPresenceEnabled = true;
    CreateOptions.SessionId = TEXT(""); // Empty for new session
    CreateOptions.bSanctionsEnabled = false;
    
    // Step 2: Create Session Modification Handle
    FEOSKitHSessionModification SessionModHandle;
    TEnumAsByte<EEOSKitResult> CreateModResult = UEOS_SessionsSubsystem::EOS_Sessions_CreateSessionModification(
        CreateOptions,
        SessionModHandle
    );
    
    if (CreateModResult != EEOSKitResult::Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session modification!"));
        return;
    }
    
    // Step 3: Configure session properties (optional)
    // You can use EOS_SessionModification_* functions here
    // For example:
    // UEOS_SessionsSubsystem::EOS_SessionModification_SetPermissionLevel(
    //     SessionModHandle,
    //     EEOSKit_EOnlineSessionPermissionLevel::PublicAdvertised
    // );
    
    // Step 4: Update Session (this creates the session)
    UEOS_Sessions_UpdateSession* UpdateSessionNode = UEOS_Sessions_UpdateSession::EOS_Sessions_UpdateSession(
        SessionModHandle
    );
    
    UpdateSessionNode->OnCallback.AddDynamic(this, &AMyGameMode::OnSessionUpdated);
    UpdateSessionNode->Activate();
}

void AMyGameMode::OnSessionUpdated(const TEnumAsByte<EEOSKitResult>& ResultCode, const FString& SessionName, const FString& SessionId)
{
    if (ResultCode == EEOSKitResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Session created! Name: %s, ID: %s"), *SessionName, *SessionId);
        
        // Step 5: Start Session (optional - makes it active for matchmaking)
        UEOS_Sessions_StartSession* StartSessionNode = UEOS_Sessions_StartSession::EOS_Sessions_StartSession(
            SessionName
        );
        
        StartSessionNode->OnCallback.AddDynamic(this, &AMyGameMode::OnSessionStarted);
        StartSessionNode->Activate();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session! Result: %d"), (int32)ResultCode);
    }
}

void AMyGameMode::OnSessionStarted(const TEnumAsByte<EEOSKitResult>& ResultCode, const FString& SessionName)
{
    if (ResultCode == EEOSKitResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Session started successfully! %s"), *SessionName);
        // Session is now active and ready for matchmaking
    }
}
```

---

## Complete Flow Example

Here's a complete example showing login → session creation:

```cpp
#include "EOSLoginAsync.h"
#include "EOSCreateEOKSessionAsync.h"
#include "EOSKitSessionStructs.h"

void AMyGameMode::LoginAndCreateSession()
{
    // Step 1: Login
    UEOSLoginAsync* LoginNode = UEOSLoginAsync::EOSLogin(
        GetWorld(),
        EEOSLoginType::AccountPortal,
        FEOSLoginCredentials()
    );
    
    LoginNode->OnSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
    LoginNode->OnFail.AddDynamic(this, &AMyGameMode::OnLoginFailed);
    LoginNode->Activate();
}

void AMyGameMode::OnLoginSuccess(const FString& EpicAccountId, const FString& ProductUserId)
{
    UE_LOG(LogTemp, Log, TEXT("Login successful! Product User ID: %s"), *ProductUserId);
    
    // Step 2: Create Session after successful login
    CreateSessionAfterLogin();
}

void AMyGameMode::OnLoginFailed(const FString& ErrorCode, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Login failed: %s - %s"), *ErrorCode, *ErrorMessage);
}

void AMyGameMode::CreateSessionAfterLogin()
{
    // Prepare session settings
    TMap<FString, FEOSKitAttribute> SessionSettings;
    
    // Add game mode attribute
    FEOSKitAttribute GameModeAttr;
    GameModeAttr.AttributeType = EEOSKitAttributeType::String;
    GameModeAttr.StringValue = TEXT("Deathmatch");
    SessionSettings.Add(TEXT("GAMEMODE"), GameModeAttr);
    
    // Add map name attribute
    FEOSKitAttribute MapAttr;
    MapAttr.AttributeType = EEOSKitAttributeType::String;
    MapAttr.StringValue = TEXT("Map01");
    SessionSettings.Add(TEXT("MAP"), MapAttr);
    
    // Create session
    UEOSCreateEOKSessionAsync* CreateSessionNode = UEOSCreateEOKSessionAsync::CreateEOKSession(
        GetWorld(),
        SessionSettings,
        FName(TEXT("MainSession")),
        4, // Max 4 players
        FEOSKitDedicatedServerSettings(),
        FEOSKitCreateSessionSettings()
    );
    
    CreateSessionNode->OnSuccess.AddDynamic(this, &AMyGameMode::OnSessionCreated);
    CreateSessionNode->OnFail.AddDynamic(this, &AMyGameMode::OnSessionCreateFailed);
    CreateSessionNode->Activate();
}

void AMyGameMode::OnSessionCreated(const FString& SessionID)
{
    UE_LOG(LogTemp, Log, TEXT("✅ Session created successfully!"));
    UE_LOG(LogTemp, Log, TEXT("Session ID: %s"), *SessionID);
    
    // Your session is now created and ready!
    // Players can now find and join this session
}

void AMyGameMode::OnSessionCreateFailed(const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("❌ Failed to create session: %s"), *ErrorMessage);
}
```

---

## Important Notes

1. **User must be logged in** - Always check that `GetProductUserId()` returns a valid ID before creating a session

2. **Session Name** - Must be unique per user. Use different names for different session types

3. **Bucket ID** - Used for matchmaking. Sessions with the same bucket ID can find each other

4. **Max Players** - Set this based on your game's needs

5. **Session Attributes** - Use these to store custom game data (game mode, map, etc.)

6. **Start Session** - Only needed if you want the session to be discoverable via matchmaking

7. **Error Handling** - Always handle both success and failure cases

---

## Common Issues

### "Product User ID is null - user must be logged in"
- **Solution:** Make sure login completed successfully before creating session

### "Session already exists"
- **Solution:** Destroy existing session first, or use a different session name

### "Invalid Parameters"
- **Solution:** Check that all required fields are filled (Session Name, Max Players, Local User ID)

---

## Next Steps

After creating a session, you can:
- **Find Sessions** - Use `EOSFindEOKSessionsAsync` to search for other sessions
- **Join Session** - Use `EOSJoinEOKSessionAsync` to join a session
- **Register Players** - Use `EOS_Sessions_RegisterPlayers` to register players in the session
- **Send Invites** - Use `EOS_Sessions_SendInvite` to invite friends

