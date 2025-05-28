// Copyright (c) 2025 Sora Mas \n All rights reserved. 


#include "MaskToolsUtils.h"
#include "TextureCompiler.h"
#include <ContentBrowserModule.h>
#include "IContentBrowserSingleton.h"

void FMaskToolsUtils::ForceTextureCompilation(UTexture2D* Texture)
{
    FTextureCompilingManager::Get().FinishCompilation({ Texture });
    Texture->SetForceMipLevelsToBeResident(30.f);
    Texture->WaitForStreaming(true);
}

TArray<UTexture2D*> FMaskToolsUtils::SyncronousLoadCBTextures()
{
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
    TArray<FAssetData> SelectedObjects;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedObjects);

    TArray<UTexture2D*> SelectedTextures;

    for (FAssetData AssetData : SelectedObjects)
    {
        TSoftObjectPtr<UTexture2D> SoftTexture(AssetData.GetSoftObjectPath());
        UTexture2D* Texture = SoftTexture.LoadSynchronous();

        if (!Texture)
        {
            continue;
        }

        //Ensure texture is fully loaded before using it for the render target
        ForceTextureCompilation(Texture);
        Texture->UpdateResource();

        SelectedTextures.Add(Texture);
    }
    return SelectedTextures;
}

FString FMaskToolsUtils::GetCleanPathName(UObject* OuterObject)
{
    FString PathName = OuterObject->GetPathName();
    int32 DotIndex = 0;
    PathName.FindLastChar(TEXT('.'), DotIndex);
    PathName = PathName.Left(DotIndex);
    return PathName;
}
