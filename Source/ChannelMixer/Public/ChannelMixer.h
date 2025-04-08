// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

/**
 * Main module class that holds state and initializes the texture mixer.
 */
class FChannelMixer : public IModuleInterface
{

public:

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // State images
    TSharedPtr<SImage> RedChannelSImage;
    TSharedPtr<SImage> GreenChannelSImage;
    TSharedPtr<SImage> BlueChannelSImage;
    TSharedPtr<SImage> AlphaChannelSImage;
    TSharedPtr<SImage> PreviewSImage;

    // Editor textures references
    UTexture2D* RedTexture;
    UTexture2D* GreenTexture;
    UTexture2D* BlueTexture;
    UTexture2D* AlphaTexture;
    UTexture2D* PreviewTexture;

    // Global resources used for the final mask preview
    UTextureRenderTarget2D* CombinedTexture;
    TSharedPtr<FSlateBrush> PreviewBrush;
    UMaterialInstanceDynamic* BlendMaterial;

    // Final mask package settings
    FString TexturePrefix;
    FString TextureName = TEXT("GeneratedTexture");
    FString TextureSuffix;
    FString ExportPath = TEXT("GeneratedMasks");
    int32 TextureResolution = 512;

    FString BuildPackagePath()
    {

        FString tempPrefix = TexturePrefix.IsEmpty() ? TEXT("") : FString::Printf(TEXT("%s_"), *TexturePrefix);
        FString tempName = TextureName.IsEmpty() ? TEXT("GeneratedMask") : TextureName;
        FString tempSuffix = TextureSuffix.IsEmpty() ? TEXT("") : FString::Printf(TEXT("_%s"), *TextureSuffix);
        FString AssetName = FString::Printf(TEXT("%s%s%s"), *tempPrefix, *tempName, *tempSuffix);
        FString PackageName = FString::Printf(TEXT("/Game/%s/%s"), *ExportPath, *AssetName);
        return PackageName;
    }

private:
    void InitToolsMenuExtension();
    void AddToolsMenuEntry(FMenuBuilder& MenuBuilder);
    void OpenTextureMixerWindow();

};
