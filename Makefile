# Makefile for Unix builds

#CXX := /home/ggljzr/cilkplus-install/bin/g++
CXX := g++

# Default to maximum optimization
OPT ?= -O3

CXXFLAGS := $(OPT)

# Use the best instructions available for this machine
CXXFLAGS += -march=native

CXXFLAGS += -fcilkplus -lcilkrts

all : primes primes2 primes_serial

primes : primes.cpp
	$(CXX) $(CXXFLAGS) -o primes.out primes.cpp

primes2 : primes2.cpp
	$(CXX) $(CXXFLAGS) -o primes2.out primes2.cpp

primes_serial : primes_serial.cpp
	$(CXX) $(CXXFLAGS) -o primes_serial.out primes_serial.cpp

clean :
	rm -f *.out
