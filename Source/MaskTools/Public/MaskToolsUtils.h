// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct MASKTOOLS_API FMaskToolsUtils
{
    static void ForceTextureCompilation(UTexture2D* Texture);

    static TArray<UTexture2D*> SyncronousLoadCBTextures();

    static FString GetCleanPathName(UObject* OuterObject);
};
