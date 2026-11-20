# CR-001: Fix rotary stepper motor homing and calibration abort

## Header

- Synopsis: A rotary stage whose reference switch is already active cannot
  calibrate. Calibration can only be aborted by disabling the motor.
- Status: READY TO IMPLEMENT (from DONE)

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

`RotaryStepperMotorStateMachine::performCalibration()`:

- Move off the switch when `isEndSwitchActive(Sign::POSITIVE)` is already true
  at the start.
- Move negatively in chunks of `getOffset()` until the switch releases.
- Then home positively as today.
- The release search detects a blocked motor itself. After each chunk it checks
  whether the target position still equals the position it had before the
  chunk, and sets `_moveInterrupted` when the motor did not move while the
  switch stays active.
- Abort the release search on stop or move interruption.
- Share the homing timeout with `findEndSwitch()`; hoist the timeout constant so
  both use it. Expiry follows the existing interrupted path.

`ReferenceStateMachine`:

- `actionStop()` sets `_stopAction` and commands standstill via
  `actionMovetoStop()`.
- The ordering of the standstill command and the worker's chunk targets comes
  from the caller holding `_motor._mutex` while it dispatches the event
  (`stop()`, `setEnabled()` and `calibrate()` all lock it before
  `setAndProcessUserEvent()`). `actionStop()`, the disable action and
  `actionMovetoStop()` must therefore not lock `_motor._mutex`; it is a
  non-recursive `boost::mutex` and locking it again self-deadlocks.
- The worker re-checks `_stopAction` while holding that mutex immediately before
  issuing each chunk target, so no chunk is issued after a stop has been
  processed.
- The guarantee holds for the release search and for `findEndSwitch()` alike.
- Add a `disableEvent` transition from `_calibrating` to `_disabled`.
- Disable action: set `_stopAction`, command standstill, perform
  `actionDisable()`.
- The transition keeps the end callback of `_calibrating`, so the machine stays
  in `_calibrating` until the worker has finished and cleared
  `_asyncActionActive`. The callback then applies the requested `_disabled`
  state: `actionEndCallback()` moves to the requested state before it performs
  the stop transition, and `_disabled` has no stop transition. An aborted
  calibration can therefore report its error before the disabled state is
  entered.
- An abort which came back without motion is reported as interrupted.
  `performCalibration()` homes only when neither `_stopAction` nor
  `_moveInterrupted` is set after the release search, so `findEndSwitch()`
  cannot overwrite the early abort with a completed homing.
- The background thread only clears `_asyncActionActive`. The requested state is
  applied by the current state's internal end callback on the next state query.
- Calibration end handling stays as is and reports the abort as a calibration
  error.

`ReferenceStepperMotor::isEndSwitchActive(Sign)`:

- Skip the negative switch checks when `hasNegativeReferenceSwitch()` is false.
  The affected checks are the both-end-switches-active error, the tolerance
  check and the `Sign::NEGATIVE` return value.
- `negativeSwitchActive()` still reports the hardware bit. A motor without a
  physical negative switch overrides `hasNegativeReferenceSwitch()` instead of
  the bit itself.
- Update the `RotaryStepperMotor` class comment accordingly.

`RotaryStepperMotor::verifyMoveAction()`:

- Verifies that a completed move ran to its end, not that the motor is
  calibrated.
- Called after every move, from
  `BasicStepperMotor::StateMachine::waitForStandstill()`.
- Keep the base disjunction: accept the move when
  `isEndSwitchActive(Sign::POSITIVE)` is true or the target position equals the
  actual position.
- The switch bit accepts a move which stopped at the switch and therefore left
  the target above the actual position.
- The search targets are full steps, so a completed move has equal target and
  actual positions; no tolerance window is compared.
- Drop only the negative switch bit, so it cannot mask a blocked move.
- A calibration guard and a hardware tolerance stay out of scope.
- Declaration comment in `steppermotor/include/RotaryStepperMotor.h`: name the
  reached target, not "the position matches".

`ConfigCalculator/CMakeLists.txt`:

- Probe Qt once with
  `find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Widgets Gui)` and add
  the `GUI` subdirectory only when `QT_FOUND` is set.
- Skip the GUI with a status message when Qt is missing. The library and its
  tests then configure and build on a host without Qt, while a host with Qt
  keeps building the GUI as before.
- The build settings are reproducible without a manual edit of the tree. No
  library or test code changes.

### Alternatives considered

- Runtime check of `referenceConfigAndRampModeData` in the rotary motor
  constructor: rejected, the switch stop inputs stay a pure XML configuration
  concern (`0x300`).
- Overriding `negativeSwitchActive()` to false: rejected, it hides the hardware
  state.
- Running the internal event callback before returning the state from
  `utility::StateMachine::getCurrentState()`: rejected. For an instantaneous
  action the transition happens inside that callback, so the caller would never
  observe the `moving` state. The one-call delay is intentional. The reorder
  currently in `util/src/StateMachine.cc` and the matching
  `tests/src/steppermotor/testStateMachine.cc` assertions are reverted.

## Test plan

- Rotary motor, positive switch already active: motor moves off, ends in
  `CalibrationMode::FULL`, both calibrated positions equal.
- Rotary motor, positive switch forced to stay active and motor blocked: the
  per-chunk blocked-move check aborts the release search; calibration error,
  `CalibrationMode::NONE`.
- Stop during calibration of the homing search (positive switch inactive at the
  start): the motor is stopped half way into the first chunk, stands still and
  ends idle.
- Stop during calibration while the release search runs (positive switch active
  at the start): the motor is stopped half way into the first negative chunk,
  which releases the switch, while the motor is still on its way to the chunk
  target. It ends idle. This covers the stop half of the release-search abort,
  while the blocked-move case covers the interruption half.
- `moveTowardsTarget(1.0)` completes the whole remaining distance to the target
  in one call and returns without moving when the target equals the actual
  position, so a chunk is only open after a fractional move. The dummy also
  reports a motor as moving as soon as a target differs from its actual
  position, so `isMotorMoving()` in a test means a chunk target was issued, not
  that a step happened.
- A stop between two chunks needs no coverage: the state machine issues the next
  chunk only after the previous one stands still.
- After the stop the tests complete the pending chunk once more and assert the
  position does not change: the standstill command left the target at the actual
  position, so the dummy has nothing to move to.
- Disable during calibration: motor stopped half way into the first chunk, state
  disabled.
- Rotary motor with the negative switch bit set: negative reference inactive,
  no both-switches error. The dummy has no setter for its both-end-switches
  flag, it is only a `moveTowardsTarget()` parameter, so a fixture helper sets
  it with a zero fraction and documents that this is not a move. The fixture
  also sets both end switch positions, because the dummy driver card is shared
  between the test cases and `resetInternalStateToDefaults()` does not restore
  them.
- Blocked rotary move: reported as interrupted, not as successful.
- Rotary motor, plain move away from the switch: the move completes, the motor
  ends idle and the error stays `Error::NO_ERROR`. Covers the disjunction in
  `verifyMoveAction()` where the positive switch is inactive, so a conjunction
  would report an interrupted move.
- The stopped calibration leaves its iteration on `_stopAction` before issuing
  the second move and settles in the idle state, so the test asserts standstill,
  target equal to actual position and, finally, the idle state.
- The rotary cases use a rotary fixture which creates a `RotaryStepperMotor` on
  the dummy motor controller.
- The fixture helpers only step the dummy and return whether the step was
  possible. Every assertion sits in the test case, which wraps the helpers in
  `BOOST_TEST_REQUIRE`.
