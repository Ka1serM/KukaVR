// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class PUREVR : ModuleRules
{
	public PUREVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"HeadMountedDisplay",
			"DebugPlugin",
			"UMG",
			"Slate",
			"SlateCore",
			"XRBase"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"ControlRig",
			"RigVM",
			"AnimationCore",
			"XRBase", 
			"Niagara",
			"ProceduralMeshComponent"
		});

		// MyProject.Build.cs
		PublicDefinitions.Add("MARCEL=1");
		PublicDefinitions.Add("WALTER=0");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
