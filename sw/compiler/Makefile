OUTPUTDIR := bin/

CFLAGS := -std=c++14 -fvisibility=hidden -Wall -Wextra

HEADERS := src/*.h

CXX = mpic++

.SUFFIXES:
.PHONY: all clean

all: nbody-$(CONFIGURATION)-v1 nbody-$(CONFIGURATION)-v2

nbody-$(CONFIGURATION)-v1: $(HEADERS) src/mpi-simulator-v1.cpp
	$(CXX) -o $@ $(CFLAGS) src/mpi-simulator-v1.cpp

nbody-$(CONFIGURATION)-v2: $(HEADERS) src/mpi-simulator-v2.cpp
	$(CXX) -o $@ $(CFLAGS) src/mpi-simulator-v2.cpp

clean:
	rm -rf ./nbody-$(CONFIGURATION)*

FILES = src/*.cpp \
		src/*.h

handin.tar: $(FILES)
	tar cvf handin.tar $(FILES)
