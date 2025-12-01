// Copyright (C) 2024, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class ISettingsModule;
class SWidget;

/**
 * EOSKit Editor Module
 * Handles editor-specific functionality including:
 * - Project Settings panel
 * - Auto-configuration tools
 * - EOS artifact importer
 * - SDK folder selectors
 */
class EOSKITEDITOR_API FEOSKitEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register Project Settings */
	void RegisterSettings();

	/** Unregister Project Settings */
	void UnregisterSettings();

	/** Register menu extensions */
	void RegisterMenuExtensions();

	/** Unregister menu extensions */
	void UnregisterMenuExtensions();

	/** Generate menu content */
	TSharedRef<SWidget> GenerateMenuContent();

	/** Handle settings modification */
	bool OnSettingsModified();

	/** Handle post-engine initialization */
	void OnPostEngineInit();

	/** Handle pre-exit */
	void OnPreExit();

	/** Settings module handle */
	ISettingsModule* SettingsModule;
};

