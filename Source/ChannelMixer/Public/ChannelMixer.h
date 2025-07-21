// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ChannelMixerEnums.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#include "Logging.h"

/**
 * Main module class that holds state and initializes the texture mixer.
 */
class FChannelMixer : public IModuleInterface
{

public:

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // Slate images
    TSharedPtr<FSlateBrush> RedBrush;
    TSharedPtr<SImage> RedChannelSImage;

    TSharedPtr<FSlateBrush> GreenBrush;
    TSharedPtr<SImage> GreenChannelSImage;

    TSharedPtr<FSlateBrush> BlueBrush;
    TSharedPtr<SImage> BlueChannelSImage;

    TSharedPtr<FSlateBrush> AlphaBrush;
    TSharedPtr<SImage> AlphaChannelSImage;

    TSharedPtr<FSlateBrush> PreviewBrush;
    TSharedPtr<SImage> PreviewSImage;

    // Editor textures references
    UTexture2D* RedTexture;
    UTexture2D* GreenTexture;
    UTexture2D* BlueTexture;
    UTexture2D* AlphaTexture;
    UTexture2D* PreviewTexture;

    UTexture2D* FallbackTexture;

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

    FReply ExportTexture();

    FReply RestoreSlotDefaultTexture(EChannelMixerChannel Channel);
    void SetNewChannelTexture(UTexture2D* NewTexture, EChannelMixerChannel Channel);

    FReply ImportTextureFromCB(EChannelMixerChannel Channel);

    void RegeneratePreviewTexture();
    void RegeneratePreviewTexturePixelData();
    void RegeneratePreviewTextureMaterial();

    void UpdateSlateChannel(EChannelMixerChannel Channel);


private:
    void InitToolsMenuExtension();
    void AddToolsMenuEntry(FMenuBuilder& MenuBuilder);
    void OpenTextureMixerWindow();
    UTexture2D* GetChannelTexture(EChannelMixerChannel Channel);

    FString BuildPackagePath();
    bool bIsImporting = false;
};
