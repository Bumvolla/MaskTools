#include "ChannelMixer.h"
#include "ChannelMixerUI.h"
#include "ChannelMixerUtils.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SWindow.h"
#include "Logging/LogMacros.h"
#include "ChannelMixerStyle.h"
#include <Kismet/KismetMaterialLibrary.h>

#define LOCTEXT_NAMESPACE "FChannelMixer"

void FChannelMixer::StartupModule()
{
    InitToolsMenuExtension();

    ChannelMixerStyle::InitializeIcons();
}

void FChannelMixer::ShutdownModule()
{

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    ChannelMixerStyle::ShutDown();
}

void FChannelMixer::InitToolsMenuExtension()
{

    TSharedRef<FExtender> MenuExtender(new FExtender());

    MenuExtender->AddMenuExtension(
        "Tools",
        EExtensionHook::Before,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FChannelMixer::AddToolsMenuEntry)
    );

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FChannelMixer::AddToolsMenuEntry(FMenuBuilder& MenuBuilder)
{

    MenuBuilder.AddMenuEntry(
        FText::FromString(TEXT("Texture Mixer")),
        FText::FromString(TEXT("Open the texture mixer window")),
        FSlateIcon(ChannelMixerStyle::GetStyleSetName(), "Tools.MixChannels"),
        FUIAction(FExecuteAction::CreateRaw(this, &FChannelMixer::OpenTextureMixerWindow))
    );
}

void FChannelMixer::OpenTextureMixerWindow()
{
    FChannelMixerUI::ShowTextureMixerWindow(this);


    UWorld* World = GEditor->GetEditorWorldContext().World();
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureMixer"));
    BlendMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);
    PreviewTexture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);

}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChannelMixer, ChannelMixer)
