cmake_minimum_required(VERSION 2.8.0)

project(MotorDriverCard)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 01)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(${PROJECT_NAME}_VERSION
  ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})
 
#use -DCMAKE_BUILD_TYPE=Debug then configuring to turn on the coverage tests (and debug symbols)

#set(CMAKE_CXX_FLAGS "-Wall  -fPIC -Wextra -Wshadow -Weffc++ -ansi -pedantic -Wuninitialized")
#skip -Weffc++ for the moment. It gives too many false positives
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall  -fPIC -Wextra -Wshadow -ansi -pedantic -Wuninitialized")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

FIND_PACKAGE(MtcaMappedDevice 00.06 REQUIRED)
include_directories(${MtcaMappedDevice_INCLUDE_DIRS})

FIND_PACKAGE(pugixml 1.3 REQUIRED)
include_directories(${pugixml_INCLUDE_DIRS})

#currently boost is only needed for the tests

FIND_PACKAGE(Boost COMPONENTS unit_test_framework)
#ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 
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
  foreach( MAPPING_FILE DFMC_MD22_test.map DFMC_MD22_broken.map )
    configure_file( tests/${MAPPING_FILE} ${PROJECT_BINARY_DIR}/${MAPPING_FILE} copyonly)
  endforeach( MAPPING_FILE )
ENDMACRO( COPY_MAPPING_FILES )

MACRO( COPY_XML_FILES )
  foreach( XML_FILE MotorDriverCardConfig_minimal_test.xml MotorDriverCardConfig_complete_test.xml
                    MotorDriverCardConfig_first_half_test.xml MotorDriverCardConfig_second_half_test.xml
		    MotorDriverCardConfig_second_half_full_test.xml )
    configure_file( tests/${XML_FILE} ${PROJECT_BINARY_DIR}/${XML_FILE} copyonly)
  endforeach( XML_FILE )
ENDMACRO( COPY_XML_FILES )

#Create the executables for automated unit testing.
#Currently we assume that they are all tests contained in one file, so
#each file gives a new executable. This section has to be adapted if this should change.
if(TESTING_IS_ENABLED)
  include_directories(${Boost_INCLUDE_DIR})
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
    configure_file(cmake/Makefile.coverage
      ${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
    add_custom_target(coverage
      make -f Makefile.coverage
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating test coverage documentation" VERBATIM
      )
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(TESTING_IS_ENABLED)

add_library(${PROJECT_NAME} SHARED ${library_sources} )
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} ${MtcaMappedDevice_LIBRARIES} ${pugixml_LIBRARIES})

#hardware test are build but not run as automated tests (yet?).
aux_source_directory(${CMAKE_SOURCE_DIR}/tests/hardware/src hardwareTestSources)
foreach( hardwareTestSourceFile ${hardwareTestSources})
  get_filename_component(excutableName ${hardwareTestSourceFile} NAME_WE)
  add_executable(${excutableName} ${hardwareTestSourceFile})
  target_link_libraries(${excutableName} ${PROJECT_NAME})     
endforeach( hardwareTestSourceFile )
 

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
install(DIRECTORY ${${PROJECT_NAME}_INCLUDE_DIRS} DESTINATION include/${PROJECT_NAME}
          FILES_MATCHING PATTERN "*.h" 
	  PATTERN ".svn" EXCLUDE
	  PATTERN "${PROJECT_NAME}" EXCLUDE
	  )

#create the config files by filling the correct variables into the template (*.cmake.in)
#All variables @VARIABLE@ are replaced with the current values, but they have to be set first....
set(${PROJECT_NAME}_CONFIG_INCLUDE_DIRS ${MtcaMappedDevice_INCLUDE_DIRS})
set(${PROJECT_NAME}_REFERENCE_HEADER "MotorDriverCard.h")
set(${PROJECT_NAME}_CONFIG_LIBRARIES ${MtcaMappedDevice_LIBRARIES})
set(${PROJECT_NAME}_CONFIG_LIBRARY_DIRS ${MtcaMappedDevice_LIBRARY_DIRS})

include(${CMAKE_SOURCE_DIR}/cmake/create_cmake_config_files.cmake)

# The instructions to create a debian package are loaded from a separate file
include(${CMAKE_SOURCE_DIR}/cmake/prepare_debian_package.cmake)


