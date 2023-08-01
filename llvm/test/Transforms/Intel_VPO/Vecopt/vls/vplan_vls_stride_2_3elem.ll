; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" < %s 2>&1 | FileCheck %s
;
; LIT test to check that VLS optimization kicks in for stride 2 accesses when the group has
; more than 2 consecutive elements (example: a[2*i], a[2*i+1], a[2*i+2]).
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Incoming HIR:
;      + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
;      |   %ld0 = (%lp)[2 * i1];
;      |   %ld1 = (%lp)[2 * i1 + 1];
;      |   %ld2 = (%lp)[2 * i1 + 2];
;      + END LOOP
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:      |   %.vls.load = undef;
; CHECK-NEXT:      |   %.vls.load = (<16 x i64>*)(%lp)[2 * i1], Mask = @{<i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>};
; CHECK-NEXT:      |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:      |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:      |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 2, i32 4, i32 6, i32 8>;
; CHECK-NEXT:      + END LOOP
;
define void @foo(ptr %lp) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.013 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.013, 1
  %arrayidx = getelementptr inbounds i64, ptr %lp, i64 %mul
  %ld0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp, i64 %add
  %ld1 = load i64, ptr %arrayidx2, align 8
  %add2 = add nuw nsw i64 %mul, 2
  %arrayidx3 = getelementptr inbounds i64, ptr %lp, i64 %add2
  %ld2 = load i64, ptr %arrayidx3, align 8
  %inc = add nuw nsw i64 %l1.013, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Incoming HIR:
;      + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
;      |   %ld0 = (%lp)[2 * i1];
;      |   %ld2 = (%lp)[2 * i1 + 2];
;      |   %ld1 = (%lp)[2 * i1 + 1];
;      + END LOOP
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:      |   %.vls.load = undef;
; CHECK-NEXT:      |   %.vls.load = (<16 x i64>*)(%lp)[2 * i1], Mask = @{<i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>};
; CHECK-NEXT:      |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:      |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:      |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 2, i32 4, i32 6, i32 8>;
; CHECK-NEXT:      + END LOOP
;
define void @foo1(ptr %lp) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.013 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.013, 1
  %arrayidx = getelementptr inbounds i64, ptr %lp, i64 %mul
  %ld0 = load i64, ptr %arrayidx, align 8
  %add2 = add nuw nsw i64 %mul, 2
  %arrayidx3 = getelementptr inbounds i64, ptr %lp, i64 %add2
  %ld2 = load i64, ptr %arrayidx3, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp, i64 %add
  %ld1 = load i64, ptr %arrayidx2, align 8
  %inc = add nuw nsw i64 %l1.013, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Incoming HIR:
;      + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
;      |   %ld2 = (%lp)[2 * i1 + 2];
;      |   %ld0 = (%lp)[2 * i1];
;      |   %ld1 = (%lp)[2 * i1 + 1];
;      + END LOOP
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:      |   %.vls.load = undef;
; CHECK-NEXT:      |   %.vls.load = (<16 x i64>*)(%lp)[2 * i1], Mask = @{<i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>};
; CHECK-NEXT:      |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:      |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:      |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 2, i32 4, i32 6, i32 8>;
; CHECK-NEXT:      + END LOOP
;
define void @foo2(ptr %lp) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.013 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.013, 1
  %add2 = add nuw nsw i64 %mul, 2
  %arrayidx3 = getelementptr inbounds i64, ptr %lp, i64 %add2
  %ld2 = load i64, ptr %arrayidx3, align 8
  %arrayidx = getelementptr inbounds i64, ptr %lp, i64 %mul
  %ld0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp, i64 %add
  %ld1 = load i64, ptr %arrayidx2, align 8
  %inc = add nuw nsw i64 %l1.013, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
