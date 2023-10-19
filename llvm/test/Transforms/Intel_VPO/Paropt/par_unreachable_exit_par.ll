; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt,function(intel-ir-optreport-emitter)' -intel-opt-report=high -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s --strict-whitespace --check-prefix=OPTREPORT

; Check that we can outline the first parallel region with unreachable exit,
; and subsequent unreachable regions are ignored.

; Test src:

; #include <omp.h>
; #include <stdio.h>
; void foo() {
; #pragma omp parallel num_threads(1)
;   abort();
;
; #pragma omp parallel num_threads(1)
;   printf("region 2\n");
; }

; Check for remarks about the second parallel construct being ignored
; CHECK: remark: <unknown>:0:0: parallel construct is unreachable from function entry
; CHECK: remark: <unknown>:0:0: parallel construct ignored

; OPTREPORT: Global optimization report for : foo

; OPTREPORT: OMP PARALLEL BEGIN
; OPTREPORT:     remark #30008: parallel construct transformed
; OPTREPORT: OMP PARALLEL END

; OPTREPORT: OMP PARALLEL BEGIN
; OPTREPORT:     remark #30010: parallel construct is unreachable from function entry
; OPTREPORT:     remark #30011: parallel construct ignored
; OPTREPORT: OMP PARALLEL END

; Check that the first parallel construct is outlined, and the fork call
; is followed by a return, since CodeExtractor determined the following code as
; unreachable
; CHECK: call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @{{.*}}, i32 0, ptr @{{foo.*}})
; CHECK-NEXT:  ret void

; Check that the original orphaned block exit1 stayed in the original routine,
; no fork_call was generated for the second construct, and and only the
; directives were removed.
; CHECK:       exit1:                                            ; No predecessors!
; CHECK-NEXT:   br label %entry2
; CHECK:       entry2:                                           ; preds = %exit1
; CHECK-NEXT:   br label %body2
; CHECK:       body2:                                            ; preds = %entry2
; CHECK-NEXT:   %call = call i32 (ptr, ...) @printf(ptr @.str)
; CHECK-NEXT:   br label %exit2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"region 2\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @foo() {
entry1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]
  br label %body1

body1:                                          ; preds = %entry1
  call void @abort()
  unreachable

exit1:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %entry2

entry2:                                         ; preds = %exit1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]
  br label %body2

body2:                                          ; preds = %entry2
  %call = call i32 (ptr, ...) @printf(ptr @.str)
  br label %exit2

exit2:                                          ; preds = %body2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @abort()
declare dso_local i32 @printf(ptr, ...)
