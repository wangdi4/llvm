#DEBUG BUILD: CXX = clang++ -O0 -g
CXX     = clang++
SOURCES = $(notdir $(wildcard src/*.cpp))
TESTS   = $(notdir $(wildcard test/test_*.cpp))
VPATH   = src test

#DEBUG BUILD: TBBPATH   = ../../tbb/bin/Lin64_Debug
#DEBUG BUILD: TBBLIB    = -ltbb_debug
TBBPATH   = ../../tbb/bin/Lin64_Release
TBBLIB    = -ltbb

INCLUDE   = -I ../../tbb/include
LIB       = -L $(TBBPATH)
CXXFLAGS += -std=c++11 -fPIC

.PHONY: clean all mkdir omptbb test test_%
.PRECIOUS: build/%.o build/test_%.exe

LD_LIBRARY_PATH:=./build:$(TBBPATH):$(LD_LIBRARY_PATH)

all: omptbb test

omptbb: build/libomptbb.a build/libomptbb.so

build/libomptbb.a: $(addprefix build/,$(SOURCES:.cpp=.o))
	$(AR) rsc $@ $^

build/libomptbb.so: $(addprefix build/,$(SOURCES:.cpp=.o))
	$(CXX) $(LIB) -shared -o $@ $^ $(TBBLIB)

test: $(TESTS:.cpp=)
test_%: build/test_%.exe
	./$^

build/test_%.o: override CXXFLAGS+=-fopenmp
build/test_%.exe: build/test_%.o omptbb
	$(CXX) -o $@ $< -Lbuild -lomptbb

build/%.o: %.cpp
	$(CXX) -o $@ -c $(CXXFLAGS) $(INCLUDE) $<

clean:
	rm -f build/*
