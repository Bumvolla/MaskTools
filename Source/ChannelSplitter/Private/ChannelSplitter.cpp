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
        case EMaskCreationMethod::Shader:
            SplitTexturesShaderBased();
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

        TArray<FColor> TexturePixelValues;


        auto PlatformData = Texture->GetPlatformData();
        FTexture2DMipMap& Mip = PlatformData->Mips[0];
        Mip.BulkData.LoadBulkDataWithFileReader();
        const void* Data = Mip.BulkData.LockReadOnly();
        int32 Width = Mip.SizeX;
        int32 Height = Mip.SizeY;
        TexturePixelValues.SetNum(Width * Height);
        FMemory::Memcpy(TexturePixelValues.GetData(), Data, Width * Height * sizeof(FColor));
        Mip.BulkData.Unlock();


        TArray<uint8> RChannel, GChannel, BChannel, AChannel;

        bool bExportRed = false;
        bool bExportGreen = false;
        bool bExportBlue = false;
        bool bExportAlpha = false;

        for (FColor Pixel : TexturePixelValues)
        {
            if (!bExportRed)
            {
                bExportRed = Pixel.R != 0 && Pixel.R != 255;
            }
            RChannel.Add(Pixel.R);

            if (!bExportGreen)
            {
                bExportGreen = Pixel.G != 0 && Pixel.G != 255;
            }
            GChannel.Add(Pixel.G);

            if (!bExportBlue)
            {
                bExportBlue = Pixel.B != 0 && Pixel.B != 255;
            }
            BChannel.Add(Pixel.B);

            if (!bExportAlpha)
            {
                bExportAlpha = Pixel.A != 0 && Pixel.A != 255;
            }
            AChannel.Add(Pixel.A);
        }

        TArray<TArray<uint8>> ChannelData;
        ChannelData.Add(RChannel);
        ChannelData.Add(GChannel);
        ChannelData.Add(BChannel);
        ChannelData.Add(AChannel);

        TArray<bool> bExportChannel;
        bExportChannel.Add(bExportRed);
        bExportChannel.Add(bExportGreen);
        bExportChannel.Add(bExportBlue);
        bExportChannel.Add(bExportAlpha);

        // Helper lambda to create a color with one channel set
        auto MakeColor = [](int32 ChannelIndex, uint8 Value) -> FColor
            {
                switch (ChannelIndex)
                {
                case 0: return FColor(Value, 0, 0, 255); // Red
                case 1: return FColor(0, Value, 0, 255); // Green
                case 2: return FColor(0, 0, Value, 255); // Blue
                case 3: return FColor(0, 0, 0, Value);   // Alpha
                default: return FColor::Black;
                }
            };

        for (int32 ChannelIndex = 0; ChannelIndex < 4; ++ChannelIndex)
        {
            if (_bDiscardEmptyChannels)
            {
                if (!bExportChannel[ChannelIndex])
                {
                    continue;
                }

            }

            UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
            NewTexture->MipGenSettings = TMGS_NoMipmaps;
            NewTexture->SRGB = false;

            FTexture2DMipMap& ChannelMip = NewTexture->GetPlatformData()->Mips[0];
            void* TextureData = ChannelMip.BulkData.Lock(LOCK_READ_WRITE);

            FColor* FormattedImageData = static_cast<FColor*>(TextureData);
            for (int32 PixelIndex = 0; PixelIndex < Width * Height; ++PixelIndex)
            {
                FormattedImageData[PixelIndex] = MakeColor(ChannelIndex, ChannelData[ChannelIndex][PixelIndex]);
            }

            ChannelMip.BulkData.Unlock();
            NewTexture->UpdateResource();

            const FString PathName = FMaskToolsUtils::GetCleanPathName(Texture);
            const FString PackageName = FString::Printf(TEXT("%s%s"), *PathName, *SuffixArray[ChannelIndex]);

            UPackage* Package = CreatePackage(*PackageName);

            // Duplicate the transient texture into the package
            UTexture2D* SavedTexture = DuplicateObject<UTexture2D>(NewTexture, Package, TEXT("miau"));

            // Mark the package dirty so the editor knows it needs to be saved
            SavedTexture->SetFlags(RF_Public | RF_Standalone);
            Package->MarkPackageDirty();

            // Notify asset registry so it appears in the content browser
            FAssetRegistryModule::AssetCreated(SavedTexture);

            // Save the package to disk (optional — useful for editor-only tools)
            FSavePackageArgs SaveArgs;
            SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
            SaveArgs.Error = GError;
            SaveArgs.bWarnOfLongFilename = true;
            SaveArgs.SaveFlags = SAVE_NoError;

            FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

            UPackage::SavePackage(Package, SavedTexture, *PackageFileName, SaveArgs);

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