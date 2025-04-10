// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "Styling/SlateStyle.h"

/**
 * 
 */
class CHANNELMIXER_API ChannelMixerStyle
{
public:
	static void InitializeIcons();
	static void ShutDown();

private:

	static FName StyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();

	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;

public:
	static FName GetStyleSetName() { return StyleSetName; }
};
