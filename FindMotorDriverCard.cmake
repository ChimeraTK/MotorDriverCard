#
# cmake module for finding MotorDriverCard
#
# returns:
#   MotorDriverCard_FOUND        : true or false, depending on whether
#                                   the package was found
#   MotorDriverCard_VERSION      : the package version
#   MotorDriverCard_INCLUDE_DIRS : path to the include directory
#   MotorDriverCard_LIBRARY_DIRS : path to the library directory
#   MotorDriverCard_LIBRARY      : the provided libraries
#
# @author Martin Killenberg, DESY
#

SET(MotorDriverCard_FOUND 0)

#FIXME: the search path for the device config has to be extended/generalised/improved
FIND_PATH(MotorDriverCard_DIR
    MotorDriverCardConfig.cmake
    ${CMAKE_CURRENT_LIST_DIR}
    )

#Once we have found the config our job is done. Just load the config which provides the required 
#varaibles.
include(${MotorDriverCard_DIR}/MotorDriverCardConfig.cmake)

#use a macro provided by CMake to check if all the listed arguments are valid
#and set MotorDriverCard_FOUND accordingly
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MotorDriverCard 
        REQUIRED_VARS MotorDriverCard_LIBRARIES MotorDriverCard_INCLUDE_DIRS
	VERSION_VAR MotorDriverCard_VERSION )

