// Copyright (c) 2025 Sora Mas
// All rights reserved.

using UnrealBuildTool;

public class ChannelMixer : ModuleRules
{
	public ChannelMixer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AssetRegistry",
                "AssetTools",
                "ContentBrowser",
                "UnrealEd",
                "RenderCore",
                "EnchancedEditorLogging",
                "InputCore",
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EditorFramework",
                "ToolMenus",
                "UnrealEd",
                "Projects",
                "MaskTools"
            }
            );
	}
}
