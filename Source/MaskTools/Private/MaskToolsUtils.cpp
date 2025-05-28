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

bool FMaskToolsUtils::GetTexturePixelData(UTexture2D* Texture, TArray<FColor>& OutData)
{


    if (!IsValid(Texture)) return false;

    FScopedSlowTask GetPixelDataTask(3.f, FText::FromString("Retrieving texture pixel data..."));
    GetPixelDataTask.MakeDialog();

    GetPixelDataTask.EnterProgressFrame(1.f, FText::FromString("Trying to acces texture Source..."));
    if (Texture->Source.IsValid())
    {
        const uint8* SourceData = Texture->Source.LockMipReadOnly(0);
        int32 Width = Texture->Source.GetSizeX();
        int32 Height = Texture->Source.GetSizeY();

        OutData.SetNum(Width * Height);
        FMemory::Memcpy(OutData.GetData(), SourceData, Width * Height * sizeof(FColor));

        Texture->Source.UnlockMip(0);
        return true;
    }

    GetPixelDataTask.EnterProgressFrame(1.f, FText::FromString("Trying to acces texture GPU Resource..."));
    FTexture2DRHIRef TextureRHI = Texture->GetResource() ? Texture->GetResource()->GetTexture2DRHI() : nullptr;
    if (TextureRHI.IsValid())
    {
        FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
        RHICmdList.ReadSurfaceData(TextureRHI, FIntRect(0, 0, Texture->GetSizeX(), Texture->GetSizeY()), OutData, FReadSurfaceDataFlags());
        return true;
    }

    GetPixelDataTask.EnterProgressFrame(1.f, FText::FromString("Trying to acces texture Bulk Data..."));
    FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    if (!PlatformData) return false;

    FTexture2DMipMap& Mip = PlatformData->Mips[0];
    if (Mip.BulkData.DoesExist())
    {
        Mip.BulkData.LoadBulkDataWithFileReader();
        const void* Data = Mip.BulkData.LockReadOnly();
        int32 Width = Mip.SizeX;
        int32 Height = Mip.SizeY;
        OutData.SetNum(Width * Height);
        FMemory::Memcpy(OutData.GetData(), Data, Width * Height * sizeof(FColor));
        Mip.BulkData.Unlock();
        return true;
    }
    return false;
}
