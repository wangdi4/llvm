; Test for generating mkl call for matrix multiplication of linearlized 1D matrices with double data type

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-generate-mkl-call,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; Before HIR Generate MKL Call-
; + DO i1 = 0, 499, 1   <DO_LOOP>
; |   + DO i2 = 0, 299, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 399, 1   <DO_LOOP>
; |   |   |   %3 = (@C)[0][400 * i1 + i3];
; |   |   |   %mul11.i = (@A)[0][300 * i1 + i2]  *  (@B)[0][400 * i2 + i3];
; |   |   |   %3 = %3  +  %mul11.i;
; |   |   |   (@C)[0][400 * i1 + i3] = %3;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; After HIR Generate MKL Call-
; CHECK: BEGIN REGION { modified }
; CHECK: 0 = &((i8*)(@C)[0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 400;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 500;
; CHECK: 10 = 3200;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@B)[0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 400;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 300;
; CHECK: 10 = 3200;
; CHECK: 11 = 1;
; CHECK: 0 = &((i8*)(@A)[0][0]);
; CHECK: 1 = 8;
; CHECK: 2 = 0;
; CHECK: 3 = 0;
; CHECK: 4 = 2;
; CHECK: 5 = 0;
; CHECK: 6 = 300;
; CHECK: 7 = 8;
; CHECK: 8 = 1;
; CHECK: 9 = 500;
; CHECK: 10 = 2400;
; CHECK: 11 = 1;
; CHECK: @matmul_mkl_f64_

;Module Before HIR
; ModuleID = 'double-linearize-matmul.cpp'
source_filename = "double-linearize-matmul.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [250000 x double] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [250000 x double] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [250000 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.inc20.i, %entry
  %indvars.iv11.i = phi i64 [ 0, %entry ], [ %indvars.iv.next12.i, %for.inc20.i ]
  %0 = mul nuw nsw i64 %indvars.iv11.i, 300
  %1 = mul nuw nsw i64 %indvars.iv11.i, 400
  br label %for.cond4.preheader.i

for.cond4.preheader.i:                            ; preds = %for.inc17.i, %for.cond1.preheader.i
  %indvars.iv7.i = phi i64 [ 0, %for.cond1.preheader.i ], [ %indvars.iv.next8.i, %for.inc17.i ]
  %2 = add nuw nsw i64 %indvars.iv7.i, %1
  %arrayidx15.i = getelementptr inbounds [250000 x double], ptr @C, i64 0, i64 %2
  %arrayidx15.promoted.i = load double, ptr %arrayidx15.i, align 8, !tbaa !2
  br label %for.body6.i

for.body6.i:                                      ; preds = %for.body6.i, %for.cond4.preheader.i
  %indvars.iv.i = phi i64 [ 0, %for.cond4.preheader.i ], [ %indvars.iv.next.i, %for.body6.i ]
  %3 = phi double [ %arrayidx15.promoted.i, %for.cond4.preheader.i ], [ %add16.i, %for.body6.i ]
  %4 = add nuw nsw i64 %indvars.iv.i, %0
  %arrayidx.i = getelementptr inbounds [250000 x double], ptr @A, i64 0, i64 %4
  %5 = load double, ptr %arrayidx.i, align 8, !tbaa !2
  %6 = mul nuw nsw i64 %indvars.iv.i, 400
  %7 = add nuw nsw i64 %6, %indvars.iv7.i
  %arrayidx10.i = getelementptr inbounds [250000 x double], ptr @B, i64 0, i64 %7
  %8 = load double, ptr %arrayidx10.i, align 8, !tbaa !2
  %mul11.i = fmul double %5, %8
  %add16.i = fadd double %3, %mul11.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, 300
  br i1 %exitcond.i, label %for.inc17.i, label %for.body6.i

for.inc17.i:                                      ; preds = %for.body6.i
  %add16.i.lcssa = phi double [ %add16.i, %for.body6.i ]
  store double %add16.i.lcssa, ptr %arrayidx15.i, align 8, !tbaa !2
  %indvars.iv.next8.i = add nuw nsw i64 %indvars.iv7.i, 1
  %exitcond10.i = icmp eq i64 %indvars.iv.next8.i, 400
  br i1 %exitcond10.i, label %for.inc20.i, label %for.cond4.preheader.i

for.inc20.i:                                      ; preds = %for.inc17.i
  %indvars.iv.next12.i = add nuw nsw i64 %indvars.iv11.i, 1
  %exitcond15.i = icmp eq i64 %indvars.iv.next12.i, 500
  br i1 %exitcond15.i, label %_ZL6matmulPdS_S_iii.exit, label %for.cond1.preheader.i

_ZL6matmulPdS_S_iii.exit:                         ; preds = %for.inc20.i
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
