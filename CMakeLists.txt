cmake_minimum_required(VERSION 2.8.0)

project(MotorDriverCard)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(${PROJECT_NAME}_VERSION
  ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})
 
#set(CMAKE_CXX_FLAGS "-Wall  -fPIC -Wextra -Wshadow -Weffc++ -ansi -pedantic -Wuninitialized")
#skip -Weffc++ for the moment. It gives too many false positives
set(CMAKE_CXX_FLAGS "-Wall  -fPIC -Wextra -Wshadow -ansi -pedantic -Wuninitialized --coverage ")

FIND_PACKAGE(MtcaMappedDevice 00.04 REQUIRED)
include_directories(${MtcaMappedDevice_INCLUDE_DIRS})

FIND_PACKAGE(pugixml 1.3 REQUIRED)
include_directories(${pugixml_INCLUDE_DIRS})

#currently boost is only needed for the tests

FIND_PACKAGE(Boost COMPONENTS unit_test_framework)
#ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK) 
message("Boost_UNIT_TEST_FRAMEWORK_FOUND " Boost_UNIT_TEST_FRAMEWORK_FOUND)
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
    message("Boost_LIBRARIES ${Boost_LIBRARIES}")
    target_link_libraries(${excutableName} ${PROJECT_NAME} ${Boost_LIBRARIES})    
    add_test(${excutableName} ${excutableName})
  endforeach( testSourceFile )

  COPY_MAPPING_FILES()
  configure_file(cmake/Makefile.coverage
		${PROJECT_BINARY_DIR}/Makefile.coverage @ONLY)
  add_custom_target(coverage
    make -f Makefile.coverage
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating test coverage documentation" VERBATIM
    )

endif(TESTING_IS_ENABLED)

add_library(${PROJECT_NAME} SHARED ${library_sources} )
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
target_link_libraries(${PROJECT_NAME} ${MtcaMappedDevice_LIBRARIES} ${pugixml_LIBRARIES})

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
install(DIRECTORY ${${PROJECT_NAME}_INCLUDE_DIRS} DESTINATION include/${PROJECT_NAME}
          FILES_MATCHING PATTERN "*.h" 
	  PATTERN ".svn" EXCLUDE)

#create the config files by filling the correct variables into the template (*.cmake.in)
#All variables @VARIABLE@ are replaced with the current values

## #We have nested @-statements, so we have to parse twice
## configure_file(cmake/${PROJECT_NAME}Config.cmake.in.in
##   "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake.in" @ONLY)
## configure_file(${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
##   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)
## 
## configure_file(cmake/${PROJECT_NAME}ConfigVersion.cmake.in.in
##   "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in" @ONLY)
## configure_file(${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake.in
##   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)
##  
## # Install the ..Config.cmake and ..ConfigVersion.cmake
## install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
##   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" 
##   "${CMAKE_SOURCE_DIR}/cmake/Find${PROJECT_NAME}.cmake"
##   DESTINATION . COMPONENT dev)
##  
## # The instructions to create a debian package are loaded from a separate file
## include(${CMAKE_SOURCE_DIR}/cmake/prepare_debian_package.cmake)


