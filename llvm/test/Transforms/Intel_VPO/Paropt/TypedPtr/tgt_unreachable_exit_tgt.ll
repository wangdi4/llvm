; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S %s | FileCheck %s

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

define hidden void @foo() {
entry1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %body1

body1:                                          ; preds = %entry1
  call void @abort()
  unreachable

exit1:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %entry2

entry2:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  br label %body2

body2:                                          ; preds = %entry2
  call void @abort()
  unreachable

exit2:                                          ; No predecessors!
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @abort()

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 66309, i32 90377734, !"_Z3foo", i32 4, i32 0, i32 0}
!1 = !{i32 0, i32 66309, i32 90377734, !"_Z3foo", i32 10, i32 1, i32 0}
