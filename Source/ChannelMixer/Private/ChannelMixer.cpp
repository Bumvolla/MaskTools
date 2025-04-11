// Copyright (c) 2025 Sora Mas
// All rights reserved.

#include "ChannelMixer.h"
#include "ChannelMixerUI.h"
#include "ChannelMixerUtils.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "LevelEditor.h"

#include "ChannelMixerStyle.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "MaskTools/Public/MaskToolsConfig.h"


#define LOCTEXT_NAMESPACE "FChannelMixer"

#pragma region Module
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

#pragma endregion

#pragma region MenuExtension

void FChannelMixer::InitToolsMenuExtension()
{

    TSharedRef<FExtender> MenuExtender(new FExtender());

    MenuExtender->AddMenuExtension(
        "Tools",
        EExtensionHook::After,
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

    // Retrieve config from project settings
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();

    ExportPath = Config->DefaultMaskSavePath.Path;
    TextureName = Config->DefaultMaskName;

    PrefixHintText = Config->DefaultMaskPrefix;
    NameHintText = TextureName;
    SuffixHintText = Config->DefaultMaskSuffix;

    const UEnum* EnumPtr = StaticEnum<EMaskResolutions>();
    FText defaultResString = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Config->DefaultMaskResolution));
    TextureResolution = FChannelMixerUtils::ResFinder(defaultResString.ToString());

    // Create default values
    UWorld* World = GEditor->GetEditorWorldContext().World();
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureMixer"));
    BlendMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);
    PreviewBrush = MakeShared<FSlateBrush>();
    CombinedTexture = UKismetRenderingLibrary::CreateRenderTarget2D(World, TextureResolution, TextureResolution);
    PreviewBrush->SetResourceObject(CombinedTexture);
    FallbackTexture = FChannelMixerUtils::CreateFallbackTexture();

    // Show main window
    FChannelMixerUI::ShowTextureMixerWindow(this);

}

FString FChannelMixer::BuildPackagePath()
{

    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();

    FString tempPrefix, tempName, tempSuffix;

    if (TexturePrefix.IsEmpty())
    {
        if (Config->bDefaultAddPrefix) tempPrefix = FString::Printf(TEXT("%s_"), *Config->DefaultMaskPrefix);
        else tempPrefix = TEXT("");
    }
    else 
        tempPrefix = FString::Printf(TEXT("%s_"), *TexturePrefix);

    TextureName.IsEmpty() ? Config->DefaultMaskName : TextureName;

    if (TextureSuffix.IsEmpty())
    {
        if (Config->bDefaultAddSuffix) tempSuffix = FString::Printf(TEXT("%s_"), *Config->DefaultMaskSuffix);
        else tempSuffix = TEXT("");
    }
    else 
        tempSuffix = FString::Printf(TEXT("%s_"), *TextureSuffix);

    FString AssetName = FString::Printf(TEXT("%s%s%s"), *tempPrefix, *tempName, *tempSuffix);
    FString PackageName = FString::Printf(TEXT("/Game/%s/%s"), *ExportPath, *AssetName);
    return PackageName;

}
#pragma endregion

#pragma region Channel Mixer Logic
FReply FChannelMixer::ImportTextureFromCB(const FString& ChannelName, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture)
{

    FContentBrowserModule* ContentBrowserModule = &FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TArray<FAssetData> SelectedAssets;
    ContentBrowserModule->Get().GetSelectedAssets(SelectedAssets);

    if (SelectedAssets.Num() == 1)
    {
        UObject* SelectedObject = SelectedAssets[0].GetAsset();
        if (UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedObject))
        {
            //This ensures texture is fully loaded before using it for the render target
            FChannelMixerUtils::ForceTextureCompilation(SelectedTexture);

            CreateAndSetPreviewBrush(SelectedTexture, ChannelTexture, ChannelImage);
            SetTextureParameterValue(ChannelName, SelectedTexture);
        }
        else
        {
            UEnchancedNotifications::LaunchNotification(TEXT("Please select a texture to import"));
        }
    }
    else
    {
        UEnchancedNotifications::LaunchNotification(TEXT("Please select a texture to import"));
    }

    UpdatePreviewTexture();
    return FReply::Handled();
}

void FChannelMixer::UpdatePreviewTexture()
{


    UWorld* World = GEditor->GetEditorWorldContext().World();
    UKismetRenderingLibrary::ClearRenderTarget2D(World, CombinedTexture, FLinearColor::Black);
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, CombinedTexture, BlendMaterial);

    PreviewSImage->SetImage(PreviewBrush.Get());

    FSlateApplication::Get().Tick();
}

FReply FChannelMixer::ExportTexture()
{
    if (!CombinedTexture)
    {
        return FReply::Handled();
    }

    FString PackageName = BuildPackagePath();

    UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
        CombinedTexture,
        PackageName,
        TextureCompressionSettings::TC_Masks,
        TextureMipGenSettings::TMGS_NoMipmaps
    );

    UEnchancedNotifications::OpenCBDirNotification(FString::Printf(TEXT("Successfully exported combined texture to /Content/%s"), *ExportPath), FString::Printf(TEXT("/Game/%s"), *ExportPath));

    return FReply::Handled();
}

FReply FChannelMixer::RestoreSlotDefaultTexture(const FString& ChannelName, TSharedPtr<SImage> SlateImage, UTexture2D* Texture)
{

    CreateAndSetPreviewBrush(FallbackTexture, &Texture, SlateImage);
    SetTextureParameterValue(ChannelName, Texture);

    UpdatePreviewTexture();
    FSlateApplication::Get().Tick();

    return FReply::Handled();
}

void FChannelMixer::CreateAndSetPreviewBrush(UTexture2D* NewTexture, UTexture2D** ChannelTexture, TSharedPtr<SImage>& ChannelImage)
{
    FSlateBrush* NewBrush = new FSlateBrush();
    NewBrush->SetResourceObject(NewTexture);
    ChannelImage->SetImage(NewBrush);
    *ChannelTexture = NewTexture;
}

void FChannelMixer::SetTextureParameterValue(const FString& ChannelName, UTexture2D* NewTexture)
{
    BlendMaterial->SetTextureParameterValue(FName(ChannelName), NewTexture);
}

#pragma endregion


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChannelMixer, ChannelMixer)
