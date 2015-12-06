project(logger)

set(SRC
	logger.h
	log2file.cpp
	log2file.h
	log-helpers.cpp
	log-helpers.h
)

add_library(${PROJECT_NAME} ${SRC})
