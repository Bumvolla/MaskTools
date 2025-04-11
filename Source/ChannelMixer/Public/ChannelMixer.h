// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EnchancedEditorLogging/Public/EnchancedNotifications.h"

#include "MaskTools/Public/MaskToolsConfig.h"

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

    UTexture2D* FallbackTexture;

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

    // UI data
    FString PrefixHintText = TEXT("T");
    FString NameHintText = TEXT("GeneratedMask");
    FString SuffixHintText = TEXT("Mask");

    void UpdatePreviewTexture();

    FReply ExportTexture();

    FReply RestoreSlotDefaultTexture(const FString& ChannelName, TSharedPtr<SImage> SlateImage, UTexture2D* Texture);

    FReply ImportTextureFromCB(const FString& ChannelName, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture);


private:
    void InitToolsMenuExtension();
    void AddToolsMenuEntry(FMenuBuilder& MenuBuilder);
    void OpenTextureMixerWindow();

    FString BuildPackagePath();

    void CreateAndSetPreviewBrush(UTexture2D* NewTexture, UTexture2D** ChannelTexture, TSharedPtr<SImage>& ChannelImage);
    void SetTextureParameterValue(const FString& ChannelName, UTexture2D* NewTexture);
};
