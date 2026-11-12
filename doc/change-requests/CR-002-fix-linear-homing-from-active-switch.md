# CR-002: Fix linear stepper motor homing from an active switch

## Header

- Synopsis: A linear stage whose positive reference switch is already active
records its current position as the calibrated switch position. An end-switch
search that never finds its switch runs without end.
- Depends on: CR-001
- Status: READY TO TEST (from IMPLEMENTATION REVIEWED)

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
is zero. The positive search ends on the switch, because its chunk target is
the switch position. The calibration measures the positive reference from the
negative switch, so the reference is the distance between the two switch
positions.
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
switch then releases.
- Linear search which does not find its switch within the timeout: a test
subclass returns a timeout shorter than the time the test needs to finish the
first chunk, and the positive switch is placed at a reachable position beyond
that chunk. The calibration thread issues the first chunk and waits for it,
the test advances the dummy to the chunk target, and the next iteration of the
search finds the timeout expired, sets `_moveInterrupted` and aborts the
calibration with a calibration error and `CalibrationMode::NONE`. A blocked
motor is not used, because the blocked-move check would fire first, and no
dummy placement keeps the positive switch inactive while the search keeps
issuing chunks. The search runs on the calibration thread, so the test only
advances the dummy and needs no thread of its own.
- The rotary homing cases must keep passing unchanged against the shared
release search.

## Deferred issue

- DI-1 [DONE] Requirements: no bullet requires the linear calibration to fail when the switch does not release, although the specification (line 38, 'the caller decides on the abort') and test plan item 2 (lines 76-77) both depend on it; CR-001 carries that requirement for the rotary stage, so its omission here leaves test plan item 2 without a requirement basis
- DI-2 [DONE] Specifications line 41-42: the contract of the new `homingTimedOut(start)` helper is ambiguous. Because every search loop relies on `_moveInterrupted` to reach the error branch, a helper that only returns true would let the caller return from a timed-out search silently, and the calibration would then report success with a bogus switch position. The plan must state that the helper sets `_moveInterrupted` as the replaced inline rotary check does
- DI-3 [REPLACED] Test plan line 78-80: the timeout case is not executable as written. A blocked motor cannot be used, because the blocked-move check (`LinearStepperMotorStateMachine.cc:80-84`) fires before the timeout; the motor must keep moving with the switch out of reach, and the loop must be stepped from a separate thread while the test subclass calls it, since `moveToEndSwitch()` blocks until the dummy has been hand-stepped
- DI-4 [DONE] Test plan item 3 cannot be implemented as written. Injecting a test subclass that shortens getHomingTimeoutInMilliseconds() requires access to BasicStepperMotor::_stateMachine, which is only possible through a new friend declaration; I added `friend class ::LinearStepperMotorHomingTestFixture;` and the forward declaration at steppermotor/include/BasicStepperMotor.h:23 and :417, i.e. a production-header change not covered by the change request. The plan must either include that change or find another route.
- DI-5 [DONE] Test plan item 3 is unrealisable against the current implementation: LinearStepperMotorStateMachine::findEndSwitch() (steppermotor/src/LinearStepperMotorStateMachine.cc:80-97) keeps a search going while either reference switch is active, so with the dummy placed as the test needs it the positive homing search never passes its first chunk and the timeout is never reached. The plan assumes the search runs on while only the sign under search is inactive.
- DI-6 [DONE] Test plan item 2 (switch does not release) cannot end in CALIBRATION_ERROR with the dummy: every placement in which the negative switch also reads active raises Error::BOTH_END_SWITCHES_ON and the errorEvent inside ReferenceStepperMotor::isEndSwitchActive (steppermotor/src/ReferenceStepperMotor.cc:229-232), so the calibration leaves with error 4, not 2. The plan needs a switch simulation that does not involve the negative switch.
- DI-7 [DONE] The change request specification states the linear homing timeout is 600 s, while steppermotor/include/LinearStepperMotorStateMachine.h:21 returns 600000 (milliseconds, i.e. 600 s by value). The units of the specification and the code have to be made consistent before tests pin the value down.
- DI-8 [DONE] Current test state: tests/src/steppermotor/testLinearStepperMotorHoming.cc (new, untracked) compiles without warnings; HomingFromActiveSwitchMovesOffFirst passes, the other four cases fail for the reasons above. Sources are otherwise clean (no leftover instrumentation); unrelated stale copy sources/mtca4u-MotorDriverCard-maki-tmp was deleted.
- DI-9 [DONE] Specifications do not cover the test access needed by test plan item 3 (DI-4): replacing the motor state machine to shorten the homing timeout needs access to the private `BasicStepperMotor::_stateMachine`, and `initStateMachine()` is private as well, so no public route exists. The working tree already carries the test-only `friend class ::LinearStepperMotorHomingTestFixture;` plus its forward declaration (steppermotor/include/BasicStepperMotor.h:23 and :417). The plan must either list that production-header change or name another route.
- DI-10 [DONE] Specifications are wrong about the release search's abort paths: `moveOffEndSwitch()` is documented to return only on stop or move interruption, but the implementation also aborts it on the homing timeout (steppermotor/src/ReferenceStateMachine.cc:125). Without that, the requirement case 'the switch does not release although the motor moves' has no specification at all.
- DI-11 [DONE] Specifications mix the timeout units (DI-7): the linear accessor is documented as returning 600 s while `getHomingTimeoutInMilliseconds()` returns 600000 ms (steppermotor/include/LinearStepperMotorStateMachine.h:21). The text must give 600000 ms (600 s) so the accessor, the value and the code agree.
- DI-12 [DONE] Test plan item 2 is not expressible with the dummy (DI-6): 'positive switch forced to stay active' has no dummy counterpart. The switch stays active only because the motor cannot move, which is the blocked-move check of moveOffEndSwitch(). The dummy's only stuck-switch mode (bothEndSwitchesAlwaysOn) makes the negative switch active too and raises Error::BOTH_END_SWITCHES_ON (steppermotor/src/ReferenceStepperMotor.cc:229-232), not the specified CALIBRATION_ERROR. The item must say that it relies on the blocked motor only.
- DI-13 [DONE] Test plan item 3's timeout scenario is not realisable as written (DI-5): the dummy resolves its switches from the hardware position and clamps every move at a switch (backend/src/MotorControlerDummy.cc:297-301, :359-363, :411-423), so no placement keeps the positive switch inactive while the search keeps issuing chunks; a chunk either reaches the positive switch or is clamped at the other switch. The timeout is reachable by giving the test subclass a timeout shorter than the time the test needs to finish the first chunk, so the next loop iteration aborts with `_moveInterrupted` set. The item also still asks for a separate thread stepping the loop, whereas the search already runs on the calibration thread and the test only advances the dummy.
- DI-14 [DONE] tests/src/steppermotor/testLinearStepperMotorHoming.cc:7-26: the include block and using-directives are out of order. The boost headers come before the project headers, the standard library headers sit between them, and `using namespace boost::unit_test_framework;` (line 16) is placed inside the include block. The sibling test tests/src/steppermotor/testRotaryStepperMotor.cc:8-25 puts the project headers first, then boost, then the standard library, followed by both using-directives. Move the using-directive below all includes and order the include groups accordingly.
- DI-15 [DONE] tests/src/steppermotor/testLinearStepperMotorHoming.cc:29: `static const std::string moduleName("");` triggers readability-redundant-string-init; use the plain declaration `static const std::string moduleName;` as in tests/src/steppermotor/testRotaryStepperMotor.cc:24 (no linter warnings allowed).
- DI-16 [DONE] getHomingTimeoutInMilliseconds() is a pure query but is not marked [[nodiscard]] (clang-tidy modernize-use-nodiscard at tests/src/steppermotor/testLinearStepperMotorHoming.cc:64). Mark the declaration in steppermotor/include/ReferenceStateMachine.h:61 and the override in steppermotor/include/LinearStepperMotorStateMachine.h the same way as the query accessors in steppermotor/include/StepperMotor.h.
- DI-17 [DONE] tests/src/steppermotor/testLinearStepperMotorHoming.cc:59: `ShortTimeoutLinearStepperMotorStateMachine::HOMING_TIMEOUT_MS = 100` hides the inherited `ReferenceStateMachine::HOMING_TIMEOUT_MS = 60000` with a different value, so the same name means two different timeouts in the hierarchy. Rename the test constant (e.g. SHORT_HOMING_TIMEOUT_MS).
- DI-18 [DONE] tests/src/steppermotor/testLinearStepperMotorHoming.cc:48: `bool _motorBlocked = false;` is a mutable flag in the anonymous namespace but carries the private-member `_` prefix, which the naming rule reserves for protected/private data members; a namespace-scope identifier starting with `_` is also in reserved-identifier territory. Every use (lines 133, 148, 210, 283) sits in a member function of `LinearStepperMotorHomingTestFixture`, so make it a data member of that fixture (declared with the other members next to `_stepperMotor`) and drop the file-global state.
- DI-19 [IMPLEMENTED] tests/src/steppermotor/testLinearStepperMotorHoming.cc:293-321 (`MovingMotorWhichDoesNotReleaseTheSwitchFailsWithCalibrationError`): the case does not exercise the scenario it documents. With the positive switch at `UNREACHABLE_SWITCH_POSITION` (400000) and the motor hand-driven onto it, the release search's first chunk targets actual-50000, so the dummy moves to 350000 and the switch releases (`MotorControlerDummy::moveTowardsTarget` clamps only a target strictly beyond the switch). The calibration then ends the positive search on the switch and aborts in the *negative* search, because the injected 100 ms timeout is shorter than the ~500 ms per-chunk wait in `moveToEndSwitch()`. The test therefore passes even if the release search is broken and gives no coverage of 'the switch does not release'; its comment ('a switch which never releases, as its position is behind both ends of the travel') has no counterpart in the dummy. The release-search timeout abort is not expressible with the dummy (a switch follows the hardware position; the blocked-move check fires first, see DI-12/DI-13), so delete the case; the blocked-motor case already covers the requirement.
- DI-20 [IMPLEMENTED] tests/src/steppermotor/testLinearStepperMotorHoming.cc:249-259 and :267 (`PositiveReferenceIsTheApproachedPosition`): the rationale is wrong and the expected value is expressed misleadingly. The positive search ends exactly on the switch (its chunk target is the switch position), not 'one chunk past the switch'; the reference is 250000 because the calibration measures it from the negative switch at -50000, i.e. it is the switch-to-switch distance `POSITIVE_SWITCH_POSITION - NEGATIVE_SWITCH_POSITION`, not `POSITIVE_SWITCH_POSITION + SEARCH_OFFSET`. The assertion happens to be correct, so state the real reason and use the difference expression. The same wrong claim is in the change request test plan (lines 99-102), see doc_updates.
- DI-21 [IMPLEMENTED] tests/src/steppermotor/testLinearStepperMotorHoming.cc:323-330 and :38-39 (`HomingSearchWhichDoesNotFindItsSwitchAborts`, `UNREACHABLE_SWITCH_POSITION`): the comments contradict the mechanism. Both switches (at +/-400000) are reachable and the searches would find them within 8 to 16 chunks; the abort comes solely from the 100 ms timeout being shorter than the ~500 ms chunk wait, exactly as the test plan item 3 describes. Rephrase the comment to that mechanism and give the constant a name matching its two uses (reachable switch position beyond the first chunk), since the release case at :305 uses it for a switch that is reached and released.
