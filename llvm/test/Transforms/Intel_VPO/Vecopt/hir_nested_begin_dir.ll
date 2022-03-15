; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s
;
; We currently have a compiler crash when we see a nested begin directive in the
; VPlan HIR path. LLVM IR path bails out for such cases. Until we properly
; support such cases, test that we we bail out in HIR patch without crashing.
;
; Incoming HIR:
;         BEGIN REGION { }
;               %tok.outer = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;
;               + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;               |   %lp = (%lpp)[i1];
;               |   %tok.inner = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;               |
;               |   + DO i2 = 0, 199, 1   <DO_LOOP> <simd>
;               |   |   (%lp)[i2] = i1 + i2;
;               |   + END LOOP
;               |
;               |   @llvm.directive.region.exit(%tok.inner); [ DIR.OMP.END.SIMD() ]
;               + END LOOP
;
;               @llvm.directive.region.exit(%tok.outer); [ DIR.OMP.END.SIMD() ]
;               ret ;
;         END REGION
;
; CHECK:               %tok.outer = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK:               + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; CHECK:               |   %tok.inner = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK:               |   + DO i2 = 0, 199, 1   <DO_LOOP> <simd>
; CHECK:               |   + END LOOP
; CHECK:               |   @llvm.directive.region.exit(%tok.inner); [ DIR.OMP.END.SIMD() ]
; CHECK:               + END LOOP
; CHECK:               @llvm.directive.region.exit(%tok.outer); [ DIR.OMP.END.SIMD() ]
;
define void @foo(i64**  %lpp) {
entry:
  %tok.outer = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.for.body

outer.for.body:
  %iv.outer = phi i64 [ 0, %entry ], [ %add4, %outer.for.end ]
  %arrayidx = getelementptr inbounds i64*, i64** %lpp, i64 %iv.outer
  %lp = load i64*, i64** %arrayidx, align 8
  br label %inner.ph

inner.ph:
  %tok.inner = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %inner.for.body

inner.for.body:
  %iv.inner = phi i64 [ 0, %inner.ph ], [ %inc, %inner.for.body ]
  %add2 = add nuw nsw i64 %iv.inner, %iv.outer
  %arrayidx3 = getelementptr inbounds i64, i64* %lp, i64 %iv.inner
  store i64 %add2, i64* %arrayidx3, align 8
  %inc = add nuw nsw i64 %iv.inner, 1
  %exitcond.not = icmp eq i64 %inc, 200
  br i1 %exitcond.not, label %inner.exit, label %inner.for.body

inner.exit:
  call void @llvm.directive.region.exit(token %tok.inner) [ "DIR.OMP.END.SIMD"() ]
  br label %outer.for.end

outer.for.end:
  %add4 = add nuw nsw i64 %iv.outer, 1
  %exitcond24.not = icmp eq i64 %add4, 100
  br i1 %exitcond24.not, label %outer.exit, label %outer.for.body

outer.exit:
  call void @llvm.directive.region.exit(token %tok.outer) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
