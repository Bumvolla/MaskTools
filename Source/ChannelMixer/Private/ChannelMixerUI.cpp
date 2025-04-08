#include "ChannelMixerUI.h"
#include "ChannelMixer.h"
#include "ChannelMixerUtils.h"
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
    TSharedRef<SWindow> TextureMixerWindow = BuildMainWindow();
    TextureMixerWindow->SetContent(CreateMainLayout(Mixer));
    FSlateApplication::Get().AddWindow(TextureMixerWindow);
}

TSharedRef<SWindow> FChannelMixerUI::BuildMainWindow()
{
    return SNew(SWindow)
        .Title(FText::FromString("Texture Mixer"))
        .ClientSize(FVector2D(1000, 800))
        .SupportsMinimize(true)
        .SupportsMaximize(false)
        .IsTopmostWindow(true);
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
                + SUniformGridPanel::Slot(0, 0)[CreateChannelWidget("Red", Mixer->RedChannelSImage, &Mixer->RedTexture, Mixer)]
                + SUniformGridPanel::Slot(1, 0)[CreateChannelWidget("Green", Mixer->GreenChannelSImage, &Mixer->GreenTexture, Mixer)]
                + SUniformGridPanel::Slot(2, 0)[CreateChannelWidget("Blue", Mixer->BlueChannelSImage, &Mixer->BlueTexture, Mixer)]
                + SUniformGridPanel::Slot(3, 0)[CreateChannelWidget("Alpha", Mixer->AlphaChannelSImage, &Mixer->AlphaTexture, Mixer)]
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
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Prefix"), TEXT("Prefix the exported texture will use"), TEXT("T"), Mixer->TexturePrefix, Mixer)]
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Name"), TEXT("Name the exported texture will use"), TEXT("GeneratedMask"), Mixer->TextureName, Mixer)]
                + SHorizontalBox::Slot()[CreateNameConfigWidget(TEXT("Texture Suffix"), TEXT("Suffix the exported texture will use"), TEXT("Mask"), Mixer->TextureSuffix, Mixer)]
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
            SNew(SButton)
                .Text(FText::FromString(TEXT("Export")))
                .OnClicked_Lambda([Mixer]() -> FReply
                {
                    return FChannelMixerUtils::ExportTexture(Mixer);
                })
        ];

}

TSharedRef<SWidget> FChannelMixerUI::CreateChannelWidget(const FString& ChannelName, TSharedPtr<SImage>& ChannelImage, UTexture2D** ChannelTexture, FChannelMixer* Mixer)
{
    // Create the image widget.
    ChannelImage = SNew(SImage);
    return SNew(SVerticalBox)
        // Label for the channel.
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock).Text(FText::FromString(ChannelName + " Channel"))
        ]
        // Box to display the channel image.
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

        // Import button that calls the utility function.
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Import")))
                        .OnClicked_Lambda([ChannelName, &ChannelImage, ChannelTexture, Mixer]() -> FReply
                        {
                            return FChannelMixerUtils::ImportTextureFromCB(Mixer, ChannelName, ChannelImage, ChannelTexture);
                        })

                ]
                +SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                        .ToolTipText(FText::FromString(TEXT("Reset to default")))
                        .OnClicked_Lambda([ChannelName, &ChannelImage, ChannelTexture, Mixer]() -> FReply
                            {
                                return FChannelMixerUtils::RestoreSlotDefaultTexture(ChannelName, ChannelImage, *ChannelTexture, Mixer);
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

    //Defaults to 512
    SelectedOption = ComboBoxOptions[4];

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
    Mixer->TextureResolution = FChannelMixerUtils::ResFinder(*NewSelection.Get());
    FChannelMixerUtils::UpdatePreviewTexture(Mixer);
}

TSharedRef<SWidget> STexResComboBox::MakeWidgetForOption(TSharedPtr<FString> InOption)
{
    return SNew(STextBlock).Text(FText::FromString(*InOption));
}

FText STexResComboBox::GetComboBoxSelection() const
{
    return FText::FromString(*SelectedOption);
}

    
