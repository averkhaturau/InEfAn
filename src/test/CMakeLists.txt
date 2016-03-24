project(test_FilePosting)


add_executable(${PROJECT_NAME} filePostingTest.cpp)
target_link_libraries(${PROJECT_NAME} libInEfAn ${ADDITIONAL_LIBS} logger ActiveAppTracker InputHooker_ Rpcrt4.lib)
