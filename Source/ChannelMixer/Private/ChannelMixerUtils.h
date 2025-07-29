// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChannelMixer.h"

#include "Materials/MaterialInstanceDynamic.h"

/**
 * Utility functions.
 */
struct  FChannelMixerUtils
{
    // Channel mixer utility functions

    static UTexture2D* CreateFallbackTexture();

    static int32 ResFinder(FString SelectedOption);

    static EChannelMixerTextureChannel ChannelFinder(FString SelectedOption);

    static  EResizeMethod ResizeFilterFinder(FString SelectedOption);

};