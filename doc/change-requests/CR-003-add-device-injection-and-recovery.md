# CR-003: Add device injection and recovery for ApplicationCore use

## Header

- Synopsis: A MotorDriverCard can be created from an ApplicationCore-owned
device and restored after that device has been reopened. Applications no
longer need a second proxy device or a recreated card to recover.
- Status: PLANNED

## Requirements

Device injection:

- Applications can create a card from an open device they own.
- Repeated requests for the same device and module return the same card while
the previously returned card is still alive.
- The library never opens, closes or replaces an injected device.
- The injected device must outlive the card.
- A motor controler must not outlive its card; this holds for both factory
overloads.
- Alias-based creation keeps its current behaviour.
- Dummy mode does not apply to an injected device.

Recovery:

- `isFunctional()` reports whether the underlying device is functional.
- `reconfigure()` restores a card after its device was closed and reopened.
- It re-applies the complete card and motor configuration from the
configuration objects already held by the library.
- It resets the cached runtime state and runtime settings, such as a user
speed limit, to the configured values.
- It leaves the actual position untouched.
- It leaves the motor current disabled unless the caller asks to enable it.
- Repeated calls leave the configured values unchanged.
- The caller does not use the card while a reconfigure runs.
- Existing `MotorControler` references stay valid across a reconfigure.
- It does not open, close or replace the device.
- A failed reconfigure throws; the card must not be used afterwards.
- The dummy card accepts both calls without effect.

## Specifications

### MotorDriverCard (modified) in backend/include/MotorDriverCard.h (modified)

- `[[nodiscard]] virtual bool isFunctional() const = 0;`
- Reports the functional state of the underlying device.
- Returns false while the device is closed or in a known error state.
- Not a card-readiness signal: it can be true while accessors are still
stale after a reopen.
- Serves both injected and alias-created cards, so it stays on the card
interface.
- `virtual void reconfigure(bool enableMotor = false) = 0;`
- Restores the card after the device has been closed and reopened.
- Re-creates every register accessor and re-applies the configuration
stored by the implementation.
- Resets cached runtime state and runtime settings to the configured values.
- Writes the motor-current enable last: enabled when `enableMotor` is true,
disabled otherwise.
- Does not write the actual position.
- Intended for the device recovery path of the owning application.
- Each SPI transaction still serialises through the SPI mutex; the
accessors of one object are replaced atomically.
- The caller must not use the card while it runs, as nothing excludes
concurrent card or controler operations.
- Adds two pure virtual methods: out-of-tree implementations of the
interface must implement them.

### MotorDriverCardImpl (modified) in backend/{include/impl/MotorDriverCardImpl.h,src/MotorDriverCardImpl.cc} (modified)

- `boost::shared_ptr<ChimeraTK::Device> _ownedDevice;`
- Holds the device for cards created by the alias-based factory overload.
- Empty for cards created from an injected device.
- Declared first so the device outlives every member that uses it.
- `ChimeraTK::Device* _device` replaces the owning shared pointer.
- Non-owning; valid for the lifetime of the card.
- New private constructor
`MotorDriverCardImpl(ChimeraTK::Device& device, std::string const& moduleName, MotorDriverCardConfig const& cardConfiguration)`.
- Non-owning; used for injected devices and by the owning constructor.
- Existing constructor taking `boost::shared_ptr<ChimeraTK::Device>`.
- Delegates to the new constructor and keeps the shared pointer in
`_ownedDevice` afterwards.
- The caller's shared pointer keeps the device alive during the delegated
configuration, which is why the member can be assigned after delegation.
- Preserves the behaviour of the alias-based factory overload.
- `MotorDriverCardConfig _config;`
- Copy of the configuration passed by the factory, used by `reconfigure()`.
- `[[nodiscard]] bool isFunctional() const override;`
- Delegates to `_device->isFunctional()`.
- `void reconfigure(bool enableMotor = false) override;`
- Locks a new `std::mutex _reconfigureMutex`.
- Re-checks the firmware version.
- Re-creates `_controlerStatusRegister`.
- Calls `_controlerSPI->reconnect()`.
- Re-applies the card-wide configuration with `applyConfiguration()`.
- Calls `reconfigure(enableMotor)` on every motor controler.
- New private `void applyConfiguration();`
- Holds the card-wide register writes currently in the constructor.
- Used by the constructor and by `reconfigure()`.
- `_motorControlers` becomes
`std::vector<boost::shared_ptr<MotorControlerImpl>>`.
- Lets `reconfigure()` call the implementation method.
- `getMotorControler()` returns the upcast pointer; existing pointers stay
valid.
- `MotorControlerImpl` is only forward-declared in the header; the
constructor and the destructor move to the source file, so the include
cycle with `MotorControlerImpl.h` stays out of the header.
- `getStatusWord()` also locks `_reconfigureMutex`.
- It uses the re-created status accessor while a reconfigure may replace it.

### MotorControlerImpl (modified) in backend/{include/impl/MotorControlerImpl.h,src/MotorControlerImpl.cc} (modified)

- Constructors take `ChimeraTK::Device&` instead of a shared pointer.
- `boost::shared_ptr<ChimeraTK::Device> _device` becomes the non-owning
`ChimeraTK::Device* _device`; the class comment about keeping the device
alive is dropped.
- Non-owning, so a `MotorControler` must not outlive its card, for both
factory overloads.
- This differs from the current class, which copies the device shared
pointer and so keeps the device alive past the card.
- New `std::string const _moduleName;`
- Stored for accessor re-creation.
- New public `void reconfigure(bool enableMotor);`
- Not part of the `MotorControler` interface; called by the card only.
- `reconfigure()`:
- Locks `_mutex`.
- Re-creates all scalar accessors from `*_device` and `_moduleName`,
including the optional end-switch power accessor.
- Calls `_driverSPI.reconnect()`.
- Re-applies `_controlerConfig` through the shared `applyConfiguration()`.
- Resets `_currentVmax` to the configured maximum velocity.
- Resets `_usrSetCurrentScale` to the configured current scale.
- Clears `_moveOnlyFullStep`.
- Recomputes `_userMicroStepSize`.
- Reads `_localTargetPosition` from the target position register after the
configuration is applied.
- Writes the motor-current enable last: enabled when `enableMotor` is true,
disabled otherwise.
- New private `void applyConfiguration(bool enableMotor);`
- Holds the configuration block currently in the constructor, ending with
the motor-current enable.
- Writes the registers through new lock-free private helpers, because the
configuration block currently calls the public setters, which each lock the
non-recursive `_mutex`.
- `reconfigure()` holds `_mutex`, so it must not go through the public setters.
The constructor does not yet share the object, so the missing locks are
harmless there as well.
- The public setters stay unchanged; they are no longer used by the
configuration path.
- The constructor passes the configured enable flag, so construction is
unchanged.
- `reconfigure()` passes its argument.
- Accessor creation moves into a private helper shared by the constructor and
`reconfigure()`.
- Keeps the current handling of the optional end-switch power register for
old firmware.

### SPIviaPCIe (modified) in backend/{include/impl/SPIviaPCIe.h,src/SPIviaPCIe.cc} (modified)

- Constructors take `ChimeraTK::Device&` instead of a shared pointer.
- New non-owning `ChimeraTK::Device* _device;`.
- The class keeps no device reference today, so the member is new rather than
stored. Both the class comment and the comment in the private section are
replaced.
- New `std::string _readbackRegisterName;` stores the readback name for
re-creation.
- Standalone users must keep the device alive; the SPI tests do.
- New `void reconnect();`
- Locks `_spiMutex`.
- Re-creates the write, synchronisation and readback accessors from the
stored names and device.
- Passes `{ChimeraTK::AccessMode::raw}` for each accessor, as the constructors
do. Without the flag the re-created accessors would get the default access
mode and the SPI transfers would change.
- Holds the mutex across the whole replacement, so concurrent `read()` and
`write()` calls see either the old or the new accessor set.

### TMC429SPI (modified) in backend/{include/impl/TMC429SPI.h,src/TMC429SPI.cc} (modified)

- Constructor takes `ChimeraTK::Device&`.
- New `void reconnect();`
- Delegates to `_spiViaPCIe.reconnect()`.

### MotorDriverCardDummy (modified) in backend/{include/MotorDriverCardDummy.h,src/MotorDriverCardDummy.cc} (modified)

- `[[nodiscard]] bool isFunctional() const override;`
- Returns true.
- `void reconfigure(bool enableMotor = false) override;`
- Does nothing.

### MotorDriverCardFactory (modified) in backend/{include/MotorDriverCardFactory.h,src/MotorDriverCardFactory.cc} (modified)

- New overload
`boost::shared_ptr<MotorDriverCard> createMotorDriverCard(ChimeraTK::Device& device, std::string mapModuleName, std::string motorConfigFileName);`
- Builds the card on the supplied open device.
- Never opens, closes or replaces the device.
- Reads the configuration file only when it creates a new card.
- The device must outlive the returned card.
- `ChimeraTK::Device` is forward-declared in the header.
- The header stays free of deviceaccess includes, as for the other
overloads.
- New cache `std::map<std::pair<ChimeraTK::Device*, std::string>, boost::weak_ptr<MotorDriverCard>> _deviceMotorDriverCards;`
- Key is the device address and the module name.
- Returns the cached card while it is alive.
- Guarded by the existing `_factoryMutex`.
- The one-card-per-device guarantee is per device object: mixing the alias
and the injected overload for the same alias is unsupported.
- A stale entry needs a destroyed device; the device must outlive the card,
so the address cannot be reused while the card is alive.
- The overload ignores dummy mode.
- A caller that passes a device has already chosen its backend.
- The existing overload is unchanged.
- It still opens and owns its device through the owning implementation
constructor.

### Alternatives considered

- An overload taking `boost::shared_ptr<ChimeraTK::Device>`: rejected,
`DeviceManager` exposes only `Device&`.
- An injected overload without caching: rejected, two cards would have
independent SPI mutexes on one device.
- Dropping and recreating the card for recovery: rejected, it invalidates
`MotorControler` pointers held by application modules and re-reads the
configuration file.
- A non-owning `shared_ptr` internally: rejected, it hides the lifetime
contract in a reference count that never dies.
- Re-applying the configured enable flag during recovery: rejected, it could
energise a motor whose position is unknown after a device reset.
- Separate public `reconnect()` and `reconfigure()` methods: rejected, callers
would have an ordering rule to get wrong.
- A card-wide lock held by every card and controler operation: rejected, it
would put a nesting order across the whole library. Recovery runs from the
application's initialisation handler, when nothing else uses the card.
- One cache for both overloads: rejected, the owning overload would have to
return a card whose device it does not own.

## Test plan

- `tests/src/backend/testMotorDriverCardFactory.cpp` (modified)
- `CreateFromDeviceReturnsTheSameCard`: two calls with one open device and
one module return the same pointer, a second module returns a different one.
- Covers the cache of the injected overload and the one-card-per-device
guarantee behind the SPI mutex.
- `CreateFromDeviceIgnoresDummyMode`: dummy mode set, injected creation
returns a real card that works on the dummy backend.
- Covers the dummy-mode exclusion; the alias overload is the only tested
dummy-mode path today.
- `CreateFromDeviceKeepsItOpen`: the device is open after creation and after
the card is released.
- Covers the no-open/no-close requirement; the recovery tests reopen
explicitly and would not notice a close.
- `tests/src/backend/testMotorDriverCardRecovery.cpp` (new)
- Creates its cards through the injected factory overload, so the private
implementation constructor needs no test friend.
- `ReconfigureRestoresConfiguration`: a configured value read back, reset by
close and open, restored by `reconfigure()`.
- Covers accessor re-creation and configuration re-application.
- `ReconfigureKeepsStatusWordReadable`: `getStatusWord()` through a
`dynamic_pointer_cast<MotorDriverCardImpl>` works after a reopen and
`reconfigure()`.
- `getStatusWord()` is a public member of the implementation only, not of
`MotorDriverCard` or `MotorDriverCardExpert`, so the cast target is the
implementation class.
- Covers the re-created `_controlerStatusRegister` and its locking through
`_reconfigureMutex`; no other case reads the status word after a
reconfigure.
- `ReconfigureLeavesMotorDisabledByDefault`: the configuration enables the
motors; after reopen and `reconfigure()` the motor current is disabled, and
`reconfigure(true)` enables it.
- Covers the enable rule; not covered elsewhere because construction is
the only current path that writes the enable register.
- `ReconfigureKeepsControlerInstances`: the controler pointer and ID are
unchanged across `reconfigure()`.
- Covers the requirement that application-held controler pointers stay
valid.
- `ReconfigureResetsRuntimeState`: a user speed limit different from the
configured maximum, reset by `reconfigure()`.
- Covers the `_currentVmax` reset; configuration re-application alone
would not update the cached limit.
- `ReconfigureOnClosedDeviceThrows`: `reconfigure()` on a closed device
throws, and a later reopen plus `reconfigure()` succeeds.
- Covers the failure reporting; no current test provokes a failure inside
a card call.
- `RepeatedReconfigureLeavesConfiguredValues`: a second call after a reopen
leaves the configured values unchanged.
- Covers the repeated-call requirement; it is the case an initialisation
handler produces when it also runs on the first open.
- `IsFunctionalFollowsTheDevice`: true while open, false while closed, true
after reopen.
- Covers the delegation to the device; the card has no other health
signal.
- `tests/src/backend/testTMC429SPI.cc` (modified)
- Existing cases construct the SPI from a shared pointer; they pass
`*_mappedDevice` after the constructor change.
- Covers the signature change; without it the file does not compile, and
`testSPIviaPCIe.cpp` alone would not show the break.
- `tests/src/backend/testSPIviaPCIe.cpp` (modified)
- `ReconnectAfterDeviceReopen`: use the SPI, close and reopen the device,
`reconnect()`, use it again.
- Covers `reconnect()` directly; the recovery tests would also fail on a
broken reconnect, but not localise it.
- `tests/src/backend/testMotorDriverCardDummy.cpp` (modified)
- `DummyIsFunctionalAndAcceptsReconfigure`: `isFunctional()` is true and both
`reconfigure()` calls leave the controlers intact.
- Covers the dummy implementation; applications and tests switch to the
dummy through the factory, so the methods must exist and be harmless.

## Deferred issue

- DI-1 [FOLDED-IN] Losing the controler's device ownership silently breaks a
  currently working use: `MotorControlerImpl::_device` becomes a non-owning
  `ChimeraTK::Device*` (CR lines 121-125), while only the card owns the device,
  and only for alias-created cards (`_ownedDevice`). Today `MotorControlerImpl`
  copies the device shared pointer, so a controler outliving its card keeps the
  device alive and stays usable (documented at
  `backend/include/impl/MotorControlerImpl.h:23-27`); after the change such a
  controler holds a dangling pointer (use-after-free) that no requirement,
  alternative or test mentions. The stated justification 'uniform with the card
  and the SPI, which hold no ownership either' is wrong for the card, which does
  own the device. Either keep shared ownership in the controler, or state the
  new lifetime rule (a `MotorControler` must not outlive its card, for both
  factory overloads) in the requirements and specifications.
- DI-2 [FOLDED-IN] Requirement 'Repeated requests for the same device and module
  return the same card' (CR line 15) is unqualified and cannot be met by the
  planned `weak_ptr` cache, which returns the cached card only while it is alive
  (CR lines 225-232, test at line 267). A cache that always returns the same
  card would keep device and card alive forever, which the alternatives reject.
  Qualify the requirement (e.g. 'while the previously returned card is still
  alive').
- DI-3 [FOLDED-IN] Test plan does not cover
  `MotorDriverCardImpl::getStatusWord()` after a reconfigure, although the plan
  re-creates `_controlerStatusRegister` and adds `_reconfigureMutex` locking
  specifically for it (CR lines 100-101, 116-117). Add a case (or extend
  `ReconfigureRestoresConfiguration`) that reads the status word after
  `reconfigure()`, e.g. via `dynamic_pointer_cast<MotorDriverCardExpert>`, as
  `tests/src/backend/testMotorDriverCard.cpp:221` already does.
- DI-4 [NEW] DI-3 fold is technically wrong: the test plan reaches
  getStatusWord() through boost::dynamic_pointer_cast<MotorDriverCardExpert>
  (CR:246-248), but getStatusWord() is declared only on MotorDriverCardImpl
  (backend/include/impl/MotorDriverCardImpl.h:61). Neither MotorDriverCardExpert
  (backend/include/MotorDriverCardExpert.h:11-43) nor MotorDriverCard
  (backend/include/MotorDriverCard.h:16-28) declares it, so the cast yields a
  type without the method and the test cannot compile. The existing test reaches
  it through a shared_ptr<MotorDriverCardImpl>
  (tests/src/backend/testMotorDriverCard.cpp:44-62,221), which also works for
  the injected factory overload because the tests already include the impl
  header.
- DI-5 [NEW] Contradiction between the planned locking and the extracted
  configuration block in MotorControlerImpl: reconfigure() locks _mutex (CR:125)
  and re-applies the configuration through applyConfiguration() (CR:129), which
  is specified as 'Lock-free, like the existing private setters: the caller
  holds _mutex' (CR:138-142). The configuration block in the constructor
  (backend/src/MotorControlerImpl.cc:104-127) does not use private setters: it
  calls the public setters, and each of them locks the non-recursive std::mutex
  _mutex (e.g. setMotorCurrentEnabled at
  backend/src/MotorControlerImpl.cc:566-570, reached through setEnabled at
  230-233; the DEFINE_SET_GET_* macros at 25-65 lock too). Calling that block
  while reconfigure() holds _mutex self-deadlocks. The plan must state that the
  block is rewritten to write through lock-free private helpers (or that _mutex
  becomes recursive, or that reconfigure() does not hold it), and drop the
  incorrect claim that private setters already exist for these writes.
- DI-6 [NEW] SPIviaPCIe::reconnect() is under-specified in a way that changes
  behaviour: all three accessors are currently created with
  {ChimeraTK::AccessMode::raw} (backend/src/SPIviaPCIe.cc:26-31), but the plan
  only says they are re-created 'from the stored names and device' (CR:159-164),
  so a naive implementation would get default-access-mode accessors. Also
  SPIviaPCIe holds no device pointer today
  (backend/include/impl/SPIviaPCIe.h:68) - the member is new, not 'stored' - and
  both class comments claim the class keeps no device reference
  (backend/include/impl/SPIviaPCIe.h:29-32 and 68), so the plan should name both
  comments to update.
