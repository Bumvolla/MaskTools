// Copyright (c) 2025 Sora Mas \n All rights reserved. 

using UnrealBuildTool;

public class EnchancedEditorLogging : ModuleRules
{
	public EnchancedEditorLogging(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
			{ 
				"Core",
				"CoreUObject",
				"Engine",
                "ContentBrowser",
            });
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "Slate",
                "SlateCore",
                "ApplicationCore",
                "EditorFramework",
                "ToolMenus",
                "UnrealEd",
            });
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			});
	}
}
