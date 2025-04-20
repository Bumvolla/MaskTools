// Copyright (c) 2025 Sora Mas
// All rights reserved.

#include "MaskTools.h"
#include "MaskToolsConfig.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FMaskToolsModule"

void FMaskToolsModule::StartupModule()
{
    RegisterSettings();
}

void FMaskToolsModule::ShutdownModule()
{
    UnregisterSettings();
}

void FMaskToolsModule::RegisterSettings()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings("Project", "Plugins", "Mask Tools",
            LOCTEXT("Mask Tools plugin configuration", "Mask Tools"),
            LOCTEXT("Settings Description", "Mask Tools configurations"),
            GetMutableDefault<UMaskToolsConfig>()
        );
    }
}

void FMaskToolsModule::UnregisterSettings()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "Mask Tools");
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMaskToolsModule, MaskTools)