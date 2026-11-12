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
- Calibration must fail with a calibration error and leave the calibration mode
  at `CalibrationMode::NONE` if the switch does not release.

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
  the inline checks of the rotary search. It sets `_moveInterrupted` and returns
  true on expiry, so a timed-out search never returns as if it had succeeded.

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
  `getHomingTimeoutInMilliseconds()` and exposes the search loop. The motor keeps
  moving with the positive switch out of reach, because a blocked motor trips the
  blocked-move check first. The loop is stepped from a separate thread, and it
  returns with `_moveInterrupted` set.
- The rotary homing cases must keep passing unchanged against the shared
  release search.

## Deferred issue

- DI-1 [NEW] Requirements: no bullet requires the linear calibration to fail when the switch does not release, although the specification (line 38, 'the caller decides on the abort') and test plan item 2 (lines 76-77) both depend on it; CR-001 carries that requirement for the rotary stage, so its omission here leaves test plan item 2 without a requirement basis
- DI-2 [NEW] Specifications line 41-42: the contract of the new `homingTimedOut(start)` helper is ambiguous. Because every search loop relies on `_moveInterrupted` to reach the error branch, a helper that only returns true would let the caller return from a timed-out search silently, and the calibration would then report success with a bogus switch position. The plan must state that the helper sets `_moveInterrupted` as the replaced inline rotary check does
- DI-3 [NEW] Test plan line 78-80: the timeout case is not executable as written. A blocked motor cannot be used, because the blocked-move check (`LinearStepperMotorStateMachine.cc:80-84`) fires before the timeout; the motor must keep moving with the switch out of reach, and the loop must be stepped from a separate thread while the test subclass calls it, since `moveToEndSwitch()` blocks until the dummy has been hand-stepped
