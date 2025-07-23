// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging.h"
#include "ChannelMixerEnums.h"

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
    FAssetData RedAssetData;
    UTexture2D* RedTexture;

    FAssetData GreenAssetData;
    UTexture2D* GreenTexture;

    FAssetData BlueAssetData;
    UTexture2D* BlueTexture;

    FAssetData AlphaAssetData;
    UTexture2D* AlphaTexture;
    
    UTexture2D* PreviewTexture;

    UTexture2D* FallbackTexture;

    // Final mask package settings
    FString TexturePrefix;
    FString TextureName = TEXT("GeneratedTexture");
    FString TextureSuffix;
    FString ExportPath = TEXT("GeneratedMasks");
    int32 TextureResolution = 512;

    EChannelMixerTextureChannel RedTextureSelectedChannel;
    EChannelMixerTextureChannel GreenTextureSelectedChannel;
    EChannelMixerTextureChannel BlueTextureSelectedChannel;
    EChannelMixerTextureChannel AlphaTextureSelectedChannel;
    
    // UI data
    FString PrefixHintText = TEXT("T");
    FString NameHintText = TEXT("GeneratedMask");
    FString SuffixHintText = TEXT("Mask");

    FReply ExportTexture();

    FReply RestoreSlotDefaultTexture(EChannelMixerChannel Channel);
    
    FReply BrowseToAsset(EChannelMixerChannel Channel);

    FReply TryImportTexture(EChannelMixerChannel Channel);

    FReply ToggleContentBrowser(EChannelMixerCBAction Action = EChannelMixerCBAction::Default);

    void RegeneratePreviewTexture();
    
    void SetSelectedAsset(const FAssetData& NewSelectedAsset);

    TSharedPtr<SBox> ContentBrowserBox;
    
private:
    void InitToolsMenuExtension();
    void AddToolsMenuEntry(FMenuBuilder& MenuBuilder);
    TSharedRef<SDockTab> SpawnTextureMixerTab(const FSpawnTabArgs& Args);
    void InitializeData();
    void OpenTextureMixerWindow();
    UTexture2D* GetChannelTexture(EChannelMixerChannel Channel);
    bool ImportTextureFromCB(EChannelMixerChannel Channel);
    bool ImportTextureFromAssetPicker(EChannelMixerChannel Channel);
    FString BuildPackagePath();
    void SetNewChannelTexture(UTexture2D* NewTexture, EChannelMixerChannel Channel);
    void RegeneratePreviewTexturePixelData();
    void RegeneratePreviewTextureMaterial();
    void UpdateSlateChannel(EChannelMixerChannel Channel);
    void SetChannelAssetData(const FAssetData& NewAssetData, EChannelMixerChannel Channel);
    FAssetData SelectedAsset;
};
