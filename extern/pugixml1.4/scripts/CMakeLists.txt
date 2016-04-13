project(pugixml)

set(pugixml_VERSION 1.4)
set(pugixml_SOVERSION 1)

cmake_minimum_required(VERSION 2.8.3)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    SET(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
endif()

set(pugixml_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR})

set(HEADERS ${pugixml_SCRIPT_DIR}/../src/pugixml.hpp ${pugixml_SCRIPT_DIR}/../src/pugiconfig.hpp)
set(SOURCES ${HEADERS} ${pugixml_SCRIPT_DIR}/../src/pugixml.cpp)

add_library(pugixml SHARED ${SOURCES})
set_target_properties(pugixml PROPERTIES VERSION ${pugixml_VERSION} SOVERSION ${pugixml_SOVERSION})

add_library(pugixml-static STATIC ${SOURCES})
set_target_properties(pugixml-static PROPERTIES OUTPUT_NAME pugixml)

install(TARGETS pugixml pugixml-static LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION include)

# The testing section
# FIXME: Not available in releases
ENABLE_TESTING()
aux_source_directory("${pugixml_SCRIPT_DIR}/../tests" TEST_SOURCES)
add_executable(pugixml_tests ${TEST_SOURCES})
target_link_libraries(pugixml_tests pugixml)
add_test(NAME all_tests WORKING_DIRECTORY ${pugixml_SCRIPT_DIR}/.. COMMAND pugixml_tests)

#create the config files by filling the correct variables into the template (*.cmake.in)
#All variables @VARIABLE@ are replaced with the current values

#We have nested @-statements, so we have to parse twice
configure_file(${PROJECT_NAME}Config.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)
configure_file(${PROJECT_NAME}ConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

# Install the ..Config.cmake and ..ConfigVersion.cmake
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" 
  "${pugixml_SCRIPT_DIR}/Find${PROJECT_NAME}.cmake"
  DESTINATION . COMPONENT dev)

# The instructions to create a debian package are loaded from a separate file
include(${pugixml_SCRIPT_DIR}/prepare_debian_package.cmake)
