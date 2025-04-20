// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#pragma once

#include "Styling/SlateStyle.h"

/**
 * 
 */
class CHANNELSPLITTER_API ChannelSpliterStyle
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