#!/bin/sh
echo building files: $1 $2 $3 $4 $5 $6 $7 $8 $9
g++ -arch i386 -arch x86_64 -dynamiclib  -DAPPLE=1 -D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS=1 -I ./ -I /Developer/usr/local/include/ -L/System/Library/Frameworks/OpenGL.framework/Libraries/ -lLLVMContainer -o Vectorizer.dylib $1 $2 $3 $4 $5 $6 $7 $8 $9
echo Done!
