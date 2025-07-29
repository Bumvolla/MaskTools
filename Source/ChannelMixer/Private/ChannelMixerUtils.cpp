// Copyright (c) 2025 Sora Mas
// All rights reserved.

#include "ChannelMixerUtils.h"
#include "ChannelMixer.h"
#include <MaskToolsUtils.h>
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Images/SImage.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "UObject/Package.h"
#include "UObject/SavePackage.h"

UTexture2D* FChannelMixerUtils::CreateFallbackTexture()
{

    UTexture2D* Texture = UTexture2D::CreateTransient(8, 8, PF_G8);

    Texture->MipGenSettings = TMGS_NoMipmaps;
    Texture->CompressionSettings = TC_Grayscale;
    Texture->SRGB = false;

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

    FMemory::Memset(Data, 0, 8 * 8);

    Mip.BulkData.Unlock();
    Texture->AddToRoot();
    Texture->UpdateResource();

    return Texture;
}

int32 FChannelMixerUtils::ResFinder(FString SelectedOption)
{
    static TMap<FString, int32> ResMap
    {
        {TEXT("32"), 32},
        {TEXT("64"), 64},
        {TEXT("128"), 128},
        {TEXT("256"), 256},
        {TEXT("512"), 512},
        {TEXT("1024"), 1024},
        {TEXT("2048"), 2048},
        {TEXT("4096"), 4096},
        {TEXT("8192"), 8192}
    };

    return *ResMap.Find(SelectedOption);
}

EChannelMixerTextureChannel FChannelMixerUtils::ChannelFinder(FString SelectedOption)
{
    static TMap<FString, EChannelMixerTextureChannel> ChannelMap
{
            {TEXT("Red"), EChannelMixerTextureChannel::Red},
            {TEXT("Green"), EChannelMixerTextureChannel::Green},
            {TEXT("Blue"), EChannelMixerTextureChannel::Blue},
            {TEXT("Alpha"), EChannelMixerTextureChannel::Alpha}
};

    return *ChannelMap.Find(SelectedOption);
}

EResizeMethod FChannelMixerUtils::ResizeFilterFinder(FString SelectedOption)
{
    static TMap<FString, EResizeMethod> ResizeMethodMap
    {
    {TEXT("Default"), EResizeMethod::Default},
    {TEXT("PointSample"), EResizeMethod::PointSample},
    {TEXT("Box"), EResizeMethod::Box},
    {TEXT("Triangle"), EResizeMethod::Triangle},
    {TEXT("Bilinear"), EResizeMethod::Triangle},
    {TEXT("CubicGaussian"), EResizeMethod::CubicGaussian},
    {TEXT("CubicSharp"), EResizeMethod::CubicSharp},
    {TEXT("CubicMitchell"), EResizeMethod::CubicMitchell},
    {TEXT("AdaptiveSharp"), EResizeMethod::AdaptiveSharp},
    {TEXT("AdaptiveSmooth"), EResizeMethod::AdaptiveSmooth}
                    
    };

    return *ResizeMethodMap.Find(SelectedOption);
    
}

