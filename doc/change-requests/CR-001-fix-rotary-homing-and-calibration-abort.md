# CR-001: Fix rotary stepper motor homing and calibration abort

## Header

- Synopsis: A rotary stage whose reference switch is already active cannot
  calibrate. Calibration can only be aborted by disabling the motor.
- Status: DONE

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
- Disabling during calibration must stop motion. This covers the calibration
  only; disabling during the tolerance calculation is out of scope.
- The motor must end in the disabled state with motor current and end-switch
  power off.

Rotary switch semantics:

- A rotary motor must report the negative end switch as inactive.
- The both-end-switches-active error must never be raised for a rotary motor.
- Move verification must not treat the negative switch bit as a reason to
  accept a move.

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
- `_calculatingTolerance` shares `actionStop()`, so stopping a tolerance
  calculation also commands standstill. Disabling during tolerance calculation
  is out of scope; that state keeps ignoring the disable event.
- Calibration end handling stays as is and reports the abort as a calibration
  error.
- The wait loops which poll `isMotorMoving()` keep two separate periods. The
  end-switch search keeps the existing 500 ms period; the tolerance calculation
  uses a separate 5 ms period, because it polls once per move and its runtime is
  otherwise dominated by that wait.

`ReferenceStepperMotor::isEndSwitchActive(Sign)`:

- Skip the negative switch checks when `hasNegativeReferenceSwitch()` is false.
  The affected checks are the both-end-switches-active error, the tolerance
  check and the `Sign::NEGATIVE` return value.
- `negativeSwitchActive()` still reports the hardware bit. A motor without a
  physical negative switch overrides `hasNegativeReferenceSwitch()` instead of
  the bit itself.
- Update the `RotaryStepperMotor` class comment accordingly.

`RotaryStepperMotor::verifyMoveAction()`:

- Keep the base disjunction: accept the move when
  `isEndSwitchActive(Sign::POSITIVE)` is true or the target position equals the
  actual position.
- Drop only the negative switch bit, so it cannot mask a blocked move.

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
  start): motor stops, state idle.
- Stop during calibration while the release search runs (positive switch active
  at the start): after the first negative chunk target the motor stands still
  and ends idle. This covers the stop half of the release-search abort, while
  the blocked-move case covers the interruption half.
- The dummy advances the motor by one step per `moveTowardsTarget()` and steps
  even a single step to the target at once, so a chunk leaves no usable window
  to stop it. The tests do not step the dummy themselves to hold a chunk open.
  The stop is issued only once the motor is moving and its target differs from
  the chunk start, i.e. the standstill command takes the target of a moving
  motor back. The tests check that before they stop. Leaving the motor alone
  would be a race, so a stop between two chunks is not covered.
- `stopDuringChunkAt()` runs with the dummy still at the chunk start, so a
  position guard would be true on entry and protect nothing. It only waits for a
  moving motor whose target differs from the chunk start before it stops.
- Disable during calibration: motor stops, state disabled, motor current and
  end-switch power off.
- Rotary motor with the negative switch bit set: negative reference inactive,
  no both-switches error.
- Blocked rotary move: reported as interrupted, not as successful.
- Rotary motor, tolerance calculation after a completed calibration: the
  calculation finishes with `Error::NO_ERROR` and the motor ends in the idle
  state. This covers the disjunction in `verifyMoveAction()`, because the
  calculation verifies moves away from the switch where the positive switch is
  inactive, so a conjunction would report an interrupted move.
- Stop during the tolerance calculation after a completed calibration: wait for
  `calculatingTolerance` and for the first move of the calculation, which leaves
  the motor moving, then stop. This test does not need the chunk window above,
  because it waits for the move of the calculation to be running before it
  stops.
- After the stop the motor is not moving and its target equals its actual
  position: the standstill command of the shared `actionStop()` took effect.
- The stopped calibration leaves its iteration on `_stopAction` before issuing
  the second move and settles in the idle state, so the test asserts standstill,
  target equal to actual position and, finally, the idle state.
- The rotary cases use a rotary fixture which creates a `RotaryStepperMotor` on
  the dummy motor controller.
