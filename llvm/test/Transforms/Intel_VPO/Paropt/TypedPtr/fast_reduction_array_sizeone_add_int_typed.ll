; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; int x[1];
; int main() {
;   x[0] = 111;
; #pragma omp parallel reduction(+ : x) num_threads(1)
;   x[0] += 222;
; //  printf("%d\n", x[0]);
; }

; Check that there is no type mismatch in the store from the private copy of
; the reduction operand x, to its counterpart in the fast-reduction struct.
; CHECK:      %x.fast_red = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; CHECK:      store i32 0, i32* %x.red, align 4
; CHECK:      %x.fast_red.cast = bitcast i32* %x.fast_red to [1 x i32]*
; CHECK-DAG   %x.fast_red.cast.cast = bitcast [1 x i32]* %x.fast_red.cast to i32*
; CHECK-DAG:  store i32 [[X_RED_VAL:[^ ]+]], i32* %x.fast_red.cast.cast, align 4
; CHECK-DAG:  [[X_RED_VAL]] = load i32, i32* %x.red, align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global [1 x i32] zeroinitializer, align 4

define dso_local i32 @main() {
entry:
  store i32 111, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"([1 x i32]* @x, i32 0, i64 1),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]

  %1 = load i32, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  %add = add nsw i32 %1, 222
  store i32 %add, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @x, i64 0, i64 0), align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
