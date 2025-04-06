// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#include "ChannelSplitter.h"
#include "Logging.h"

#include "Modules/ModuleManager.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include <Kismet/KismetMaterialLibrary.h>
#include <Kismet/KismetRenderingLibrary.h>
#include "TextureCompiler.h"

#include "ChannelSpliterStyle.h"


#define LOCTEXT_NAMESPACE "FChannelSplitter"

void FChannelSplitter::StartupModule()
{
    InitCBMenuExtension();
    ChannelSpliterStyle::InitializeIcons();
}

void FChannelSplitter::ShutdownModule()
{
    ChannelSpliterStyle::ShutDown();
}

void FChannelSplitter::InitCBMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedAssets>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FChannelSplitter::CreateCBMenuExtension));
}

TSharedRef<FExtender> FChannelSplitter::CreateCBMenuExtension(const TArray<FAssetData>& SelectedAssets)
{
    TSharedRef<FExtender> MenuExtender(new FExtender());

    if (SelectedAssets.Num() > 0)
    {
        MenuExtender->AddMenuExtension(FName("Texture_Texture2DArray"),
            EExtensionHook::After,
            TSharedPtr<FUICommandList>(),
            FMenuExtensionDelegate::CreateRaw(this, &FChannelSplitter::AddCBMenuExtension));

        AssetsSelected = SelectedAssets;
    }

    return MenuExtender;
}

void FChannelSplitter::AddCBMenuExtension(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Split Channels")),
        FText::FromString(TEXT("Split each channel of this texture to a unique grayscale texture")),
        FSlateIcon(ChannelSpliterStyle::GetStyleSetName(), "ContentBrowser.SplitChannels"),
        FExecuteAction::CreateRaw(this, &FChannelSplitter::SplitTextures));
}

void FChannelSplitter::SplitTextures()
{

    UWorld* World = GEditor->GetEditorWorldContext().World();

    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    TArray<FAssetData> SelectedObjects;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedObjects);

    TArray<UTexture2D*> SelectedTextures;

    for (FAssetData AssetData : SelectedObjects)
    {
        TSoftObjectPtr<UTexture2D> SoftTexture(AssetData.GetSoftObjectPath());
        UTexture2D* Texture = SoftTexture.LoadSynchronous();

        if (!Texture)
        {
            continue;
        }

        //This ensures texture is fully loaded before using it for the render target
        FTextureCompilingManager::Get().FinishCompilation({ Texture });
        Texture->SetForceMipLevelsToBeResident(30.f);
        Texture->WaitForStreaming(true);

        SelectedTextures.Add(Texture);
    }


    for (UTexture2D* Texture : SelectedTextures)
    {

        TArray<UMaterialInstanceDynamic*> SplitMaterialsArray;

        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_R")),FName("Red"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_G")),FName("Green"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_B")),FName("Blue"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_A")),FName("Alpha"), EMIDCreationFlags::Transient));

        // Copy original texture values and settings
        int32 OgTexResX = Texture->GetImportedSize().X;
        int32 OgTexResY = Texture->GetImportedSize().Y;
        TEnumAsByte<TextureMipGenSettings> MipGenSettings = Texture->MipGenSettings;
        int32 LodBias = Texture->LODBias;
        int32 MaxTextureSize = Texture->MaxTextureSize;
        bool bOodlePreserveExtremes = Texture->bOodlePreserveExtremes;
        bool bPreserveBorders = Texture->bPreserveBorder;
        TEnumAsByte<TextureCookPlatformTilingSettings> CookPlatformTilingSettings = Texture->CookPlatformTilingSettings;
        uint8 NeverStream = Texture->NeverStream;

        // Iterator for the suffixes
        int i = 0;

        // Export each one of the channels
        for (UMaterialInstanceDynamic* Material : SplitMaterialsArray)
        {

            Material->SetTextureParameterValue(TEXT("Texture"), Texture);
            Material->EnsureIsComplete();

            // I'm using GetPathName because it returns editor relative path
            // It must be cleaned afterwards
            FString PathName = Texture->GetPathName();
            int32 DotIndex = 0;
            PathName.FindLastChar(TEXT('.'), DotIndex);
            PathName = PathName.Left(DotIndex);
            const FString PackageName = FString::Printf(TEXT("%s%s"), *PathName, *SuffixArray[i]);

            UTextureRenderTarget2D* tempRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, OgTexResX, OgTexResY, RTF_R16f);
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, tempRT , Material);

            UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
                tempRT,
                PackageName,
                TextureCompressionSettings::TC_Grayscale,
                MipGenSettings);

            // Paste original texture values
            ExportedTexture->LODBias = LodBias;
            ExportedTexture->MaxTextureSize = MaxTextureSize;
            ExportedTexture->bOodlePreserveExtremes = bOodlePreserveExtremes;
            ExportedTexture->bPreserveBorder = bPreserveBorders;
            ExportedTexture-> CookPlatformTilingSettings = CookPlatformTilingSettings;
            ExportedTexture->NeverStream = NeverStream;

            i++;
        }

        i = 0;

    }

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChannelSplitter, ChannelSplitter);