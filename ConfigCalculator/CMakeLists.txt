project(ConfigCalculator)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src calculator_library_sources)

#not a shared library, just link it to all the executables statically
add_library(${PROJECT_NAME} ${calculator_library_sources} )

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
  
endif(TESTING_IS_ENABLED)
