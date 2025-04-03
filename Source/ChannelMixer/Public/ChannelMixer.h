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

    FString GetExportPath() const { return ExportPath; }
    FString GetTexturePrefix() const { return TexturePrefix; }
    FString GetTextureName() const { return TextureName; }
    FString GetTextureSuffix() const { return TextureSuffix; }
    int32 GetTextureResolution()const { return TextureResolution; }

    // State variables shared with UI & utils.
    TSharedPtr<SImage> RedChannelImage;
    TSharedPtr<SImage> GreenChannelImage;
    TSharedPtr<SImage> BlueChannelImage;
    TSharedPtr<SImage> AlphaChannelImage;
    TSharedPtr<SImage> PreviewImage;

    UTextureRenderTarget2D* CombinedTexture;

    UTexture2D* PreviewTexture;
    UTexture2D* RedTexture;
    UTexture2D* GreenTexture;
    UTexture2D* BlueTexture;
    UTexture2D* AlphaTexture;

    UMaterialInstanceDynamic* BlendMaterial;
    TSharedPtr<FSlateBrush> PreviewBrush;

    FString TexturePrefix;
    FString TextureName = TEXT("GeneratedTexture");
    FString TextureSuffix;
    FString ExportPath = TEXT("GeneratedMasks");
    int32 TextureResolution = 512;


private:
    void InitToolsMenuExtension();
    void AddToolsMenuEntry(FMenuBuilder& MenuBuilder);
    void OpenTextureMixerWindow();

    const FString PluginContentDir;

    TSharedPtr<FExtender> ToolbarExtender;
};
