; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='vpo-paropt' -S %s | FileCheck %s
;
; Check that duplicate values in target entry's map bundles are renamed before
; outlining. That would guarantee that each map clause's base pointer always
; results in a separate parameter after outlining.
;
; This test is idetical to target_map_rename_duplicates_mappers.ll, except that
; MAP clauses use 4-operand form without mappers support.

;
; int foo() {
;   int yy = 55;
;   int zz = 57;
;   int *yy0 = &yy;
;   int *yy1 = yy0;
;   int *zz0 = &zz;
;
; #pragma omp target
;   {
;     (void) yy0;
;     (void) yy1;
;     *zz0 = *yy0;
;   }
;
;   return zz;
; }
;
; CHECK-LABEL: define i32 @foo()
; CHECK: entry:
; CHECK:   [[YY0:%yy.*]] = alloca i32, align 4
; CHECK:   [[ZZ0:%zz.*]] = alloca i32, align 4
; CHECK:   [[YY1:%.*]] = bitcast ptr [[YY0]] to ptr
; CHECK:   call void [[OUTLINED:@__omp_offloading_.*]](ptr [[YY0]], ptr [[YY1]], ptr [[ZZ0]])
;
; CHECK:   define internal void [[OUTLINED]](ptr %{{.*}}, ptr %{{.*}}, ptr noalias %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define i32 @foo() {
entry:
  %yy = alloca i32, align 4
  %zz = alloca i32, align 4
  %yy0.map.ptr.tmp = alloca ptr, align 8
  %yy1.map.ptr.tmp = alloca ptr, align 8
  %zz0.map.ptr.tmp = alloca ptr, align 8
  store i32 55, ptr %yy, align 4
  store i32 57, ptr %zz, align 4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %yy, ptr %yy, i64 0, i64 544),
    "QUAL.OMP.MAP.TOFROM"(ptr %yy, ptr %yy, i64 0, i64 544),
    "QUAL.OMP.MAP.TOFROM"(ptr %zz, ptr %zz, i64 0, i64 544),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %yy0.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %yy1.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %zz0.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  br label %DIR.OMP.TARGET.38

DIR.OMP.TARGET.38:                                ; preds = %DIR.OMP.TARGET.2
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.4.split, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.38
  store ptr %yy, ptr %yy0.map.ptr.tmp, align 8
  store ptr %yy, ptr %yy1.map.ptr.tmp, align 8
  store ptr %zz, ptr %zz0.map.ptr.tmp, align 8
  %1 = load ptr, ptr %yy0.map.ptr.tmp, align 8
  %2 = load i32, ptr %1, align 4
  store i32 %2, ptr %zz, align 4
  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %DIR.OMP.TARGET.38, %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  %3 = load i32, ptr %zz, align 4
  ret i32 %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 56, i32 -684555047, !"_Z3foo", i32 8, i32 0, i32 0}
