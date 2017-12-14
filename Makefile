# Makefile for Unix builds

CXX := /home/ggljzr/cilkplus-install/bin/g++

# Default to maximum optimization
OPT ?= -O3

CXXFLAGS := $(OPT)

# Use the best instructions available for this machine
CXXFLAGS += -march=native

CXXFLAGS += -fcilkplus -lcilkrts

SRC := spawn_for.cpp

all : primes

primes : rimes.cpp
	$(CXX) $(CXXFLAGS) -o primes primes.cpp


clean :
	rm -f primes
