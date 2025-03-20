// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectTR : ModuleRules
{
	public ProjectTR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		// Core dependencies
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Niagara", "NetCore", "Slate", "SlateCore" });

		// AI dependencies
		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "NavigationSystem" });

        // Gameplay Ability System dependencies
        PublicDependencyModuleNames.AddRange(new string[] { "GameplayTags", "GameplayTasks" });

        // Recoil system dependencies
        PublicDependencyModuleNames.AddRange(new string[] { "RecoilAnimation" });
    }
}
