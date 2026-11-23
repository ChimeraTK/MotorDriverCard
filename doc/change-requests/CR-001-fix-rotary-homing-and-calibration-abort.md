# CR-001: Fix rotary stepper motor calibration abort and move verification

## Header

- Synopsis: Disabling a rotary motor during calibration has no effect today.
  Move verification counts an active reference switch as a completed move, which
  a rotary stage never stops at.
- Status: PLANNED

## Requirements

Calibration abort:

- Disabling a rotary motor during calibration must stop the motion.
- The motor must end in the disabled state.
- The aborted calibration must report a calibration error and reset the
  calibration mode.
- A disable issued before the calibration worker starts must not be lost.

Rotary move verification:

- Move verification must accept a move which reached its target.
- Move verification must report any other finished move as interrupted.
- An active reference switch must not make a rotary move count as successful:
  normal movement stops at neither switch.

Rotary switch documentation:

- The class documentation must state that the stage has a physical negative
  reference switch which does not stop the motion when passed and which the
  class reports as inactive.
- The documentation must state that both hardware stops stay disabled for
  normal movement, and that the calibration search enables the positive stop and
  disables it again.
- The documentation must name the stop configuration register and the value a
  rotary stage needs.

## Specifications

### ReferenceStateMachine (modified)

Location: `steppermotor/src/ReferenceStateMachine.cc` (modified).

- `actionStartCalib()` (modified): resets `_stopAction` before `_asyncActionActive`
  is set and the worker starts.
  - A disable dispatched between `calibrate()` and the worker start is therefore
    not overwritten.
  - The reset comes before the event dispatch returns, so no disable can precede
    it.
  - The linear machine resets the flag again in its worker, so its behaviour is
    unchanged.

### RotaryStepperMotorStateMachine (modified)

Location: `steppermotor/src/RotaryStepperMotorStateMachine.cc` (modified).

- Constructor: `_calibrating` gains a `disableEvent` transition to `_disabled`
  (new):
  - The entry lambda calls `actionStop()`, which sets `_stopAction`, and then
    `actionDisable()`.
  - `actionDisable()` moves the motor to the full step and switches motor
    current and end-switch power off.
  - The lambda does not lock `_motor._mutex`: `setEnabled()` holds it while it
    dispatches the event, and the mutex is a non-recursive `boost::mutex`.
  - The lambda runs at dispatch; only the state change waits while
    `_asyncActionActive` is set. `actionEndCallback()` applies `_disabled` on the
    next state query, after the worker has cleared the flag.
  - Its trailing `stopEvent` has no transition in `_disabled` and is dropped, as
    it already is in `_idle` after a stop abort.
  - The transition is added here and not in `ReferenceStateMachine`, so the
    linear machine keeps dropping the event.
- `performCalibration()` (modified): drops its `_stopAction` reset, which moves
  to `actionStartCalib()`. `_calibrationFailed` and `_moveInterrupted` stay reset
  at the start.
- Abort path (unchanged): `findEndSwitch()` and the final check see
  `_stopAction`, and `failCalibration()` reports `Error::CALIBRATION_ERROR`,
  sets `CalibrationMode::NONE` and clears the calibration data.
  - A search target written by a racing `moveToEndSwitch()` is cleared there;
    the current is already off.
  - The stall guard may set `_moveInterrupted` when the disable prevented the
    first step; the reported outcome is the same.

### RotaryStepperMotor (modified)

Location: `steppermotor/include/RotaryStepperMotor.h` (modified).

- `bool verifyMoveAction() override` (new, public):
  - Returns `BasicStepperMotor::verifyMoveAction()`, which compares the target
    position with the actual position.
  - No switch bit is consulted: normal rotary movement stops at neither switch,
    so neither bit may accept a move.
  - A move which finished anywhere but on its target is reported as interrupted.
- Class comment (modified), positive statements only:
  - Describe the stage as rotating continuously over 360 degrees.
  - State the physical negative reference switch, that it does not stop the
    motion when passed, and that the class reports it as inactive.
  - State that both hardware stops stay disabled for normal movement, that the
    constructor requires them disabled, and that the calibration search enables
    the positive stop and disables it again.
  - State that `limitsOK()` applies the configured position limits instead of
    the calibrated end-switch positions.
  - State that `verifyMoveAction()` accepts a move only when it reached its
    target.
  - State that the TMC429 end-switch stop inputs are configured through
    `referenceConfigAndRampModeData`, with the value `0x300` for a rotary stage.
  - Mark the XML example with a fenced code block, so Doxygen keeps it verbatim.

### Alternatives considered

- Overriding `verifyMoveAction()` to accept the positive switch or the reached
  target: rejected, normal movement does not stop at the switch, which is
  enabled for the calibration search only.
- Correcting `ReferenceStepperMotor::verifyMoveAction()` for every reference
  stage: rejected, a linear stage does stop at its switches and must accept a
  move which stopped there.
- Adding the transition in `ReferenceStateMachine`: rejected, the change covers
  the rotary abort; the linear abort stays as it is.

## Test plan

- `tests/src/steppermotor/testStepperMotorRotary.cc` (modified)
  - `testDisableDuringCalibrationStopsAndDisables`: disable while the home
    search runs.
    - Waits with a bounded deadline for a search target that differs from the
      actual position, then disables.
    - Asserts the `disabled` state through `waitForState()`, `getEnabled()`
      false, target and actual position equal, `getCalibrationFailed()`,
      `Error::CALIBRATION_ERROR`, `CalibrationMode::NONE` and a calibration time
      of 0.
    - The error and the failed flag are the assertions which fail without the
      abort; the mode and the time already have these values on an uncalibrated
      motor.
    - Covers the new `disableEvent` transition and the abort path; no existing
      case disables during calibration.
  - `testSwitchStopDoesNotVerifyMove`: a negative move which the dummy stops
    short of its target with the negative switch active.
    - Sets the target, waits for the `moving` state, drives the dummy with
      `moveTowardsTarget(1.0)` and lets it stop at the switch.
    - Asserts the `error` state and `Error::MOVE_INTERRUPTED`.
    - Covers the interrupted half of the verification; the reached-target half
      is exercised by `testCalibrateRebasesToZero` and
      `testCalibrationSurvivesRestart`.

Fixture premises:

- The cases use the existing `StepperMotorRotaryTestFixture`.
- The dummy reports a switch active only while that switch is enabled. After
  calibration both are disabled, so the move case enables the negative switch
  itself, deliberately breaking the constructor's configuration invariant to
  make the dummy report the switch active.
- The dummy's hardware counter still sits on the positive switch after
  calibration. The move case places the negative switch at 49900, just below the
  positive switch at 50000, so the dummy stops the negative move at the switch
  and reports standstill with the target still beyond the actual position.

Not covered:

- The requirement bullet about a disable before the worker start: no test hook
  places a disable in that window. The reset in `actionStartCalib()` is what the
  specification states; the observable part, a second calibration after an
  abort, already works with the reset in the worker.
- The positive switch cannot mark a rotary move as successful in a test: normal
  movement keeps both stops disabled.
