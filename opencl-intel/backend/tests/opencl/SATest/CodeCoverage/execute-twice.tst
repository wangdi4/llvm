; REQUIRES: coverage
; RUN: rm -rf %t
; RUN: mkdir %t
; RUN: cd %t
; RUN: cp %p/basic.tst.cl .

; RUN: SATest -PERF -execute-iterations=2 --config=%p/basic.tst.cfg
; RUN: llvm-cov gcov basic.tst.cl | FileCheck --check-prefix=OUT %s
; RUN: FileCheck --check-prefix=GCOV %s < basic.tst.cl.gcov

; OUT: File 'basic.tst.cl'
; OUT: Lines executed:100.00% of 3
; OUT: Creating 'basic.tst.cl.gcov'

; GCOV:   -: 0:Source:basic.tst.cl
; GCOV:   -: 0:Graph:basic.tst.gcno
; GCOV:   -: 0:Data:basic.tst.gcda
; GCOV:   -: 0:Runs:1
; GCOV:   -: 0:Programs:1
; GCOV:   2: 1:__kernel void test_kernel(__global int *ptr) {
; GCOV:   2: 2:  *ptr = 10;
; GCOV:   2: 3:}
