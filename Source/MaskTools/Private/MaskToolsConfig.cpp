// Copyright (c) 2025 Sora Mas \n All rights reserved. 


#include "MaskToolsConfig.h"

UMaskToolsConfig::UMaskToolsConfig()
{
	DefaultMaskPrefix = TEXT("T");
	DefaultMaskName = TEXT("GeneratedMask");
	DefaultMaskSuffix = TEXT("Mask");
	DefaultMaskSavePath = FDirectoryPath("GeneratedThumbnails");
	DefaultMaskResolution = EMaskResolutions::FiveHundredTwelve;
	bDiscardEmptyChannels = true;
}
