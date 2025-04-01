// Copyright (c) 2025 Sora Mas \n All rights reserved. 

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
                "UMG",
                "Niagara",
                "AssetTools",
                "ContentBrowser",
                "EditorScriptingUtilities",
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
                "Projects",
                "InputCore",
                "EditorFramework",
                "ToolMenus",
                "UnrealEd",
                "Projects",




				// ... add private dependencies that you statically link with here ...	
			}
            );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			});
	}
}
