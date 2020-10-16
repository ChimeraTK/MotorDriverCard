project(QtMotorConfigCalculator)

FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

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

include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} 
  ${${PROJECT_NAME}_HEADERS_MOC}
  ${${PROJECT_NAME}_FORMS_HEADERS} 
  ${${PROJECT_NAME}_RESOURCES_RCC})
# Link the required Qt libraries componentwise
TARGET_LINK_LIBRARIES(${PROJECT_NAME} mtca4u-MotorDriverCard ConfigCalculator  Qt5::Core  Qt5::Widgets Qt5::Gui)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${COMMON_INLCUDE_DIRS}) 

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)
