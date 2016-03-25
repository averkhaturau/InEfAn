project(ActiveAppTracker)

set(SRC
	ActiveWindowTracker.cpp
	ActiveWindowTracker.h
	WindowInfo.cpp
	WindowInfo.h
)

add_library(${PROJECT_NAME} ${SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "libs")

add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${ADDITIONAL_LIBS})
set_property(TARGET ${PROJECT_NAME}_test PROPERTY FOLDER "Tests")

