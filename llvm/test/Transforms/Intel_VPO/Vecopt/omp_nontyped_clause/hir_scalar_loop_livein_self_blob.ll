; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-temp-cleanup -hir-vplan-vec -vplan-enable-general-peeling-hir -vplan-force-vf=4 -hir-details -print-after=hir-vplan-vec -disable-output 2>&1 < %s | FileCheck %s
;
; When initializing required live-in temps before the scalar remainder loop,
; the initialized temps were not being converted to self blobs. As a result,
; we ended up with HIR that looked like the following that caused HIR
; verification errors.
;
;              %3 = %phi.temp;
;              <LVAL-REG> NON-LINEAR i64 %4 {sb:3}
;                 <BLOB> NON-LINEAR i64 %4 {sb:15}
;
;              ...
;
;
;              + LiveIn symbases: 3, 11, 14, 15, 21
;              + LiveOut symbases: 3
;              + DO i64 i1 = %lb.tmp, %N + -1, 1   <DO_LOOP> <vectorize>
;
; which caused HIR verification to fail with "Temp expected to be livein to region."
;
; CHECK:              %3 = %phi.temp;
; CHECK:              <LVAL-REG> NON-LINEAR i64 %3 {sb:3}

; CHECK:              + LiveIn symbases: 3, 11, 14, 21
; CHECK:              + LiveOut symbases: 3
; CHECK:              + DO i64 i1 = %lb.tmp, %N + -1, 1   <DO_LOOP>
; CHECK:              |   if (i1 == %N + -1)
; CHECK:              |   {
; CHECK:              |      %4 = (%lp)[%N + -1];
; CHECK:              |      <LVAL-REG> NON-LINEAR i64 %4 {sb:15}

; CHECK:              |      %3 = %4;
; CHECK:              |      <LVAL-REG> NON-LINEAR i64 %4 {sb:3}
; CHECK:              |         <BLOB> NON-LINEAR i64 %4 {sb:15}
; CHECK:              |      <RVAL-REG> NON-LINEAR i64 %4 {sb:15}
; CHECK:              |   }
; CHECK:              + END LOOP
;
define dso_local i64 @foo(i64* nocapture noundef readonly %lp, i64 noundef %N) local_unnamed_addr #0 {
entry:
  %lpriv.lpriv = alloca i64, align 8
  %l1.linear.iv = alloca i64, align 8
  %sub1 = add nsw i64 %N, -1
  %cmp = icmp slt i64 %N, 1
  br i1 %cmp, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(i64* %lpriv.lpriv), "QUAL.OMP.LINEAR:IV"(i64* %l1.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.126

DIR.OMP.SIMD.126:                                 ; preds = %DIR.OMP.SIMD.1
  %lpriv.lpriv.promoted = load i64, i64* %lpriv.lpriv, align 8
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %sub1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.126, %omp.inner.for.inc
  %1 = phi i64 [ %lpriv.lpriv.promoted, %DIR.OMP.SIMD.126 ], [ %3, %omp.inner.for.inc ]
  %.omp.iv.local.020 = phi i64 [ 0, %DIR.OMP.SIMD.126 ], [ %add7, %omp.inner.for.inc ]
  %cmp6 = icmp eq i64 %.omp.iv.local.020, %sub1
  br i1 %cmp6, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %2 = load i64, i64* %arrayidx, align 8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %3 = phi i64 [ %2, %if.then ], [ %1, %omp.inner.for.body ]
  %add7 = add nuw nsw i64 %.omp.iv.local.020, 1
  %exitcond.not = icmp eq i64 %add7, %N
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  %.lcssa = phi i64 [ %3, %omp.inner.for.inc ]
  store i64 %N, i64* %l1.linear.iv, align 8
  store i64 %.lcssa, i64* %lpriv.lpriv, align 8
  br label %DIR.OMP.END.SIMD.227

DIR.OMP.END.SIMD.227:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.227, %entry
  %lpriv.1 = phi i64 [ 0, %entry ], [ %.lcssa, %DIR.OMP.END.SIMD.227 ]
  ret i64 %lpriv.1
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
