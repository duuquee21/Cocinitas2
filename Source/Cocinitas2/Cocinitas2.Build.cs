// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Cocinitas2 : ModuleRules
{
	public Cocinitas2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Cocinitas2",
			"Cocinitas2/Variant_Strategy",
			"Cocinitas2/Variant_Strategy/UI",
			"Cocinitas2/Variant_TwinStick",
			"Cocinitas2/Variant_TwinStick/AI",
			"Cocinitas2/Variant_TwinStick/Gameplay",
			"Cocinitas2/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
