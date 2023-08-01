; RUN: opt -bugpoint-enable-legacy-pm -instcombine -S %s | FileCheck %s
; RUN: opt -passes='function(instcombine)' -S %s | FileCheck %s
;
; // Test C source:
; void bar(int*);
; void f1()
; {
;     int *a = (int *)malloc(sizeof(int));
;     #pragma omp target
;     {}
;     bar(&a[1]); // was "a" originally which caused the bitcast.
; }
;
; void f2()
; {
;     int *a = (int *)malloc(sizeof(int));
;     #pragma omp target
;     {}
;     bar(&a[1]); // was "a" originally which caused the bitcast.
; }
;
; Check that instcombine does not move the GEP `%gep` into the TARGET region.
; The fix applies to "f1", as it has the "may-have-openmp-directive" attribute.
; Before the fix instcombine moved %gep from "entry" to the
; top of "split" to be closer to its use (as it happens for "f2").
; As a result, it landed inside the TARGET region, corrupting the OpenMP semantics.

; Check that "%gep" stays in "entry" BB for "f1":
; CHECK-LABEL: define{{.*}} void @f1
; CHECK:         entry:
; CHECK:         %gep = getelementptr inbounds i32, ptr %call, i64 1
; CHECK:         split:
; CHECK:         call void @bar(ptr nonnull %gep)

; Whereas it is present in "split" BB for "f2":
; CHECK-LABEL: define{{.*}} void @f2
; CHECK:         split:
; CHECK:         %gep = getelementptr inbounds i32, ptr %call, i64 1
; CHECK:         call void @bar(ptr nonnull %gep)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @f1() local_unnamed_addr #0 {
entry:
  %call = call ptr @malloc(i64 4)
  %gep = getelementptr inbounds i32, ptr %call, i64 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %split

split:                           ; preds = %entry
  ; make sure instcombine does not move the bitcast instruction here
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call void @bar(ptr %gep)
  ret void
}

define dso_local void @f2() local_unnamed_addr {
entry:
  %call = call ptr @malloc(i64 4)
  %gep = getelementptr inbounds i32, ptr %call, i32 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  br label %split

split:                           ; preds = %entry
  ; make sure instcombine does not move the bitcast instruction here
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call void @bar(ptr nonnull %gep)
  ret void
}

declare dso_local noalias ptr @malloc(i64) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @bar(ptr) local_unnamed_addr

attributes #0 = { "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 66306, i32 40917030, !"_Z2f1", i32 5, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 66306, i32 40917030, !"_Z2f2", i32 13, i32 0, i32 1, i32 0}
