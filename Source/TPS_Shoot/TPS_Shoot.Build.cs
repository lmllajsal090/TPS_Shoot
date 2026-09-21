// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TPS_Shoot : ModuleRules
{
	public TPS_Shoot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TPS_Shoot",
			"TPS_Shoot/Variant_Platforming",
			"TPS_Shoot/Variant_Platforming/Animation",
			"TPS_Shoot/Variant_Combat",
			"TPS_Shoot/Variant_Combat/AI",
			"TPS_Shoot/Variant_Combat/Animation",
			"TPS_Shoot/Variant_Combat/Gameplay",
			"TPS_Shoot/Variant_Combat/Interfaces",
			"TPS_Shoot/Variant_Combat/UI",
			"TPS_Shoot/Variant_SideScrolling",
			"TPS_Shoot/Variant_SideScrolling/AI",
			"TPS_Shoot/Variant_SideScrolling/Gameplay",
			"TPS_Shoot/Variant_SideScrolling/Interfaces",
			"TPS_Shoot/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
