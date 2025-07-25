// Copyright (c) 2025 Sora Mas
// All rights reserved.


#include "MaskToolsUtils.h"
#include "TextureCompiler.h"
#include <ContentBrowserModule.h>

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "ImageCore.h"
#include "Logging.h"
#include "MaskToolsEnums.h"
#include "MaskToolsConfig.h"
#include "PackageTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Logging.h"

void FMaskToolsUtils::ForceTextureCompilation(UTexture2D* Texture)
{
    FScopedSlowTask ForceTextureCompilationTask(1.f, FText::FromString("Forcing texture compilation..."));
    ForceTextureCompilationTask.MakeDialog();
    
    FTextureCompilingManager::Get().FinishCompilation({ Texture });
    Texture->SetForceMipLevelsToBeResident(1.f);
    
}

TArray<UTexture2D*> FMaskToolsUtils::SyncronousLoadCBTextures(TArray<FAssetData>& LoadedAssetData)
{
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    TArray<FAssetData> SelectedObjects;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedObjects);

    TArray<UTexture2D*> SelectedTextures;

    for (FAssetData AssetData : SelectedObjects)
    {
        UTexture2D* Texture = LoadTextureFromAssetData(AssetData);
        SelectedTextures.Add(Texture);
    }

    LoadedAssetData = SelectedObjects;
    return SelectedTextures;
}

UTexture2D* FMaskToolsUtils::LoadTextureFromAssetData(const FAssetData& AssetData)
{
    TSoftObjectPtr<UTexture2D> SoftTexture(AssetData.GetSoftObjectPath());
    UTexture2D* Texture = SoftTexture.LoadSynchronous();

    if (!Texture)
    {
        return nullptr;
    }

    //Ensure texture is fully loaded before using it for the render target
    ForceTextureCompilation(Texture);
    Texture->UpdateResource();
    return Texture;
}

FString FMaskToolsUtils::GetCleanPathName(UObject* OuterObject)
{
    FString PathName = OuterObject->GetPathName();
    int32 DotIndex = 0;
    PathName.FindLastChar(TEXT('.'), DotIndex);
    PathName = PathName.Left(DotIndex);
    return PathName;
}

bool FMaskToolsUtils::GetTexturePixelData(UTexture2D* Texture, int32 DestinationSize, EResizeMethod ResizeMethod, TArray<FLinearColor>& OutData)
{
    if (!IsValid(Texture)) return false;
    
    FScopedSlowTask GetPixelDataTask(2.f, FText::FromString("Retrieving texture pixel data..."));
    GetPixelDataTask.MakeDialog();

    auto FillOutDataFromImage = [&OutData, DestinationSize, ResizeMethod](const FImage& Image)
    {    
        FImage ResizedImage;
        ResizedImage.SizeX = DestinationSize;
        ResizedImage.SizeY = DestinationSize;
        ResizedImage.Format = Image.Format;
        ResizedImage.NumSlices = Image.NumSlices;
        ResizedImage.Init(DestinationSize, DestinationSize, Image.Format);
    
        FImageCore::ResizeImage(Image, ResizedImage, FMaskToolsPrivateHelpers::FindResizeMethod(ResizeMethod));
    
        TArray<FLinearColor> tmpData;
        tmpData.Reserve(ResizedImage.SizeX * ResizedImage.SizeY);
    
        for (int Y = 0; Y < ResizedImage.SizeY; Y++) 
        {
            for (int X = 0; X < ResizedImage.SizeX; X++)
            {
                tmpData.Add(ResizedImage.GetOnePixelLinear(X, Y));
            }
        }
        OutData = tmpData;
    };

    GetPixelDataTask.EnterProgressFrame(1.f, FText::FromString("Trying to access texture CPU Copy..."));
    if (FSharedImageConstRef TextureCPUCopy = Texture->GetCPUCopy())
    {
        FImage Image;
        TextureCPUCopy->CopyTo(Image);
        FillOutDataFromImage(Image);
        return true;
    }

    GetPixelDataTask.EnterProgressFrame(1.f, FText::FromString("Trying to access texture Source..."));
    if (Texture->Source.IsValid())
    {
        FImage Image;
        if (Texture->Source.GetMipImage(Image, 0, 0, 0))
        {
            FillOutDataFromImage(Image);
            return true;
        }
    }

    return false;
}

UTexture2D* FMaskToolsUtils::CreateStaticTextureEditorOnly(UTexture2D* TransientTexture, FString InName,
    TextureCompressionSettings InCompressionSettings, TextureMipGenSettings InMipSettings)
{
    if (TransientTexture == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("EmptyTexture"))
        return nullptr;
    }

    FString Name;
    FString PackageName;
    IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    //Use asset name only if directories are specified, otherwise full path
    if (!InName.StartsWith(TEXT("/")))
    {
        FString AssetName = TransientTexture->GetOutermost()->GetName();
        const FString SanitizedBasePackageName = UPackageTools::SanitizePackageName(AssetName);
        const FString PackagePath = FPackageName::GetLongPackagePath(SanitizedBasePackageName) + TEXT("/");
        AssetTools.CreateUniqueAssetName(PackagePath, InName, PackageName, Name);
    }
    else
    {
        AssetTools.CreateUniqueAssetName(InName, TEXT(""), PackageName, Name);
    }

    UPackage* NewPackage = CreatePackage(*PackageName);
    TransientTexture->Rename(*Name, NewPackage, REN_DontCreateRedirectors | REN_DoNotDirty);
    
    TransientTexture->SetFlags(RF_Public | RF_Standalone);
    TransientTexture->MarkPackageDirty();
    TransientTexture->CompressionSettings = InCompressionSettings;
    TransientTexture->MipGenSettings = InMipSettings;
    TransientTexture->SRGB = false;
    TransientTexture->PostEditChange();

    FAssetRegistryModule::AssetCreated(TransientTexture);
    return TransientTexture;
}

FImageCore::EResizeImageFilter FMaskToolsPrivateHelpers::FindResizeMethod(EResizeMethod Method)
{
    switch (Method)
    {
    case EResizeMethod::AdaptiveSharp:
        return FImageCore::EResizeImageFilter::AdaptiveSharp;

    case EResizeMethod::Bilinear:
        return FImageCore::EResizeImageFilter::Triangle;

    case EResizeMethod::AdaptiveSmooth:
        return FImageCore::EResizeImageFilter::AdaptiveSmooth;

    case EResizeMethod::Box:
        return FImageCore::EResizeImageFilter::Box;

    case EResizeMethod::CubicGaussian:
        return FImageCore::EResizeImageFilter::CubicGaussian;

    case EResizeMethod::CubicMitchell:
        return FImageCore::EResizeImageFilter::CubicMitchell;

    case EResizeMethod::CubicSharp:
         return FImageCore::EResizeImageFilter::CubicSharp;

    case EResizeMethod::Default:
        return FImageCore::EResizeImageFilter::Default;

    case EResizeMethod::Flag_WrapX:
        return FImageCore::EResizeImageFilter::Flag_WrapX;

    case EResizeMethod::Flag_WrapY:
        return FImageCore::EResizeImageFilter::Flag_WrapY;

    case EResizeMethod::PointSample:
        return FImageCore::EResizeImageFilter::PointSample;

    case EResizeMethod::Triangle:
        return FImageCore::EResizeImageFilter::Triangle;

    case EResizeMethod::WithoutFlagsMask:
        return FImageCore::EResizeImageFilter::WithoutFlagsMask;

    default:
        return FImageCore::EResizeImageFilter::Default;
        
    }
}

UMaterialInterface* FMaskToolsPrivateHelpers::LoadPluginMaterial(const FString& MaterialName)
{
    const FString PluginName = TEXT("MaskTools");
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);

    if (!Plugin.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Plugin %s not found"), *PluginName);
        return nullptr;
    }

    FString PluginAssetPath = Plugin->GetMountedAssetPath();  // e.g., "/MaskTools"
    FString MaterialPath = PluginAssetPath / TEXT("MM/") / MaterialName + TEXT(".") + MaterialName;
        
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);

    if (!BaseMaterial)
    {
        UE_LOG(LogMaskToolsUtils, Error, TEXT("Failed to load material at path: %s"), *MaterialPath);
        return nullptr;
    }

    return BaseMaterial;
}

