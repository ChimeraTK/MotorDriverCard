# CR-001: Fix rotary stepper motor homing and calibration abort

## Header

- Synopsis: A rotary stage whose reference switch is already active cannot
  calibrate. Calibration can only be aborted by disabling the motor.
- Status: READY TO TEST (from DONE)

## Requirements

Rotary homing from an active switch:

- Calibration must move the motor off an active positive reference switch
  before homing.
- Homing must still stop at the positive switch and store it as both calibrated
  end-switch positions.
- Calibration must fail with a calibration error and reset the calibration mode
  if the switch does not release.

Calibration abort:

- Stopping during calibration must stop motion.
- The motor must end in the idle state.
- Disabling during calibration must stop motion.
- The motor must end in the disabled state.

Rotary switch semantics:

- A rotary motor must report the negative end switch as inactive.
- The both-end-switches-active error must never be raised for a rotary motor.
- Move verification must not treat the negative switch bit as a reason to
  accept a move.

Build without Qt:

- The library and its tests must configure and build on a host without Qt.
- A host with Qt keeps building the `ConfigCalculator` GUI as before.

## Specifications

### RotaryStepperMotorStateMachine (modified)

Location: `steppermotor/include/RotaryStepperMotorStateMachine.h`,
`steppermotor/src/RotaryStepperMotorStateMachine.cc` (both modified).

- `bool waitForSwitchState(Sign watchSign, Sign direction, bool wanted)` (new,
  protected):
  - Loops while `isEndSwitchActive(watchSign) != wanted`.
  - Checks in this order.
  - Returns false without setting `_moveInterrupted` when `_stopAction` is set:
    a stop is an abort, not an interruption.
  - Returns false and sets `_moveInterrupted` when `HOMING_TIMEOUT_MS` expires.
  - Returns false and sets `_moveInterrupted` when `moveToEndSwitch()` left the
    target unchanged: the motor is blocked.
  - Returns false and sets `_moveInterrupted` when the target differs from the
    actual position while neither reference is active.
  - Otherwise calls `moveToEndSwitch(direction)` and loops.
- `void moveOffEndSwitch()` (new, protected): calls
  `waitForSwitchState(Sign::POSITIVE, Sign::NEGATIVE, false)`.
- `void findEndSwitch(Sign sign)` (modified): calls
  `waitForSwitchState(sign, sign, true)`.
- `void performCalibration()` (modified):
  - Move off the switch when `isEndSwitchActive(Sign::POSITIVE)` is true at the
    start.
  - Home positively only when neither `_stopAction` nor `_moveInterrupted` is
    set afterwards, so an aborted release search is not overwritten by a
    completed homing.
  - Store the home position as both calibrated end-switch positions.
  - On abort, set the calibration time to 0, `_calibrationFailed` to true,
    `Error::CALIBRATION_ERROR` and `CalibrationMode::NONE`.
  - The `ChimeraTK::runtime_error` catch reports the same abort.

### ReferenceStateMachine (modified)

Location: `steppermotor/include/ReferenceStateMachine.h`,
`steppermotor/src/ReferenceStateMachine.cc` (both modified).

- `static constexpr int HOMING_TIMEOUT_MS = 60000` (new, protected): the search
  timeout, owned here so further searches can share it.
- Stop transition of `_calibrating` (modified): the lambda sets `_stopAction`
  and calls `actionMovetoStop()`. The transition requests `_idle` and keeps
  `actionEndCallback()` as its internal callback.
- `disableEvent` transition from `_calibrating` to `_disabled` (new): the lambda
  sets `_stopAction`, calls `actionMovetoStop()`, then `actionDisable()`.
  `actionDisable()` moves to the full step and switches motor current and
  end-switch power off. The transition keeps `actionEndCallback()` as its
  internal callback.
- `actionStop()` keeps its present body, so no other state which shares it
  changes.
- These lambdas and `actionMovetoStop()` do not lock `_motor._mutex`: `stop()`,
  `setEnabled()` and `calibrate()` hold it while they dispatch the event, and it
  is a non-recursive `boost::mutex`.
- `void moveToEndSwitch(Sign sign)` (modified): re-checks `_stopAction` under
  `_motor._mutex` immediately before setting the target, so no chunk target
  follows a processed stop.
- Both new transitions keep the end callback of `_calibrating`, so the machine
  stays in `_calibrating` and `motorActive()` stays true until the worker has
  cleared `_asyncActionActive`.
  - `actionEndCallback()` then moves to the requested `_idle` or `_disabled`
    state, so no second calibration can run beside the worker.
  - `_disabled` has no stop transition, so the aborted calibration reports its
    error before the disabled state is entered.
- The background thread only clears `_asyncActionActive`; the calibration flags
  are reset when the next calibration starts.

### ReferenceStepperMotor (modified)

Location: `steppermotor/include/ReferenceStepperMotor.h`,
`steppermotor/src/ReferenceStepperMotor.cc` (both modified).

- `bool hasNegativeReferenceSwitch() const` (new, virtual, protected): returns
  true.
  - `RotaryStepperMotor` overrides it to false.
  - This is the single place which decides that a negative switch is absent.
- `bool isEndSwitchActive(Sign sign)` (modified):
  - Computes the negative state as
    `hasNegativeReferenceSwitch() && negativeSwitchActive()`.
  - Skips the both-end-switches-active error and the `Sign::NEGATIVE` return
    value when the negative switch is absent.
  - The tolerance check keeps its positive branch; only the negative branch is
    skipped.
  - `negativeSwitchActive()` still reports the hardware bit.

### RotaryStepperMotor (modified)

Location: `steppermotor/include/RotaryStepperMotor.h`,
`steppermotor/src/RotaryStepperMotor.cc` (both modified).

- `bool hasNegativeReferenceSwitch() const override` (new): returns false.
  - Replaces the present `negativeSwitchActive()` override, which hid the
    hardware bit.
- `bool verifyMoveAction() override` (new):
  - Verifies that a completed move ran to its end, not that the motor is
    calibrated.
  - Accepts the move when `isEndSwitchActive(Sign::POSITIVE)` is true or the
    target position equals the actual position.
  - The switch bit accepts a move which stopped at the switch and left the
    target above the actual position.
  - The search targets are full steps, so a completed move has equal target and
    actual positions; no tolerance window is compared.
  - The negative switch bit is not consulted, so it cannot mask a blocked move.
  - A calibration guard and a hardware tolerance stay out of scope.
  - Callers: `BasicStepperMotor::StateMachine::waitForStandstill()` and
    `ReferenceStateMachine::getToleranceEndSwitch()`.
- Class comment (modified):
  - State that the stage has no physical negative reference switch.
  - State that `hasNegativeReferenceSwitch()` returns false and that
    `limitsOK()` skips the end-switch range check.
  - State that the TMC429 can ignore end switches via
    `referenceConfigAndRampModeData`.
  - Mark the XML example with a fenced code block, so Doxygen keeps it
    verbatim.
  - Documents the rotary switch semantics requirement.
- Declaration comment of `verifyMoveAction()`, name the reached target, not
  "the position matches".

### ConfigCalculator/CMakeLists.txt (modified)

Location: `ConfigCalculator/CMakeLists.txt` (modified).

- Probe Qt once with
  `find_package(QT 5.14 NAMES Qt6 Qt5 COMPONENTS Core Network Widgets Gui)` and
  add the `GUI` subdirectory only when `QT_FOUND` is set.
  - The version bound matches the GUI's own Qt 5.14 requirement, so an older Qt
    skips the GUI instead of failing inside it.
- Skip the GUI with a status message when Qt is missing.
- The library and its tests configure and build without Qt; a host with Qt
  keeps building the GUI as before.
- Verification: configure with `-DCMAKE_DISABLE_FIND_PACKAGE_QT=ON` and with Qt
  present.

### Alternatives considered

- Runtime check of `referenceConfigAndRampModeData` in the rotary motor
  constructor: rejected, the switch stop inputs stay a pure XML configuration
  concern (`0x300`).
- Overriding `negativeSwitchActive()` to false: rejected, it hides the hardware
  state.
- Running the internal event callback before returning the state from
  `utility::StateMachine::getCurrentState()`: rejected. For an instantaneous
  action the transition happens inside that callback, so the caller would never
  observe the `moving` state. The one-call delay is intentional.
- One search loop for both directions without a helper: rejected, the two loops
  differ in their exit condition and share everything else, so
  `waitForSwitchState()` carries the difference as a parameter.

## Test plan

### `tests/src/steppermotor/testRotaryStepperMotor.cc` (modified)

- `HomingFromActiveSwitchMovesOffFirst`: positive switch active at the start.
  - The motor moves off, ends in `CalibrationMode::FULL`, both calibrated
    positions equal.
  - Covers the release search and the homing of `performCalibration()`.
- `BlockedReleaseSearchReportsCalibrationError`: positive switch forced active
  and motor blocked.
  - The unchanged target aborts the search; calibration error and
    `CalibrationMode::NONE`.
  - Covers the blocked-move check of `waitForSwitchState()`.
- `StopDuringHomingSearchStopsMotor`: positive switch inactive at the start,
  stop half way into the first chunk.
  - The motor stands still, target equals the actual position, then idle.
  - The error is `Error::CALIBRATION_ERROR` and the mode
    `CalibrationMode::NONE`.
  - Covers the stop abort on the interrupted path.
- `StopDuringReleaseSearchStopsMotor`: positive switch active at the start,
  stop half way into the first negative chunk while the motor is still on its
  way to the chunk target.
  - The motor stands still, ends idle, with calibration error and
    `CalibrationMode::NONE`.
  - Covers the stop abort of the release search, which the blocked case does not
    reach.
- `DisableDuringCalibrationStopsMotor`: disable half way into the first chunk.
  - The motor is stopped and the machine ends in the disabled state.
  - Covers the new `disableEvent` transition and its ordering against the end
    callback.
- `NegativeSwitchBitIgnoredForRotary`: negative switch bit set.
  - The negative reference reports inactive and no both-end-switches error is
    raised.
  - Covers `isEndSwitchActive()`; the bit alone is not a move.
- `BlockedRotaryMoveReportedInterrupted`: a blocked move.
  - Reported as interrupted, not as successful.
  - Covers the dropped negative bit in `verifyMoveAction()`.
- `MoveAwayFromSwitchCompletes`: plain move away from the switch.
  - The move completes, the motor ends idle and the error stays
    `Error::NO_ERROR`.
  - Covers the disjunction of `verifyMoveAction()` while the positive switch is
    inactive, where a conjunction would report an interrupted move.

Fixture premises for the cases above:

- The rotary fixture creates a `RotaryStepperMotor` on the dummy motor
  controller.
- `moveTowardsTarget(1.0)` covers the whole remaining distance in one call, so a
  chunk is only open after a fractional move.
- The dummy reports moving only while motor current is enabled and the target
  differs from the actual position.
- The dummy driver card is shared between the cases and
  `resetInternalStateToDefaults()` does not restore its end-switch positions, so
  the fixture sets both.
- The dummy has no setter for the both-end-switches flag; a fixture helper sets
  it through a zero fraction and documents that this is not a move.
- Not covered: the `HOMING_TIMEOUT_MS` expiry, which outlasts the fixture's 10 s
  wait, and the active-reference exit, which needs a reference to report active
  while the target differs from the actual position.
- No case for the tolerance calculation, which the plan leaves untouched.

### Qt-less configure (new)

- Configure with `-DCMAKE_DISABLE_FIND_PACKAGE_QT=ON` and build.
- The GUI subdirectory is skipped with a status message, the library and the
  tests build.
- Covers the build-without-Qt requirement, which no unit test reaches.

## Deferred issue

- DI-1 [DONE] Class comment of `RotaryStepperMotor`: specify the four points and
  mark the XML example with a fenced code block, so Doxygen keeps it verbatim
  and reports no warning.
- DI-2 [ACCEPTED] The fixture block of the Test plan
  (`tests/src/steppermotor/testRotaryStepperMotor.cc`) says
  `resetInternalStateToDefaults()` does not restore the end-switch positions, so
  the fixture sets both. `RotaryStepperMotorFixture`
  (tests/src/steppermotor/testRotaryStepperMotor.cc:102-104) calls
  `setPositiveEndSwitch()`/`setNegativeEndSwitch()` before
  `resetInternalStateToDefaults()`, but the latter only resets the enable flags
  and not the positions (backend/src/MotorControlerDummy.cc,
  `resetInternalStateToDefaults()`). The stated reasoning is therefore wrong,
  while the resulting fixture is correct and has to stay. The premise should be
  corrected in a later plan revision.
- DI-3 [IMPLEMENTED] Test plan of
  `tests/src/steppermotor/testRotaryStepperMotor.cc` lists the case
  `MoveAwayFromSwitchCompletes` (plain move away from the switch, covers the
  disjunction of `verifyMoveAction()` while the positive switch is inactive),
  naming the test case and its reasoning. The case does not exist:
  `tests/src/steppermotor/testRotaryStepperMotor.cc` defines no such
  `BOOST_AUTO_TEST_CASE` and `./testRotaryStepperMotor --list_content` does not
  show it, so the planned test is missing.
- DI-4 [ACCEPTED] The case names in the test plan and in the code differ for
  several cases whose behaviour is covered:
  `BlockedReleaseSearchReportsCalibrationError` vs
  `BlockedMotorDuringReleaseSearchFailsWithCalibrationError`,
  `NegativeSwitchBitIgnoredForRotary` vs `NegativeSwitchBitIsIgnored`,
  `BlockedRotaryMoveReportedInterrupted` vs `BlockedMoveReportedAsInterrupted`,
  `DisableDuringCalibrationStopsMotor` vs
  `DisableDuringCalibrationStopsMotorAndDisables`. Behaviour and coverage match
  the plan; only the names differ.
- DI-5 [ACCEPTED] Beyond the `MoveAwayFromSwitchCompletes` content, the
  disjunction of `verifyMoveAction()` while the positive switch is inactive is
  covered by `ToleranceCalculationSucceedsAfterHoming`, which the Test plan does
  not list. The other direction of the disjunction (switch bit accepts a move
  that stopped at the switch with target above actual) is not exercised by any
  test, because the plan's cases leave the calibration guard out of scope.
