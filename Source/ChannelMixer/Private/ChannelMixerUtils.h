#pragma once

#include "CoreMinimal.h"
#include "ChannelMixer.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Images/SImage.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "LevelEditor.h"
#include "Engine/Texture2D.h"

/**
 * Utility functions for updating the preview texture and handling import/export functionality.
 */
class FChannelMixerUtils
{
public:

    // Channel mixer specific functions

    static void UpdatePreviewTexture(FChannelMixer* Mixer);

    static FReply ImportTextureFromCB(FChannelMixer* Mixer, const FString& ChannelName, TSharedPtr<class SImage>& ChannelImage, UTexture2D** ChannelTexture);

    static FReply RestoreSlotDefaultTexture(const FString& ChannelName, TSharedPtr<SImage> SlateImage, UTexture2D* Texture, FChannelMixer* Mixer);
    static void CreateAndSetPreviewBrush(UTexture2D* NewTexture, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture);
    static void SetTextureParameterValue(const FString& ChannelName, UTexture2D** ChannelTexture, FChannelMixer* Mixer);
    static FReply ExportTexture(FChannelMixer* Mixer);

    // Channel mixer utility functions

    static int32 ResFinder(FString SelectedOption);


    // Usefull functions

    static UTexture2D* CreateMaskFromGrayscales(
        UTexture2D* RedChannel,
        UTexture2D* GreenChannel,
        UTexture2D* BlueChannel,
        UTexture2D* AlphaChannel,
        const int32& TargetResolution);

    static bool SaveTextureToAsset(UTexture2D* Texture, const FString& SavePath);

    static UTexture2D* CreateTextureFromRT(UTextureRenderTarget2D* RenderTarget);


    // General utility functions

    static void ForceTextureCompilation(UTexture2D* Texture);

    

};