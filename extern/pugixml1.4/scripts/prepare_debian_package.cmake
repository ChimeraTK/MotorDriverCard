#Prepare the debian control files from the template.
#Basically this is setting the correct version number in most of the files

#The debian version string must not contain ".", so we use "-"
STRING(REGEX REPLACE "\\." "-" pugixml_DEBVERSION ${pugixml_SOVERSION})

execute_process(COMMAND dpkg-architecture -qDEB_HOST_MULTIARCH OUTPUT_VARIABLE DEB_HOST_MULTIARCH)

#Nothing to change, just copy
file(COPY ${pugixml_SCRIPT_DIR}/debian_package_templates/compat
          ${pugixml_SCRIPT_DIR}/debian_package_templates/rules
	  ${pugixml_SCRIPT_DIR}/debian_package_templates/copyright
	  ${pugixml_SCRIPT_DIR}/debian_package_templates/pugixml-doc.install
	  ${pugixml_SCRIPT_DIR}/debian_package_templates/changelog	  
     DESTINATION ${CMAKE_BINARY_DIR}/debian_from_template)

file(COPY ${pugixml_SCRIPT_DIR}/debian_package_templates/source/format
     DESTINATION ${CMAKE_BINARY_DIR}/debian_from_template/source)

#Adapt the file name
configure_file(${pugixml_SCRIPT_DIR}/debian_package_templates/libpugixmlDEBVERSION.install.in
               ${CMAKE_BINARY_DIR}/debian_from_template/libpugixml${pugixml_DEBVERSION}.install @ONLY)

#Adapt the file name and/or set the version number
configure_file(${pugixml_SCRIPT_DIR}/debian_package_templates/control.in
               ${CMAKE_BINARY_DIR}/debian_from_template/control @ONLY)

configure_file(${pugixml_SCRIPT_DIR}/debian_package_templates/libpugixmlDEBVERSION.shlib.in
               ${CMAKE_BINARY_DIR}/debian_from_template/libpugixml${pugixml_DEBVERSION}.shlib @ONLY)

configure_file(${pugixml_SCRIPT_DIR}/debian_package_templates/libpugixml-dev.install.in
               ${CMAKE_BINARY_DIR}/debian_from_template/libpugixml-dev.install @ONLY)

configure_file(${pugixml_SCRIPT_DIR}/debian_package_templates/pugixml-doc.doc-base.in
               ${CMAKE_BINARY_DIR}/debian_from_template/pugixml-doc.doc-base @ONLY)

#Copy and configure the shell script which performs the actual 
#building of the package
configure_file(${pugixml_SCRIPT_DIR}/make_debian_package.sh.in
               ${CMAKE_BINARY_DIR}/make_debian_package.sh @ONLY)

#A custom target so you can just run make debian_package
#(You could instead run make_debian_package.sh yourself, hm...)
add_custom_target(debian_package ${CMAKE_BINARY_DIR}/make_debian_package.sh
                  COMMENT Building debian package for tag ${pugixml_VERSION})

#For convenience: Also create an install script for DESY
#The shared library package has the version number in the package name
set(PACKAGE_NAME "libpugixml${pugixml_DEBVERSION}")
#The development package does not have the version in the name
set(PACKAGE_DEV_NAME "libpugixml-dev")
set(PACKAGE_DOC_NAME "pugixml-doc")
set(PACKAGE_FILES_WILDCARDS "${PACKAGE_NAME}_*.deb ${PACKAGE_DEV_NAME}_*.deb ${PACKAGE_DOC_NAME}_*.deb pugixml_*.changes")

configure_file(${CMAKE_SOURCE_DIR}/scripts/install_debian_package_at_DESY.sh.in
               ${CMAKE_BINARY_DIR}/install_debian_package_at_DESY.sh @ONLY)
