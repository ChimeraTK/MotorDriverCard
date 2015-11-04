#Prepare the debian control files from the template.
#Basically this is setting the correct version number in most of the files

#The debian version string must not contain ".", so we use "-"
set(MotorDriverCard_DEBVERSION ${MotorDriverCard_MAJOR_VERSION}-${MotorDriverCard_MINOR_VERSION})

#Nothing to change, just copy
file(COPY ${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/compat
           ${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/rules
	   ${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/mtca4u-motordrivercard-configcalculator.install
     DESTINATION debian_from_template)

file(COPY ${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/source/format
     DESTINATION debian_from_template/source)

#Adapt the file name
configure_file(${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/mtca4u-motordrivercardDEBVERSION.install.in
               debian_from_template/mtca4u-motordrivercard${MotorDriverCard_DEBVERSION}.install)

#Adapt the file name and/or set the version number
configure_file(${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/control.in
               debian_from_template/control @ONLY)

configure_file(${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/copyright.in
               debian_from_template/copyright @ONLY)

configure_file(${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/mtca4u-motordrivercardDEBVERSION.shlib.in
               debian_from_template/mtca4u-motordrivercard${MotorDriverCard_DEBVERSION}.shlib @ONLY)

configure_file(${CMAKE_SOURCE_DIR}/cmake/debian_package_templates/dev-mtca4u-motordrivercard.install.in
               debian_from_template/dev-mtca4u-motordrivercard.install @ONLY)

#Copy and configure the shell script which performs the actual 
#building of the package
configure_file(${CMAKE_SOURCE_DIR}/cmake/make_debian_package.sh.in
               make_debian_package.sh @ONLY)

#A custom target so you can just run make debian_package
#(You could instead run make_debian_package.sh yourself, hm...)
add_custom_target(debian_package ${CMAKE_BINARY_DIR}/make_debian_package.sh
                  COMMENT Building debian package for tag ${MotorDriverCard_VERSION})

#For convenience: Also create an install script for DESY
#The shared library package has the version number in the package name
set(PACKAGE_NAME "mtca4u-motordrivercard${MotorDriverCard_DEBVERSION}")
#The development package does not have the version in the name
set(PACKAGE_DEV_NAME "dev-mtca4u-motordrivercard")
#The binaries are in a separate package because they also do not have a version number in the package name
set(PACKAGE_BIN_NAME "mtca4u-motordrivercard-configcalculator")
set(PACKAGE_FILES_WILDCARDS "${PACKAGE_NAME}_*.deb ${PACKAGE_DEV_NAME}_*.deb ${PACKAGE_BIN_NAME}_*.deb mtca4u-motordrivercard_*.changes")

configure_file(${CMAKE_SOURCE_DIR}/cmake/install_debian_package_at_DESY.sh.in
               install_debian_package_at_DESY.sh @ONLY)

