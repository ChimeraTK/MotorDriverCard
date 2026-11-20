# CR-004: Accept an external logger in the library

Synopsis: The library accepts a caller-supplied logger with the levels `debug()`, `info()`, `warn()` and `error()`,
each taking a topic and returning a writable `std::ostream&`.
The caller's logger type is accepted through a template and a concept; inside the library one concrete handle class
holds it.

Status: PLANNED

## Requirements

The library reports its lifecycle and its own error sites through a logger that the caller owns.

- Accept any object with `debug()`, `info()`, `warn()` and `error()`.
  - Each method takes a `std::string` topic.
  - A topic is a short string naming the message source; its layout is given under Topic convention.
  - Each method returns `std::ostream&`.
- Accept the object without inheritance: no pure virtual interface, no base class to derive from.
- Pass an lvalue logger; the handle is non-owning.
- Require the logger to outlive every object it was handed to.
  - A dummy-mode card lives in the factory cache for the whole process, so its logger must be process-lived.
- Accept the logger at the two entry points only:
  - `MotorDriverCardFactory::createMotorDriverCard()`;
  - `StepperMotorParameters`.
  - The library forwards it from there to the classes it constructs; those constructors are not user entry points.
- Keep the existing calls compiling: the logger argument and member are optional, and default to an unbound handle.
- Allow calls from any thread.
  - The library adds no locking and no shared mutable state around the logger.
  - Library calls into the logger may happen while the library holds its own mutexes.
  - Reentrancy means that a logger method may log through the same handle; it must not call back into the card.
- Emit diagnostics at lifecycle points and at the error sites of the classes listed in Specifications.
  - No per-register logging.
- Compile the library and its public headers as C++23.
- Accept the ABI change of `StepperMotorParameters`, which grows by one member.

## Specifications

### Logger (new)

Location: `util/include/Logger.h` (new)

- Namespace `ChimeraTK::MotorDriver`.
  - Canonical include path `ChimeraTK/MotorDriverCard/Logger.h`; the existing install rule also exposes it under
    `mtca4u/MotorDriverCard/`.
  - `util` becomes the shared base layer used by both `mtca4u` and `ChimeraTK::MotorDriver` code.
- `template<typename L> concept LoggerLike`
  - Satisfied when an lvalue of `L` provides `debug`, `info`, `warn` and `error`.
  - Each takes `std::string` and returns exactly `std::ostream&`.
  - The concept is the only contract; the logger type stays unknown to the library.
- `class Logger`
  - Concrete, non-virtual, non-owning handle.
  - Manual type erasure: one `void*` for the logger object and four function pointers of type
    `std::ostream& (*)(void*, std::string)`.
  - `Logger() = default`
    - Unbound handle.
    - Calling `debug()`, `info()`, `warn()` or `error()` on it returns a stream that discards everything: a
      function-local `thread_local` `std::ostream` over a `std::streambuf` whose `overflow()` drops the character.
    - The stream and its buffer are heap-allocated once per thread and intentionally never destroyed.
  - `template<LoggerLike L> Logger(L& logger)`
    - Stores the logger address and instantiates the four forwarders.
    - Not `explicit`, so a logger object converts where a `Logger` is expected.
    - Constrained to reject `L = Logger`, so copies bind to the copy constructor.
  - Copy and move defaulted.
    - Copies refer to the same external logger.
  - `std::ostream& debug(std::string topic) const`
    - Same for `info`, `warn` and `error`.
    - Forwards object, topic and the returned stream reference; no other work.
  - Thread safety: the handle is immutable after construction and the calls share no state.
  - Reentrancy: a logger method may log through the same handle.
  - The handle never calls the logger during construction or destruction.
- Topic convention: the component path, built only from data the class holds.
  - `MotorDriverCardFactory`
  - `MotorDriverCard/<moduleName>`, and `MotorDriverCard/dummy` for the dummy card
  - `MotorControler/<id>`
  - `SPI/<moduleName>/<writeRegisterName>`
  - `StepperMotor/<deviceName>/<moduleName>/<driverId>`

### StepperMotorParameters (modified)

Location: `steppermotor/include/StepperMotor.h` (modified)

- `Logger logger{}`
  - Last member; defaults to the unbound handle, so existing aggregate initialisations stay valid.

### MotorDriverCardFactory (modified)

Location: `backend/include/MotorDriverCardFactory.h` (modified), `backend/src/MotorDriverCardFactory.cc` (modified)

- `createMotorDriverCard(std::string alias, std::string mapModuleName, std::string motorConfigFileName, Logger logger = {})`
  - New trailing parameter.
  - Binds the logger into the created `MotorDriverCardImpl` or `MotorDriverCardDummy`.
  - The card is cached per `(alias, mapModuleName)`; the logger of the first creation stays bound to it.
    - Later requests for the same card ignore their logger argument.
    - The cache-hit message goes to the bound logger, not to the ignored argument.
    - Documented in the function comment together with the lifetime rule.
  - Messages, topic `MotorDriverCardFactory`:
    - `debug` on a cache hit and in dummy mode.
    - `info` with alias, map module name and config file name before creating a card.
    - `info` after opening the device.

### MotorDriverCardImpl (modified)

Location: `backend/include/impl/MotorDriverCardImpl.h` (modified), `backend/src/MotorDriverCardImpl.cc` (modified)

- Constructor gains `Logger logger = {}`.
  - Stores it and passes it to `TMC429SPI` and `MotorControlerImpl`.
- Messages, topic `MotorDriverCard/<moduleName>`:
  - `debug` with the accepted firmware version in `checkFirmwareVersion()`.
  - `error` with the error text, then throw `ChimeraTK::runtime_error`, on a wrong firmware version.
  - `info` after the common registers and the motors are initialised.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, on an invalid `motorControlerID`.
  - `info` in `powerDown()`.

### MotorControlerImpl (modified)

Location: `backend/include/impl/MotorControlerImpl.h` (modified), `backend/src/MotorControlerImpl.cc` (modified)

- Constructor gains `Logger logger = {}`.
  - Stores it and passes it to the `_driverSPI` channel it constructs.
  - `_controlerSPI` is constructed by the card and already carries the logger.
- Messages, topic `MotorControler/<id>`:
  - `debug` after the configuration registers are written.
  - `warn` when the end switch power indicator accessor is not initialised (old firmware).
  - The silent early returns of `setEndSwitchPowerEnabled()` and `isEndSwitchPowerEnabled()` stay silent; the
    constructor message is the one place reporting the condition.

### TMC429SPI (modified)

Location: `backend/include/impl/TMC429SPI.h` (modified), `backend/src/TMC429SPI.cc` (modified)

- Constructor gains `Logger logger = {}`.
  - Forwards it to the `SPIviaPCIe` member; no messages of its own.

### SPIviaPCIe (modified)

Location: `backend/include/impl/SPIviaPCIe.h` (modified), `backend/src/SPIviaPCIe.cc` (modified)

- Both constructors gain `Logger logger = {}` after `spiWaitingTime`.
  - The write-only overload forwards it to the delegating constructor.
- Messages, topic `SPI/<moduleName>/<writeRegisterName>`:
  - `error` with the existing error details, then throw `ChimeraTK::runtime_error`, for the timeout and the error
    handshake result.
  - No message for a successful or a repeated handshake.

### MotorDriverCardDummy (modified)

Location: `backend/include/MotorDriverCardDummy.h` (modified), `backend/src/MotorDriverCardDummy.cc` (modified)

- Constructor gains `Logger logger = {}`.
- Messages, topic `MotorDriverCard/dummy`:
  - `debug` on creation.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, on an invalid `motorControlerID` and in
    `getPowerMonitor()`.
- `MotorControlerDummy` stays without a logger; its not-implemented throws are unchanged.

### BasicStepperMotor (modified)

Location: `steppermotor/include/BasicStepperMotor.h` (modified), `steppermotor/src/BasicStepperMotor.cc` (modified)

- Constructor passes `parameters.logger` to `createMotorDriverCard()` and stores it.
  - The derived motors and `StepperMotor::StepperMotorFactory::create()` pass the struct through unchanged.
- Messages, topic `StepperMotor/<deviceName>/<moduleName>/<driverId>`:
  - `info` after the motor is initialised.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, in each of the routines not available for this
    motor.

### CMake configuration (modified)

Location: `CMakeLists.txt` (modified)

- `cmake_minimum_required(VERSION 3.20)`
  - Required for `cxx_std_23` and `CMAKE_CXX_STANDARD 23`.
- `set(CMAKE_CXX_STANDARD 23)` and `set(CMAKE_CXX_STANDARD_REQUIRED ON)` before the targets.
  - Also compiles `ConfigCalculator`, the examples and the test targets as C++23.
- `target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)`
  - Propagates the standard to consuming projects, which include the public `Logger.h`.
- One test loop for all `tests/src/*` subdirectories.
  - Iterate `backend steppermotor util`, link the union of the current link libraries and add
    `steppermotor/include` to every test target.
  - Replaces the two near-identical loops; adds `tests/src/util`.

### Alternatives considered

- Pure virtual `LoggerInterface`: requires the caller to derive from a library class, which the requirements forbid.
- Templating every library class on the logger type: makes the whole implementation header-only, breaks the shared
  library ABI and the singleton factory cache, which must return one card per device whatever logger type created it.
- Owning the logger through a `std::shared_ptr`: the caller cannot easily supply an object it already owns by
  reference, and lifetime ownership is not part of the requirement.
- `std::function` per level instead of function pointers: allocates and adds an indirection per call for no gain,
  because the handle is captured for the caller's lifetime anyway.
- Library-owned logger with an `std::ostream` sink: the caller could not route messages into its own logging system.

## Test plan

- `tests/src/util/testLogger.cpp` (new)
  - `RecordsLevelTopicAndText`: a recording logger receives the topic and the streamed text on each of the four
    levels; a message written through a copy of the handle arrives at the same logger.
    - Covers the four forwarders, the topic argument and the defaulted copy operations.
  - `UnboundLoggerDiscards`: level calls on a default-constructed handle return a usable stream and write nothing.
    - Covers the discarding stream.
  - `ConceptAcceptsAndRejects`: `static_assert` that a recording logger satisfies `LoggerLike`, that a type missing
    one method does not, and that a method returning `std::ostream` by value does not.
    - Covers the concept constraint, which no runtime case can show.
  - `CPlusPlusVersion`: `static_assert(__cplusplus >= 202302L)`.
    - Covers the C++23 requirement; a compiler would otherwise accept the document while building as C++20.
  - `ConcurrentCallsArriveIntact`: eight threads write 100 messages each through one handle into a serialising
    logger; every message arrives complete with its own topic.
    - Covers the thread-safety requirement of `Logger`; sequential cases cannot show interleaving.
  - `ReentrantCall`: with a depth guard, the logger writes a second line through the same handle while handling the
    first; exactly two lines arrive and the inner stream is writable.
    - Covers the reentrancy requirement; no other case nests calls.
- `tests/src/backend/testMotorDriverCardFactory.cpp` (modified)
  - `LoggerReceivesCreationMessages`: dummy mode, card created under a key no other case uses, yields at least one
    message with topic `MotorDriverCardFactory`.
    - Covers the factory parameter and its forward to the card.
  - `CachedCardKeepsFirstLogger`: a second creation under the same key with another logger returns the same card,
    and the second logger stays silent.
    - Covers the cache semantics and the receiver of the cache-hit message.
  - `DummyCardErrorsAreLogged`: `getMotorControler()` with an invalid id on a dummy card yields the
    `MotorDriverCard/dummy` error before the exception.
    - Covers the error site of `MotorDriverCardDummy`; the other cases never leave the success path.
- `tests/src/backend/testMotorControler.cpp` (modified)
  - `OldFirmwareLogsWarning`: constructing the controler with the fixture map, which has no `VOLTAGE_EN` register,
    yields the `MotorControler/<id>` warning.
    - Covers the warn site; the fixture map makes the condition deterministic.
- `tests/src/backend/testSPIviaPCIe.cpp` (modified)
  - `HandshakeErrorIsLogged`: an invalid SPI datagram makes the dummy answer with the error handshake; the
    `SPI/...` error arrives before the `ChimeraTK::runtime_error`.
    - Covers the error site of `SPIviaPCIe`; the existing cases only use valid datagrams.
- `tests/src/steppermotor/testStepperMotor.cc` (modified)
  - `LoggerFromParametersReceivesCreationMessage`: a motor built with a recording logger in
    `StepperMotorParameters` yields the `StepperMotor/...` creation message.
    - Covers the new member and its use by `BasicStepperMotor`; the card is created by the fixture beforehand, so
      only the motor's own message is asserted.
