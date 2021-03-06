# Makefile for Unix builds

#CXX := /home/ggljzr/cilkplus-install/bin/g++
CXX := g++

# Default to maximum optimization
OPT ?= -O3

CXXFLAGS := $(OPT)

# Use the best instructions available for this machine
CXXFLAGS += -march=native

CXXFLAGS += -fcilkplus -lcilkrts

all : primes primes64 seg_primes

primes : primes.cpp
	$(CXX) $(CXXFLAGS) -o primes.out primes.cpp

primes64 : primes64.cpp
	$(CXX) $(CXXFLAGS) -o primes64.out primes64.cpp

seg_primes : seg_primes.cpp
	$(CXX) $(CXXFLAGS) -o seg_primes.out seg_primes.cpp

clean :
	rm -f *.out
