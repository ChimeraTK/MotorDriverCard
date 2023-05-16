project(ConfigCalculator)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src calculator_library_sources)
file(GLOB calculator_library_includes ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

#not a shared library, just link it to all the executables statically
add_library(${PROJECT_NAME} ${calculator_library_sources} ${calculator_library_includes})
target_link_libraries(${PROJECT_NAME} mtca4u-MotorDriverCard)
target_include_directories(${PROJECT_NAME} PUBLIC ${COMMON_INCLUDE_DIRS})


add_executable(MotorConfigCalculator executables/ConfigCalculatorCLI.cpp)
target_link_libraries(MotorConfigCalculator ${PROJECT_NAME} ChimeraTK::ChimeraTK-DeviceAccess)
install(TARGETS MotorConfigCalculator
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)

add_executable(write_default_config executables/write_default_config.cpp)
target_link_libraries(write_default_config PRIVATE ${PROJECT_NAME} mtca4u-MotorDriverCard ChimeraTK::ChimeraTK-DeviceAccess)
add_custom_target(MotorDriverCardDefaultConfig.xml ALL write_default_config
  DEPENDS write_default_config)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MotorDriverCardDefaultConfig.xml DESTINATION share/doc/mtca4u-MotorDriverCard-${mtca4u-MotorDriverCard_SOVERSION})

if(TESTING_IS_ENABLED)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/include)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/tests/src calculatorTestSources)
  foreach( testSourceFile ${calculatorTestSources})
    #NAME_WE means the base name without path and (longest) extension
    get_filename_component(executableName ${testSourceFile} NAME_WE)
    add_executable(${executableName} ${testSourceFile})
    target_include_directories(${executableName} PUBLIC ${COMMON_INCLUDE_DIRS})
    target_link_libraries(${executableName} PRIVATE ${PROJECT_NAME} ChimeraTK::ChimeraTK-DeviceAccess
        Boost::thread Boost::system Boost::filesystem)
    add_test(${executableName} ${executableName})
  endforeach( testSourceFile )
  
  # add a test for the CLI executable
  FILE(COPY tests/generatedVT21Config.xml tests/testMotorConfigCalculator.sh
       DESTINATION ${PROJECT_BINARY_DIR})
  add_test(testMotorConfigCalculator
    ${PROJECT_BINARY_DIR}/testMotorConfigCalculator.sh)

endif(TESTING_IS_ENABLED)

add_subdirectory(GUI)
