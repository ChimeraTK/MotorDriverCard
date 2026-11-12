# CR-002: Fix linear stepper motor homing from an active switch

## Header

- Synopsis: A linear stage whose positive reference switch is already active
  records its current position as the calibrated switch position. An end-switch
  search that never finds its switch runs without end.
- Depends on: CR-001
- Status: PLANNED

## Requirements

Releasing an active switch:

- Calibration must move the motor off an already-active positive reference
  switch before homing.
- The positive reference must be the position where the switch activates when
  approached positively, independent of the start position.

Bounded search:

- A search that does not find its switch within the homing timeout of the stage
  must abort the calibration.
- The abort reports a calibration error and leaves the calibration mode at
  `CalibrationMode::NONE`.

Stopping and disabling a linear calibration need no change; CR-001 covers them.

## Specifications

`ReferenceStateMachine`:

- New protected `moveOffEndSwitch()`, moved from
  `RotaryStepperMotorStateMachine`: moves negatively by `getOffset()` per chunk
  until `isEndSwitchActive(Sign::POSITIVE)` is false.
- Per chunk it compares the target position before and after the chunk. An
  unchanged target means a blocked motor and sets `_moveInterrupted`.
- Returns on stop or move interruption; the caller decides on the abort.
- New virtual `getHomingTimeoutInMilliseconds()`, defaulting to 60 s. The rotary
  state machine keeps the default.
- New protected `homingTimedOut(start)` helper for the search loops, replacing
  the inline checks of the rotary search.

`RotaryStepperMotorStateMachine`:

- Drops `moveOffEndSwitch()` and `HOMING_TIMEOUT_MS` and uses the inherited
  release search.
- Behaviour is unchanged.

`LinearStepperMotorStateMachine`:

- `performCalibration()` releases an active positive switch before homing and
  homes only when neither `_stopAction` nor `_moveInterrupted` is set.
- `getHomingTimeoutInMilliseconds()` returns 600 s. One search chunk is
  50 000 steps, so the rotary limit would expire within a single chunk.
- `findEndSwitch()` keeps its blocked-move check and gains the timeout check.
- The existing `_moveInterrupted` handling of `performCalibration()` reports
  both aborts as `Error::CALIBRATION_ERROR`, so there is no new error path.

### Alternatives considered

- One 60 s timeout for both stages: rejected, too short for a 50 000-step
  chunk.
- A separate constant per state machine: rejected, the virtual accessor
  follows the existing `getOffset()` pattern.
- Duplicating the release search in the linear state machine: rejected, both
  copies would be identical.
- A separate, smaller release chunk: rejected, the overshoot past the release
  point is harmless because the homing search returns to the switch.

## Test plan

- Linear motor, positive switch already active: the motor moves off, the
  calibration completes with `CalibrationMode::FULL` and the negative reference
  is zero.
- Linear motor, positive switch forced to stay active with a blocked motor: the
  release search aborts, calibration error and `CalibrationMode::NONE`.
- Linear search which never reaches its switch: a test subclass shortens
  `getHomingTimeoutInMilliseconds()` and exposes the search loop, which aborts
  without finding the switch.
- The rotary homing cases must keep passing unchanged against the shared
  release search.
