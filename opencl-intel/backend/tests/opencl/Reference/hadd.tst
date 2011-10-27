; RUN: python %S/../test_deploy.py %s.in .
; RUN: clang -O0 -x cl -emit-llvm -include %S/../../../clang_headers/opencl_.h -S %s.cl -o %s.ll
; RUN: llvm-as %s.ll -o hadd.tst.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=0 -basedir=. > %t
; RUN: FileCheck %s <%t
; CHECK: 6
