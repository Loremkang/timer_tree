
SRC_DIR := src
INCLUDE_DIR := include

SOURCE := $(wildcard ${SRC_DIR}/*.cpp)
INCLUDE := $(wildcard ${INCLUDE_DIR}/*.hpp)

FLAGS := -std=c++11 -O3 -I${INCLUDE_DIR}

example: ${SOURCE} ${INCLUDE}
	g++ -o $@ ${SOURCE} ${FLAGS}

.PHONY: test

test: example
	make
	./example