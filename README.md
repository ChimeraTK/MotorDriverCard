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

- A C++17 capable compiler (as required by ChimeraTK-DeviceAccess)
- CMake
- ChimeraTK-DeviceAccess
- ChimeraTK-cppext
- libxml++-2.6
- Boost (thread, system, filesystem; unit_test_framework for the tests)
- Qt (only for the `QtMotorConfigCalculator` GUI, optional)

## Building and installing

```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=../install -GNinja ..
ninja
```

Tests are built by default; disable them with `-DBUILD_TESTS=OFF`.
Doxygen documentation is generated with `make doc` if Doxygen is available.

## Using the library

Find the package in the `CMakeLists.txt` of your project:

```cmake
find_package(ChimeraTK-DeviceAccess REQUIRED)
find_package(mtca4u-MotorDriverCard REQUIRED)

add_executable(myApp myApp.cpp)
target_link_libraries(myApp PRIVATE ChimeraTK::mtca4u-MotorDriverCard)
```

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

Alternatively, `QtMotorConfigCalculator` offers the same calculation in a
graphical form, and `write_default_config` (run automatically by the build)
produces `MotorDriverCardDefaultConfig.xml` with all default parameters.

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

## License

This project is licensed under the GNU Lesser General Public License v3.0 or
later, see [LICENSE](LICENSE). Copyright Deutsches Elektronen-Synchrotron DESY,
MSK, ChimeraTK Project.
