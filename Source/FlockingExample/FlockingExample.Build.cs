// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class FlockingExample : ModuleRules
{
	public FlockingExample(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        var rootPath = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "../../"));
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils", "UMG", "JHNET"});
        PublicAdditionalLibraries.Add(rootPath + @"\Source\ThirdParty\Steamworks\steam_api64.lib");
        PrivateDependencyModuleNames.AddRange(new string[] { "JHNET" });
    }
}
