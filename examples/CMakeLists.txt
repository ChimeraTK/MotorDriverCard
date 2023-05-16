#Note: This CMakeLists.txt is a fully working example how to use the
# *installed* MotorDriverCard library. It is not included from the main CMakeLists.txt
# which is used to build the library.

#Usage:
# * Create a build directory
#   $ mkdir build; cd build
#
# * Run 'cmake' and point it to the example source directory. You might have to
#   specify where to find the DeviceAccess library if it is not in the
#   system configuration, using CMAKE_MODULE_PATH (You have to adapt the CMAKE version).
#   $ cmake WHEREEVER/YOU/SAVED/MotorDriverCard/examples -DCMAKE_MODULE_PATH=MotorDriverCard_CUSTOM_INSTALLATION_PATH/share/share/cmake-3.5/Modules
#  
# * Build the examples by running make.
#   $ make
#
# * You can run the examples directly from the build directory.
#   $ ./basic

cmake_minimum_required(VERSION 3.16)

# Search for the MotorDriverCard library, set the include path and the
# required compiler flags.
FIND_PACKAGE(mtca4u-MotorDriverCard 01.08 REQUIRED)
include_directories(SYSTEM ${mtca4u-MotorDriverCard_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${mtca4u-MotorDriverCard_CXX_FLAGS}")

# Get the name of all .cpp files in the directory
FILE(GLOB SOURCE_FILES *.cpp)

# Create one executable for each source file, with the same base name
foreach( SOURCE_FILE ${SOURCE_FILES} )
    # NAME_WE is the file name without ending
    get_filename_component(TARGET ${SOURCE_FILE} NAME_WE)
    add_executable(${TARGET} ${SOURCE_FILE})
    # Link the MotorDriverCard library to the target executable
    target_link_libraries( ${TARGET} ${mtca4u-MotorDriverCard_LIBRARIES})
     set_target_properties(${TARGET} PROPERTIES LINK_FLAGS ${mtca4u-MotorDriverCard_LINK_FLAGS})
endforeach( SOURCE_FILE )

# To run the examples we need the map files. Copy them to the build folder.
FILE(COPY example.dmap DFMC_MD22.map DESTINATION ${PROJECT_BINARY_DIR})
