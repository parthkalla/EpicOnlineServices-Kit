# EOSKit Feature Matrix

## Features Overview

? = Fully Implemented | ?? = Partially Implemented | ? = Not Implemented | ?? = Planned

---

## Authentication & Identity

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Epic Account Login | ? | ? | ? | Multiple login types supported |
| Auto Login | ? | ? | ? | Config-based automatic login |
| Connect Login (External Auth) | ? | ? | ? | Steam, Xbox, PSN, etc. |
| Device ID Creation | ? | ? | ? | Guest accounts |
| Account Linking | ? | ? | ? | Link multiple auth methods |
| Account Unlinking | ? | ? | ? | Remove linked accounts |
| Device Transfer | ? | ? | ? | Migrate device accounts |
| ID Token Verification | ? | ? | ? | Secure token validation |
| External Account Mapping | ? | ? | ? | Cross-platform IDs |

---

## Sessions & Matchmaking

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Session Creation | ? | ? | ? | Full customization |
| Session Search | ? | ? | ? | Attribute filtering |
| Session Join | ? | ? | ? | Auto-connection |
| Session Update | ? | ? | ? | Modify settings |
| Session Start | ? | ? | ? | Mark in-progress |
| Session End | ? | ? | ? | Mark completed |
| Session Destroy | ? | ? | ? | Delete session |
| Find by ID | ? | ? | ? | Direct session lookup |
| Player Registration | ? | ? | ? | Add/remove players |
| Session Invites | ? | ? | ? | Send/accept invites |
| Query Invites | ? | ? | ? | List pending invites |
| Accept/Reject Invites | ? | ? | ? | Invite management |
| Custom Attributes | ? | ? | ? | Key-value pairs |
| Presence Integration | ? | ? | ? | Online status |
| Join-in-Progress | ? | ? | ? | Late joins |

---

## Lobbies

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Lobby Creation | ? | ? | ? | Full control |
| Lobby Search | ? | ? | ? | Attribute filters |
| Lobby Join | ? | ? | ? | Multiple join methods |
| Lobby Update | ? | ? | ? | Settings modification |
| Lobby Leave | ? | ? | ? | Graceful exit |
| Lobby Destroy | ? | ? | ? | Owner only |
| Kick Member | ? | ? | ? | Remove players |
| Promote Member | ? | ? | ? | Transfer ownership |
| Lobby Invites | ? | ? | ? | Invite system |
| Query Lobby Invites | ? | ? | ? | Pending invites |
| Lobby Attributes | ? | ? | ? | Custom metadata |
| Permission Levels | ? | ? | ? | Public/Private/Friends |
| Bucket System | ? | ? | ? | Matchmaking pools |
| Real-time Events | ? | ? | ? | Member join/leave |
| Voice Integration | ? | ? | ? | Auto voice room |

---

## Voice Chat (RTC)

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Voice Room Join | ? | ? | ? | RTC integration |
| Voice Room Leave | ? | ? | ? | Clean disconnect |
| Microphone Mute | ? | ? | ? | Local control |
| Speaker Mute | ? | ? | ? | Per-player |
| Input Device Selection | ? | ? | ? | Multi-device |
| Output Device Selection | ? | ? | ? | Multi-device |
| Device Enumeration | ? | ? | ? | List devices |
| Volume Control (Input) | ? | ? | ? | Adjustable |
| Volume Control (Output) | ? | ? | ? | Adjustable |
| Per-Player Volume | ? | ? | ? | Individual control |
| Audio Synthesis | ? | ? | ? | AudioMixer based |
| Talking Indicator | ? | ? | ? | Real-time events |
| Voice Events | ? | ? | ? | User joined/left/talking |
| IVoiceChat Interface | ? | ? | ? | Standard UE interface |
| 3D Positional Audio | ?? | ? | ? | Basic support |
| Echo Cancellation | ? | ? | ? | EOS SDK feature |
| Noise Suppression | ? | ? | ? | EOS SDK feature |

---

## Achievements

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Query Definitions | ? | ? | ? | All achievements |
| Query Player Progress | ? | ? | ? | Individual progress |
| Unlock Achievement | ? | ? | ? | Single unlock |
| Batch Unlock | ? | ? | ? | Not implemented |
| Achievement Events | ? | ? | ? | Planned |
| Progress Tracking | ?? | ?? | ?? | Basic only |
| Hidden Achievements | ? | ? | ? | Via EOS config |
| Achievement Icons | ? | ? | ? | URL-based |

---

## Statistics & Leaderboards

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Write Player Stats | ? | ? | ? | Batch updates |
| Query Leaderboards | ? | ? | ? | Ranked lists |
| Leaderboard Pagination | ? | ? | ? | Start rank + count |
| Multiple Leaderboards | ? | ? | ? | Named boards |
| Stat Aggregation | ? | ? | ? | EOS SDK handles |
| Leaderboard Filters | ?? | ?? | ?? | Basic support |
| Time-based Leaderboards | ? | ? | ? | Via EOS config |
| Regional Leaderboards | ? | ? | ? | Via EOS config |

---

## Title Storage

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Query File List | ? | ? | ? | List all files |
| Read/Download File | ? | ? | ? | Binary data |
| File Metadata | ? | ? | ? | Size, hash |
| Large File Support | ? | ? | ? | Chunked transfer |
| File Upload | ? | ? | ? | Dev Portal only |
| File Caching | ?? | ?? | ?? | Basic |
| Versioning | ? | ? | ? | Via filenames |

---

## Networking & Sockets

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| EOS Socket Subsystem | ? | ? | ? | Low-level |
| EOS NetDriver | ? | ? | ? | Replication |
| P2P Connections | ? | ? | ? | NAT traversal |
| Socket Send/Receive | ? | ? | ? | Binary data |
| Connection Management | ? | ? | ? | Auto-connect |
| Address Resolution | ? | ? | ? | PUID-based |
| NetDriver Configuration | ? | ? | ? | INI-based |

---

## Web Services

| Feature | Status | Blueprint | C++ | Notes |
|---------|--------|-----------|-----|-------|
| Anti-Cheat Session | ? | ? | ? | Server-side |
| Player Reports | ? | ? | ? | Report violations |
| Query Sanctions | ? | ? | ? | Check bans |
| Ecommerce Queries | ? | ? | ? | Catalog items |
| Voice Room Tokens | ? | ? | ? | Auth tokens |

---

## Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Windows | ? | Full support |
| Mac | ? | Full support |
| Linux | ? | Full support |
| iOS | ? | Mobile support |
| Android | ? | Mobile support |
| Xbox | ?? | Via Connect |
| PlayStation | ?? | Via Connect |
| Nintendo Switch | ?? | Via Connect |
| Steam Deck | ? | Linux-based |

---

## Unreal Engine Features

| Feature | Status | Notes |
|---------|--------|-------|
| Blueprint Support | ? | Full async nodes |
| C++ API | ? | Complete interface |
| Online Subsystem | ? | IOnlineSubsystem integration |
| Replication | ? | Via NetDriver |
| Dedicated Server | ? | Full support |
| Listen Server | ? | Full support |
| PIE (Play in Editor) | ? | Multi-process support |
| Packaged Builds | ? | All configurations |
| Editor Tools | ?? | Basic settings panel |

---

## Subsystems

| Subsystem | Type | Status | Purpose |
|-----------|------|--------|---------|
| `UEOSKitSubsystem` | GameInstance | ? | Main platform interface |
| `UEOSKitEngineSubsystem` | Engine | ? | PIE management |
| `UEOSKitVoiceSubsystem` | Engine | ? | Voice chat |
| `UEOSKitSessionsSubsystem` | GameInstance | ? | Session management |
| `UEOSKitLobbySubsystem` | GameInstance | ? | Lobby management |
| `UEOSKitConnectSubsystem` | GameInstance | ? | Connect interface |
| `UEOSKitGameInstanceSubsystem` | GameInstance | ? | Additional features |

---

## Configuration Options

| Setting | Location | Status | Purpose |
|---------|----------|--------|---------|
| Auto Login | DefaultEngine.ini | ? | Automatic auth |
| Product ID | DefaultEngine.ini | ? | EOS project |
| Sandbox ID | DefaultEngine.ini | ? | EOS environment |
| Client Credentials | DefaultEngine.ini | ? | OAuth |
| NetDriver | DefaultEngine.ini | ? | Networking |
| Voice Settings | DefaultEngine.ini | ? | RTC config |
| Log Levels | DefaultEngine.ini | ? | Debugging |

---

## Async Operations

All async operations support:
- ? Blueprint delegates (OnSuccess, OnFailure)
- ? C++ delegates (FDelegateName)
- ? Automatic activation
- ? Error handling
- ? Thread safety
- ? Garbage collection

---

## Events & Delegates

### Voice Chat Events
- ? OnVoiceConnectionComplete
- ? OnVoiceUserJoined
- ? OnVoiceUserLeft
- ? OnVoiceUserTalking

### Lobby Events
- ? OnLobbyMemberJoined
- ? OnLobbyMemberLeft
- ? OnLobbyUpdated
- ? OnLobbyOwnerChanged

### Session Events (via Online Subsystem)
- ? OnCreateSessionComplete
- ? OnFindSessionsComplete
- ? OnJoinSessionComplete
- ? OnDestroySessionComplete

### Login Events
- ? OnLoginComplete
- ? OnLogoutComplete
- ? OnLoginStatusChanged

---

## Testing & Debugging

| Feature | Status | Notes |
|---------|--------|-------|
| Verbose Logging | ? | All modules |
| PIE Testing | ? | Multi-client |
| Error Messages | ? | Detailed errors |
| Debug Visualization | ?? | Basic |
| Performance Profiling | ?? | Via UE profiler |

---

## Documentation

| Document | Status | Location |
|----------|--------|----------|
| Features Documentation | ? | `FEATURES_DOCUMENTATION.md` |
| Quick Reference | ? | `QUICK_REFERENCE.md` |
| Feature Matrix | ? | `FEATURE_MATRIX.md` |
| API Reference | ? | `Documentation/API_Reference.md` |
| Quick Start Guide | ? | `Documentation/QuickStart.md` |
| Login System Guide | ? | `Documentation/Login_System_Complete.md` |
| Voice Module Guide | ? | `Source/EOSKitVoice/README_VOICE_MODULE.md` |

---

## Known Limitations

| Limitation | Impact | Workaround |
|------------|--------|------------|
| No file upload in Title Storage | Low | Use Dev Portal |
| Basic 3D audio | Low | Use UE audio system |
| No achievement progress UI | Low | Custom implementation |
| Limited editor tools | Low | Use INI files |
| Blueprint-only stats are int32 | Low | Use C++ for float stats |

---

## Roadmap / Planned Features

| Feature | Priority | Status |
|---------|----------|--------|
| Enhanced 3D Audio | Medium | ?? Planned |
| Achievement Progress Events | Low | ?? Planned |
| In-Editor Settings UI | Low | ?? Planned |
| More Ecommerce Features | Medium | ?? Planned |
| Custom Matchmaking | High | ?? Planned |
| Friends List Integration | Medium | ?? Planned |
| User Profiles | Low | ?? Planned |

---

## Version Compatibility

| EOS SDK Version | Plugin Status | Notes |
|----------------|---------------|-------|
| 1.16.x | ? | Fully tested |
| 1.17.x | ? | Compatible |
| 1.18.x | ? | Compatible |
| 1.15.x | ?? | Mostly compatible |
| < 1.15 | ? | Not supported |

| Unreal Engine | Plugin Status | Notes |
|---------------|---------------|-------|
| 5.5 | ? | Fully supported |
| 5.4 | ? | Fully supported |
| 5.3 | ? | Fully supported |
| 5.2 | ?? | Might work |
| < 5.2 | ? | Not supported |

---

## Module Dependencies

### External Dependencies
- ? Epic Online Services SDK (1.16+)
- ? EOSSDK Module (UE)
- ? OnlineSubsystem (UE)
- ? Sockets (UE)
- ? Networking (UE)
- ? AudioMixer (UE)
- ? SignalProcessing (UE)

### Internal Dependencies
```
EOSKit (Core)
??? EOSKitShared (Required)
??? EOSSDK (Required)
??? OnlineSubsystem (Required)

EOSKitAuth
??? EOSKit (Required)
??? EOSKitShared (Required)

EOSKitSessions
??? EOSKit (Required)
??? EOSKitShared (Required)

EOSKitVoice
??? EOSKit (Required)
??? EOSKitShared (Required)
??? AudioMixer (Required)
??? SignalProcessing (Required)

EOSKitSockets
??? EOSKit (Required)
??? EOSKitShared (Required)
??? Sockets (Required)

(Other modules follow similar pattern)
```

---

## Security Features

| Feature | Status | Notes |
|---------|--------|-------|
| OAuth2 Authentication | ? | Client credentials |
| Token Validation | ? | ID token verification |
| Secure Communication | ? | TLS via EOS SDK |
| Anti-Cheat Integration | ? | Server-side |
| Player Reporting | ? | Moderation |
| Sanctions System | ? | Bans/suspensions |

---

## Performance Metrics

| Metric | Performance | Notes |
|--------|-------------|-------|
| Session Search | < 1s | Typical |
| Voice Latency | < 100ms | P2P |
| Login Time | 1-3s | Network dependent |
| Lobby Join | < 500ms | Typical |
| Stat Write | < 500ms | Batch recommended |
| File Download | Varies | Size dependent |

---

*Last Updated: 2024*  
*Plugin Version: 1.0.0*

For detailed usage information, see:
- [FEATURES_DOCUMENTATION.md](FEATURES_DOCUMENTATION.md) - Complete feature details
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Code snippets and examples
- [Documentation/QuickStart.md](Documentation/QuickStart.md) - Getting started guide
