; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Check that we can outline successive target regions with unreachable exit blocks.

; Test src:

; #include <omp.h>
; #include <stdio.h>
; void foo() {
; #pragma omp target
;   {
;     if (omp_is_initial_device())
;       abort();
;     printf("1 on device\n");
;   }
; #pragma omp target
;   {
;     if (omp_is_initial_device())
;       abort();
;     printf("2 on device\n");
;   }
; }
;
; //int main() { foo(); }

; CHECK: call i32 @__tgt_target_mapper(%struct.ident_t* @{{[^,]+}}, i64 %{{[^,]+}}, i8* @[[OUTLINED_ENTRY1:__omp_offloading[^, ]+]].region_id, i32 0, i8** null, i8** null, i64* null, i64* null, i8** null, i8** null)
; CHECK: call i32 @__tgt_target_mapper(%struct.ident_t* @{{[^,]+}}, i64 %{{[^,]+}}, i8* @[[OUTLINED_ENTRY2:__omp_offloading[^, ]+]].region_id, i32 0, i8** null, i8** null, i64* null, i64* null, i8** null, i8** null)

; Check that the outlined functions contain the orphaned exit blocks with
; no predecessors.

; CHECK: define internal void @[[OUTLINED_ENTRY1]]()
; CHECK:  body1:
; CHECK:    call void @abort()
; CHECK:    unreachable
; CHECK:  exit1:                                            ; No predecessors!

; CHECK: define internal void @[[OUTLINED_ENTRY2]]()
; CHECK:  body2:
; CHECK:    call void @abort()
; CHECK:    unreachable
; Since the second target region doesn't have any successors, make sure we
; didn't force CodeExtractor to pull-in any orphaned blocks.
; CHECK-NOT: exit2:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [13 x i8] c"1 on device\0A\00", align 1
@.str.1 = private unnamed_addr constant [13 x i8] c"2 on device\0A\00", align 1

; Function Attrs: convergent noinline nounwind uwtable
define hidden void @foo() #0 {
entry1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %body1

body1:                                          ; preds = %entry1
  call void @abort() #6
  unreachable

exit1:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %entry2

entry2:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  br label %body2

body2:                                          ; preds = %entry2
  call void @abort() #6
  unreachable

exit2:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noreturn nounwind
declare void @abort() #3

; Function Attrs: convergent
declare i32 @printf(i8*, ...) #4

attributes #0 = { convergent noinline nounwind uwtable "contains-openmp-target"="true" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { convergent nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #3 = { convergent noreturn nounwind "frame-pointer"="none" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #4 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #5 = { convergent nounwind }
attributes #6 = { convergent noreturn nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3}

!0 = !{i32 0, i32 66309, i32 90377734, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 0, i32 66309, i32 90377734, !"_Z3foo", i32 10, i32 1, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"PIC Level", i32 2}
