// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FChannelSplitter : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	void InitCBMenuExtension();
	TSharedRef<FExtender> CreateCBMenuExtension(const TArray<FAssetData>& SelectedAssets);
	void AddCBMenuExtension(FMenuBuilder& MenuBuilder);
	void SplitTextures();

	TArray<FAssetData> AssetsSelected;

	const TArray<FString> SuffixArray = { TEXT("_R"), TEXT("_G") ,TEXT("_B"), TEXT("_A") };
	
};
