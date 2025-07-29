// Copyright (c) 2025 Sora Mas
// All rights reserved.

#include "ChannelMixer.h"

#include "ChannelMixerUI.h"
#include "ChannelMixerUtils.h"
#include "MaskToolsUtils.h"

#include "ChannelMixerEnums.h"

#include "Modules/ModuleManager.h"
#include "LevelEditor.h"

#include "ChannelMixerStyle.h"
#include "ContentBrowserModule.h"
#include "EnchancedNotifications.h"
#include "IContentBrowserSingleton.h"
#include "ImageUtils.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "MaskTools/Public/MaskToolsConfig.h"


#define LOCTEXT_NAMESPACE "FChannelMixer"
#pragma region Module
void FChannelMixer::StartupModule()
{
    InitToolsMenuExtension();

    ChannelMixerStyle::InitializeIcons();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        "TextureMixerTab",
        FOnSpawnTab::CreateRaw(this, &FChannelMixer::SpawnTextureMixerTab))
        .SetDisplayName(NSLOCTEXT("ChannelMixer", "Channel Mixer", "Channel Mixer"))
        .SetMenuType(ETabSpawnerMenuType::Hidden)
        .SetIcon(FSlateIcon(ChannelMixerStyle::GetStyleSetName(), "Tools.MixChannels"));
}

void FChannelMixer::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("TextureMixerTab");
    
    ChannelMixerStyle::ShutDown();
}

#pragma endregion

#pragma region MenuExtension

void FChannelMixer::SetSelectedAsset(const FAssetData& NewSelectedAsset)
{
    SelectedAsset = NewSelectedAsset;
}

void FChannelMixer::InitToolsMenuExtension()
{

    TSharedRef<FExtender> MenuExtender(new FExtender());

    MenuExtender->AddMenuExtension(
        "Tools",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateRaw(this, &FChannelMixer::AddToolsMenuEntry)
    );

    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FChannelMixer::AddToolsMenuEntry(FMenuBuilder& MenuBuilder)
{

    MenuBuilder.AddMenuEntry(
        FText::FromString(TEXT("Texture Mixer")),
        FText::FromString(TEXT("Open the texture mixer window")),
        FSlateIcon(ChannelMixerStyle::GetStyleSetName(), "Tools.MixChannels"),
        FUIAction(FExecuteAction::CreateRaw(this, &FChannelMixer::OpenTextureMixerWindow))
    );
}

void FChannelMixer::InitializeData()
{
    // Retrieve config from project settings
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();

    ExportPath = Config->DefaultMaskSavePath.Path;
    TextureName = Config->DefaultMaskName;

    TexturePrefix = Config->DefaultMaskPrefix;
    TextureName = Config->DefaultMaskName;
    TextureSuffix = Config->DefaultMaskSuffix;

    PrefixHintText = Config->DefaultMaskPrefix;
    NameHintText = TextureName;
    SuffixHintText = Config->DefaultMaskSuffix;

    const UEnum* EnumPtr = StaticEnum<EMaskResolutions>();
    FText defaultResString = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Config->DefaultMaskResolution));
    TextureResolution = FChannelMixerUtils::ResFinder(defaultResString.ToString());
    
    RedBrush = MakeShared<FSlateBrush>();
    GreenBrush = MakeShared<FSlateBrush>();
    BlueBrush = MakeShared<FSlateBrush>();
    AlphaBrush = MakeShared<FSlateBrush>();
    PreviewBrush = MakeShared<FSlateBrush>();
    
    // Create default values
    FallbackTexture = FChannelMixerUtils::CreateFallbackTexture();
    FallbackTexture->AddToRoot();

    RedTexture = FallbackTexture;
    GreenTexture = FallbackTexture;
    BlueTexture = FallbackTexture;
    AlphaTexture = FallbackTexture;
}

void FChannelMixer::OpenTextureMixerWindow()
{

    FChannelMixerUI::ShowTextureMixerWindow(this);

}

TSharedRef<SDockTab> FChannelMixer::SpawnTextureMixerTab(const FSpawnTabArgs& Args)
{
    InitializeData();
    
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        .Label(NSLOCTEXT("ChannelMixer", "TextureMixerTabTitle", "Texture Mixer"))
        [
            FChannelMixerUI::CreateMainLayout(this)
        ];
}

FString FChannelMixer::BuildPackagePath()
{

    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();

    FString tempPrefix, tempName, tempSuffix;

    if (TexturePrefix.IsEmpty())
    {
        if (Config->bDefaultAddPrefix) tempPrefix = FString::Printf(TEXT("%s_"), *Config->DefaultMaskPrefix);
        else tempPrefix = TEXT("");
    }
    else 
        tempPrefix = FString::Printf(TEXT("%s_"), *TexturePrefix);

    tempName = TextureName.IsEmpty() ? Config->DefaultMaskName : TextureName;

    if (TextureSuffix.IsEmpty())
    {
        if (Config->bDefaultAddSuffix) tempSuffix = FString::Printf(TEXT("_%s"), *Config->DefaultMaskSuffix);
        else tempSuffix = TEXT("");
    }
    else 
        tempSuffix = FString::Printf(TEXT("_%s"), *TextureSuffix);

    FString AssetName = FString::Printf(TEXT("%s%s%s"), *tempPrefix, *tempName, *tempSuffix);
    FString PackageName = FString::Printf(TEXT("/Game/%s/%s"), *ExportPath, *AssetName);
    return PackageName;

}
#pragma endregion

#pragma region Channel Mixer Logic
 
FReply FChannelMixer::TryImportTexture(EChannelMixerChannel Channel)
{
    if (ImportTextureFromAssetPicker(Channel))
    {
        RegeneratePreviewTexture();
        return FReply::Handled();
    }

    if (ImportTextureFromCB(Channel))
    {
        RegeneratePreviewTexture();
        return FReply::Handled();
    }

    UEnchancedNotifications::LaunchNotification(TEXT("Please select a texture to import"));
    return FReply::Handled();
}

bool FChannelMixer::ImportTextureFromCB(EChannelMixerChannel Channel)
{

    TArray<FAssetData> AssetData;
    TArray<UTexture2D*> SelectedTextures = FMaskToolsUtils::SyncronousLoadCBTextures(AssetData);
    if (SelectedTextures.Num() == 1)
    {
        if (IsValid(SelectedTextures[0]) && SelectedTextures[0] != GetChannelTexture(Channel))
        {
            UTexture2D* SelectedTexture = SelectedTextures[0];   
            SetNewChannelTexture(SelectedTexture, Channel);
            SetChannelAssetData(AssetData[0], Channel);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        UEnchancedNotifications::LaunchNotification(TEXT("Please select a single texture to import"));
        return false;
    }
}

bool FChannelMixer::ImportTextureFromAssetPicker(EChannelMixerChannel Channel)
{
    if (!SelectedAsset.IsValid())
    {
        return false;
    }
    
    UTexture2D* SelectedTexture = FMaskToolsUtils::LoadTextureFromAssetData(SelectedAsset);
    if (IsValid(SelectedTexture))
    {
        SetNewChannelTexture(SelectedTexture, Channel);
        SetChannelAssetData(SelectedAsset, Channel);
        return true;
    }

    return false;
}

void FChannelMixer::RegeneratePreviewTexture()
{
    // const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
    // switch (Config->MixerCreationMethod)
    EMaskCreationMethod TempFixedMethod = EMaskCreationMethod::PixelData;
    switch (TempFixedMethod)
    {
        case EMaskCreationMethod::PixelData:
            RegeneratePreviewTexturePixelData();
            break;

        case EMaskCreationMethod::Material:
            RegeneratePreviewTextureMaterial();
            break;
        
        default:
            RegeneratePreviewTexturePixelData();
    }
    
}

void FChannelMixer::RegeneratePreviewTexturePixelData()
{
    TArray<FColor> RChannel, GChannel, BChannel, AChannel;
    TArray<TArray<FColor>> ChannelData;

    auto GetTextureChannelData = [this] (UTexture2D* Texture, EChannelMixerTextureChannel SelectedChannel, EResizeMethod SelectedResizeMethod,  TArray<FColor>& ChannelData)
    {
        TArray<FLinearColor> TexturePixelValues;

        EResizeMethod ResizeMethod;
        if (SelectedResizeMethod == EResizeMethod::Default)
        {
            const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
            ResizeMethod = Config->MixerResizeMethod;
        }
        else
        {
            ResizeMethod = SelectedResizeMethod;
        }
        
        if (!FMaskToolsUtils::GetTexturePixelData(Texture, TextureResolution,  ResizeMethod, TexturePixelValues))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to get texture pixel data"));
            return;
        }
        for (FLinearColor LinearPixelData : TexturePixelValues)
        {
            FColor Color = LinearPixelData.ToFColor(true);
            switch (SelectedChannel)
            {
            case EChannelMixerTextureChannel::Red:
                ChannelData.Add(FColor(Color.R, 0 ,0 ,0));
                break;
            case EChannelMixerTextureChannel::Green:
                ChannelData.Add(FColor(Color.G, 0 ,0 ,0));
                break;
            case EChannelMixerTextureChannel::Blue:
                ChannelData.Add(FColor(Color.B, 0 ,0 ,0));
                break;
            case EChannelMixerTextureChannel::Alpha:
                ChannelData.Add(FColor(Color.A, 0 ,0 ,0));
                break;
            }
        }
    };
    
    GetTextureChannelData(RedTexture, RedTextureSelectedChannel, RedResizeMethod, RChannel);
    GetTextureChannelData(GreenTexture, GreenTextureSelectedChannel, GreenResizeMethod, GChannel);
    GetTextureChannelData(BlueTexture, BlueTextureSelectedChannel, BlueResizeMethod, BChannel);
    GetTextureChannelData(AlphaTexture, AlphaTextureSelectedChannel, AlphaResizeMethod, AChannel);
    
    ChannelData.Add(RChannel);
    ChannelData.Add(GChannel);
    ChannelData.Add(BChannel);
    ChannelData.Add(AChannel);

    int32 PixelCount = TextureResolution*TextureResolution;

    TArray<FColor> FinalTextureData;
    FinalTextureData.Reserve(PixelCount);

    for (int32 i = 0; i < PixelCount; ++i)
    {
        uint8 R = RChannel.IsValidIndex(i) ? RChannel[i].R : 0;
        uint8 G = GChannel.IsValidIndex(i) ? GChannel[i].R : 0;
        uint8 B = BChannel.IsValidIndex(i) ? BChannel[i].R : 0;
        uint8 A = AChannel.IsValidIndex(i) ? AChannel[i].R : 255;

        FinalTextureData.Add(FColor(R, G, B, A));
    }

    FCreateTexture2DParameters TextureParameters;
    TextureParameters.TextureGroup = TextureGroup::TEXTUREGROUP_World;
    TextureParameters.bSRGB = false;
    TextureParameters.CompressionSettings = TC_Masks;
    TextureParameters.bUseAlpha = true;
    TextureParameters.bDeferCompression = true;
    TextureParameters.bVirtualTexture = false;

    UWorld* World = GEditor->GetEditorWorldContext().World();
    UTexture2D* NewTexture = FImageUtils::CreateTexture2D(TextureResolution, TextureResolution, FinalTextureData, World, TEXT(""), EObjectFlags::RF_KeepForCooker, TextureParameters);
    PreviewTexture = NewTexture;
    FMaskToolsUtils::ForceTextureCompilation(PreviewTexture);
    UpdateSlateChannel(EChannelMixerChannel::Result);
}

void FChannelMixer::RegeneratePreviewTextureMaterial()
{
    
    UWorld* World = GEditor->GetEditorWorldContext().World();
    UMaterialInterface* BaseMaterial = FMaskToolsPrivateHelpers::LoadPluginMaterial(TEXT("MM_TextureMixer"));
    if (BaseMaterial == nullptr)
    {
        UE_LOG(LogChannelMixer, Error, TEXT("Failed to load texture mixer material, make sure you got the necessary content"));
        return;
    }
    
    UMaterialInstanceDynamic* BlendMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(World, BaseMaterial);
    UTextureRenderTarget2D* CombinedTexture = UKismetRenderingLibrary::CreateRenderTarget2D(World, TextureResolution, TextureResolution);

    PreviewBrush = MakeShared<FSlateBrush>();
    CombinedTexture->AddToRoot();
    PreviewBrush->SetResourceObject(CombinedTexture);

    if (RedTexture) RedTexture->UpdateResource();
    if (GreenTexture) GreenTexture->UpdateResource();  
    if (BlueTexture) BlueTexture->UpdateResource();
    if (AlphaTexture) AlphaTexture->UpdateResource();

    BlendMaterial->SetTextureParameterValue(FName("Red"), RedTexture);
    BlendMaterial->SetTextureParameterValue(FName("Green"), GreenTexture);
    BlendMaterial->SetTextureParameterValue(FName("Blue"), BlueTexture);
    BlendMaterial->SetTextureParameterValue(FName("Alpha"), AlphaTexture);

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(World, CombinedTexture, BlendMaterial);
    ETextureSourceFormat SourceFormat;
    EPixelFormat PixelFormat;
    FText* ErrorMessage = nullptr;
    if (!CombinedTexture->CanConvertToTexture(SourceFormat, PixelFormat, ErrorMessage))
    {
        FString Error = *ErrorMessage->ToString();
        UE_LOG(LogChannelMixer, Error, TEXT("Can't convert render target to texture, error: %s"), *Error);
        return;
    }
    PreviewTexture = CombinedTexture->ConstructTexture2D(World, TextureName, RF_Public | RF_Standalone);
    UpdateSlateChannel(EChannelMixerChannel::Result);
    
    
}

void FChannelMixer::UpdateSlateChannel(EChannelMixerChannel Channel)
{
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        RedBrush->SetResourceObject(RedTexture);
        RedChannelSImage->SetImage(RedBrush.Get());
        break;
    case EChannelMixerChannel::Green:
        GreenBrush->SetResourceObject(GreenTexture);
        GreenChannelSImage->SetImage(GreenBrush.Get());
        break;
    case EChannelMixerChannel::Blue:
        BlueBrush->SetResourceObject(BlueTexture);
        BlueChannelSImage->SetImage(BlueBrush.Get());
        break;
    case EChannelMixerChannel::Alpha:
        AlphaBrush->SetResourceObject(AlphaTexture);
        AlphaChannelSImage->SetImage(AlphaBrush.Get());
        break;
    case EChannelMixerChannel::Result:
        PreviewBrush->SetResourceObject(PreviewTexture);
        PreviewSImage->SetImage(PreviewBrush.Get());
        break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected slate channel to update"))
    }
    
    FSlateApplication::Get().Tick();
    
}

void FChannelMixer::SetChannelAssetData(const FAssetData& NewAssetData, EChannelMixerChannel Channel)
{
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        RedAssetData = NewAssetData;
        break;
    case EChannelMixerChannel::Green:
        GreenAssetData = NewAssetData;
        break;
    case EChannelMixerChannel::Blue:
        BlueAssetData = NewAssetData;
        break;
    case EChannelMixerChannel::Alpha:
        AlphaAssetData = NewAssetData;
        break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected slate channel to update"))
    }
}


FReply FChannelMixer::ToggleContentBrowser(EChannelMixerCBAction Action)
{
    if (!ContentBrowserBox.IsValid())
    {
        return FReply::Handled();
    }

    const EVisibility CurrentVisibility = ContentBrowserBox->GetVisibility();
    const bool bIsVisible = (CurrentVisibility == EVisibility::Visible);

    switch (Action)
    {
        case EChannelMixerCBAction::Open:
            if (bIsVisible)
                break;
            else
            {
                ContentBrowserBox->SetVisibility(EVisibility::Visible);
                break;
            }
        case EChannelMixerCBAction::Close:
            if (bIsVisible)
            {
                ContentBrowserBox->SetVisibility(EVisibility::Collapsed);
                break;
            }
            else
            {
                break;
            }
        case EChannelMixerCBAction::Default:
            ContentBrowserBox->SetVisibility(bIsVisible ? EVisibility::Collapsed : EVisibility::Visible);
            break;
        
        default:
        ContentBrowserBox->SetVisibility(bIsVisible ? EVisibility::Collapsed : EVisibility::Visible);
        
    }
    return FReply::Handled();
}

UTexture2D* FChannelMixer::GetChannelTexture(EChannelMixerChannel Channel)
{
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        return RedTexture;
        
    case EChannelMixerChannel::Green:
        return GreenTexture;
        
    case EChannelMixerChannel::Blue:
        return BlueTexture;
        
    case EChannelMixerChannel::Alpha:
        return AlphaTexture;
        
    case EChannelMixerChannel::Result:
        return PreviewTexture;
        
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected slate channel to update"))
        return nullptr;
    }
}

FReply FChannelMixer::ExportTexture()
{
    if (!PreviewTexture)
    {
        return FReply::Handled();
    }

    FString PackageName = BuildPackagePath();

    UTexture2D* SavedTexture = FMaskToolsUtils::CreateStaticTextureEditorOnly(PreviewTexture, PackageName, TC_Masks, TMGS_FromTextureGroup);
    SavedTexture->MarkPackageDirty();

    UEnchancedNotifications::OpenCBDirNotification(FString::Printf(TEXT("Successfully exported combined texture to /Content/%s"), *ExportPath), FString::Printf(TEXT("/Game/%s"), *ExportPath));

    return FReply::Handled();
}

FReply FChannelMixer::RestoreSlotDefaultTexture(EChannelMixerChannel Channel)
{
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        RedTexture = FallbackTexture;
        break;
    case EChannelMixerChannel::Green:
        GreenTexture = FallbackTexture;
        break;
    case EChannelMixerChannel::Blue:
        BlueTexture = FallbackTexture;
        break;
    case EChannelMixerChannel::Alpha:
        AlphaTexture = FallbackTexture;
        break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected channel to update"))
    }
    
    RegeneratePreviewTexture();
    UpdateSlateChannel(Channel);
    FSlateApplication::Get().Tick();
    return FReply::Handled();
}

void FChannelMixer::SetNewChannelTexture(UTexture2D* NewTexture, EChannelMixerChannel Channel)
{
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        RedTexture = NewTexture;
        break;
    case EChannelMixerChannel::Green:
        GreenTexture = NewTexture;
        break;
    case EChannelMixerChannel::Blue:
        BlueTexture = NewTexture;
        break;
    case EChannelMixerChannel::Alpha:
        AlphaTexture = NewTexture;
        break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected channel to update"))
    }
    UpdateSlateChannel(Channel);
}

FReply FChannelMixer::BrowseToAsset(EChannelMixerChannel Channel)
{
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    ContentBrowserModule.Get().FocusPrimaryContentBrowser(false);
    FAssetData AssetToFocus;
    
    switch (Channel)
    {
        case EChannelMixerChannel::Red:
            AssetToFocus = RedAssetData;
            break;
        case EChannelMixerChannel::Green:
            AssetToFocus = GreenAssetData;
            break;
        case EChannelMixerChannel::Blue:
            AssetToFocus = BlueAssetData;
            break;
        case EChannelMixerChannel::Alpha:
            AssetToFocus = AlphaAssetData;
            break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No selected channel to update"))
    }
    
    ContentBrowserModule.Get().SyncBrowserToAssets({AssetToFocus});
    return FReply::Handled();
}

#pragma endregion // Close Channel Mixer Logic region
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FChannelMixer, ChannelMixer)