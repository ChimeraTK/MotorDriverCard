# CR-004: Add a global logger to the library

Synopsis: The library logs through one process-wide logger with the levels `debug()`, `info()`, `warn()` and `error()`,
each taking a topic and returning a writable `std::ostream&`.
The caller installs any suitable object with one call; until then a built-in default writes to `stdout` and `stderr`.

Depends on: CR-003

Status: PLANNED

## Requirements

The library reports its lifecycle and its own error sites through a logger that the caller controls.

- Use one logger for the whole process.
  - The caller may install one before the first card is created; until then a built-in default is used.
- Accept either of two shapes, duck-typed at compile time; no inheritance either way.
  - Topic-taking: `debug(std::string)`, `info(std::string)`, `warn(std::string)` and `error(std::string)`, each
    returning `std::ostream&`.
  - Module-style: `debug()`, `info()`, `warning()` (or `warn()`) and `error()`, each returning an `std::ostream`-derived
    object that emits the line when it is destroyed.
  - A `ChimeraTK::ApplicationModule` fits the module-style shape unchanged, so it can be installed with
    `Logger::setGlobal(module)`.
- A topic is a short string naming the message source, so output can be filtered by prefix.
  - The topic-taking shape receives it as the argument; for the module-style shape the library prepends it to the
    text.
- Accept the object without inheritance: no pure virtual interface, no base class to derive from.
- Provide a built-in default logger, so an unconfigured library still reports.
  - `debug` and `info` go to `stdout`, `warn` and `error` to `stderr`.
- Allow installing or replacing the logger only before the first card is created.
  - A later call is rejected with `ChimeraTK::logic_error` and leaves the installed logger in place.
- Allow calls from any thread.
  - The library adds no locking around the caller's logger; it may call it while holding its own mutexes.
  - A logger method may log through the installed logger, and must not call back into the library.
  - Nested calls on one thread work with the topic-taking shape; the module-style shape supports several threads but
    not nesting.
- Emit diagnostics at lifecycle points and at error sites.
  - Each class subsection under Specifications lists the messages it emits; no other class emits any.
  - No per-register logging.
- Existing classes may change freely, including signature, layout and ABI.
  - No logger parameter and no logger member is added to `MotorDriverCard`, `MotorControler` or `StepperMotor`.
  - A caller that includes `Logger.h` compiles as C++23.
- Compile the library and the public `Logger.h` as C++23.

## Specifications

### Logger (new)

Location: `util/include/Logger.h` (new), `util/src/Logger.cc` (new)

- Namespace `ChimeraTK::MotorDriver`.
  - `util` becomes the shared base layer used by both `mtca4u` and `ChimeraTK::MotorDriver` code.
  - Library sources include `"Logger.h"`; installed callers include `ChimeraTK/MotorDriverCard/Logger.h`.
  - The existing install rule also exposes the header under `mtca4u/MotorDriverCard/`.
- `template<typename L> concept TopicLogger`
  - Satisfied when an lvalue of `L` provides `debug`, `info`, `warn` and `error`.
  - Each takes `std::string` and returns exactly `std::ostream&`.
- `template<typename L> concept ModuleLogger`
  - Satisfied when an lvalue of `L` provides `debug()`, `info()` and `error()` with no argument, and `warning()` or
    `warn()` with no argument.
  - Each returns a type derived from `std::ostream`; the object is a temporary that emits on destruction, as
    `ChimeraTK::Logger::StreamProxy` and `ApplicationModule::debug()` do.
- `template<typename L> concept LoggerLike = TopicLogger<L> || ModuleLogger<L>`
  - The concepts are the only contract; the logger type stays unknown to the library.
- `class Logger`
  - Concrete, non-virtual, non-owning handle; trivially copyable.
  - Manual type erasure: one `void*` for the logger object and four function pointers of type
    `std::ostream& (*)(void*, std::string)`.
  - `template<LoggerLike L> Logger(L& logger)`
    - Stores the logger address and instantiates the four forwarders; the logger object itself is never copied.
    - `TopicLogger` forwarders pass object, topic and stream through unchanged.
    - `ModuleLogger` forwarders normalise to the same signature:
      - Per level, a function that replaces a `thread_local` heap-allocated line object with the one the logger
        returns, then writes `topic` and `": "` into it, and returns its `std::ostream&`.
      - The heap object is constructed with guaranteed copy elision, so a non-movable `StreamProxy` works.
      - Replacing the object destroys the previous one, which emits the previous line; the `thread_local` holder
        does the same at thread exit.
      - The forwarder looks up the topic-taking member first and falls back to `warning()` for `warn`.
    - Not `explicit`, so a logger object converts where a `Logger` is expected.
    - Constrained to reject `L = Logger`: otherwise this constructor would beat the copy constructor for a non-const
      `Logger` lvalue and store a pointer to a handle.
    - No default constructor: a handle is always bound, so no call site needs a null check.
  - Copy and move defaulted.
    - Copies of the handle refer to the same external logger.
  - `std::ostream& debug(std::string topic) const`
    - Same for `info`, `warn` and `error`.
    - Forwards object, topic and the returned stream reference; no other work.
    - With a module-style logger the reference stays valid until the next log call on the same thread.
    - With a module-style logger the returned reference stays valid until the next log call on the same thread.
  - `static void setGlobal(Logger logger)`
    - Replaces the process-wide handle.
    - The caller keeps ownership: the library stores a pointer to the logger object, not a copy.
    - The logger object must stay alive until the next `setGlobal()` or the end of the process.
    - Throws `ChimeraTK::logic_error` once the global logger is frozen, without storing the argument.
  - `static Logger global()`
    - Returns a copy of the process-wide handle; every library log statement starts here.
    - Returns the built-in `DefaultLogger` handle until the first `setGlobal()`.
  - `void detail::freezeGlobalLogger()`
    - Declared in `Logger.h`, defined in `Logger.cc`; called by the library only.
    - Sets the frozen flag that `setGlobal()` checks; repeated calls are harmless.
  - Storage: one `std::atomic<Logger>` in `Logger.cc`, initialised with the default handle, plus one
    `std::atomic<bool>` frozen flag.
    - `global()` loads the handle; `setGlobal()` stores it while the flag is clear.
  - Thread safety: the handle is immutable after construction and the global slot is atomic, so concurrent log calls
    are safe; installing is supported only before the freeze.
  - Reentrancy: a logger method may log through the installed logger.
  - The handle never calls the logger during construction or destruction.
  - The library applies no level threshold; the installed logger decides what to write.
- Topic convention: the component path.
  - `MotorDriverCardFactory`
  - `MotorDriverCard/<moduleName>`, and `MotorDriverCard/dummy` for the dummy card
  - `MotorControler/<moduleName>/<id>`
  - `SPI/<moduleName>`
  - `StepperMotor/<deviceName>/<moduleName>/<driverId>`
  - Each instrumented class builds its topic once in the constructor and keeps it in a `std::string _logTopic` member,
    so no call site repeats the format.

### DefaultLogger (new)

Location: `util/include/Logger.h` (new), `util/src/Logger.cc` (new)

- `class DefaultLogger`
  - Satisfies `LoggerLike`; the logger the library uses before the first `setGlobal()`.
  - `explicit DefaultLogger(std::ostream& out = std::cout, std::ostream& err = std::cerr)`
    - Stores both streams; they must outlive the logger.
  - `std::ostream& debug(std::string topic)`
    - Same for `info`, `warn` and `error`.
    - Returns a `thread_local` stream over a line buffer.
  - Line buffer
    - Holds the instance and the target stream of the message in progress, so concurrent calls and several
      `DefaultLogger` instances on one thread never mix.
    - Starting a message on another instance or another stream first flushes the pending text.
    - Writes a completed line under a mutex of the instance that the message belongs to.
    - A newline in the text ends the line and starts the next one.
    - Pending text is flushed when the next message starts on the same thread and by a `thread_local` holder at thread
      exit, so no message is lost while the thread lives.
    - Line format `[<level>] <topic>: <text>`, with the levels named `debug`, `info`, `warning` and `error`.
  - `debug` and `info` lines go to `out`, `warn` and `error` lines to `err`.
  - Its own methods never log, so its non-recursive mutex cannot be re-entered.
- A function-local static `DefaultLogger` in `Logger.cc` backs `Logger::global()` until the first `setGlobal()`.
  - Created on first use, destroyed at process exit, so a handle never points at a dead object before then.
  - Logging from a static destructor after that point is unsupported.

### MotorDriverCardFactory (modified)

Location: `backend/include/MotorDriverCardFactory.h` (modified), `backend/src/MotorDriverCardFactory.cc` (modified)

- Messages, topic `MotorDriverCardFactory`:
  - `debug` on a cache hit and in dummy mode.
  - `info` with alias, map module name and config file name in the alias-based overload, and with the module name in
    the injected-device overload.
  - `info` after the alias-based overload has opened the device.
- Applies to both factory overloads of CR-003.

### MotorDriverCardImpl (modified)

Location: `backend/include/impl/MotorDriverCardImpl.h` (modified), `backend/src/MotorDriverCardImpl.cc` (modified)

- Messages, topic `MotorDriverCard/<moduleName>`:
  - `debug` with the accepted firmware version in `checkFirmwareVersion()`.
  - `info` at the end of `applyConfiguration()`.
  - `info` in `powerDown()`.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, on an invalid `motorControlerID` in
    `getMotorControler()`.
- `applyConfiguration()` and `checkFirmwareVersion()` are also called by `reconfigure()`, so a recovery reports the
  same two messages.
- New private `std::string _logTopic`, built in the constructor as `MotorDriverCard/` plus the module name.
- Calls `Logger::detail::freezeGlobalLogger()` first in the constructor.
  - Every card is created here or in `MotorDriverCardDummy`, so this is the freeze point of the logger.

### MotorControlerImpl (modified)

Location: `backend/include/impl/MotorControlerImpl.h` (modified), `backend/src/MotorControlerImpl.cc` (modified)

- Messages, topic `MotorControler/<moduleName>/<id>`:
  - `debug` at the end of the constructor.
  - `debug` in `createRegisterAccessors()` when the end switch power indicator accessor is not initialised (old
    firmware).
- New private `std::string _logTopic`, built in the constructor from the module name CR-003 stores and the id.
- `setEndSwitchPowerEnabled()` and `isEndSwitchPowerEnabled()` stay silent; the accessor-creation message is the one
  place reporting the condition.

### SPIviaPCIe (modified)

Location: `backend/include/impl/SPIviaPCIe.h` (modified), `backend/src/SPIviaPCIe.cc` (modified)

- Messages, topic `SPI/<moduleName>`:
  - `error` with the existing error details, then throw `ChimeraTK::runtime_error`, for the error handshake result.
  - No message for a successful or a repeated handshake, and none per register access.
- `TMC429SPI` and the other SPI helpers keep their signatures and stay silent.
- New private `std::string _logTopic`, built in the constructor from the module name.

### MotorDriverCardDummy (modified)

Location: `backend/include/MotorDriverCardDummy.h` (modified), `backend/src/MotorDriverCardDummy.cc` (modified)

- Messages, topic `MotorDriverCard/dummy`:
  - `debug` in the constructor.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, on an invalid `motorControlerID` and in
    `getPowerMonitor()`.
- Calls `Logger::detail::freezeGlobalLogger()` first in the constructor.
- `MotorControlerDummy` stays without messages; its not-implemented throws are unchanged.

### BasicStepperMotor (modified)

Location: `steppermotor/include/BasicStepperMotor.h` (modified), `steppermotor/src/BasicStepperMotor.cc` (modified)

- Messages, topic `StepperMotor/<deviceName>/<moduleName>/<driverId>`:
  - `info` after the motor is initialised.
  - `error` with the error text, then throw `ChimeraTK::logic_error`, in each of the routines not available for this
    motor.
- New private `std::string _logTopic`, built in the constructor from `StepperMotorParameters`.
  - The protected parameterless constructor leaves it empty; that instance never logs.
- `StepperMotorParameters` itself is untouched.

### CMake configuration (modified)

Location: `CMakeLists.txt` (modified)

- `cmake_minimum_required(VERSION 3.20)`
  - Required for `cxx_std_23`; applies to the top level, the sub-projects keep their own minimum.
- `target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)`
  - `Logger.cc` needs C++23.
- `target_compile_features(CTK_OBJECTS PRIVATE cxx_std_23)` in `steppermotor/CMakeLists.txt` (modified)
  - Its objects are linked into the library.
- `target_compile_features(${executableName} PRIVATE cxx_std_23)` in the test loops
  - The tests include `Logger.h`.
- Not `PUBLIC`: no installed library header includes `Logger.h`, so consumers of the library are unaffected; a caller
  that includes `Logger.h` sets C++23 for its own targets.
- A third test loop for `tests/src/util`, copied from the backend loop with its own link list
  (`${PROJECT_NAME} Boost::thread Boost::unit_test_framework`).
  - The existing two loops stay as they are.
- No source-list change: `util/src` is already globbed by the existing `aux_source_directory`.

### Alternatives considered

- Passing the logger per card or per motor: the card factory is a singleton and caches cards for the process
  lifetime, so a per-card logger is either ignored on a cache hit or outlived by its card.
- Pure virtual `LoggerInterface`: requires the caller to derive from a library class, which the requirements forbid.
- Templating every library class on the logger type: moves the whole implementation into headers, so the library no
  longer has a binary to link against and every class layout depends on the caller's logger type.
- Owning the installed logger through a `std::shared_ptr`: the caller cannot easily supply an object it already owns
  by reference, and lifetime ownership is not part of the requirement.
- `std::atomic<std::shared_ptr<Logger const>>` for the global slot: allocates and adds a control block, while the
  external logger object still stays non-owning.
- `std::function` per level instead of function pointers: allocates and adds an indirection per call for no gain.
- An unbound handle that discards: an unconfigured library would stay silent, contrary to the default-logger
  requirement.
- Requiring the caller to write an adapter for `ApplicationModule`: the module already has the four stream accessors, so
  the library normalises them instead of pushing that boilerplate onto every application.

## Test plan

- Every test executable installs one recording logger through a `BOOST_GLOBAL_FIXTURE` before any fixture creates an
  object, because the logger cannot change after the first card. Cases clear the recorder at their start.
- `tests/src/util/testLogger.cpp` (new)
  - `RecordsLevelTopicAndText`: a recording logger installed with `setGlobal()` receives the topic and the streamed
    text on each of the four levels; a message written through a copy of the handle arrives at the same logger.
    - Covers the four forwarders, the topic argument, `setGlobal()`/`global()` and the defaulted copy operations.
  - `ConceptAcceptsAndRejects`: `static_assert` that a recording logger satisfies `TopicLogger`, that a mock module
    with `debug()`, `info()`, `warning()` and `error()` satisfies `ModuleLogger`, that a type missing one method
    satisfies neither, and that a method returning `std::ostream` by value does not satisfy `TopicLogger`.
    - Covers both concept constraints and the exact return type, which no runtime case can show.
  - `AcceptsModuleStyleLogger`: a mock module whose accessors return a `std::ostream`-derived temporary emitting on
    destruction is installed with `setGlobal()`; a message arrives as `<topic>: <text>` and the level maps `warn` to
    `warning()`.
    - Covers the module-style normalisation, the topic prefix and the delayed emission; the topic-taking path is
      covered by `RecordsLevelTopicAndText`.
  - `CPlusPlusVersion`: `static_assert(__cplusplus >= 202302L)`.
    - Covers the C++23 setting of the test targets; the library has no runtime check.
  - `DefaultLoggerIsUsedUntilReplaced`: a handle taken from `global()` before any `setGlobal()` writes a line; after
    installing another logger and installing the taken handle again, the default writes again.
    - Covers the built-in default and repeated installation before a freeze; `RecordsLevelTopicAndText` covers
      replacement only.
  - `DefaultLoggerSplitsStreamsAndLevels`: a `DefaultLogger` over two string streams, installed with `setGlobal()`,
    puts `debug` and `info` lines on `out` and `warn` and `error` lines on `err`, each as
    `[<level>] <topic>: <text>`.
    - Covers the stream routing and the line format; no other case inspects a formatted line.
  - `DefaultLoggerKeepsPartialLines`: a message without a newline appears once the next message on the same thread
    starts; a message containing a newline yields two lines; the last message of a joined thread still appears.
    - Covers the buffer flush points, including the thread-exit flush, which the other cases never reach.
  - `ConcurrentCallsArriveIntact`: eight threads write 100 messages each through `global()` into a `DefaultLogger`
    over a string stream; all 800 lines arrive intact, each with its own topic and level.
    - Covers the thread-safety requirement of the logger and of the global slot; sequential cases cannot show
      interleaving.
  - `ReentrantCall`: with a depth guard, a recording logger installed via `setGlobal()` writes a second line through
    `global()` while handling the first; exactly two lines arrive and the outer stream stays writable afterwards.
    - Covers the reentrancy requirement; no other case nests calls.
- `tests/src/backend/testMotorDriverCardFactory.cpp` (modified)
  - `LoggerReceivesCreationMessages`: dummy mode and a key no other case uses yield the `MotorDriverCardFactory`
    messages, including the dummy-mode `debug`.
    - Covers the factory messages on a first creation; the logger itself is covered by `testLogger.cpp`.
  - `CachedCardHitIsLogged`: a second creation under that key yields the cache-hit `debug`.
    - Covers the cache-hit message, which a first creation cannot produce.
  - `LoggerCannotBeChangedAfterCardCreation`: after a card exists, `setGlobal()` throws `ChimeraTK::logic_error` and
    the installed recorder keeps receiving messages.
    - Covers the freeze point and the rejected installation; no other case installs after a card exists.
- `tests/src/backend/testMotorDriverCardDummy.cpp` (modified)
  - `CreationIsLogged`: constructing the dummy card yields the `MotorDriverCard/dummy` `debug`.
    - Covers the dummy creation message.
  - `ErrorsAreLogged`: `getMotorControler()` with an invalid id and `getPowerMonitor()` yield the `error` before their
    exceptions.
    - Covers both dummy error sites; the existing cases only check the exceptions.
- `tests/src/backend/testMotorDriverCard.cpp` (modified)
  - `CreationIsLogged`: constructing a card from the dummy device yields the firmware `debug` and the
    `applyConfiguration()` `info`.
    - Covers both card lifecycle messages; the error paths are covered below and in the dummy tests.
  - `InvalidMotorControlerIdIsLogged`: `getMotorControler()` with an invalid id yields the `error` before the
    exception.
    - Covers the card error site, which no other case reaches.
  - `PowerDownIsLogged`: `powerDown()` yields the `info`.
    - Covers the power-down message.
- `tests/src/backend/testMotorControler.cpp` (modified)
  - `ConstructionIsLogged`: constructing the controler yields its `debug`.
    - Covers the constructor message.
  - `OldFirmwareIsLogged`: the fixture map, which lacks `WORD_M1_VOLTAGE_EN`, yields the accessor-creation `debug`.
    - Covers the condition that the silent getters never report; the fixture map makes it deterministic.
- `tests/src/backend/testSPIviaPCIe.cpp` (modified)
  - `HandshakeErrorIsLogged`: an invalid SPI datagram makes the dummy answer with the error handshake; the `SPI/<moduleName>`
    `error` arrives before the `ChimeraTK::runtime_error`.
    - Covers the SPI error site; the existing cases only use valid datagrams.
- `tests/src/steppermotor/testStepperMotor.cc` (modified)
  - `MotorCreationIsLogged`: a motor built while a recording logger is installed yields the
    `StepperMotor/<deviceName>/<moduleName>/<driverId>` `info`.
    - Covers the motor creation message, the stored topic and the global lookup below the card factory.
  - `UnavailableRoutineIsLogged`: calling a routine that this motor does not support yields the `StepperMotor/...`
    `error` before the `ChimeraTK::logic_error`.
    - Covers the error sites of the unsupported routines.
