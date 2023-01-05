; CMPLRLLVM-19368: Verifies that points-to info correctly computed for
; for_allocate and for_alloc_allocatable.

; RUN: opt < %s  -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: pointer_$A<mem>        --> ({{[0-9]+}}): MAIN__:func_result3
; CHECK: pointer_$B<mem>        --> ({{[0-9]+}}): MAIN__:func_result1
; CHECK: pointer_$A    --> ({{[0-9]+}}): pointer_$A<mem>
; CHECK: pointer_$B    --> ({{[0-9]+}}): pointer_$B<mem>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"pointer_$A" = internal global %"QNCA_a0$float*$rank1$" { float* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"pointer_$B" = internal global %"QNCA_a0$double*$rank2$" { double* null, i64 0, i64 0, i64 0, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

define void @MAIN__() {
entry:
  %func_result1 = call i32 @for_allocate(i64 128, i8** bitcast (%"QNCA_a0$double*$rank2$"* @"pointer_$B" to i8**), i32 262144)
  %fetch1 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* @"pointer_$B", i32 0, i32 0)
  %ptr1 = bitcast double* %fetch1 to i8*
  %func_result2 = call i32 @for_dealloc_allocatable(i8* %ptr1, i32 256)
  %func_result3 = call i32 @for_alloc_allocatable(i64 16, i8** bitcast (%"QNCA_a0$float*$rank1$"* @"pointer_$A" to i8**), i32 512)
  %fetch2 = load float*, float** getelementptr inbounds (%"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* @"pointer_$A", i32 0, i32 0)
  %ptr2 = bitcast float* %fetch2 to i8*
  %func_result4 = call i32 @for_deallocate(i8* %ptr2, i32 256)
  br label %bb
bb:                                             ; preds = %entry
  ret void
}

declare i32 @for_allocate(i64, i8**, i32)
declare i32 @for_alloc_allocatable(i64, i8**, i32)
declare i32 @for_deallocate(i8*, i32)
declare i32 @for_dealloc_allocatable(i8*, i32)
