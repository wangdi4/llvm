#!/bin/sh

### Create a temp directory
mkdir -p tmp
rm -rf tmp/*.*

### Generate OCL code generator
g++ gen_ocl/main.cpp -I gen_ocl -o tmp/gen_ocl.a

### Generate OCL code
./tmp/gen_ocl.a > tmp/funcs_ocl.cl

### Generate "module printer" hook for OCL runtime
g++ -arch i386 -dynamiclib  -DAPPLE=1 -D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS=1 -I /Developer/usr/local/include/ -L/System/Library/Frameworks/OpenGL.framework/Libraries/ -lLLVMContainer -o tmp/Vectorizer.dylib gen_llvm/moduleprint.cpp $1

### Install module printer hook
./gen_llvm/transfer tmp/Vectorizer.dylib

### Clear any module prints if exist
rm -f /tmp/vectorizer.txt

### Build the OCL application
g++ -arch i386 gen_llvm/main.cpp -o tmp/gen_llvm.a -framework OpenCL $1

## Execute the OCL application (generating the llvm module)
CL_FORCE_AUTOVEC=1 ./tmp/gen_llvm.a tmp/funcs_ocl.cl

### Copy the LLVM module locally
cp /tmp/vectorizer.txt tmp/module.ll

### Convert module to bitcode format
/Developer/usr/local/bin/llvm-as < tmp/module.ll > tmp/module.bc

### Build function names parser
g++ -Wl,-flat_namespace -I /Developer/usr/local/include/ parse_llvm/main.cpp `/Developer/usr/local/bin/llvm-config --cxxflags --ldflags --libs` -o tmp/parse_llvm.a $1

### Parse module to create functions list
tmp/parse_llvm.a tmp/module.bc > tmp/funcs.list

### Build generator of functions hash
g++ parse_lists/main.cpp -o tmp/parse_list.a

### Generate actual functions hash
./tmp/parse_list.a > functions.cpp
