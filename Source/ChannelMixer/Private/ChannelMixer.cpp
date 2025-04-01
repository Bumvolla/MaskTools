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
#include <Kismet/KismetMaterialLibrary.h>

#define LOCTEXT_NAMESPACE "FChannelMixer"

FChannelMixer::FChannelMixer()
    : PluginContentDir(IPluginManager::Get().FindPlugin(TEXT("UnrealToolsPlugin"))->GetContentDir())
    , ExportPath(TEXT("GeneratedMasks"))
    , TexturePrefix(TEXT(""))
    , TextureName(TEXT("GeneratedTexture"))
    , TextureSuffix(TEXT(""))
    , TextureResolution(512)
    , CombinedTexture(nullptr)
    , RedTexture(nullptr)
    , GreenTexture(nullptr)
    , BlueTexture(nullptr)
    , AlphaTexture(nullptr)
    , BlendMaterial(nullptr)
    , PreviewTexture(UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8))
{

}

void FChannelMixer::StartupModule()
{
    InitToolsMenuExtension();
}

void FChannelMixer::ShutdownModule()
{
    if (ToolbarExtender.IsValid())
    {
        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
        LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(ToolbarExtender);
    }
}

void FChannelMixer::InitToolsMenuExtension()
{
    ToolbarExtender = MakeShareable(new FExtender);

    ToolbarExtender->AddMenuExtension(
        "Tools",
        EExtensionHook::Before,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FChannelMixer::AddToolsMenuEntry)
    );

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(ToolbarExtender);
}

void FChannelMixer::AddToolsMenuEntry(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString(TEXT("Open Tex Mixer")),
        FText::FromString(TEXT("Open the texture mixer window")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FChannelMixer::OpenTextureMixerWindow))
    );
}

void FChannelMixer::OpenTextureMixerWindow()
{
    FChannelMixerUI::ShowTextureMixerWindow(this);


    UWorld* World = GEditor->GetEditorWorldContext().World();
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureMixer"));
    BlendMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);

}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChannelMixer, ChannelMixer)
