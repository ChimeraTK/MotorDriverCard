# mtca4u-MotorDriverCard

C++ library for controlling stepper motors through the [DESY DFMC-MD22 motor
controller and driver (FMC) card](https://mtca.desy.de/FMC/DFMC-MD22/).

The card carries a TMC429 motion controller and two TMC260 stepper driver
chips, so one card drives two independent motors. The library provides:

- a low-level access layer that implements the SPI protocol of the TMC429 and
  TMC260 chips on top of ChimeraTK-DeviceAccess register access,
- a high-level interface with state machines for basic, rotary, linear and
  reference stepper motors, including end switch handling, error checking and
  a user-defined position unit,
- tools and a Qt GUI to generate the XML motor configuration required by the
  driver and controller chips.

The library is based on [ChimeraTK-DeviceAccess](https://github.com/ChimeraTK/DeviceAccess).

## Requirements

- A C++23 capable compiler (as required by ChimeraTK-cppext)
- CMake
- ChimeraTK-DeviceAccess
- ChimeraTK-cppext
- libxml++-2.6
- Boost (thread, system, filesystem; unit_test_framework for the tests)
- Qt (only for the `QtMotorConfigCalculator` GUI, optional)

## Building and installing

```sh
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/wherever/you/want/to/install /path/to/the/source/code
make
make install
```

Tests are built by default; disable them with `-DBUILD_TESTS=OFF`.
Doxygen documentation is built by default as well and can be suppressed with
`-DSUPPRESS_AUTO_DOC_BUILD=true`; `make doc` builds it on demand. Disabling the
tests and the documentation build saves compilation time, but tests should not
stay disabled permanently during development.

## Using the library

Find the package in the `CMakeLists.txt` of your project:

```cmake
find_package(mtca4u-MotorDriverCard REQUIRED)

add_executable(myApp myApp.cpp)
target_link_libraries(myApp PRIVATE ChimeraTK::mtca4u-MotorDriverCard)
```

The exported target carries ChimeraTK-DeviceAccess, ChimeraTK-cppext and
Boost as public dependencies, so `find_package` locates them as well and no
separate `find_package` calls are needed.

Header installation paths:

- `mtca4u/MotorDriverCard/` - low-level interface (`MotorDriverCard`,
  `MotorControler`, config classes, TMC word classes)
- `ChimeraTK/MotorDriverCard/` - high-level interface (`StepperMotor`,
  `BasicStepperMotor`, `RotaryStepperMotor`, `LinearStepperMotor`,
  `ReferenceStepperMotor`, `StepperMotorFactory`) and utilities

The older `mtca4u` namespace variants are still shipped but are deprecated in
favour of the `ChimeraTK` implementations.

A `StepperMotor` is created from a `StepperMotorParameters` structure through
the `StepperMotorFactory`; the structure selects the device and module in the
DMAP and map file, the driver ID (0 or 1) and the motor configuration file.
See [examples/basicStepperMotor.cpp](examples/basicStepperMotor.cpp).

## Motor configuration

The TMC429 and TMC260 need to be configured for each motor; all parameters are
stored in one XML file per MD22 card (one file for both motors). The XML format
is defined by [motorcardconfig.rng](motorcardconfig.rng).

The `MotorConfigCalculator` executable written during the build generates a
config file from physical motor parameters:

```sh
MotorConfigCalculator out.xml iMax nStepsPerTurn maxRPM [timeToVMax] [endSwitchConfig] [microsteps]
```

`QtMotorConfigCalculator` is a graphical user interface for the same purpose
that covers the configuration much more completely than the basic command line
tool. It can load an existing config file into its expert settings and write
the result back to a (sparse) config file. In addition, `write_default_config`
(run automatically by the build) produces `MotorDriverCardDefaultConfig.xml`
with all default parameters.

## Repository layout

- `backend/` - low-level layer: DFMC-MD22 backend, SPI via PCIe, TMC429/TMC260
  word and register definitions, motor configuration classes
- `steppermotor/` - high-level ChimeraTK user interface
- `util/` - shared utilities (state machine base, unit converters)
- `ConfigCalculator/` - configuration calculator library, CLI, default config
  writer and Qt GUI
- `examples/` - example program
- `tests/` - unit tests (Boost.Test), mapping files and hardware tests
- `doc/` - Doxygen main page and additional documentation
