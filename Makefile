# Makefile for Unix builds

#CXX := /home/ggljzr/cilkplus-install/bin/g++
CXX := g++

# Default to maximum optimization
OPT ?= -O3

CXXFLAGS := $(OPT)

# Use the best instructions available for this machine
CXXFLAGS += -march=native

CXXFLAGS += -fcilkplus -lcilkrts

all : primes2 primes64

primes2 : primes2.cpp
	$(CXX) $(CXXFLAGS) -o primes2.out primes2.cpp

primes64 : primes64.cpp
	$(CXX) $(CXXFLAGS) -o primes64.out primes64.cpp

clean :
	rm -f *.out
