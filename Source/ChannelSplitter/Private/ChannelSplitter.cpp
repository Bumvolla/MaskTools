// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#include "ChannelSplitter.h"
#include "Logging.h"
#include "MaskEnums.h"
#include "MaskToolsUtils.h"

#include "Modules/ModuleManager.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "TextureCompiler.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include "ChannelSpliterStyle.h"
#include "ImageUtils.h"
#include "MaterialDomain.h"
#include "MaskTools/Public/MaskToolsConfig.h"

#include "Misc/EngineVersionComparison.h"

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
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();

    switch (Config->SplitterCreationMethod)
    {
        case EMaskCreationMethod::Material:
            SplitTexturesMaterialBased();
            break;
        case EMaskCreationMethod::PixelData:
            SplitTexturesPixelData();
            break;

    }

}

void FChannelSplitter::SplitTexturesMaterialBased()
{
    // General setup
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    bool _bDiscardEmptyChannels = Config->bDiscardEmptyChannels;

    // Cast and store selected content browser assets if textures
    const TArray<UTexture2D*> SelectedTextures = FMaskToolsUtils::SyncronousLoadCBTextures();

    for (UTexture2D* Texture : SelectedTextures)
    {
        TArray<UMaterialInstanceDynamic*> SplitMaterialsArray;

        SplitMaterialsArray.Add(UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_R")), World, FName("Red")));
        SplitMaterialsArray.Add(UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_G")), World, FName("Green")));
        SplitMaterialsArray.Add(UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_B")), World, FName("Blue")));
        SplitMaterialsArray.Add(UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureSplitter_A")), World, FName("Alpha")));

        // Copy original texture values and settings
        const int32 OgTexResX = Texture->GetImportedSize().X;
        const int32 OgTexResY = Texture->GetImportedSize().Y;
        const TEnumAsByte<TextureMipGenSettings> MipGenSettings = Texture->MipGenSettings;
        const int32 LodBias = Texture->LODBias;
        const int32 MaxTextureSize = Texture->MaxTextureSize;
        const bool bPreserveBorders = Texture->bPreserveBorder;
        const uint8 NeverStream = Texture->NeverStream;

#if UE_VERSION_NEWER_THAN(5, 2, 0)
        const TEnumAsByte<TextureCookPlatformTilingSettings> CookPlatformTilingSettings = Texture->CookPlatformTilingSettings;
#endif // UE_VERSION_NEWER_THAN(5, 2, 0)

#if UE_VERSION_NEWER_THAN(5, 4, 0)
        const bool bOodlePreserveExtremes = Texture->bOodlePreserveExtremes;
#endif // UE_VERSION_NEWER_THAN(5, 4, 0)

        // Iterator for the suffixes
        int i = 0;

        // Export each one of the channels
        for (UMaterialInstanceDynamic* Material : SplitMaterialsArray)
        {
            Material->SetTextureParameterValue(TEXT("Texture"), Texture);
            Material->EnsureIsComplete();

            // Try discarting current iteration if channel is empty
            if (_bDiscardEmptyChannels)
            {
                if (IsChannelEmpty(Material))
                {
                    i++;
                    continue;
                }

            }

            // I'm using GetPathName because it returns editor content folder relative path
            // It must be cleaned afterwards
            const FString PathName = FMaskToolsUtils::GetCleanPathName(Texture);
            const FString PackageName = FString::Printf(TEXT("%s%s"), *PathName, *SuffixArray[i]);

            // Create and draw material to render target
            UTextureRenderTarget2D* tempRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, OgTexResX, OgTexResY, RTF_R16f);
            UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, tempRT, Material);

            // Export the texture
            UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
                tempRT,
                PackageName,
                TextureCompressionSettings::TC_Grayscale,
                MipGenSettings);

            // Notify the editor about changes for safety
            ExportedTexture->PreEditChange(nullptr);

            // Paste original texture values
            ExportedTexture->LODBias = LodBias;
            ExportedTexture->MaxTextureSize = MaxTextureSize;
            ExportedTexture->bPreserveBorder = bPreserveBorders;
            ExportedTexture->NeverStream = NeverStream;

#if UE_VERSION_NEWER_THAN(5, 2, 0)
            ExportedTexture->CookPlatformTilingSettings = CookPlatformTilingSettings;
#endif // UE_VERSION_NEWER_THAN(5, 2, 0)

#if UE_VERSION_NEWER_THAN(5, 4, 0)
            ExportedTexture->bOodlePreserveExtremes = bOodlePreserveExtremes;
#endif // UE_VERSION_NEWER_THAN(5, 4, 0)

            // Notify the editor changes finished
            ExportedTexture->PostEditChange();
            ExportedTexture->MarkPackageDirty();

            i++;
        }

        i = 0;

    }
}

void FChannelSplitter::SplitTexturesPixelData()
{
    // General setup
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
    UWorld* World = GEditor->GetEditorWorldContext().World();
    bool _bDiscardEmptyChannels = Config->bDiscardEmptyChannels;

    // Cast and store selected content browser assets if textures
    const TArray<UTexture2D*> SelectedTextures = FMaskToolsUtils::SyncronousLoadCBTextures();

    for (UTexture2D* Texture : SelectedTextures)
    {

        // Copy original texture values and settings
        const FString PathName = FMaskToolsUtils::GetCleanPathName(Texture);
        
        const TEnumAsByte<TextureMipGenSettings> MipGenSettings = Texture->MipGenSettings;
        const int32 LodBias = Texture->LODBias;
        const int32 MaxTextureSize = Texture->MaxTextureSize;
        const bool bPreserveBorders = Texture->bPreserveBorder;
        const uint8 NeverStream = Texture->NeverStream;
        const uint32 Size = Texture ->GetSizeX();

        if (Size != Texture->GetSizeY())
        {
            UE_LOG(LogTemp, Warning, TEXT("Texture width and height mismatch, currently not supported"))
            return;
        }
#if UE_VERSION_NEWER_THAN(5, 2, 0)
        const TEnumAsByte<TextureCookPlatformTilingSettings> CookPlatformTilingSettings = Texture->CookPlatformTilingSettings;
#endif // UE_VERSION_NEWER_THAN(5, 2, 0)

#if UE_VERSION_NEWER_THAN(5, 4, 0)
        const bool bOodlePreserveExtremes = Texture->bOodlePreserveExtremes;
#endif // UE_VERSION_NEWER_THAN(5, 4, 0)

        TArray<FLinearColor> TexturePixelValues;
        if (!FMaskToolsUtils::GetTexturePixelData(Texture, Size, TexturePixelValues))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get texture pixel data"));
            return;
        }
        if (TexturePixelValues.Num() <= 0)
        {
            // UE_LOG(LogTemp, Warning, TEXT("Empty pixel data array"));
            return;
        }
        TArray<FColor> RChannel, GChannel, BChannel, AChannel;

        bool bExportRed = false;
        bool bExportGreen = false;
        bool bExportBlue = false;
        bool bExportAlpha = false;

        auto AddPixelToArray = [](const uint8& ColorValue, bool& bExportColor, TArray<FColor>& OutPixelValues)
        {
            if (!bExportColor)
            {
                bExportColor = ColorValue != 0 && ColorValue != 255;
            }
            OutPixelValues.Add(FColor(ColorValue, 0, 0, 0));
        };
        
        for (FLinearColor LinearPixel : TexturePixelValues)
        {

            FColor Pixel = LinearPixel.ToFColor(false);
            
            AddPixelToArray(Pixel.R, bExportRed, RChannel);
            AddPixelToArray(Pixel.G, bExportGreen, GChannel);
            AddPixelToArray(Pixel.B, bExportBlue, BChannel);
            AddPixelToArray(Pixel.A, bExportAlpha, AChannel);
            
        }
        
        TArray<bool> bExportChannel;
        bExportChannel.Add(bExportRed);
        bExportChannel.Add(bExportGreen);
        bExportChannel.Add(bExportBlue);
        bExportChannel.Add(bExportAlpha);

        TArray<TArray<FColor>> ChannelPixelValues;
        ChannelPixelValues.Add(RChannel);
        ChannelPixelValues.Add(GChannel);
        ChannelPixelValues.Add(BChannel);
        ChannelPixelValues.Add(AChannel);

        for (int32 ChannelIndex = 0; ChannelIndex < 4; ++ChannelIndex)
        {
            if (_bDiscardEmptyChannels)
            {
                if (!bExportChannel[ChannelIndex])
                {
                    FString DebugLine = FString::Printf(TEXT("%s channel discarded in texture %s"), *SuffixArray[ChannelIndex], *PathName);
                    UE_LOG(LogTemp, Warning, TEXT("%s"), *DebugLine);
                    continue;
                }
            }

            FCreateTexture2DParameters TextureParameters;
            TextureParameters.TextureGroup = TextureGroup::TEXTUREGROUP_World;
            TextureParameters.bSRGB = false;
            TextureParameters.CompressionSettings = TC_Grayscale;
            TextureParameters.MipGenSettings = MipGenSettings;
            TextureParameters.bUseAlpha = false;
            
            UTexture2D* NewTexture = FImageUtils::CreateTexture2D(Size, Size, ChannelPixelValues[ChannelIndex], World, TEXT(""), EObjectFlags::RF_KeepForCooker, TextureParameters);

            const FString PackageName = FString::Printf(TEXT("%s%s"), *PathName, *SuffixArray[ChannelIndex]);
            UTexture2D* SavedTexture = FMaskToolsUtils::CreateStaticTextureEditorOnly(NewTexture, PackageName, TC_Grayscale, TMGS_FromTextureGroup);

            // Notify the editor about changes for safety
            SavedTexture->PreEditChange(nullptr);

            // Paste original texture values
            SavedTexture->LODBias = LodBias;
            SavedTexture->MaxTextureSize = MaxTextureSize;
            SavedTexture->bPreserveBorder = bPreserveBorders;
            SavedTexture->NeverStream = NeverStream;

#if UE_VERSION_NEWER_THAN(5, 2, 0)
            SavedTexture->CookPlatformTilingSettings = CookPlatformTilingSettings;
#endif // UE_VERSION_NEWER_THAN(5, 2, 0)

#if UE_VERSION_NEWER_THAN(5, 4, 0)
            SavedTexture->bOodlePreserveExtremes = bOodlePreserveExtremes;
#endif // UE_VERSION_NEWER_THAN(5, 4, 0)

            // Notify the editor changes finished
            SavedTexture->PostEditChange();
            SavedTexture->MarkPackageDirty();

        }

    }
}

void FChannelSplitter::SplitTexturesShaderBased()
{

}

bool FChannelSplitter::IsChannelEmpty(UMaterialInstanceDynamic* Material)
{

    UWorld* World = GEditor->GetEditorWorldContext().World();
    UTextureRenderTarget2D* tempRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, 256 , 256, RTF_RGBA8);

    FTextureRenderTargetResource* RTResource = tempRT->GameThread_GetRenderTargetResource();
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, tempRT, Material);

    // Read the render target pixels into an array.
    TArray<FColor> PixelData;
    RTResource->ReadPixels(PixelData);

    // Iterate over pixel data checking the red channel.
    for (const FColor& Pixel : PixelData)
    {
        if (Pixel.R != 0 && Pixel.R != 255)
        {
            return false;
        }
    }

    return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FChannelSplitter, ChannelSplitter);