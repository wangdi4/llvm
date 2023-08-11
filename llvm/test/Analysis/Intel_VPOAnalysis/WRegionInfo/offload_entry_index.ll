; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

; This test checks that QUAL.OMP.OFFLOAD.ENTRY.IDX is parsed
; and properly represented in the WRNTargetNode.
;
; Test src:
;
; int foo() {
;   int x = 123;
;   #pragma omp target map(x)
;     x = x + 456;
;   return x;
; }
;
; The WRN Graph should have this:
; BEGIN TARGET ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   DEVICE: UNSPECIFIED
;   NOWAIT: false
;   DEFAULTMAP: UNSPECIFIED
;   OFFLOAD_ENTRY_IDX: 0              <--- CHECK this
;   EXT_DO_CONCURRENT: false
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause: UNSPECIFIED
;   LIVEIN clause: UNSPECIFIED
;   ALLOCATE clause: UNSPECIFIED
;   MAP clause (size=1): CHAIN(<ptr %x, ptr %x, i64 4, 35 (0x0000000000000023), null, null> )
;   SUBDEVICE clause: UNSPECIFIED
;   IS_DEVICE_PTR clause: UNSPECIFIED
;   DEPEND clause: UNSPECIFIED
;   DEPARRAY: UNSPECIFIED
;
;   EntryBB: DIR.OMP.TARGET.2
;   ExitBB: DIR.OMP.END.TARGET.4
;
; } END TARGET ID=1

; CHECK: OFFLOAD_ENTRY_IDX: 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo() #0 {
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

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 52, i32 -1927830709, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
