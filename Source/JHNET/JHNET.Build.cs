// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class JHNET : ModuleRules
{
	public JHNET(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        var rootPath = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "../../"));
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils", "UMG", "NavigationSystem", "AIModule", "GameplayTasks" });
        PublicAdditionalLibraries.Add(rootPath + @"\Source\ThirdParty\Steamworks\steam_api64.lib");
        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}