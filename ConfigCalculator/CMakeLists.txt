project(ConfigCalculator)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src calculator_library_sources)

#not a shared library, just link it to all the executables statically
add_library(${PROJECT_NAME} ${calculator_library_sources} )
target_link_libraries(${PROJECT_NAME} MotorDriverCard)    

add_executable(ConfigCalculatorCLI  executables/ConfigCalculatorCLI.cpp)
target_link_libraries(ConfigCalculatorCLI ${PROJECT_NAME})    


if(TESTING_IS_ENABLED)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/include)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/tests/src calculatorTestSources)
  foreach( testSourceFile ${calculatorTestSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(excutableName ${testSourceFile} NAME_WE)
    add_executable(${excutableName} ${testSourceFile})
    target_link_libraries(${excutableName} ${PROJECT_NAME} ${Boost_LIBRARIES})    
    add_test(${excutableName} ${excutableName})
  endforeach( testSourceFile )
  
  # add a test for the CLI executable
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/generatedVT21Config.xml
    ${PROJECT_BINARY_DIR}/generatedVT21Config.xml copyonly)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/testConfigCalculatorCLI.sh
    ${PROJECT_BINARY_DIR}/testConfigCalculatorCLI.sh copyonly)
  add_test(testConfigCalculatorCLI
    ${PROJECT_BINARY_DIR}/testConfigCalculatorCLI.sh)

endif(TESTING_IS_ENABLED)

FIND_PACKAGE(Qt4)

if(QT4_FOUND)
  add_subdirectory(GUI)
endif(QT4_FOUND)
