#include "ChannelMixerUtils.h"
#include "ChannelMixer.h"
#include "Engine/Texture2D.h"
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

#include "TextureCompiler.h"


FReply FChannelMixerUtils::ExportTexture(FChannelMixer* Mixer)
{
    if (!Mixer || !Mixer->CombinedTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("ExportTexture: CombinedTexture is null"));
        return FReply::Handled();
    }

    FString PackageName = BuildPackagePath(Mixer);

    UTexture2D* ExportedTexture = UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
        Mixer->CombinedTexture,
        PackageName,
        TextureCompressionSettings::TC_Masks,
        TextureMipGenSettings::TMGS_NoMipmaps
    );


    UEnchancedNotifications::OpenCBDirNotification(FString::Printf(TEXT("Successfully exported combined texture to /Content/%s"), *Mixer->ExportPath), FString::Printf(TEXT("/Game/%s"), *Mixer->ExportPath));

    return FReply::Handled();
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
            FTextureCompilingManager::Get().FinishCompilation({ SelectedTexture });
            SelectedTexture->SetForceMipLevelsToBeResident(30.f);
            SelectedTexture->WaitForStreaming(true);

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

FReply FChannelMixerUtils::RestoreSlotDefaultTexture(const FString& ChannelName, FChannelMixer* Mixer)
{

    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TEXT("/UnrealToolsPlugin/MM/Textures/Checkers/T_Black"));;

    if (ChannelName == TEXT("Red"))
    {
        CreateAndSetPreviewBrush(Texture, Mixer->RedChannelImage, &Mixer->RedTexture);
        SetTextureParameterValue(ChannelName, &Mixer->RedTexture, Mixer);
    }
    if (ChannelName == TEXT("Green"))
    {
        CreateAndSetPreviewBrush(Texture, Mixer->GreenChannelImage, &Mixer->GreenTexture);
        SetTextureParameterValue(ChannelName, &Mixer->GreenTexture, Mixer);
    }
    if (ChannelName == TEXT("Blue"))
    {
        CreateAndSetPreviewBrush(Texture, Mixer->BlueChannelImage, &Mixer->BlueTexture);
        SetTextureParameterValue(ChannelName, &Mixer->BlueTexture, Mixer);
    }
    if (ChannelName == TEXT("Alpha"))
    {
        CreateAndSetPreviewBrush(Texture, Mixer->AlphaChannelImage, &Mixer->AlphaTexture);
        SetTextureParameterValue(ChannelName, &Mixer->AlphaTexture, Mixer);
    }

    FChannelMixerUtils::UpdatePreviewTexture(Mixer);
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
    if (Mixer->BlendMaterial)
    {
        Mixer->BlendMaterial->SetTextureParameterValue(FName(ChannelName), *ChannelTexture);
    }
}

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
    if (Mixer->PreviewImage.IsValid())
    {
        Mixer->PreviewImage->SetImage(Mixer->PreviewBrush.Get());
    }

    FSlateApplication::Get().Tick();
}

FString FChannelMixerUtils::BuildPackagePath(FChannelMixer* Mixer)
{
    if (!Mixer) return FString();

    FString tempPrefix = Mixer->GetTexturePrefix().IsEmpty() ? TEXT("") : FString::Printf(TEXT("%s_"), *Mixer->GetTexturePrefix());
    // This should be overrided with a settings default name
    FString tempName = Mixer->GetTextureName().IsEmpty() ? TEXT("GeneratedMask") : Mixer->GetTextureName();
    FString tempSuffix = Mixer->GetTextureSuffix().IsEmpty() ? TEXT("") : FString::Printf(TEXT("_%s"), *Mixer->GetTextureSuffix());
    FString AssetName = FString::Printf(TEXT("%s%s%s"), *tempPrefix, *tempName, *tempSuffix);
    FString PackageName = FString::Printf(TEXT("/Game/%s/%s"), *Mixer->GetExportPath(), *AssetName);
    return PackageName;
}
