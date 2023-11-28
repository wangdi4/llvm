; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that WRegionUtils::hasTargetDirective() correctly detects
; whether a function contains OpenMP TARGET constructs or not.
; The original C test has two functions, one with a TARGET pragma, and the
; other without:
;
; int fn_with_target() {
;   int x = 123;
;   #pragma omp target map(x)
;     x = x + 456;
;   return x;
; }
; void bar();
; void fn_without_target() {
;   #pragma omp parallel
;      bar();
; }
;
; Dumping The WRN Graph from WRegionCollection should show this:
;
; Printing analysis 'VPO Work-Region Collection' for function 'fn_with_target':
;
; Function contains OpenMP Target construct(s).        <--- CHECK this
;
; BEGIN TARGET ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   DEVICE: UNSPECIFIED
;   NOWAIT: false
;   DEFAULTMAP: UNSPECIFIED
;   OFFLOAD_ENTRY_IDX: 0
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause: UNSPECIFIED
;   MAP clause (size=1): CHAIN(<ptr %x, ptr %x, i64 4, 35 (0x0000000000000023), null, null> )
;   IS_DEVICE_PTR clause: UNSPECIFIED
;   DEPEND clause: UNSPECIFIED
;
;   EntryBB: DIR.OMP.TARGET.2
;   ExitBB: DIR.OMP.END.TARGET.4
;
; } END TARGET ID=1
;
; Printing analysis 'VPO Work-Region Collection' for function 'fn_without_target':
;
; Function does not contain OpenMP Target constructs.  <--- CHECK this
;
; BEGIN PARALLEL ID=2 {
;
;   IF_EXPR: UNSPECIFIED
;   NUM_THREADS: UNSPECIFIED
;   DEFAULT: UNSPECIFIED
;   PROCBIND: UNSPECIFIED
;   SHARED clause: UNSPECIFIED
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause: UNSPECIFIED
;   REDUCTION clause: UNSPECIFIED
;   COPYIN clause: UNSPECIFIED
;
;   EntryBB: DIR.OMP.PARALLEL.2
;   ExitBB: DIR.OMP.END.PARALLEL.4
;
; } END PARALLEL ID=2


; CHECK: Function contains OpenMP Target
; CHECK: Function does not contain OpenMP Target

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; FUNCTION WITH A TARGET CONSTRUCT ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @fn_with_target() #0 {
entry:
  %x = alloca i32, align 4
  store i32 123, ptr %x, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x, ptr %x, i64 4, i64 35, ptr null, ptr null) ]
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  %1 = load i32, ptr %x, align 4
  %add = add nsw i32 %1, 456
  store i32 %add, ptr %x, align 4
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  %2 = load i32, ptr %x, align 4
  ret i32 %2
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; FUNCTION WITH NO TARGET CONSTRUCTS ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @fn_without_target() #0 {
entry:
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  call void (...) @bar() #1
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret void
}

declare dso_local void @bar(...) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 53, i32 -1941482902, !"_Z14fn_with_target", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
