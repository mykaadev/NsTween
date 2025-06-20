// Copyright (C) 2025 mykaadev. All rights reserved.

using UnrealBuildTool;

public class NsTween : ModuleRules
{
    public NsTween(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                System.IO.Path.Combine(ModuleDirectory, "Public", "Classes")
            }
        );

        PrivateIncludePaths.AddRange(
            new string[]
            {
                System.IO.Path.Combine(ModuleDirectory, "Private", "Classes")
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "DeveloperSettings",
                // ... add other public dependencies that you statically link with here ...
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                // ... add private dependencies that you statically link with here ...
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );
    }
}
