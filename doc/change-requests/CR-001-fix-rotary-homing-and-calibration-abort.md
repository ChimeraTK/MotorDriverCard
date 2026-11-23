# CR-001: Fix rotary stepper motor calibration abort and move verification

## Header

- Synopsis: Disabling a rotary motor during calibration has no effect today,
  while stopping it aborts the calibration. Move verification counts an active
  reference switch as a completed move, which a rotary stage never stops at.
- Status: IMPLEMENTED

## Requirements

A rotary calibration cannot be aborted by disabling the motor.

Calibration abort:

- Stopping a rotary motor during calibration must stop the motion, end the
  calibration and leave the motor idle.
- Disabling a rotary motor during calibration must stop the motion and leave the
  motor disabled.
- Either abort must report a calibration error and reset the calibration mode.
- An abort issued after `calibrate()` but before the calibration thread starts
  must not be lost.

Rotary move verification:

- Move verification must accept a rotary move which reached its target.
- Move verification must report any other finished rotary move as interrupted.
- An active reference switch must not make a rotary move count as successful.

Rotary switch documentation:

- The `RotaryStepperMotor` class comment must state that the stage has a
  physical negative reference switch which does not stop the motion when passed
  and which the class reports as inactive.
- It must state that both hardware stops stay disabled for normal movement, and
  that the calibration search drives the positive stop.
- It must name `referenceConfigAndRampModeData` and the value `0x300` a rotary
  stage needs.

## Specifications

### ReferenceStateMachine (modified)

Location: `steppermotor/src/ReferenceStateMachine.cc` (modified).

- `actionStartCalib()` (modified): clears the abort flag `_stopAction` before
  `_asyncActionActive` is set and the worker starts.
  - `calibrate()` dispatches the event under `_motor._mutex` and returns after
    the entry action, so a disable or stop dispatched afterwards is not
    overwritten.

### RotaryStepperMotorStateMachine (modified)

Location: `steppermotor/src/RotaryStepperMotorStateMachine.cc` (modified).

- Constructor: `BasicStepperMotor::StateMachine::disableEvent` gains a
  transition from `_calibrating` to `_disabled` (new):
  - The entry action calls `actionStop()` and then `actionDisable()`.
  - `actionStop()` sets `_stopAction`; `actionDisable()` moves the motor to the
    full step and switches motor current and end-switch power off.
  - The entry action does not lock `_motor._mutex`: `setEnabled()` holds it
    while it dispatches the event, and the mutex is a non-recursive
    `boost::mutex`.
  - `actionDisable()` writes the standstill target without that mutex,
    concurrently with `moveToEndSwitch()`; `failCalibration()` resets the target
    under the mutex afterwards.
  - The entry action runs in the disabling thread. The state change waits while
    `_asyncActionActive` is set: `actionEndCallback()` applies `_disabled` on the
    next state query, after the worker has cleared the flag.
  - The transition registers no internal callback. The `stopEvent` which
    `actionEndCallback()` emits afterwards is dropped, as `_disabled` has no
    transition for it.
- `performCalibration()` (modified): no longer clears `_stopAction`;
  `actionStartCalib()` does.
- Abort path (unchanged): `findEndSwitch()` and the final check see
  `_stopAction`, and `failCalibration()` reports `Error::CALIBRATION_ERROR`,
  sets `CalibrationMode::NONE` and clears the calibration data.
  - The existing `stopEvent` transition of `_calibrating` takes this path. It
    sets `_stopAction` and leaves for `_idle`, so the motor stays in the chunk it
    is in and the target reset of `failCalibration()` leaves it standing.
  - The stall guard may set `_moveInterrupted` when the disable prevented the
    first step; the reported outcome is the same.

### RotaryStepperMotor (modified)

Location: `steppermotor/include/RotaryStepperMotor.h` (modified).

- `bool verifyMoveAction() override` (new, public, defined inline as
  `limitsOK()` is):
  - Returns `BasicStepperMotor::verifyMoveAction()`, which compares the target
    position with the actual position.
  - The inherited `ReferenceStepperMotor::verifyMoveAction()`, which also
    accepts an active switch, no longer applies.
  - A move which finished anywhere but on its target is reported as interrupted.
- Class comment (modified):
  - Describe the stage as rotating continuously over 360 degrees.
  - State the physical negative reference switch, that it does not stop the
    motion when passed, and that the class reports it as inactive.
  - State that both hardware stops stay disabled for normal movement, that the
    constructor requires them disabled, and that the calibration search drives
    the positive stop.
  - State that `verifyMoveAction()` accepts a move only when it reached its
    target.
  - State that the TMC429 end-switch stop inputs are configured through
    `referenceConfigAndRampModeData`, with the value `0x300` for a rotary stage.
  - Mark the XML example with a fenced code block, so Doxygen keeps it verbatim.

### Alternatives considered

- Accepting the positive switch or the reached target: rejected, normal
  movement does not stop at the switch, which is driven for the calibration
  search only.
- Correcting `ReferenceStepperMotor::verifyMoveAction()` for every reference
  stage: rejected, a linear stage does stop at its switches and must accept a
  move which stopped there.
- Adding the transition in `ReferenceStateMachine`: rejected, the change covers
  the rotary abort; the linear machine keeps dropping the event.

## Test plan

- `tests/src/steppermotor/testStepperMotorRotary.cc` (modified)
  - `testCancellationClearsTargetDespiteReferenceWriteFailure` (modified): the
    existing stop during the home search, extended with `getCalibrationFailed()`
    and `Error::CALIBRATION_ERROR`.
    - Covers the stop half of the abort requirement; the case already asserts
      idle, a cleared target and `CalibrationMode::NONE`.
  - `testStopDuringCalibrationStopsTheSearch`: stop while the home search runs.
    - Advances half of the running chunk, calls `stop()`, finishes the chunk and
      waits for idle.
    - Asserts idle, the motor still enabled, target and actual position equal,
      `getCalibrationFailed()`, `Error::CALIBRATION_ERROR`,
      `CalibrationMode::NONE`, a calibration time of 0, and an unchanged target
      after more than one wakeup period of the search loop.
    - Covers the stop half of the abort requirement and that the abort commands
      no further search chunk; the extended
      `testCancellationClearsTargetDespiteReferenceWriteFailure` covers the
      teardown which must also work when the calibration write fails.
  - `testDisableDuringCalibrationStopsAndDisables`: disable while the home
    search runs.
    - Starts a calibration, waits with a bounded deadline for a search target
      that differs from the actual position, advances the dummy with
      `moveTowardsTarget(0.5)` and disables while the motor is still moving.
    - Asserts the `disabled` state through `waitForState()`, `getEnabled()`
      false, target and actual position equal, `getCalibrationFailed()`,
      `Error::CALIBRATION_ERROR`, `CalibrationMode::NONE` and a calibration time
      of 0.
    - The error and the failed flag differ between today's code and the fix; the
      mode and the time already hold on an uncalibrated motor.
    - Covers the new `disableEvent` transition and the abort path; no existing
      case disables during calibration.
  - `testPositiveSwitchDoesNotVerifyMove`: a positive move which the dummy
    refuses to step.
    - Runs after `performCalibrationForTest()`, then enables the positive stop,
      which reports active because the dummy counter sits on it.
    - Sets a target above the actual position, calls `start()` and asserts the
      `error` state and `Error::MOVE_INTERRUPTED`.
    - Covers the target-only verification; without the override the active
      switch bit accepts the move.
    - The reached-target half is exercised by `testCalibrateRebasesToZero` and
      `testCalibrationSurvivesRestart`, which fail if a reached target is
      rejected.

Fixture premises:

- The cases use the existing `StepperMotorRotaryTestFixture`, which enables the
  motor with `setEnabled(true)`. The dummy steps while a search target is open,
  and `moveTowardsTarget(0.5)` advances it half of the remaining distance.
- The dummy reports a switch active only while that switch is enabled, and it
  stays put while the active switch lies in the travel direction.
- The `StepperMotorRotaryTestFixture` constructor ends with
  `_motorControlerDummy->moveTowardsTarget(0, false, false, true)`: the reboot
  leaves both hardware stops enabled and the motor current off, so the fixture
  switches the stops off and the motor current on afterwards.
- After calibration both stops are disabled and the dummy counter still sits on
  the positive switch. The move case enables the positive stop at runtime, which
  breaks the configuration the constructor allows, to make the dummy report the
  switch active.
- `driveUntilEndSwitchActive()` asserts that the negative stop is disabled, so
  the move case enables the positive stop only.

Not covered:

- An abort before the calibration thread starts: no hook places one in that
  window, so the reset in `actionStartCalib()` is verified by inspection.
- The documentation bullets: checked by reading the class comment.

## Deferred issue

- DI-1 [REJECTED] The claim that the fixture leaves the motor current off, so
  the abort cases see no running search, does not hold. The fixture constructor
  creates the motor and calls `setEnabled(true)`; it does not call
  `moveTowardsTarget(0, false, false, true)`. A probe at
  `awaitCalibrationSearch()` reports `currentOn=1`, `moving=1`, target 5000 and
  an actual position of 2500 after `moveTowardsTarget(0.5)`, so the search chunk
  is in flight. The fixture premise now states the enabled motor explicitly.
- DI-2 [ACCEPTED] Pre-existing test failure outside CR-001's scope:
  StepperMotorRotaryTest/testRequiresDisabledHardwareStops fails (also
  standalone) with a fatal ChimeraTK::logic_error at
  tests/src/steppermotor/testStepperMotorRotary.cc:174, because the test expects
  std::logic_error while ChimeraTK::logic_error derives from std::exception.
  Both the throwing constructor (steppermotor/src/RotaryStepperMotor.cc) and
  that test case come from commit af6e8f4 and are untouched by CR-001. All
  CR-001 cases pass; fixing this test is not part of this change request.
