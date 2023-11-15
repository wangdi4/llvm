; REQUIRES: coverage
; RUN: rm -rf %t
; RUN: mkdir %t
; RUN: cd %t
; RUN: cp %s.cl .

; RUN: SATest -VAL -noref --config=%s.cfg
; RUN: llvm-cov gcov conditional-branch.tst.cl | FileCheck --check-prefix=OUT %s
; RUN: FileCheck --check-prefix=GCOV %s < conditional-branch.tst.cl.gcov

; OUT: File 'conditional-branch.tst.cl'
; OUT: Lines executed:83.33% of 6
; OUT: Creating 'conditional-branch.tst.cl.gcov'

; GCOV:   -:     0:Source:conditional-branch.tst.cl
; GCOV:   -:     0:Graph:conditional-branch.tst.gcno
; GCOV:   -:     0:Data:conditional-branch.tst.gcda
; GCOV:   -:     0:Runs:1
; GCOV:   -:     0:Programs:1
; GCOV:   1:     1:__kernel void test_kernel(__global int *ptr) {
; GCOV:   1:     2: if (get_local_id(0) == 0)
; GCOV:   1:     3:   *ptr = 10;
; GCOV:   1:     4: if (get_local_id(0) == 1)
; GCOV:   #####: 5:   *ptr = 20;
; GCOV:   1:     6:}
