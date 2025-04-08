// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "ChannelMixerCS.generated.h"

class CHANNELMIXER_API FChannelMixerCS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FChannelMixerCS);
    SHADER_USE_PARAMETER_STRUCT(FChannelMixerCS, FGlobalShader);

    // Shader parameters
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_TEXTURE(Texture2D, RedTexture)
        SHADER_PARAMETER_TEXTURE(Texture2D, GreenTexture)
        SHADER_PARAMETER_TEXTURE(Texture2D, BlueTexture)
        SHADER_PARAMETER_TEXTURE(Texture2D, AlphaTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
        SHADER_PARAMETER_UAV(RWTexture2D<float4>, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

public:
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
        OutEnvironment.SetDefine(TEXT("THREAD_GROUP_SIZE_X"), 8);
        OutEnvironment.SetDefine(TEXT("THREAD_GROUP_SIZE_Y"), 8);
    }
};

// Declare the shader implementation
IMPLEMENT_GLOBAL_SHADER(FChannelMixerCS, "/MaskTools/ChannelMixerCS.usf", "ChannelMixerMain", SF_Compute);

class ChannelMixerStaticLib 
{

public:
    // Main function to merge three textures
    static UTexture2D* CreateMask(
        UTexture2D* RedTexture,
        UTexture2D* GreenTexture,
        UTexture2D* BlueTexture,
        UTexture2D* AlphaTexture,
        int32 TargetResolution = 512,
        FString OutputName = "GeneratedMask");


private:
    // GPU data readback helper struct
    struct FTextureReadbackData
    {
        TArray<FColor> Data;
        int32 Width;
        int32 Height;
        bool bSuccess;
    };

    // Helper to execute the shader
    static void ExecuteComputeShader(
        FRHICommandListImmediate& RHICmdList,
        FTextureRHIRef RedTextureRHI,
        FTextureRHIRef GreenTextureRHI,
        FTextureRHIRef BlueTextureRHI,
        FTextureRHIRef AlphaTextureRHI,
        FUnorderedAccessViewRHIRef OutputUAV,
        FIntVector ThreadGroupCounts);


    // Helper to read back texture data from GPU to CPU
    static FTextureReadbackData ReadbackTextureData(FRHICommandListImmediate& RHICmdList, FTextureRHIRef TextureRHI);

    // Helper to create UTexture2D from raw data
    static UTexture2D* CreateTextureFromData(const TArray<FColor>& PixelData, int32 Width, int32 Height, FString Name);
};