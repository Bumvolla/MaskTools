// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "ImageCore.h"
#include "MaskToolsUtils.generated.h"

/**
 * 
 */
USTRUCT()
struct MASKTOOLS_API FMaskToolsUtils
{

    GENERATED_BODY()
    
    static void ForceTextureCompilation(UTexture2D* Texture);

    static TArray<UTexture2D*> SyncronousLoadCBTextures();

    static FString GetCleanPathName(UObject* OuterObject);

    static bool GetTexturePixelData(UTexture2D* Texture, int32 DestinationSize, TArray<FLinearColor>& OutData);

    static UTexture2D* CreateStaticTextureEditorOnly(UTexture2D* TransientTexture, FString InName, TextureCompressionSettings InCompressionSettings, TextureMipGenSettings InMipSettings);

};

struct MASKTOOLS_API FMaskToolsPrivateHelpers
{
    static FImageCore::EResizeImageFilter FindResizeMethod(EResizeMethod Method);
};
