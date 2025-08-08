// Copyright Epic Games, Inc. All Rights Reserved.

#include "VivoxIntegration.h"
#include "VivoxSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FVivoxIntegrationModule"

void FVivoxIntegrationModule::StartupModule()
{
	Settings = NewObject<UVivoxSettings>(GetTransientPackage(), "VivoxSettings", RF_Standalone);
	Settings->AddToRoot();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Vivox",
			LOCTEXT("VivoxSettingsName", "Vivox"),
			LOCTEXT("VivoxSettingsDescription", "Settings for Vivox"),
			Settings);
	}
}

void FVivoxIntegrationModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Vivox");
	}

	if (!GExitPurge)
	{
		Settings->RemoveFromRoot();
	}
	else
	{
		Settings = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVivoxIntegrationModule, VivoxIntegration)