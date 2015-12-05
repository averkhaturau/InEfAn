project(logger)

set(SRC
	log2file.cpp
	log2file.h
	log-helpers.cpp
	log-helpers.h
)

add_library(${PROJECT_NAME} ${SRC})
