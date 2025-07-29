// Copyright (c) 2025 Sora Mas
// All rights reserved.

#include "ChannelMixerUI.h"
#include "ChannelMixer.h"
#include "ChannelMixerUtils.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "MaskToolsConfig.h"
#include "Editor/ContentBrowser/Private/SContentBrowser.h"

#include "Widgets/SWindow.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"


void FChannelMixerUI::ShowTextureMixerWindow(FChannelMixer* Mixer)
{
    FGlobalTabmanager::Get()->TryInvokeTab(FTabId("TextureMixerTab"));
}

TSharedRef<SWidget> FChannelMixerUI::CreateMainLayout(FChannelMixer* Mixer)
{
    return SNew(SVerticalBox)
        // Grid with channel widgets.
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SUniformGridPanel)
                .SlotPadding(FMargin(5))
                + SUniformGridPanel::Slot(0, 0)[CreateChannelWidget("Red", EChannelMixerChannel::Red, Mixer->RedChannelSImage, Mixer)]
                + SUniformGridPanel::Slot(1, 0)[CreateChannelWidget("Green", EChannelMixerChannel::Green,Mixer->GreenChannelSImage, Mixer)]
                + SUniformGridPanel::Slot(2, 0)[CreateChannelWidget("Blue", EChannelMixerChannel::Blue,Mixer->BlueChannelSImage, Mixer)]
                + SUniformGridPanel::Slot(3, 0)[CreateChannelWidget("Alpha", EChannelMixerChannel::Alpha,Mixer->AlphaChannelSImage, Mixer)]
        ]
        // Preview image area.
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .VAlign(VAlign_Fill)
        .HAlign(HAlign_Fill)
        .Padding(10)
        [
            SNew(SScaleBox)
                .Stretch(EStretch::ScaleToFit)
                [
                    SNew(SBox)
                        .WidthOverride_Lambda([]() -> float {return FindDesiredSizeKeepRatio(); })
                        .HeightOverride_Lambda([]() -> float {return FindDesiredSizeKeepRatio(); })
                        [
                            SAssignNew(Mixer->PreviewSImage, SImage)
                        ]
                ]
        ]

        // Texture name config
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Prefix"), TEXT("Prefix the exported texture will use"), Mixer->PrefixHintText, Mixer->TexturePrefix, Mixer)]
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Name"), TEXT("Name the exported texture will use"), Mixer->NameHintText, Mixer->TextureName, Mixer)]
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Suffix"), TEXT("Suffix the exported texture will use"), Mixer->SuffixHintText, Mixer->TextureSuffix, Mixer)]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Export folder"), TEXT("Export path for the generated texture\nRelative to project content folder"), TEXT("GeneratedMasks"), Mixer->ExportPath, Mixer)]
                + SHorizontalBox::Slot()[CreateTexResSelectionComboBox(Mixer)]
        ]

        // Export button.
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
            [
                SNew(SButton)
                    .Text(FText::FromString(TEXT("Export")))
                    .OnClicked_Lambda([Mixer]() -> FReply
                    {
                        return Mixer->ExportTexture();
                    })
            ]
            + SHorizontalBox::Slot()
           .FillWidth(0.1f)
           [
               SNew(SButton)
               .OnClicked_Lambda([Mixer]() -> FReply
               {
                   return Mixer->ToggleContentBrowser();
               })
               [
                   SNew(SHorizontalBox)
                   + SHorizontalBox::Slot()
                   .AutoWidth()
                   .VAlign(VAlign_Center)
                   [
                       SNew(SImage)
                       .Image(FAppStyle::GetBrush("LevelEditor.OpenContentBrowser"))
                   ]
                   + SHorizontalBox::Slot()
                   .VAlign(VAlign_Center)
                   .Padding(FMargin(5, 0, 0, 0))
                   [
                       SNew(STextBlock)
                       .Text(FText::FromString(TEXT("Content Drawer")))
                   ]
               ]
           ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .MaxHeight(500)
        .Padding(4)
        [
         SAssignNew(Mixer->ContentBrowserBox, SBox)
         .Visibility(EVisibility::Collapsed)
         [
             CreateContentBrowser(Mixer)
         ]
 ];

}

TSharedRef<SWidget> FChannelMixerUI::CreateChannelWidget(const FString& ChannelName, EChannelMixerChannel Channel, TSharedPtr<SImage>& ChannelImage, FChannelMixer* Mixer)
{
    EChannelMixerTextureChannel newChannel;
    switch (Channel)
    {
    case EChannelMixerChannel::Red:
        newChannel = EChannelMixerTextureChannel::Red;
        break;
    case EChannelMixerChannel::Green:
        newChannel = EChannelMixerTextureChannel::Green;
        break;
    case EChannelMixerChannel::Blue:
        newChannel = EChannelMixerTextureChannel::Blue;
        break;
    case EChannelMixerChannel::Alpha:
        newChannel = EChannelMixerTextureChannel::Alpha;
        break;
    default:
        newChannel = EChannelMixerTextureChannel::Red;
    }
    
    ChannelImage = SNew(SImage);
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock).Text(FText::FromString(ChannelName + " Channel"))
        ]
        + SVerticalBox::Slot()
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .Padding(5)
        [
            SNew(SBox)
                .WidthOverride(100)
                .HeightOverride(100)
                [
                    ChannelImage.ToSharedRef()
                ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Import")))
                        .OnClicked_Lambda([Channel, Mixer]() -> FReply
                        {
                            return Mixer->TryImportTexture(Channel);
                        })

                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .ToolTipText(FText::FromString(TEXT("Reset to default")))
                        .OnClicked_Lambda([Channel, Mixer]() -> FReply
                            {
                                return Mixer->RestoreSlotDefaultTexture(Channel);
                            })
                        [
                            SNew(SImage).Image(FAppStyle::GetBrush("GenericCommands.Undo"))
                        ]

                ]
            +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .ToolTipText(FText::FromString(TEXT("Browse to asset")))
                        .OnClicked_Lambda([Channel, Mixer]() -> FReply
                            {
                                return Mixer->BrowseToAsset(Channel);
                            })
                        [
                            SNew(SImage).Image(FAppStyle::GetBrush("LevelEditor.OpenContentBrowser"))
                        ]

                ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            CreateTextureChannelSelectionComboBox(Mixer, newChannel)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            CreateResizeMethodSelectionComboBox(Mixer, newChannel)
        ];
}

float FChannelMixerUI::FindDesiredSizeKeepRatio()
{
    TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr);
    if (ActiveWindow.IsValid())
    {
        FVector2D WindowSize = ActiveWindow->GetClientSizeInScreen();
        return FMath::Min(WindowSize.X, WindowSize.Y) * 0.8f;
    }
    return 300.0f;
}

TSharedRef<SWidget> FChannelMixerUI::CreateNameConfigWidget(const FString& Name, const FString& ToolTip, const FString& HintText, FString& ChangedText, FChannelMixer* Mixer)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Black)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock).Text(FText::FromString(Name))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
                        .HeightOverride(30)
                        [
                            SNew(SEditableTextBox)
                                .BackgroundColor(FLinearColor(10,10,10))
                                .HintText(FText::FromString(HintText))
                                .ToolTipText(FText::FromString(ToolTip))
                                .OnTextCommitted_Lambda([&ChangedText, Mixer](const FText& ComitedText, ETextCommit::Type) -> void
                                    {
                                        ChangedText = ComitedText.ToString();
                                    })
                        ]

                ]
        ];
}

TSharedRef<SWidget> FChannelMixerUI::CreateTexResSelectionComboBox(FChannelMixer* Mixer)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Black)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock).Text(FText::FromString(TEXT("Texture Resolution")))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
                        .HeightOverride(30)
                        [
                            SNew(STexResComboBox)
                                .ToolTipText(FText::FromString(TEXT("Resolution assigned to the final export texture")))
                                .Mixer(Mixer)
                        ]

                ]
        ];
}

TSharedRef<SWidget> FChannelMixerUI::CreateTextureChannelSelectionComboBox(FChannelMixer* Mixer, EChannelMixerTextureChannel Channel)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Black)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
                        .HeightOverride(30)
                        [
                            SNew(SChannelSelectionComboBox)
                                .ToolTipText(FText::FromString(TEXT("Selected channel")))
                                .Mixer(Mixer)
                                .Channel(Channel)
                        ]

                ]
        ];
}

TSharedRef<SWidget> FChannelMixerUI::CreateResizeMethodSelectionComboBox(FChannelMixer* Mixer, EChannelMixerTextureChannel Channel)
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Black)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBox)
                        .HeightOverride(30)
                        [
                            SNew(SResizeAlgorithmComboBox)
                                .ToolTipText(FText::FromString(TEXT("Selected resize method")))
                                .Mixer(Mixer)
                                .Channel(Channel)
                        ]

                ]
        ];
}

TSharedRef<SWidget> FChannelMixerUI::CreateContentBrowser(FChannelMixer* Mixer)
{

    FARFilter Filter;
    Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
    
    FAssetPickerConfig AssetPickerConfig;
    AssetPickerConfig.bCanShowFolders = true;
    AssetPickerConfig.Filter = Filter;
    AssetPickerConfig.bCanShowClasses = false;
    AssetPickerConfig.bForceShowEngineContent = false;
    AssetPickerConfig.bShowPathInColumnView = true;
    AssetPickerConfig.bSortByPathInColumnView = true;
    
    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda(
        [Mixer](const FAssetData& SelectedAsset)
        {
            Mixer->SetSelectedAsset(SelectedAsset);
        });
    
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TSharedRef<SWidget> FullContentBrowser = ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);
    
    return FullContentBrowser;
}

void STexResComboBox::Construct(const FArguments& InArgs)
{

    Mixer = InArgs._Mixer;

    ComboBoxOptions.Add(MakeShared<FString>(TEXT("32")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("64")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("128")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("256")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("512")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("1024")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("2048")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("4096")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("8192")));

    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
    int32 EnumIndex = StaticEnum<EMaskResolutions>()->GetIndexByValue(static_cast<int64>(Config->DefaultMaskResolution));
    SelectedOption = ComboBoxOptions[EnumIndex];

    ChildSlot
        [
            SNew(SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&ComboBoxOptions)
                .OnSelectionChanged(this, &STexResComboBox::OnSelectionChanged)
                .OnGenerateWidget(this, &STexResComboBox::MakeWidgetForOption)
                .InitiallySelectedItem(SelectedOption)
                .Content()
                [SNew(STextBlock).Text(this, &STexResComboBox::GetComboBoxSelection)]
        ];
    
}

void STexResComboBox::OnSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!Mixer)
    {
        UE_LOG(LogChannelMixer, Warning, TEXT("Assign mixer reference to texRes combo box"));
        return;
    }

    SelectedOption = NewSelection;
    int32 newSize = FChannelMixerUtils::ResFinder(*NewSelection.Get());
    Mixer->TextureResolution = newSize;

    Mixer->RegeneratePreviewTexture();
}

TSharedRef<SWidget> STexResComboBox::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText STexResComboBox::GetComboBoxSelection() const
{
    return FText::FromString(*SelectedOption);
}

void SChannelSelectionComboBox::Construct(const FArguments& InArgs)
{
    Mixer = InArgs._Mixer;
    Channel = InArgs._Channel;

    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Red")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Green")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Blue")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Alpha")));

    SelectedOption = ComboBoxOptions[0];

    ChildSlot
        [
            SNew(SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&ComboBoxOptions)
                .OnSelectionChanged(this, &SChannelSelectionComboBox::OnSelectionChanged)
                .OnGenerateWidget(this, &SChannelSelectionComboBox::MakeWidgetForOption)
                .InitiallySelectedItem(SelectedOption)
                .Content()
                [SNew(STextBlock).Text(this, &SChannelSelectionComboBox::GetComboBoxSelection)]
        ];
}

void SChannelSelectionComboBox::OnSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!Mixer)
    {
        UE_LOG(LogChannelMixer, Warning, TEXT("Assign mixer reference to texture channel combo box"));
        return;
    }

    SelectedOption = NewSelection;
    EChannelMixerTextureChannel newChannel = FChannelMixerUtils::ChannelFinder(*NewSelection.Get());
    switch (Channel)
    {
        case EChannelMixerTextureChannel::Red:
            Mixer->RedTextureSelectedChannel = newChannel;
            break;
        case EChannelMixerTextureChannel::Green:
            Mixer->GreenTextureSelectedChannel = newChannel;
            break;
        case EChannelMixerTextureChannel::Blue:
            Mixer->BlueTextureSelectedChannel = newChannel;
            break;
        case EChannelMixerTextureChannel::Alpha:
            Mixer->AlphaTextureSelectedChannel = newChannel;
            break;
        default:
            UE_LOG(LogChannelMixer, Warning, TEXT("No channel assigned to channel combo box"))
    }
    
    Mixer->RegeneratePreviewTexture();
}

TSharedRef<SWidget> SChannelSelectionComboBox::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText SChannelSelectionComboBox::GetComboBoxSelection() const
{
    return FText::FromString(*SelectedOption);
}

void SResizeAlgorithmComboBox::Construct(const FArguments& InArgs)
{
    Mixer = InArgs._Mixer;
    Channel = InArgs._Channel;

    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Default")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("PointSample")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Box")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Triangle")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("Bilinear")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("CubicGaussian")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("CubicSharp")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("CubicMitchell")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("AdaptiveSharp")));
    ComboBoxOptions.Add(MakeShared<FString>(TEXT("AdaptiveSmooth")));

    
    const UMaskToolsConfig* Config = GetDefault<UMaskToolsConfig>();
    int32 EnumIndex = StaticEnum<EResizeMethod>()->GetIndexByValue(static_cast<int64>(Config->DefaultMaskResolution));
    SelectedOption = ComboBoxOptions[EnumIndex];

    ChildSlot
        [
            SNew(SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&ComboBoxOptions)
                .OnSelectionChanged(this, &SResizeAlgorithmComboBox::OnSelectionChanged)
                .OnGenerateWidget(this, &SResizeAlgorithmComboBox::MakeWidgetForOption)
                .InitiallySelectedItem(SelectedOption)
                .Content()
                [SNew(STextBlock).Text(this, &SResizeAlgorithmComboBox::GetComboBoxSelection)]
        ];
}

void SResizeAlgorithmComboBox::OnSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (!Mixer)
    {
        UE_LOG(LogChannelMixer, Warning, TEXT("Assign mixer reference to texture channel combo box"));
        return;
    }

    SelectedOption = NewSelection;
    EResizeMethod newResizeMethod = FChannelMixerUtils::ResizeFilterFinder(*NewSelection);
    switch (Channel)
    {
    case EChannelMixerTextureChannel::Red:
        Mixer->RedResizeMethod = newResizeMethod;
        break;
    case EChannelMixerTextureChannel::Green:
        Mixer->GreenResizeMethod = newResizeMethod;
        break;
    case EChannelMixerTextureChannel::Blue:
        Mixer->BlueResizeMethod = newResizeMethod;
        break;
    case EChannelMixerTextureChannel::Alpha:
        Mixer->AlphaResizeMethod = newResizeMethod;
        break;
    default:
        UE_LOG(LogChannelMixer, Warning, TEXT("No channel assigned to resize method combo box"))
    }
    
    Mixer->RegeneratePreviewTexture();
}

TSharedRef<SWidget> SResizeAlgorithmComboBox::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText SResizeAlgorithmComboBox::GetComboBoxSelection() const
{
    return FText::FromString(*SelectedOption);
}

    
