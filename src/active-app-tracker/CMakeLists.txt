project(ActiveAppTracker)

set(SRC
	ActiveWindowTracker.cpp
	ActiveWindowTracker.h
	WindowInfo.cpp
	WindowInfo.h
)

add_library(${PROJECT_NAME} ${SRC})

add_executable(${PROJECT_NAME}_test test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} ${ADDITIONAL_LIBS})
