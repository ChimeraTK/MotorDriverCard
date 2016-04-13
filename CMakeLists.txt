cmake_minimum_required(VERSION 2.8.0)

project(MotorDriverCard)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 09)
set(${PROJECT_NAME}_PATCH_VERSION 01)
set(${PROJECT_NAME}_VERSION
  ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})
#This library depends on deviceaccess, which is likely to change often.
#We add a build version to the version number to be able to install
#the same software version of the library linked against different versions
#of deviceacces (or other dependencies)
set(${PROJECT_NAME}_BUILDVERSION $ENV{${PROJECT_NAME}_BUILDVERSION})
if( ${PROJECT_NAME}_BUILDVERSION )
  set(${PROJECT_NAME}_SOVERSION "${${PROJECT_NAME}_SOVERSION}${${PROJECT_NAME}_BUILDVERSION}")
endif( ${PROJECT_NAME}_BUILDVERSION )
message("${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_SOVERSION}")
#We need the build name to be part of the libraries full version name
set(${PROJECT_NAME}_FULL_LIBRARY_VERSION ${${PROJECT_NAME}_SOVERSION}.${${PROJECT_NAME}_PATCH_VERSION})
 
#use -DCMAKE_BUILD_TYPE=Debug then configuring to turn on the coverage tests (and debug symbols)

#set(CMAKE_CXX_FLAGS "-Wall -g -fPIC -Wextra -Wshadow -Weffc++ -ansi -pedantic -Wuninitialized")
#skip -Weffc++ for the moment. It gives too many false positives
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -Wshadow -pedantic -Wuninitialized")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 --coverage")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#use -DSUPPRESS_AUTO_DOC_BUILD=true to suppress to create the doc with every
#build. The 'make doc' target will still exist, but make install will fail if you don't run it.
if(SUPPRESS_AUTO_DOC_BUILD)
    unset(DOC_DEPENDENCY)
else(SUPPRESS_AUTO_DOC_BUILD)
    set(DOC_DEPENDENCY ALL)
endif(SUPPRESS_AUTO_DOC_BUILD)

# The MIN versions are needed for automatic replacement in the debian control file
SET(mtca4u-deviceaccess_MIN_VERSION 00.08)
FIND_PACKAGE(mtca4u-deviceaccess ${mtca4u-deviceaccess_MIN_VERSION} REQUIRED)
include_directories(SYSTEM ${mtca4u-deviceaccess_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${mtca4u-deviceaccess_CXX_FLAGS}")

#pugixml now comes with the MotorDriverCard source code
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/extern/pugixml1.4/src)
aux_source_directory(${CMAKE_SOURCE_DIR}/extern/pugixml1.4/src pugixml_sources)

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

#For some reason the system library has to be mentioned explicitly to the linker.
#This is strange because the thead library has it as a dependency.
FIND_PACKAGE(Boost COMPONENTS thread system unit_test_framework filesystem)
IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  set(TESTING_IS_ENABLED "true")
  ENABLE_TESTING()
ELSE(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  message("Boost unit_test_framework not found, disabling testing")
ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)

include_directories(${CMAKE_SOURCE_DIR}/include)
set(${PROJECT_NAME}_INCLUDE_DIRS ${${PROJECT_NAME}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/)
aux_source_directory(${CMAKE_SOURCE_DIR}/src library_sources)

MACRO( COPY_MAPPING_FILES )
  foreach( MAPPING_FILE DFMC_MD22_test.map DFMC_MD22_test.mapp DFMC_MD22_broken.map DFMC_MD22_broken.mapp
           mtcadummy_DFMC_MD22_mock.map mtcadummy_DFMC_MD22_mock.mapp MD22_on_DAMC2.dmap dummies.dmap)
    configure_file( tests/${MAPPING_FILE} ${PROJECT_BINARY_DIR}/${MAPPING_FILE} copyonly)
  endforeach( MAPPING_FILE )
  foreach( HW_CONFIG_FILE VT21-MotorDriverCardConfig.xml md22_damc2_r1066.mapp )
    configure_file( tests/hardware/${HW_CONFIG_FILE} ${PROJECT_BINARY_DIR}/${HW_CONFIG_FILE} copyonly)
  endforeach( HW_CONFIG_FILE )
ENDMACRO( COPY_MAPPING_FILES )

MACRO( COPY_XML_FILES )
  foreach( XML_FILE MotorDriverCardConfig_minimal_test.xml MotorDriverCardConfig_complete_test.xml
                    MotorDriverCardConfig_first_half_test.xml MotorDriverCardConfig_second_half_test.xml
		    MotorDriverCardConfig_second_half_full_test.xml
		    MotorDriverCardConfig_invalid_maximumAccelleration.xml)
    configure_file( tests/${XML_FILE} ${PROJECT_BINARY_DIR}/${XML_FILE} copyonly)
  endforeach( XML_FILE )
ENDMACRO( COPY_XML_FILES )

#Create the executables for automated unit testing.
#Currently we assume that they are all tests contained in one file, so
#each file gives a new executable. This section has to be adapted if this should change.
if(TESTING_IS_ENABLED)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  include_directories(${CMAKE_SOURCE_DIR}/tests/include)
  aux_source_directory(${CMAKE_SOURCE_DIR}/tests/src testSources)
  foreach( testSourceFile ${testSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(excutableName ${testSourceFile} NAME_WE)
    add_executable(${excutableName} ${testSourceFile})
    target_link_libraries(${excutableName} ${PROJECT_NAME} ${Boost_LIBRARIES})    
    add_test(${excutableName} ${excutableName})
  endforeach( testSourceFile )
  
  add_test(compareXmlOutput ${PROJECT_BINARY_DIR}/compareXmlOutput.sh)

  COPY_MAPPING_FILES()
  COPY_XML_FILES()
  configure_file(tests/compareXmlOutput.sh ${PROJECT_BINARY_DIR}/compareXmlOutput.sh copyonly)

  #The make coverage command is only available in debug mode
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    configure_file(cmake/Makefile.coverage.in
      ${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
    add_custom_target(coverage
      make -f Makefile.coverage
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating test coverage documentation" VERBATIM
      )
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(TESTING_IS_ENABLED)

add_library(${PROJECT_NAME} SHARED ${library_sources} ${pugixml_sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} ${mtca4u-deviceaccess_LIBRARIES} ${Boost_THREAD_LIBRARIES} pthread)

#hardware test are build but not run as automated tests (yet?).
aux_source_directory(${CMAKE_SOURCE_DIR}/tests/hardware/src hardwareTestSources)
foreach( hardwareTestSourceFile ${hardwareTestSources})
  get_filename_component(excutableName ${hardwareTestSourceFile} NAME_WE)
  add_executable(${excutableName} ${hardwareTestSourceFile})
  target_link_libraries(${excutableName} ${PROJECT_NAME} ${Boost_LIBRARIES})    
endforeach( hardwareTestSourceFile )
configure_file(tests/hardware/scripts/runForwardBackward ${PROJECT_BINARY_DIR}/runForwardBackward copyonly) 

#change the install prefix to the source directory in case the user has not specified a destination 
#i. e. CMAKE_INSTALL_PREFIX is not set manually
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR} CACHE PATH "Install directory
                 prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#Install the library and the executables
install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION bin
	    LIBRARY DESTINATION lib
  )

#all include files go into include/PROJECT_NAME
#The exclusion of ${PROJECT_NAME} prevents the recursive installation of the files just being installed.
#The original headers are in include/*.h, the installed ones in include/PROJECT_NAME/*.h.
install(DIRECTORY ${${PROJECT_NAME}_INCLUDE_DIRS} DESTINATION include/mtca4u/${PROJECT_NAME}
          FILES_MATCHING PATTERN "*.h" 
	  PATTERN ".svn" EXCLUDE
	  PATTERN "${PROJECT_NAME}" EXCLUDE
	  )

#create the config files by filling the correct variables into the template (*.cmake.in)
#All variables @VARIABLE@ are replaced with the current values, but they have to be set first....
# Note: Most of them are empty, but have to be set. The library itself does not include
# mtca4u-deviceaccess in the headers any more, so most of them are not needed at the 
# moment.
set(${PROJECT_NAME}_CONFIG_INCLUDE_DIRS " ")
set(${PROJECT_NAME}_REFERENCE_HEADER "MotorDriverCard.h")
set(${PROJECT_NAME}_CONFIG_LIBRARIES " ")
set(${PROJECT_NAME}_CONFIG_LIBRARY_DIRS " ")
set(${PROJECT_NAME}_CONFIG_CXX_FLAGS " ")

include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

configure_file(cmake/mtca4u-${PROJECT_NAME}-config.in
  "${PROJECT_BINARY_DIR}/mtca4u-${PROJECT_NAME}-config" @ONLY)		
install(PROGRAMS "${PROJECT_BINARY_DIR}/mtca4u-${PROJECT_NAME}-config"
  DESTINATION bin COMPONENT dev)

add_subdirectory(ConfigCalculator) 

# The instructions to create a debian package are loaded from a separate file
include(${CMAKE_SOURCE_DIR}/cmake/prepare_debian_package.cmake)
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
