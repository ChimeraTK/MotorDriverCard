# - Config file for the MotorDriverCard package
# It defines the following variables
# MotorDriverCard_INCLUDE_DIRS : include directories for MotorDriverCard
# MotorDriverCard_LIBRARIES    : libraries to link against
# MotorDriverCard_LIBRARY_DIRS : path to the library directory
# MotorDriverCard_VERSION      : the version of this library
 
# Compute paths
get_filename_component(MotorDriverCard_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# There only is one include path, search for it.
# We use a reference header file to check if we have the right path.
FIND_PATH(MotorDriverCard_INCLUDE_DIR MotorDriverCard/MotorDriverCard.h
          ${MotorDriverCard_CMAKE_DIR}/include
          /usr/include/mtca4u)

# There only is one library, search for it
# The "NO_DEFALUT" prevents the finding the library in /usr/lib first. We want to give the
# precedence to the project specific one.
FIND_LIBRARY(MotorDriverCard_LIBRARY MotorDriverCard
  ${MotorDriverCard_CMAKE_DIR}/lib
  /usr/lib
  NO_DEFAULT_PATH
  )
#extract the library dir from the library path
get_filename_component(MotorDriverCard_LIBRARY_DIR "${MotorDriverCard_LIBRARY}" PATH)
 
#Now set the variables to be exported. These are the plural versions as
#for instance more so files might be implemented later.
#The ${PROJECT_NAME}_CONFIG_INCLUDE_DIRS variable contains the include directories of this
# packages dependencies, for instance other mtca4u components
set(MotorDriverCard_INCLUDE_DIRS ${MotorDriverCard_INCLUDE_DIR} /space/nshehzad/work/deviceaccess/build/mtca_local_install/include)
set(MotorDriverCard_LIBRARIES ${MotorDriverCard_LIBRARY}  /space/nshehzad/work/deviceaccess/build/mtca_local_install/lib/libmtca4u-deviceaccess.so;/usr/lib/libboost_system-mt.so;/usr/lib/libboost_filesystem-mt.so)
set(MotorDriverCard_LIBRARY_DIRS ${MotorDriverCard_LIBRARY_DIR} /space/nshehzad/work/deviceaccess/build/mtca_local_install/lib)

set(MotorDriverCard_VERSION 00.05)

