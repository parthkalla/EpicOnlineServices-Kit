# 🎮 EOSKit — Epic Online Services Integration for Unreal Engine

<div align="center">

![EOSKit Logo](https://img.shields.io/badge/EOSKit-Production%20Ready-success?style=for-the-badge&logo=unrealengine)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.3%20|%205.4%20|%205.5-informational?style=for-the-badge&logo=unrealengine)](https://www.unrealengine.com/)
[![License](https://img.shields.io/badge/License-Commercial-blue?style=for-the-badge)]()
[![EOS SDK](https://img.shields.io/badge/EOS%20SDK-1.16+-orange?style=for-the-badge)]()

A production-ready, full-featured Unreal Engine plugin that provides seamless integration with Epic Online Services (EOS).

[🚀 Quick Start](#-quick-start) • [📚 Documentation](#-documentation) • [✨ Features](#-features) • [💡 Examples](#-examples) • [🆚 Comparison](#-why-eoskit-over-other-plugins)

</div>

---

## 🌟 Overview

EOSKit is a comprehensive, enterprise-grade plugin for Unreal Engine that wraps the Epic Online Services SDK with robust, well-documented, and Blueprint-friendly APIs. Built with production use in mind, EOSKit focuses on completeness, stability, and developer experience.

### Why choose EOSKit?

- ✅ Production-ready and battle-tested
- ✅ 80+ implemented EOS features across multiple modules
- ✅ Clean, modular architecture following UE best practices
- ✅ Full Blueprint coverage (async nodes + events)
- ✅ Detailed documentation and examples
- ✅ Zero third-party dependencies — pure EOS SDK integration

---

## 🚀 Quick Start

### Installation

1. Download the plugin from the [Releases](../../releases) page.
2. Extract the plugin into your project's `Plugins/` directory.
3. Enable the plugin in your `.uproject` file.
4. Configure EOS credentials in `DefaultEngine.ini`.

### Minimal Configuration

```ini
[/Script/EOSKit.EOSKitSettings]
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
bEnableAutoLogin=True
```

### First Login (Blueprint / C++)

![Login Example](https://placehold.co/800x400/667eea/ffffff?text=Blueprint+Login+Example)

```cpp
// C++ example
UEOSLoginAsync* Login = UEOSLoginAsync::EOSLogin(
    this,
    EEOSLoginType::AccountPortal,
    FEOSLoginCredentials()
);
Login->OnSuccess.AddDynamic(this, &AMyClass::OnLoginSuccess);
Login->Activate();
```

---

## ✨ Features

### 🔐 Authentication & Identity
- Multiple authentication methods (Epic Account, Steam, Xbox, PlayStation, Device ID, etc.)
- Auto-login, account linking, device transfer, cross-platform ID mapping

### 🎮 Sessions & Matchmaking
- Create, search, join, update, destroy sessions
- Custom attributes, invites, join-in-progress, presence integration

### 👥 Lobbies
- Full lobby lifecycle with real-time updates, permissions, bucketed matchmaking, voice room creation

### 🎙️ Voice Chat (RTC)
- Device management, per-player controls, 3D positional audio, echo cancellation, noise suppression

### 🏆 Achievements
- Query definitions, progress tracking, unlocking (including hidden achievements)

### 📊 Statistics & Leaderboards
- Batch stat updates, global and regional leaderboards, time-based boards

### ☁️ Title Storage
- Cloud file management with chunked transfer, metadata, and caching

### 🌐 Networking
- Custom NetDriver, P2P with NAT traversal, socket subsystem, seamless auto-connect

### 🛡️ Security & Moderation
- Server-side anti-cheat hooks, player reports, sanctions queries, secure token validation

### 💰 E-Commerce (Planned)
- Catalog queries and offer management (purchase integration coming soon)

---

## 🆚 Why EOSKit over other plugins

EOSKit focuses on production-readiness, maintainability, and full feature coverage. Below is an easy comparison against a generic "other plugin" alternative to highlight where EOSKit adds value.

<table>
<tr>
<th>Feature</th>
<th>EOSKit</th>
<th>Other plugin</th>
<th>Notes</th>
</tr>

<tr>
<td><strong>Total Features</strong></td>
<td>✅ <strong>80+ features</strong></td>
<td>⚠️ ~40 features</td>
<td>More breadth & depth across EOS modules</td>
</tr>

<tr>
<td><strong>Voice Chat</strong></td>
<td>✅ Full implementation (device management, 3D audio, echo cancellation)</td>
<td>⚠️ Basic / partial support</td>
<td>Production-ready voice with per-player controls</td>
</tr>

<tr>
<td><strong>Lobby System</strong></td>
<td>✅ Complete: real-time updates, permissions, buckets</td>
<td>⚠️ Basic: limited real-time features</td>
<td>Designed for scalable multiplayer</td>
</tr>

<tr>
<td><strong>Blueprint Support</strong></td>
<td>✅ 100% coverage (async nodes + events)</td>
<td>⚠️ Partial</td>
<td>Blueprint-first developer experience</td>
</tr>

<tr>
<td><strong>Documentation</strong></td>
<td>✅ Extensive (guides, API reference, examples)</td>
<td>⚠️ Minimal</td>
<td>Ship faster with quality docs</td>
</tr>

</table>

---

## 💡 Examples

### Complete multiplayer flow (high-level)

```cpp
// 1. Login
UEOSLoginAsync* Login = UEOSLoginAsync::EOSLogin(this, EEOSLoginType::AccountPortal, FEOSLoginCredentials());
Login->OnSuccess.AddDynamic(this, &AMyGameMode::OnLoginSuccess);
Login->Activate();

// 2. Create session
// 3. Enable voice chat
// 4. Play!
```

(See Documentation for full samples and step-by-step guides.)

---

## 📚 Documentation

Available guides:
- Plugins/EOSKit/FEATURES_DOCUMENTATION.md — Feature overview & usage
- Plugins/EOSKit/QUICK_REFERENCE.md — Snippets & common patterns
- Plugins/EOSKit/FEATURE_MATRIX.md — Feature status & compatibility
- Plugins/EOSKit/Documentation/QuickStart.md — Step-by-step quick start
- Plugins/EOSKit/Documentation/API_Reference.md — API reference
- Plugins/EOSKit/Documentation/Login_System_Complete.md — Login system deep-dive
- Plugins/EOSKit/Source/EOSKitVoice/README_VOICE_MODULE.md — Voice module guide

---

## 🛠️ Technical Specs

Supported platforms:
- Windows, macOS, Linux, iOS, Android, Steam Deck
- Xbox / PlayStation / Nintendo via Connect integration

Requirements:
- Unreal Engine 5.3, 5.4, or 5.5
- EOS SDK 1.16+
- Visual Studio 2022 (Windows) or Xcode (macOS)

Performance targets (typical):
- Session search < 1s
- Voice latency < 100ms P2P
- Login 1–3s (network dependent)

---

## 🤝 Support & Contributing

Getting help:
- Email: support@eoskit.dev
- Issues: [GitHub Issues](../../issues)
- Docs: Plugins/EOSKit/FEATURES_DOCUMENTATION.md

Contributions are welcome — please read CONTRIBUTING.md before opening PRs.

---

## 📄 License

Commercial License — see LICENSE file for details.

This plugin integrates Epic Online Services SDK. EOS SDK licensing: https://dev.epicgames.com/docs/services

---

<div align="center">

**Made with ❤️ for the Unreal Engine community**

⭐ If EOSKit helps your project, please star the repo!

</div>
