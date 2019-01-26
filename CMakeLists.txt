cmake_minimum_required(VERSION 2.8.0)

project(mtca4u-MotorDriverCard)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 08)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(${PROJECT_NAME}_VERSION
  ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})
#This library depends on deviceaccess, which is likely to change often.
#We add a build version to the version number to be able to install
#the same software version of the library linked against different versions
#of deviceacces (or other dependencies)
#Note: The environment variable must not contain hyphens, so we just use MotorDriverCard
#We store it in a variable because we need it later
set(${PROJECT_NAME}_BUILDVERSION_ENVIRONMENT_VARIABLE_NAME "MotorDriverCard_BUILDVERSION")
set(${PROJECT_NAME}_BUILDVERSION $ENV{PROJECT_BUILDVERSION}$ENV{${${PROJECT_NAME}_BUILDVERSION_ENVIRONMENT_VARIABLE_NAME}})
if( ${PROJECT_NAME}_BUILDVERSION )
  set(${PROJECT_NAME}_SOVERSION "${${PROJECT_NAME}_SOVERSION}${${PROJECT_NAME}_BUILDVERSION}")
endif( ${PROJECT_NAME}_BUILDVERSION )
message("${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_SOVERSION}")
#We need the build name to be part of the libraries full version name
set(${PROJECT_NAME}_FULL_LIBRARY_VERSION ${${PROJECT_NAME}_SOVERSION}.${${PROJECT_NAME}_PATCH_VERSION})
 
# Set the build type to Release if none is specified
# Force it into Release if "None" is specified (needed to overrule dkpg_buildpackage)
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")

#Technically we only need c++11 in the MotorDriverCard interface. For consistency in libraries that also use deviceaccess, which needs c++14, we also set
#the c++ version to 14. No harm done, as it is used for the compilation anyway.
set(${PROJECT_NAME}_CXX_FLAGS "-std=c++14")

#use -DCMAKE_BUILD_TYPE=Debug then configuring to turn on the coverage tests (and debug symbols)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -Wshadow -pedantic -Wuninitialized -Weffc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 --coverage")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#use -DSUPPRESS_AUTO_DOC_BUILD=true to suppress to create the doc with every
#build. The 'make doc' target will still exist
if(SUPPRESS_AUTO_DOC_BUILD)
    unset(DOC_DEPENDENCY)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/doc)  # otherwise make install might fail
else(SUPPRESS_AUTO_DOC_BUILD)
    set(DOC_DEPENDENCY ALL)
endif(SUPPRESS_AUTO_DOC_BUILD)

FIND_PACKAGE(ChimeraTK-DeviceAccess 01.00 REQUIRED)
include_directories(SYSTEM ${ChimeraTK-DeviceAccess_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS}")

#pugixml now comes with the MotorDriverCard source code
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/extern/pugixml1.4/src)
aux_source_directory(${CMAKE_SOURCE_DIR}/extern/pugixml1.4/src pugixml_sources)
#turn off all warnings for the pugixml library. It's not our code.
set_source_files_properties(${CMAKE_SOURCE_DIR}/extern/pugixml1.4/src/pugixml.cpp PROPERTIES COMPILE_FLAGS -w)

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
FIND_PACKAGE(Boost COMPONENTS thread system unit_test_framework filesystem REQUIRED)
IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  set(TESTING_IS_ENABLED "true")
  ENABLE_TESTING()
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
set(subdirs util backend)
foreach(subdir ${subdirs})
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/src library_sources)
  #set(SOURCE ${SOURCE} ${sources})
  #include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include)
  set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/include/)
  #set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${subdir}/include/)
endforeach()

add_library(COMMON_OBJECTS OBJECT ${library_sources})
target_include_directories(COMMON_OBJECTS PUBLIC ${COMMON_INCLUDE_DIRS})

install(DIRECTORY ${COMMON_INCLUDE_DIRS} DESTINATION /include/mtca4u/MotorDriverCard)

# Add  targets for mtca4u and ChimeraTK interfaces
add_subdirectory(steppermotor)
add_subdirectory(mtca4u_steppermotor)


MACRO( COPY_MAPPING_FILES )
  foreach( MAPPING_FILE DFMC_MD22_test.map DFMC_MD22_test.mapp DFMC_MD22_broken.map DFMC_MD22_broken.mapp
           mtcadummy_DFMC_MD22_mock.map mtcadummy_DFMC_MD22_mock.mapp MD22_on_DAMC2.dmap dummies.dmap newer_firmware_mapfile.map)
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
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  include_directories(${CMAKE_SOURCE_DIR}/tests/include)

  # mtca4u interface and backend tests
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testSources)
  foreach( testSourceFile ${testSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(executableName ${testSourceFile} NAME_WE)

    add_executable(${executableName} ${testSourceFile} $<TARGET_OBJECTS:MTCA4U_OBJECTS> $<TARGET_OBJECTS:COMMON_OBJECTS>)
    set_target_properties(${executableName} PROPERTIES LINK_FLAGS ${ChimeraTK-DeviceAccess_LINK_FLAGS})
    target_link_libraries(${executableName} ${PROJECT_NAME} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
                          ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    target_include_directories(${executableName} PUBLIC mtca4u_steppermotor/include ${COMMON_INCLUDE_DIRS})

    add_test(${executableName} ${executableName})
  endforeach( testSourceFile )

  # ChimeraTK StepperMotor interace test
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src/steppermotor ctkTestSources)
  foreach( testSourceFile ${ctkTestSources})

    get_filename_component(executableName ${testSourceFile} NAME_WE)

    add_executable(${executableName} ${testSourceFile} $<TARGET_OBJECTS:CTK_OBJECTS> $<TARGET_OBJECTS:COMMON_OBJECTS>)
    set_target_properties(${executableName} PROPERTIES LINK_FLAGS ${ChimeraTK-DeviceAccess_LINK_FLAGS})
    target_link_libraries(${executableName} ${PROJECT_NAME} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
                          ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    target_include_directories(${executableName} PUBLIC steppermotor/include ${COMMON_INCLUDE_DIRS})

    add_test(${executableName} ${executableName})
  endforeach( testSourceFile )
  
  add_test(compareXmlOutput ${PROJECT_BINARY_DIR}/compareXmlOutput.sh)

  COPY_MAPPING_FILES()
  COPY_XML_FILES()
  FILE(COPY tests/compareXmlOutput.sh DESTINATION ${PROJECT_BINARY_DIR})

  #The make coverage command is only available in debug mode
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #exclude the 'extern' directory, which contains pugixml which we don't want in the coverage report
    set(EXCLUDE_FROM_COVERAGE "extern")
    configure_file(cmake/make_coverage.sh.in
      ${PROJECT_BINARY_DIR}/make_coverage.sh @ONLY)
    add_custom_target(coverage
      ./make_coverage.sh
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating test coverage documentation" VERBATIM
      )
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(TESTING_IS_ENABLED)

add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:MTCA4U_OBJECTS> $<TARGET_OBJECTS:CTK_OBJECTS> ${pugixml_sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} ${ChimeraTK-DeviceAccess_LIBRARIES} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} pthread)


#hardware test are build but not run as automated tests (yet?).
aux_source_directory(${CMAKE_SOURCE_DIR}/tests/hardware/src hardwareTestSources)
foreach( hardwareTestSourceFile ${hardwareTestSources})
  get_filename_component(executableName ${hardwareTestSourceFile} NAME_WE)
  add_executable(${executableName} $<TARGET_OBJECTS:MTCA4U_OBJECTS> $<TARGET_OBJECTS:COMMON_OBJECTS> ${hardwareTestSourceFile})
  target_include_directories(${executableName} PUBLIC mtca4u_steppermotor/include ${COMMON_INCLUDE_DIRS})
  set_target_properties(${executableName} PROPERTIES LINK_FLAGS ${ChimeraTK-DeviceAccess_LINK_FLAGS})
  target_link_libraries(${executableName} ${PROJECT_NAME} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
endforeach( hardwareTestSourceFile )
FILE(COPY tests/hardware/scripts/runForwardBackward DESTINATION ${PROJECT_BINARY_DIR}) 

#change the install prefix to the source directory in case the user has not specified a destination 
#i. e. CMAKE_INSTALL_PREFIX is not set manually
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Install directory prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


#Install the library and the executables
install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION bin
	    LIBRARY DESTINATION lib
)


#create the config files by filling the correct variables into the template (*.cmake.in)
#All variables @VARIABLE@ are replaced with the current values, but they have to be set first....
# Note: Most of them are empty, but have to be set. The library itself does not include
# ChimeraTK-DeviceAccess in the headers any more, so most of them are not needed at the 
# moment.
set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(${PROJECT_NAME}_LIBRARIES "")
set(${PROJECT_NAME}_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib")
#set(${PROJECT_NAME}_CXX_FLAGS "") #have already been set by enable_cxx11_support
set(${PROJECT_NAME}_LINKER_FLAGS "-Wl,-rpath=${CMAKE_INSTALL_PREFIX}/lib,--enable-new-dtags ${ChimeraTK-DeviceAccess_LINKER_FLAGS}")
#no matlab bindings foreseen yet
#set(${PROJECT_NAME}_MEXFLAGS "-I${CMAKE_INSTALL_PREFIX}/include -L${CMAKE_INSTALL_PREFIX}/lib -l${PROJECT_NAME}")

include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

add_subdirectory(ConfigCalculator) 

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  # The write_default_config target is defined in the ConfigCalculator subdirectory
  # the 
  add_custom_target(doc ${DOC_DEPENDENCY}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    DEPENDS MotorDriverCardDefaultConfig.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/ DESTINATION share/doc/${PROJECT_NAME}-${${PROJECT_NAME}_SOVERSION} )
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation cannot be build.")
endif(DOXYGEN_FOUND)
