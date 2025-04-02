// Copyright (c) 2025 Sora Mas \n All rights reserved. 


#include "ChannelSpliterStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"


FName ChannelSpliterStyle::StyleSetName = FName("ChannelSplitterStyle");
TSharedPtr<FSlateStyleSet> ChannelSpliterStyle::CreatedSlateStyleSet = nullptr;

void ChannelSpliterStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
}


void ChannelSpliterStyle::ShutDown()
{

	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> ChannelSpliterStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("MaskTools"))->GetBaseDir() / "Resources";
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon16x16(16.f, 16.f);


	const FString IconDir = IconDirectory + TEXT("/T_SplitChannels.png");
	CustomStyleSet->Set("ContentBrowser.SplitChannels", new FSlateImageBrush(IconDir, Icon16x16));

	return CustomStyleSet;
}