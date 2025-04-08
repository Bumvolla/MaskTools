#include "ChannelMixerUtils.h"
#include "ChannelMixer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Images/SImage.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "LevelEditor.h"
#include "EnchancedEditorLogging/Public/EnchancedNotifications.h"
#include "AssetRegistry/AssetRegistryModule.h"



#include "TextureCompiler.h"


void FChannelMixerUtils::UpdatePreviewTexture(FChannelMixer* Mixer)
{
    if (!Mixer) return;

    UWorld* World = GEditor->GetEditorWorldContext().World();

    Mixer->CombinedTexture = UKismetRenderingLibrary::CreateRenderTarget2D(World, Mixer->TextureResolution, Mixer->TextureResolution);
    Mixer->CombinedTexture->AddToRoot();

    UKismetRenderingLibrary::ClearRenderTarget2D(World, Mixer->CombinedTexture, FLinearColor(0, 0, 0, 255));

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, Mixer->CombinedTexture, Mixer->BlendMaterial);

    if (!Mixer->PreviewBrush.IsValid())
    {
        Mixer->PreviewBrush = MakeShared<FSlateBrush>();
    }

    Mixer->PreviewBrush->SetResourceObject(Mixer->CombinedTexture);
    if (Mixer->PreviewSImage.IsValid())
    {
        Mixer->PreviewSImage->SetImage(Mixer->PreviewBrush.Get());
    }

    FSlateApplication::Get().Tick();
}

FReply FChannelMixerUtils::ImportTextureFromCB(FChannelMixer* Mixer, const FString& ChannelName, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture)
{

    FContentBrowserModule* ContentBrowserModule = &FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TArray<FAssetData> SelectedAssets;
    ContentBrowserModule->Get().GetSelectedAssets(SelectedAssets);

    if (SelectedAssets.Num() == 1)
    {
        UObject* SelectedObject = SelectedAssets[0].GetAsset();
        if (UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedObject))
        {
            //This ensures texture is fully loaded before using it for the render target
            ForceTextureCompilation(SelectedTexture);


            CreateAndSetPreviewBrush(SelectedTexture, ChannelImage, ChannelTexture);
            SetTextureParameterValue(ChannelName, ChannelTexture, Mixer);
        }
        else
        {
            UEnchancedNotifications::LaunchNotification(TEXT("Please select a texture to import"));
        }
    }
    else
    {
        UEnchancedNotifications::LaunchNotification(TEXT("Please select a texture to import"));
    }

    UpdatePreviewTexture(Mixer);
    return FReply::Handled();
}

FReply FChannelMixerUtils::ExportTexture(FChannelMixer* Mixer)
{
    if (!Mixer || !Mixer->CombinedTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("ExportTexture: CombinedTexture is null"));
        return FReply::Handled();
    }

    FString PackageName = Mixer->BuildPackagePath();

    UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
        Mixer->CombinedTexture,
        PackageName,
        TextureCompressionSettings::TC_Masks,
        TextureMipGenSettings::TMGS_NoMipmaps
    );

    UEnchancedNotifications::OpenCBDirNotification(FString::Printf(TEXT("Successfully exported combined texture to /Content/%s"), *Mixer->ExportPath), FString::Printf(TEXT("/Game/%s"), *Mixer->ExportPath));

    return FReply::Handled();
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

FReply FChannelMixerUtils::RestoreSlotDefaultTexture(const FString& ChannelName, TSharedPtr<SImage> SlateImage, UTexture2D* Texture, FChannelMixer* Mixer)
{

    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TEXT("/UnrealToolsPlugin/MM/Textures/Checkers/T_Black"));;

    CreateAndSetPreviewBrush(Texture, SlateImage, &Texture);
    SetTextureParameterValue(ChannelName, &Texture, Mixer);

    UpdatePreviewTexture(Mixer);
    FSlateApplication::Get().Tick();

    return FReply::Handled();
}

void FChannelMixerUtils::CreateAndSetPreviewBrush(UTexture2D* NewTexture, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture)
{
    FSlateBrush* NewBrush = new FSlateBrush();
    NewBrush->SetResourceObject(NewTexture);
    ChannelImage->SetImage(NewBrush);
    *ChannelTexture = NewTexture;
}

void FChannelMixerUtils::SetTextureParameterValue(const FString& ChannelName, UTexture2D** ChannelTexture, FChannelMixer* Mixer)
{
    if (Mixer->BlendMaterial) Mixer->BlendMaterial->SetTextureParameterValue(FName(ChannelName), *ChannelTexture);
}







#pragma region General utilities
UTexture2D* FChannelMixerUtils::CreateMaskFromGrayscales(UTexture2D* RedChannel, UTexture2D* GreenChannel, UTexture2D* BlueChannel, UTexture2D* AlphaChannel, const int32& TargetResolution)
{

    UWorld* World = GEditor->GetEditorWorldContext().World();

    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/MaskTools/MM/MM_TextureMixer"));
    UMaterialInstanceDynamic* MaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);

    TArray<UTexture2D*> TexturesArray = { RedChannel, GreenChannel, BlueChannel, AlphaChannel };
    for (UTexture2D* Texture : TexturesArray)
    {
        ForceTextureCompilation(Texture);
    }

    MaterialInstance->SetTextureParameterValue(TEXT("Red"), RedChannel);
    MaterialInstance->SetTextureParameterValue(TEXT("Green"), GreenChannel);
    MaterialInstance->SetTextureParameterValue(TEXT("Blue"), BlueChannel);
    MaterialInstance->SetTextureParameterValue(TEXT("Alpha"), AlphaChannel);
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
    return UPackage::SavePackage(Package, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
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

void FChannelMixerUtils::ForceTextureCompilation(UTexture2D* Texture)
{
    FTextureCompilingManager::Get().FinishCompilation({ Texture });
    Texture->SetForceMipLevelsToBeResident(30.f);
    Texture->WaitForStreaming(true);
}
#pragma endregion


