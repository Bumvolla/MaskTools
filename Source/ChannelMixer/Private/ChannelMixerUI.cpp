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
                .Text(FText::FromString(TEXT("Content Drawer")))
                .OnClicked_Lambda([Mixer]() -> FReply
                {
                    return Mixer->ToggleContentBrowser();
                })
            ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .MaxHeight(500)
        .Padding(4)
        [
         SAssignNew(Mixer->ContentBrowserBox, SBox)
         .Visibility(EVisibility::Visible)
         [
             CreateContentBrowser(Mixer)
         ]
 ];

}

TSharedRef<SWidget> FChannelMixerUI::CreateChannelWidget(const FString& ChannelName, EChannelMixerChannel Channel, TSharedPtr<SImage>& ChannelImage, FChannelMixer* Mixer)
{
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
                            return Mixer->ImportTextureFromCB(Channel);
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

TSharedRef<SWidget> FChannelMixerUI::CreateContentBrowser(FChannelMixer* Mixer)
{

    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    
    static int32 BrowserInstanceId = 1;
    FName BrowserInstanceName = *FString::Printf(TEXT("ChannelMixerContentBrowser_%d"), BrowserInstanceId++);

    FContentBrowserConfig ContentBrowserConfig;
    ContentBrowserConfig.bCanShowFilters = true;
    ContentBrowserConfig.bCanShowClasses = false;
    
    TSharedRef<SWidget> FullContentBrowser = ContentBrowserModule.Get().CreateContentBrowser(
        BrowserInstanceName,
        nullptr,
        &ContentBrowserConfig
    );
    
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

    
