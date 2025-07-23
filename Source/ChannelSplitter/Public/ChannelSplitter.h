// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <Kismet/KismetMaterialLibrary.h>
#include <Kismet/KismetRenderingLibrary.h>

class FChannelSplitter : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

private:
	void InitCBMenuExtension();
	TSharedRef<FExtender> CreateCBMenuExtension(const TArray<FAssetData>& SelectedAssets);
	void AddCBMenuExtension(FMenuBuilder& MenuBuilder);
	void SplitTextures();

	void SplitTexturesMaterialBased();
	void SplitTexturesPixelData();
	void SplitTexturesShaderBased();

	TArray<FAssetData> AssetsSelected;
	const TArray<FString> SuffixArray = { TEXT("_R"), TEXT("_G") ,TEXT("_B"), TEXT("_A") };

	/*
	* Returns true if grayscale material is completely black or white.
	*/
	bool IsChannelEmpty(UMaterialInstanceDynamic* Material);
	
};
