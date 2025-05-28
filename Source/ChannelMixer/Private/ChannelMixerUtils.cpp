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

#pragma region General utilities
UTexture2D* FChannelMixerUtils::CreateMaskFromGrayscales(UTexture2D* RedChannel, UTexture2D* GreenChannel, UTexture2D* BlueChannel, UTexture2D* AlphaChannel, const int32& TargetResolution)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureMixer"));
    UMaterialInstanceDynamic* MaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);

    UTexture2D* FallbackTexture = CreateFallbackTexture();

    if (RedChannel) FMaskToolsUtils::ForceTextureCompilation(RedChannel);
    if (GreenChannel) FMaskToolsUtils::ForceTextureCompilation(GreenChannel);
    if (BlueChannel) FMaskToolsUtils::ForceTextureCompilation(RedChannel);
    if (AlphaChannel) FMaskToolsUtils::ForceTextureCompilation(GreenChannel);

    MaterialInstance->SetTextureParameterValue(TEXT("Red"), RedChannel ? RedChannel : FallbackTexture);
    MaterialInstance->SetTextureParameterValue(TEXT("Green"), GreenChannel ? GreenChannel : FallbackTexture);
    MaterialInstance->SetTextureParameterValue(TEXT("Blue"), BlueChannel ? BlueChannel : FallbackTexture);
    MaterialInstance->SetTextureParameterValue(TEXT("Alpha"), AlphaChannel ? AlphaChannel : FallbackTexture);
    MaterialInstance->EnsureIsComplete();

    UTextureRenderTarget2D* tempRT = UKismetRenderingLibrary::CreateRenderTarget2D(World, TargetResolution, TargetResolution, RTF_RGBA16f);
    UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, tempRT, MaterialInstance);

    return CreateTextureFromRT(tempRT);    
}

bool FChannelMixerUtils::SaveTextureToAsset(UTexture2D* Texture, const FString& SavePath)
{
    UPackage* Package = CreatePackage(*SavePath);
    UTexture2D* NewTexture = DuplicateObject<UTexture2D>(Texture, Package, *SavePath);

    NewTexture->SetFlags(RF_Public | RF_Standalone);
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewTexture);

    FString PackageFileName = FPackageName::LongPackageNameToFilename(SavePath, FPackageName::GetAssetPackageExtension());

    FSavePackageArgs PackageArgs;
    PackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
    PackageArgs.SaveFlags = SAVE_NoError;
    PackageArgs.Error = GLog;
    PackageArgs.bForceByteSwapping = false;
    PackageArgs.bWarnOfLongFilename = true;
    PackageArgs.bSlowTask = true;

    return UPackage::SavePackage(Package, NewTexture, *PackageFileName, PackageArgs);
}

UTexture2D* FChannelMixerUtils::CreateTextureFromRT(UTextureRenderTarget2D* RenderTarget)
{

    const int32 TexResX = RenderTarget->SizeX;
    const int32 TexResY = RenderTarget->SizeY;

    UTexture2D* OutTexture = UTexture2D::CreateTransient(TexResX, TexResY);
    OutTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
    OutTexture->MipGenSettings = TextureMipGenSettings::TMGS_FromTextureGroup;
    OutTexture->AddToRoot();

    TArray<FColor> RTColor;
    RenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(RTColor);
    const int32 TextureDataSize = RTColor.Num() * sizeof(FColor);

    void* Data = OutTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

    FMemory::Memcpy(Data, RTColor.GetData(), TextureDataSize);

    OutTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

    OutTexture->UpdateResource();

    return OutTexture;

}
#pragma endregion

void UChannelMixerBPLib::CreateMaskFromGrayscales(UTexture2D*& ResultTexture, UTexture2D* RedChannel, UTexture2D* GreenChannel, UTexture2D* BlueChannel, UTexture2D* AlphaChannel, const int32& TargetResolution)
{
    ResultTexture = FChannelMixerUtils::CreateMaskFromGrayscales(RedChannel, GreenChannel, BlueChannel, AlphaChannel, TargetResolution);
}
