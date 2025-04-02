// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#include "ChannelMixerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName ChannelMixerStyle::StyleSetName = FName("ChannelMixerStyle");
TSharedPtr<FSlateStyleSet> ChannelMixerStyle::CreatedSlateStyleSet = nullptr;

void ChannelMixerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

void ChannelMixerStyle::ShutDown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> ChannelMixerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("MaskTools"))->GetBaseDir() / "Resources";
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon16x16(16.f, 16.f);


	const FString IconDir = IconDirectory + TEXT("/T_MixChannels.png");
	CustomStyleSet->Set("Tools.MixChannels", new FSlateImageBrush(IconDir, Icon16x16));

	return CustomStyleSet;
}
