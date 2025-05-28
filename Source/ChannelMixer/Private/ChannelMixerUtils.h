// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChannelMixer.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "ChannelMixerUtils.generated.h"

/**
 * Utility functions.
 */
struct FChannelMixerUtils
{
    // Channel mixer utility functions

    static UTexture2D* CreateFallbackTexture();

    static int32 ResFinder(FString SelectedOption);

    static UTexture2D* CreateMaskFromGrayscales(
        UTexture2D* RedChannel = nullptr,
        UTexture2D* GreenChannel = nullptr,
        UTexture2D* BlueChannel = nullptr,
        UTexture2D* AlphaChannel = nullptr,
        const int32& TargetResolution = 512);

    static bool SaveTextureToAsset(UTexture2D* Texture, const FString& SavePath);

    static UTexture2D* CreateTextureFromRT(UTextureRenderTarget2D* RenderTarget);

};

UCLASS(BlueprintType)
class UChannelMixerBPLib : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "Channel Mixer")
    static void CreateMaskFromGrayscales(
        UTexture2D*& ResultTexture,
        UTexture2D* RedChannel = nullptr,
        UTexture2D* GreenChannel = nullptr,
        UTexture2D* BlueChannel = nullptr,
        UTexture2D* AlphaChannel = nullptr,
        const int32& TargetResolution = 512);


};