project(InputHooker)

set(DLL_NAME ${PROJECT_NAME})

set(SRC
	input-hooker.h
	input-hooker.cpp
	input-event.cpp
	input-event.h
	input-event-types.h
	event-logger.h
	keycodes.h
	keycodes-anonimized.h
)

add_definitions(-DDLL_NAME=${DLL_NAME})

add_library(${DLL_NAME} SHARED input-hooker-dll.cpp input-hooker-dll.h)

add_library(${DLL_NAME}_ STATIC ${SRC})

add_executable(${PROJECT_NAME}_test hooker-test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME}_)

install(TARGETS ${DLL_NAME} DESTINATION .)