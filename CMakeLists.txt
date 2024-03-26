cmake_minimum_required(VERSION 3.16)

project(mtca4u-MotorDriverCard)

include(GNUInstallDirs)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 13)
set(${PROJECT_NAME}_PATCH_VERSION 06)

include(cmake/set_version_numbers.cmake)
include(cmake/set_default_build_to_release.cmake)
include(cmake/set_default_flags.cmake)
include(cmake/enable_code_coverage_report.cmake)
include(cmake/enable_doxygen_documentation.cmake)

find_package(ChimeraTK-DeviceAccess 02.04 REQUIRED)
find_package(PkgConfig)
set(LIBXML++_VERSION "2.6")
pkg_check_modules(LibXML++ REQUIRED IMPORTED_TARGET libxml++-${LIBXML++_VERSION})

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
FIND_PACKAGE(Boost REQUIRED COMPONENTS thread system unit_test_framework filesystem)
option(BUILD_TESTS "Build test programs" ON)
IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  if(BUILD_TESTS)
    set(TESTING_IS_ENABLED "true")
    ENABLE_TESTING()
  endif(BUILD_TESTS)
ELSE(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  message("Boost unit_test_framework not found, disabling testing")
ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)


# Add source files and include directories and define object targets
# Three object targets are defined:
# - mtca4u interface
# - ChimeraTK interface
# - Commonly used sources
#
# This is mainly done avoid name conflicts between files of the interfaces
# First, add common sources
set(COMMON_INCLUDE_DIRS "")
set(library_includes "")
set(subdirs util backend)
foreach(subdir ${subdirs})
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/src library_sources)
  file(GLOB subdir_includes ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/*.h)
  list(APPEND library_includes ${subdir_includes})
  list(APPEND COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/)
endforeach()

install(DIRECTORY ${COMMON_INCLUDE_DIRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mtca4u/MotorDriverCard)
install(DIRECTORY util/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ChimeraTK/MotorDriverCard)

# Add  targets for mtca4u and ChimeraTK interfaces
add_subdirectory(steppermotor)
add_subdirectory(mtca4u_steppermotor)

MACRO( COPY_MAPPING_FILES )
  foreach( MAPPING_FILE DFMC_MD22_test.map DFMC_MD22_test.mapp DFMC_MD22_broken.map DFMC_MD22_broken.mapp
           mtcadummy_DFMC_MD22_mock.map mtcadummy_DFMC_MD22_mock.mapp MD22_on_DAMC2.dmap dummies.dmap newer_firmware_mapfile.map backwards_compat.xlmap)
    FILE(COPY tests/${MAPPING_FILE} DESTINATION ${PROJECT_BINARY_DIR})
  endforeach()
  foreach( HW_CONFIG_FILE VT21-MotorDriverCardConfig.xml md22_damc2_r1066.mapp )
    FILE(COPY tests/hardware/${HW_CONFIG_FILE} DESTINATION ${PROJECT_BINARY_DIR})
  endforeach( HW_CONFIG_FILE )
ENDMACRO( COPY_MAPPING_FILES )

MACRO( COPY_XML_FILES )
  foreach( XML_FILE MotorDriverCardConfig_minimal_test.xml MotorDriverCardConfig_complete_test.xml
                    MotorDriverCardConfig_first_half_test.xml MotorDriverCardConfig_second_half_test.xml
		    MotorDriverCardConfig_second_half_full_test.xml custom_speed_and_curruent_limits.xml
		    MotorDriverCardConfig_invalid_maximumAccelleration.xml)
    FILE(COPY tests/${XML_FILE} DESTINATION ${PROJECT_BINARY_DIR})
  endforeach( XML_FILE )
ENDMACRO( COPY_XML_FILES )

# Create the executables for automated unit testing.
#
# Current structure of the tests:
#  - All tests of a module are place in a single source
#    -> one executable per tested module
#  - To avoid conflicts, headers of the ChimeraTK interface
#    are placed in their own subfolder and have to be linked against
#    the specific objects. All other tests can be linked with the mtca4u objects.
if(TESTING_IS_ENABLED)
  include_directories(${CMAKE_SOURCE_DIR}/tests/include)

  # mtca4u interface and backend tests
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testSources)
  foreach( testSourceFile ${testSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(executableName ${testSourceFile} NAME_WE)

    add_executable(${executableName} ${testSourceFile})
    target_link_libraries(${executableName} PRIVATE ${PROJECT_NAME} Boost::thread Boost::system
                                                    Boost::unit_test_framework PkgConfig::LibXML++)

    target_include_directories(${executableName} PUBLIC mtca4u_steppermotor/include)

    add_test(${executableName} ${executableName})
  endforeach( testSourceFile )

  # ChimeraTK StepperMotor interface test
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src/steppermotor ctkTestSources)
  foreach( testSourceFile ${ctkTestSources})

    get_filename_component(executableName ${testSourceFile} NAME_WE)

    add_executable(${executableName} ${testSourceFile})
    target_link_libraries(${executableName} ${PROJECT_NAME} Boost::thread Boost::system Boost::unit_test_framework)

    target_include_directories(${executableName} PUBLIC steppermotor/include)

    add_test(${executableName} ${executableName})
  endforeach( testSourceFile )
  
  add_test(compareXmlOutput ${PROJECT_BINARY_DIR}/compareXmlOutput.sh)

  # Make compareXmlOutput depend on testMotorDriverCardConfigXML having run in parallel tests
  set_tests_properties(testMotorDriverCardConfigXML PROPERTIES FIXTURES_SETUP XmlResultFiles)
  set_tests_properties(compareXmlOutput PROPERTIES FIXTURES_REQUIRED XmlResultFiles)

  COPY_MAPPING_FILES()
  COPY_XML_FILES()
  FILE(COPY tests/compareXmlOutput.sh DESTINATION ${PROJECT_BINARY_DIR})
endif(TESTING_IS_ENABLED)

add_custom_command(
    PRE_BUILD
    OUTPUT schema.h
    COMMAND ${CMAKE_SOURCE_DIR}/build-aux/generate-schema ${CMAKE_SOURCE_DIR}/motorcardconfig.rng schema.h
    DEPENDS motorcardconfig.rng
)

# Define the main library
add_library(${PROJECT_NAME} SHARED ${library_sources} ${library_includes} schema.h)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} PUBLIC ChimeraTK::ChimeraTK-DeviceAccess Boost::thread Boost::system
                                      PRIVATE PkgConfig::LibXML++ MTCA4U_OBJECTS_TARGET CTK_OBJECTS_TARGET)
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${COMMON_INCLUDE_DIRS}>"
                                                   $<INSTALL_INTERFACE:include>
                                           PRIVATE ${PROJECT_BINARY_DIR})

#hardware test are build but not run as automated tests (yet?).
aux_source_directory(${CMAKE_SOURCE_DIR}/tests/hardware/src hardwareTestSources)
foreach( hardwareTestSourceFile ${hardwareTestSources})
  get_filename_component(executableName ${hardwareTestSourceFile} NAME_WE)
  add_executable(${executableName} ${hardwareTestSourceFile})
  target_include_directories(${executableName} PUBLIC mtca4u_steppermotor/include)
  target_link_libraries(${executableName} PRIVATE ${PROJECT_NAME} Boost::thread Boost::system PkgConfig::LibXML++)
endforeach( hardwareTestSourceFile )
FILE(COPY tests/hardware/scripts/runForwardBackward DESTINATION ${PROJECT_BINARY_DIR}) 

#Install the library and the executables
install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(PROVIDES_EXPORTED_TARGETS 1)
include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

add_subdirectory(ConfigCalculator) 
