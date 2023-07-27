; RUN: opt -S -passes=vplan-vec \
; RUN:        -vplan-enable-peeling=true -vplan-force-dyn-alignment \
; RUN:        -vplan-vec-scenario='m4;v4;m4' \
; RUN:        < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,DYN-PEEL
; RUN: opt -S -passes=vplan-vec \
; RUN:        -vplan-enable-peeling=false \
; RUN:        -vplan-vec-scenario='m4;v4;m4' \
; RUN:        < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,STA-PEEL

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external global [1024 x i64], align 16

define dso_local void @test_store(i64 %N) {
; CHECK-LABEL: define dso_local void @test_store
; Peel loop
; CHECK:    call void @llvm.masked.store.v4i64.p0(<4 x i64> {{.*}}, ptr {{.*}}, i32 8, <4 x i1> {{.*}})

; Main loop
; STA-PEEL: store <4 x i64> {{.*}}, ptr {{.*}}, align 16
; DYN-PEEL: store <4 x i64> {{.*}}, ptr {{.*}}, align 8, !intel.preferred_alignment

; Remainder loop
; STA-PEEL: call void @llvm.masked.store.v4i64.p0(<4 x i64> {{.*}}, ptr {{.*}}, i32 16, <4 x i1> {{.*}})
; DYN-PEEL: call void @llvm.masked.store.v4i64.p0(<4 x i64> {{.*}}, ptr {{.*}}, i32 8, <4 x i1> {{.*}})
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]

  %add = add nuw nsw i64 %iv, 1
  %arrayidx = getelementptr [1024 x i64], ptr @A, i64 0, i64 %add
  store i64 %iv, ptr %arrayidx, align 8

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %N
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define dso_local void @test_load(i64 %N) {
; CHECK-LABEL: define dso_local void @test_load
; Peel loop
; CHECK:    call <4 x i64> @llvm.masked.load.v4i64.p0(ptr {{.*}}, i32 8, <4 x i1> {{.*}}, <4 x i64> poison)

; Main loop
; STA-PEEL: load <4 x i64>, ptr {{.*}}, align 16
; DYN-PEEL: load <4 x i64>, ptr {{.*}}, align 8, !intel.preferred_alignment

; Remainder loop
; STA-PEEL: call <4 x i64> @llvm.masked.load.v4i64.p0(ptr {{.*}}, i32 16, <4 x i1> {{.*}}, <4 x i64> poison)
; DYN-PEEL: call <4 x i64> @llvm.masked.load.v4i64.p0(ptr {{.*}}, i32 8, <4 x i1> {{.*}}, <4 x i64> poison)
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]

  %add = add nuw nsw i64 %iv, 1
  %arrayidx = getelementptr [1024 x i64], ptr @A, i64 0, i64 %add
  %res = load i64, ptr %arrayidx, align 8

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %N
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
