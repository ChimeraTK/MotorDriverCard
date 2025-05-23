project(QtMotorConfigCalculator)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} 5.14 REQUIRED COMPONENTS Core Network Widgets Gui)

# Instruct CMake to run moc, rcc and uic automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# current binary dir for the generated headers from the ui and the moc
# Treat them as system directories because they contain generated code which cannot directly be controlled by us.
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}  ${PROJECT_NAME}_SOURCES)

SET(${PROJECT_NAME}_QOBJECT_HEADERS MotorConfigWidget.h QtMotorConfigCalculator.h HexSpinBox.h)
SET(${PROJECT_NAME}_FORMS MotorConfigWidget.ui)

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_QOBJECT_HEADERS}
  ${${PROJECT_NAME}_HEADERS_MOC}
  ${${PROJECT_NAME}_FORMS_HEADERS}
  ${${PROJECT_NAME}_RESOURCES_RCC})
# Link the required Qt libraries componentwise
TARGET_LINK_LIBRARIES(${PROJECT_NAME} mtca4u-MotorDriverCard ConfigCalculator  Qt${QT_VERSION_MAJOR}::Core  Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
