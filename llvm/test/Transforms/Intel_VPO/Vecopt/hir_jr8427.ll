; Test from CMPLRLLVM-8427. Check that we can generate vector code.
; RUN: opt -scoped-noalias-aa -hir-ssa-deconstruction -hir-temp-cleanup -hir-runtime-dd -hir-lmm -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -print-after=hir-vplan-vec -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-lmm,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

; HIR Loop before vectorization:
;
; DO i2 = 0, 100, 1   <DO_LOOP>  <MVTag: 25>
;   %and = %limm  &&  i2;
;   %limm3 = 2 * i1 + %and * i2 + 2;
; END LOOP
;
; HIR Loop after vectorization:
;
; DO i2 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
;   %and.vec = %limm  &&  i2 + <i32 0, i32 1, i32 2, i32 3>;
;   %.BlobMul = %and.vec  *  <i32 0, i32 1, i32 2, i32 3>;
;   %copy.vec = 2 * i1 + %and.vec * i2 + %.BlobMul + 2;
; END LOOP
;
; When generating LLVM IR during hir-cg, the type of value for (2 * i1) is i32.
; The type of value for (%and.vec * i2) is <4 x i32>. We hit an assert when
; we try to generate an add instruction using these two values.
;
; CHECK: DO i2 = 0, {{.*}}, 4
; CHECK-LABEL: entry:
; CHECK: ret i32 0
;Module Before HIR
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common dso_local local_unnamed_addr global i32 0, align 4
@b = common dso_local local_unnamed_addr global i32* null, align 8
@a = common dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @d() local_unnamed_addr #0 {
entry:
  store i32 2, i32* @c, align 4, !tbaa !2
  %0 = load i32*, i32** @b, align 8, !tbaa !6
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end
  %storemerge12 = phi i32 [ 2, %entry ], [ %add5, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %e.011 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %1 = load i32, i32* %0, align 4, !tbaa !2
  %and = and i32 %1, %e.011
  %mul = mul i32 %and, %e.011
  %add = add i32 %mul, %storemerge12
  store i32 %add, i32* @a, align 4, !tbaa !2
  %inc = add nuw nsw i32 %e.011, 1
  %exitcond = icmp eq i32 %inc, 101
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add5 = add nuw nsw i32 %storemerge12, 2
  store i32 %add5, i32* @c, align 4, !tbaa !2
  %cmp = icmp ult i32 %add5, 1000
  br i1 %cmp, label %for.cond1.preheader, label %for.end6

for.end6:                                         ; preds = %for.end
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPi", !4, i64 0}
