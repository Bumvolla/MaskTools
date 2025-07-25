// Copyright (c) 2025 Sora Mas
// All rights reserved.


#include "MaskToolsConfig.h"

UMaskToolsConfig::UMaskToolsConfig()
{
	DefaultMaskPrefix = TEXT("T");
	DefaultMaskName = TEXT("GeneratedMask");
	DefaultMaskSuffix = TEXT("Mask");
	DefaultMaskSavePath.Path = TEXT("GeneratedMasks");
	DefaultMaskResolution = EMaskResolutions::FiveHundredTwelve;
	bDiscardEmptyChannels = true;
}
