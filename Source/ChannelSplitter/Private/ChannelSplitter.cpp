// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#include "ChannelSplitter.h"
#include "Logging.h"

#include "Modules/ModuleManager.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"


#include <Kismet/KismetMaterialLibrary.h>
#include <Kismet/KismetRenderingLibrary.h>

#define LOCTEXT_NAMESPACE "FChannelSplitter"

void FChannelSplitter::StartupModule()
{
    InitCBMenuExtension();
}

void FChannelSplitter::ShutdownModule()
{
	
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
        FSlateIcon(),//(FUnrealToolsStyle::GetStyleSetName(), "ContentBrowser.MakeMask"),
        FExecuteAction::CreateRaw(this, &FChannelSplitter::SplitTextures));
}

void FChannelSplitter::SplitTextures()
{

    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    TArray<FAssetData> SelectedObjects;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedObjects);

    bool bAllSelectedAssetsAreTextures = true;
    TArray<UTexture2D*> SelectedTextures;

    for (FAssetData AssetData : SelectedObjects)
    {
        UObject* Object = AssetData.GetAsset();
        UTexture2D* Texture = Cast<UTexture2D>(Object);
        if (!Texture) bAllSelectedAssetsAreTextures = false;
        else SelectedTextures.Add(Texture);
    }

    if (!bAllSelectedAssetsAreTextures) return;

    UWorld* World = GEditor->GetEditorWorldContext().World();

    for (UTexture2D* Texture : SelectedTextures)
    {

        TArray<UMaterialInstanceDynamic*> SplitMaterialsArray;

        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_R")),FName("Red"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_G")),FName("Green"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_B")),FName("Blue"), EMIDCreationFlags::Transient));
        SplitMaterialsArray.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_A")),FName("Alpha"), EMIDCreationFlags::Transient));


        int32 TexResX = Texture->GetImportedSize().X;
        int32 TexResY = Texture->GetImportedSize().Y;

        int i = 0;
        for (UMaterialInstanceDynamic* Material : SplitMaterialsArray)
        {
            Material->SetTextureParameterValue(TEXT("Texture"), Texture);

            const FString PackageName = FString::Printf(TEXT("%s%s"), *Texture->GetPathName(), *SuffixArray[i]);

            UTextureRenderTarget2D* tempRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, TexResX, TexResY, RTF_R16f);
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, tempRT , Material);

            UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
                tempRT,
                PackageName,
                TextureCompressionSettings::TC_Grayscale,
                TextureMipGenSettings::TMGS_NoMipmaps);

            i++;
        }

        i = 0;

    }

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChannelSplitter, ChannelSplitter);