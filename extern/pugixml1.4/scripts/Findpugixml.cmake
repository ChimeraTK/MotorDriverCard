#
# cmake module for finding pugixml
#
# returns:
#   pugixml_FOUND        : true or false, depending on whether
#                                   the package was found
#   pugixml_VERSION      : the package version
#   pugixml_INCLUDE_DIRS : path to the include directory
#   pugixml_LIBRARY_DIRS : path to the library directory
#   pugixml_LIBRARY      : the provided libraries
#
# @author Martin Killenberg, DESY
#

SET(pugixml_FOUND 0)

#FIXME: the search path for the device config has to be extended/generalised/improved
FIND_PATH(pugixml_DIR
    pugixmlConfig.cmake
    ${CMAKE_CURRENT_LIST_DIR}
    )

#Once we have found the config our job is done. Just load the config which provides the required 
#varaibles.
include(${pugixml_DIR}/pugixmlConfig.cmake)

#use a macro provided by CMake to check if all the listed arguments are valid
#and set pugixml_FOUND accordingly
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(pugixml 
        REQUIRED_VARS pugixml_LIBRARIES pugixml_INCLUDE_DIRS
	VERSION_VAR pugixml_VERSION )

#Workaround for CMake 10.8.3:
#The ok-variable is PUGIXML_FOUND, not pugixml_FOUND. Just set them both in
# this case.
if(PUGIXML_FOUND)
  set(pugixml_FOUND ${PUGIXML_FOUND})
endif(PUGIXML_FOUND)
