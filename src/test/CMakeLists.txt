project(tests)

macro(do_test testname)
	add_executable(test_${testname} ${ARGN})
	target_link_libraries(test_${testname} libInEfAn ${ADDITIONAL_LIBS} logger ActiveAppTracker InputHooker_ Rpcrt4.lib)
	add_test(test_${testname} test_${testname})
	set_property(TARGET test_${testname} PROPERTY FOLDER "Tests")
endmacro(do_test)

do_test(filePostingTest filePostingTest.cpp)
