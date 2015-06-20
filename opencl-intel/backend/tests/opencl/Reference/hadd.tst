; RUN: clang -O0 -x cl -emit-llvm -include %S/../../../../fe_compilers/common_clang/cl_headers/opencl_.h -S %s.cl -o %s.ll
; RUN: llvm-as %s.ll -o %s.bin
; RUN: SATest -REF -config=%s.cfg -neat=0 > %t
; RUN: FileCheck %s <%t
; CHECK: 6
