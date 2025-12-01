#include "EOSKitEditor.h"
#include "ToolMenus.h"
#include "ToolMenuSection.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "EOSKitSettings.h"
#include "EOSKitAutoConfigurator.h"
#include "LevelEditor.h"
#include "Misc/CoreDelegates.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "FEOSKitEditorModule"

void FEOSKitEditorModule::StartupModule()
{
	// ToolMenus guaranteed available at this point in UE 5.5
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this, &FEOSKitEditorModule::RegisterMenuExtensions));

	// Register Project Settings AFTER engine init
	FCoreDelegates::OnPostEngineInit.AddRaw(
		this, &FEOSKitEditorModule::OnPostEngineInit);
}

void FEOSKitEditorModule::ShutdownModule()
{
	UToolMenus::UnregisterOwner(this);
	UToolMenus::UnRegisterStartupCallback(this);

	UnregisterSettings();
}

void FEOSKitEditorModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModulePtr =
		FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule = SettingsModulePtr;

		ISettingsSectionPtr SettingsSection =
			SettingsModule->RegisterSettings(
				"Project",
				"Plugins",
				"EOSKit",
				LOCTEXT("EOSKitSettingsName", "EOSKit"),
				LOCTEXT("EOSKitSettingsDesc", "Epic Online Services Kit settings."),
				GetMutableDefault<UEOSKitSettings>()
			);

		if (SettingsSection.IsValid())
			SettingsSection->OnModified().BindRaw(this, &FEOSKitEditorModule::OnSettingsModified);
	}
}

void FEOSKitEditorModule::UnregisterSettings()
{
	if (ISettingsModule* SettingsModulePtr =
		FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModulePtr->UnregisterSettings("Project", "Plugins", "EOSKit");
	}
}

void FEOSKitEditorModule::RegisterMenuExtensions()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	if (UToolMenu* ToolbarMenu =
		UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"))
	{
		FToolMenuSection& Section =
			ToolbarMenu->AddSection("EOSKitSection",
				LOCTEXT("EOSKitSection", "EOSKit"));

		Section.AddEntry(FToolMenuEntry::InitComboButton(
			"EOSKitMenu",
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FEOSKitEditorModule::GenerateMenuContent),
			LOCTEXT("EOSKitMenu", "EOSKit"),
			LOCTEXT("EOSKitTooltip", "EOSKit Tools"),
			FSlateIcon(),
			false
		));
	}
}

TSharedRef<SWidget> FEOSKitEditorModule::GenerateMenuContent()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("OpenSettings", "Project Settings"),
		LOCTEXT("OpenSettingsTooltip", "Open EOSKit Settings"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			if (ISettingsModule* SettingsModulePtr =
				FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
			{
				SettingsModulePtr->ShowViewer("Project", "Plugins", "EOSKit");
			}
		}))
	);

	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		LOCTEXT("ConfigureProject", "Configure Project"),
		LOCTEXT("ConfigTip", "Auto-configure EOS files"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			UEOSKitAutoConfigurator::ConfigureProject();
		}))
	);

	return MenuBuilder.MakeWidget();
}

bool FEOSKitEditorModule::OnSettingsModified()
{
	if (UEOSKitSettings* Settings = GetMutableDefault<UEOSKitSettings>())
		Settings->SaveConfig();
	return true;
}

void FEOSKitEditorModule::OnPostEngineInit()
{
	RegisterSettings();
	FCoreDelegates::OnPreExit.AddRaw(this, &FEOSKitEditorModule::OnPreExit);
}

void FEOSKitEditorModule::OnPreExit()
{
	// No cleanup needed now
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEOSKitEditorModule, EOSKitEditor)
