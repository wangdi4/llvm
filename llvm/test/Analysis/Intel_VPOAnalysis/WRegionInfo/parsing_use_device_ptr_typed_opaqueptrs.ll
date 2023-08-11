; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze -debug-only=vpo-wrninfo -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug-only=vpo-wrninfo -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int *array_device;
; int main() {
;   #pragma omp target data use_device_ptr(array_device)
;   printf("%p\n", array_device);
; }

; The test was hand-modified to use a typed use_device_ptr clause.

; CHECK: USE_DEVICE_PTR clause (size=1): TYPED,PTR_TO_PTR(ptr @array_device, TYPE: ptr, POINTEE_TYPE: i32, NUM_ELEMENTS: i64 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@array_device = dso_local global ptr null, align 8
@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local i32 @main() {
entry:
  %0 = load ptr, ptr @array_device, align 8
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR.TYPED"(ptr @array_device, i32 0, i64 1),
    "QUAL.OMP.MAP.TOFROM"(ptr %0, ptr %0, i64 0, i64 64, ptr null, ptr null) ]
  br label %DIR.OMP.TARGET.DATA.2

DIR.OMP.TARGET.DATA.2:                            ; preds = %DIR.OMP.TARGET.DATA.1
  %2 = load ptr, ptr @array_device, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %2)
  br label %DIR.OMP.END.TARGET.DATA.3

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.TARGET.DATA.2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.4

DIR.OMP.END.TARGET.DATA.4:                        ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!llvm.module.flags = !{!0}
!0 = !{i32 7, !"openmp", i32 50}
