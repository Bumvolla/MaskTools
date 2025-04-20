// Copyright (c) 2025 Sora Mas
// All rights reserved.

using UnrealBuildTool;

public class ChannelSplitter : ModuleRules
{
	public ChannelSplitter(ReadOnlyTargetRules Target) : base(Target)
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
                "EnchancedEditorLogging"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ApplicationCore",
                "EditorFramework",
                "ToolMenus",
                "UnrealEd",
                "Projects",
                "MaskTools",

			}
            );
	}
}
