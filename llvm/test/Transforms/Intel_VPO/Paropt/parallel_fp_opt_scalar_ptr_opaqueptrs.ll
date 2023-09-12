; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; int main(void) {
;     int *a[2];
;     int *b;
;
; //    a[0] = a[1] = b = (int*) 255;
;
; #pragma omp parallel firstprivate(a, b) num_threads(1)
; /*    printf("%p %p %p\n", a[0], a[1], b)*/;
; }

; Make sure that "b" is passed into the outlined function by value, but not "a",
; as "a" is not a scalar.

; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid, ptr [[B_VAL:%[^,]+]], ptr %a)
; CHECK:   %b.fpriv = alloca ptr, align 8
; CHECK:   %a.fpriv = alloca [2 x ptr], align 16
; CHECK:   %a.fpriv.gep = getelementptr inbounds [2 x ptr], ptr %a.fpriv, i32 0, i32 0
; CHECK:   store ptr [[B_VAL]], ptr %b.fpriv, align 8
; CHECK:   call void @llvm.memcpy.p0.p0.i64(ptr align 8 %a.fpriv.gep, ptr align 8 %a, i64 16, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %a = alloca [2 x ptr], align 16
  %b = alloca ptr, align 8
  %array.begin = getelementptr inbounds [2 x ptr], ptr %a, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, ptr null, i64 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %b, ptr null, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
