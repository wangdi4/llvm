; RUN: opt -bugpoint-enable-legacy-pm -S -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s | FileCheck %s
; RUN: opt -S -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),module(vpo-paropt)" %s | FileCheck %s

; Test parallelization of code containing && BlockAddresses.
; The "outside" address should remain in @main, and the "inside" address
; should be outlined to the parallel function.

; Test src:
;
; #include <stdio.h>
; int main() {
;   outside:
;     printf("%p\n", && outside);
; #pragma omp parallel
;   {
;     inside:
;       printf("%p\n", && inside);
;   }
;   return 0;
; }

; CHECK-LABEL: @main
; CHECK: call{{.*}}printf{{.*}}blockaddress(@main, %outside)

; CHECK: define internal void @main.DIR.OMP.PARALLEL
; CHECK: call{{.*}}printf{{.*}}blockaddress(@main.DIR.OMP.PARALLEL{{.*}}, %inside)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  br label %outside

outside:                                          ; preds = %indirectgoto, %entry
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef blockaddress(@main, %outside))
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]

  br label %inside

inside:                                           ; preds = %indirectgoto, %outside
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef blockaddress(@main, %inside))
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0

indirectgoto:                                     ; No predecessors!
  indirectbr ptr undef, [label %outside, label %inside]
}

declare dso_local i32 @printf(ptr noundef, ...)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
