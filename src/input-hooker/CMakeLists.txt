project(InputHooker)

set(DLL_NAME ${PROJECT_NAME})

set(SRC input-hooker.h input-hooker.cpp)

add_definitions(-DDLL_NAME=${DLL_NAME})

add_library(${DLL_NAME} SHARED input-hooker-dll.cpp)

add_library(${DLL_NAME}_ STATIC ${SRC})

add_executable(${PROJECT_NAME}_test hooker-test.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME}_)

