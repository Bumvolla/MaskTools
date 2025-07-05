// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
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


};

UCLASS(BlueprintType)
class UChannelMixerBPLib : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

};