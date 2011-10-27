
WIDTH ?= 4
LOOP ?= 4
NAME = $(WIDTH)_$(LOOP)
CC = g++
CFLAGS = -arch i386 -arch x86_64 -D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS=1 
LINK = -dynamiclib
LIBS = -L/System/Library/Frameworks/OpenGL.framework/Libraries/ -lLLVMContainer
INCS = -I ./ -I /Developer/usr/local/include/
VFLAGS = -DAPPLE=1 -DASSERT_UNEXPECTED -DOVERALL_TARGET_WIDTH=$(WIDTH) -DMAX_LOOP_SIZE=$(LOOP)
ifdef SL
VFLAGS += -DSNOWLEOPARD
endif
ifdef RELEASE
VFLAGS += -DRELEASE
endif

TARGET = Vectorizer.dylib

HEADERS = *.h controlflow/*.h funcScans/*.h masterLoop/*.h scalarize/*.h vectorize/*.h moduleManager/*.h
SOURCES = \
	main.cpp properties.cpp specialCaseFuncs.cpp \
	moduleManager/ModuleManager.cpp moduleManager/resolveSpecialCase.cpp \
	vectorize/vectorize.cpp vectorize/instData.cpp vectorize/vectorizeSpecialCase.cpp \
	scalarize/scalarize.cpp scalarize/scalarizeSpecialCase.cpp \
	Apple_funcs/functions.cpp \
	funcScans/scanfunction.cpp \
	controlflow/controlflow.cpp controlflow/earlyExits.cpp \
	masterLoop/masterLoop.cpp

OBJECTS = $(patsubst %.cpp,%.$(NAME).o,$(SOURCES))

all : $(TARGET).$(NAME)
	cp -f $(TARGET).$(NAME) $(TARGET)
	@echo "***" Done vectorizing WIDTH=$(NAME) "***"

$(TARGET).$(NAME) : $(OBJECTS)
	$(CC) $(CFLAGS) $(VFLAGS) $(LINK) $(INCS) $(LIBS) $(OBJECTS) -o $(TARGET).$(NAME)

%.$(NAME).o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(VFLAGS) $(INCS) -c $< -o $(patsubst %.cpp,%.$(NAME).o,$<)

skeleton : skeleton.cpp
	$(CC) $(CFLAGS) $(VFLAGS) $(LINK) $(INCS) $(LIBS) skeleton.cpp -o $(TARGET)

clean :
	rm -f *.o
	rm -f moduleManager/*.o
	rm -f Apple_funcs/*.o
	rm -f funcScans/*.o
	rm -f controlflow/*.o
	rm -f vectorize/*.o
	rm -f scalarize/*.o
	rm -f masterLoop/*.o
	rm -f Vectorizer.dylib*

