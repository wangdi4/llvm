; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; C source test for OMP5.0 defaultmap construct. Need to compile with -fopenmp-version=50.
; int AAA[10];
; int foo() {
;   int iii = 123;
;   #pragma omp target defaultmap(to:aggregate) defaultmap(tofrom:scalar)
;   {
;     iii = iii + AAA[7];
;   }
;   return iii;
; }
;
; Defaultmap is processed in the FE, and the info is passed to the BE via IR
; in case BE needs it (eg, for compiler-created vars).
; This does not affect codegen currently, so we check by
; dumping the WRN and verify that it was parsed.
;
; BEGIN TARGET ID=1 {
;
;   DEFAULTMAP: TO : AGGREGATE                <--- CHECK
;   DEFAULTMAP: TOFROM : SCALAR               <--- CHECK
;   OFFLOAD_ENTRY_IDX: 0
;   MAP clause (size=2): CHAIN(<ptr %iii, ptr %iii, i64 4, 547 (0x0000000000000223), null, null> ) CHAIN(<ptr @AAA, ptr @AAA, i64 40, 545 (0x0000000000000221), null, null> )
;
;   EntryBB: DIR.OMP.TARGET.2
;   ExitBB: DIR.OMP.END.TARGET.4
;
; } END TARGET ID=1
;
; CHECK: DEFAULTMAP: TO : AGGREGATE
; CHECK: DEFAULTMAP: TOFROM : SCALAR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@AAA = dso_local global [10 x i32] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo() #0 {
entry:
  %iii = alloca i32, align 4
  store i32 123, ptr %iii, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.DEFAULTMAP.TO:AGGREGATE"(),
    "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %iii, ptr %iii, i64 4, i64 547, ptr null, ptr null),
    "QUAL.OMP.MAP.TO"(ptr @AAA, ptr @AAA, i64 40, i64 545, ptr null, ptr null) ]
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  %1 = load i32, ptr %iii, align 4
  %2 = load i32, ptr getelementptr inbounds ([10 x i32], ptr @AAA, i64 0, i64 7), align 4
  %add = add nsw i32 %1, %2
  store i32 %add, ptr %iii, align 4
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  %3 = load i32, ptr %iii, align 4
  ret i32 %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 53, i32 -1922206981, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
