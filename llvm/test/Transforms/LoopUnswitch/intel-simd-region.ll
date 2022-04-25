; RUN: opt < %s -enable-new-pm=0 -loop-unswitch -verify-loop-info -S < %s 2>&1 | FileCheck %s

; This simple test would normally unswitch, but should be inhibited by the presence of
; SIMD region.

; CHECK-LABEL: @test_simd_region(
; CHECK: call void @decf()
; CHECK-NOT: call void @decf()

define i32 @test_simd_region(i32* %var) {
  %mem = alloca i32
  store i32 2, i32* %mem
  %c = load i32, i32* %mem
  %simd = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop_begin

loop_begin:
  %var_val = load i32, i32* %var
  switch i32 %c, label %default [
      i32 1, label %inc
      i32 2, label %dec
  ]

inc:
  call void @incf() noreturn nounwind
  br label %loop_begin

dec:
  call void @decf() noreturn nounwind
  br label %loop_begin

default:
  br label %loop_exit

loop_exit:
  call void @llvm.directive.region.exit(token %simd) [ "DIR.OMP.END.SIMD"() ]
  ret i32 0
}

declare void @incf() noreturn
declare void @decf() noreturn
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
