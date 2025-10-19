// Copyright (C) 2025 nulled.softworks. All rights reserved.

#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "NsTweenSubsystem.h"
#include "NsTweenTypeLibrary.h"
#include "Interfaces/ITweenValue.h"

namespace
{
    /** Minimal tween strategy used to satisfy spawn requirements during tests. */
    class FAutomationTestTweenValue final : public ITweenValue
    {
    public:
        virtual void Initialize() override {}
        virtual void Apply(float) override {}
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNsTweenSubsystemCommandQueueTest, "NsTween.Subsystem.CommandEnqueue", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FNsTweenSubsystemCommandQueueTest::RunTest(const FString& Parameters)
{
    UNsTweenSubsystem* Subsystem = NewObject<UNsTweenSubsystem>();
    check(Subsystem);

    FNsTweenSpec Spec;
    const TSharedPtr<ITweenValue> Strategy = MakeShared<FAutomationTestTweenValue>();

    // Spawn command
    const FNsTweenHandle SpawnHandle = Subsystem->EnqueueSpawn(Spec, Strategy);
    TestTrue(TEXT("Spawn returns a valid handle"), SpawnHandle.IsValid());

    FNsTweenCommand Command;
    TestTrue(TEXT("Spawn command is enqueued"), FNsTweenSubsystemTestAccessor::DequeueCommand(*Subsystem, Command));
    TestEqual(TEXT("Spawn command type"), Command.Type, ENsTweenCommandType::Spawn);
    TestEqual(TEXT("Spawn command handle"), Command.Handle.Id.Value, SpawnHandle.Id.Value);

    // Cancel command
    Subsystem->EnqueueCancel(SpawnHandle, /*bApplyFinal*/ true);
    TestTrue(TEXT("Cancel command is enqueued"), FNsTweenSubsystemTestAccessor::DequeueCommand(*Subsystem, Command));
    TestEqual(TEXT("Cancel command type"), Command.Type, ENsTweenCommandType::Cancel);
    TestTrue(TEXT("Cancel command requests final application"), Command.bApplyFinalOnCancel);

    // Pause command
    Subsystem->EnqueuePause(SpawnHandle);
    TestTrue(TEXT("Pause command is enqueued"), FNsTweenSubsystemTestAccessor::DequeueCommand(*Subsystem, Command));
    TestEqual(TEXT("Pause command type"), Command.Type, ENsTweenCommandType::Pause);

    // Resume command
    Subsystem->EnqueueResume(SpawnHandle);
    TestTrue(TEXT("Resume command is enqueued"), FNsTweenSubsystemTestAccessor::DequeueCommand(*Subsystem, Command));
    TestEqual(TEXT("Resume command type"), Command.Type, ENsTweenCommandType::Resume);

    return true;
}

#endif // WITH_AUTOMATION_TESTS
