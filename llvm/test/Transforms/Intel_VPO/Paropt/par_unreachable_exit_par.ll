; RUN: opt -vpo-paropt -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s

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

; Check that the first parallel construct is outlined, and the fork call
; is followed by a return, since CodeExtractor determined the following code as
; unreachable
; CHECK: call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t* @{{.*}}, i32 0, void (i32*, i32*, ...)* bitcast (void (i32*, i32*)* @{{foo.*}} to void (i32*, i32*, ...)*))
; CHECK-NEXT:  ret void

; Check that the original orphaned block exit1 stayed in the original routine,
; no fork_call was generated for the second construct, and and only the
; directives were removed.
; CHECK:       exit1:                                            ; No predecessors!
; CHECK-NEXT:   br label %entry2
; CHECK:       entry2:                                           ; preds = %exit1
; CHECK-NEXT:   br label %body2
; CHECK:       body2:                                            ; preds = %entry2
; CHECK-NEXT:   %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0)) #1
; CHECK-NEXT:   br label %exit2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"region 2\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1) ]
  br label %body1

body1:                                          ; preds = %entry1
  call void @abort() #4
  unreachable

exit1:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %entry2

entry2:                                         ; preds = %exit1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1) ]
  br label %body2

body2:                                          ; preds = %entry2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0)) #1
  br label %exit2

exit2:                                          ; preds = %body2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn nounwind
declare dso_local void @abort() #2

declare dso_local i32 @printf(i8*, ...) #3

attributes #0 = { nounwind uwtable "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #3 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #4 = { noreturn nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
