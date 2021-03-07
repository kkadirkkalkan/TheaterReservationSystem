.PHONY: all compile

all: compile

compile:
	@g++ simulation.cpp -std=c++14 -o simulation.o -lpthread
