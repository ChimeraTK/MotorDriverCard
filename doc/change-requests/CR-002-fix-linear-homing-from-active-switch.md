# CR-002: Fix linear stepper motor homing from an active switch

## Header

- Synopsis: A linear stage whose positive reference switch is already active
records its current position as the calibrated switch position. An end-switch
search that never finds its switch runs without end.
- Depends on: CR-001
- Status: DONE

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

`BasicStepperMotor`:

- Forward-declares and befriends the linear homing test fixture, so the fixture
can replace `_stateMachine` with a state machine whose
`getHomingTimeoutInMilliseconds()` is short. The state machine and
`initStateMachine()` are private, so the tests have no other route to a short
timeout.
- The declaration is test-only access; no behaviour changes.

`ReferenceStateMachine`:

- New protected `moveOffEndSwitch()`, moved from
`RotaryStepperMotorStateMachine`: moves negatively by `getOffset()` per chunk
until `isEndSwitchActive(Sign::POSITIVE)` is false.
- Per chunk it compares the target position before and after the chunk. An
unchanged target means a blocked motor and sets `_moveInterrupted`.
- Returns on stop, on move interruption and on the homing timeout, which sets
`_moveInterrupted`; the caller decides on the abort. A switch which stays
active although the motor keeps moving therefore ends in the timeout abort of
the release search.
- New virtual `getHomingTimeoutInMilliseconds()`, defaulting to 60 s, and marked
`[[nodiscard]]` like the query accessors of `StepperMotor`. The rotary state
machine keeps the default.
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
- The release search, the positive search and the negative search run in that
order, each only when the previous one did not set `_stopAction` or
`_moveInterrupted`.
- A search issues its next chunk only after the previous move has finished, so
no chunk is issued over an end switch. The positive search therefore waits for
the motor to reach the switch after the release search has passed it.
- `getHomingTimeoutInMilliseconds()` is overridden in the class declaration and
returns 600000 ms (600 s), marked `[[nodiscard]]` like the base accessor. One
search chunk is 50 000 steps, so the rotary limit would expire within a single
chunk.
- `findEndSwitch()` gains the timeout check. Its blocked-move check stays as it
is: the search keeps running while neither reference switch is active. A motor
which does not move therefore ends in the timeout abort, not in the
blocked-move check.
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
is zero. The positive search ends on the switch, because its chunk target is
the switch position. The calibration measures the positive reference from the
negative switch, so the reference is `POSITIVE_SWITCH_POSITION -
NEGATIVE_SWITCH_POSITION`, i.e. the distance between the two switch positions.
- Linear motor, positive switch active and the motor blocked: the motor cannot
move, so the switch stays active and the per-chunk blocked-move check aborts
the release search. Calibration error and `CalibrationMode::NONE`. The
both-end-switches-always-on mode of the dummy is not used, because it raises
`Error::BOTH_END_SWITCHES_ON` instead. A switch which stays active although
the motor keeps moving, i.e. the timeout abort of the release search, is not
expressible with the dummy: a switch follows the hardware position and the
blocked-move check fires first. The blocked motor is therefore the only case
covering 'the switch does not release'; placing the switch out of the release
chunk's reach does not help, because the chunk stops on the switch and the
switch then releases. No separate case exercises the timeout abort of the
release search.
- Linear search which does not find its switch within the timeout: a test
subclass returns a timeout shorter than the time the test needs to finish the
first chunk, and both switches are placed at a reachable position beyond that
chunk. The calibration thread issues the first chunk and waits for it,
the test advances the dummy once, to the target of that chunk. The test issues
no chunk of its own, neither before nor after that step, so it never moves the
dummy to a position the search has not requested. The next iteration of the
search finds the timeout expired, sets `_moveInterrupted` and aborts the
calibration with a calibration error and `CalibrationMode::NONE`. The switches
are reachable, so the abort comes solely from the timeout being shorter than
the per-chunk wait; a blocked motor is not used, because the blocked-move check
would fire first. The search runs on the calibration thread, so the test only
advances the dummy and needs no thread of its own. The constant for that
position names it as reachable and beyond the first chunk, so its comment
describes the timeout as the sole abort reason and never claims that a search
reaches or passes the switch unused.
- The rotary homing cases must keep passing unchanged against the shared
release search.
