; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -S -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check the ordering of temp assignments between PiBlock within new ordering list.

; BEGIN REGION { }
;       + DO i1 = 0, 19, 1   <DO_LOOP>
;       |   %t.0 = 1.000000e+00;
;       |   if (i1 <u 8)
;       |   {
;       |      %0 = (@E)[0][i1];
;       |      %t.0 = %0;
;       |   }
;       |   %1 = (@B)[0][i1];
;       |   %2 = (@C)[0][i1];
;       |   %add = %1  +  %2;
;       |   %4 = (@E)[0][i1 + 8];
;       |   %add9 = %add  +  %4;
;       |   (@A)[0][i1] = %add9;
;       |   %5 = (@A)[0][i1 + 1];
;       |   %add15 = %t.0  +  %5;
;       |   (@D)[0][i1] = %add15;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK: modified

; CHECK: DO i1
; CHECK: %t.0 =
; CHECK: if
; CHECK:   %t.0 =
; CHECK: }
; CHECK-NOT: %t.0 =
; CHECK: END LOOP

; CHECK: DO i1
; CHECK: END LOOP

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@E = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %cmp1 = icmp ult i64 %indvars.iv, 8
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x float], ptr @E, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %t.0 = phi float [ %0, %if.then ], [ 1.000000e+00, %for.body ]
  %arrayidx3 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds [100 x float], ptr @C, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %2 = load float, ptr %arrayidx5, align 4, !tbaa !2
  %add = fadd float %1, %2
  %3 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx8 = getelementptr inbounds [100 x float], ptr @E, i64 0, i64 %3, !intel-tbaa !2
  %4 = load float, ptr %arrayidx8, align 4, !tbaa !2
  %add9 = fadd float %add, %4
  %arrayidx11 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %add9, ptr %arrayidx11, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx14 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  %5 = load float, ptr %arrayidx14, align 4, !tbaa !2
  %add15 = fadd float %t.0, %5
  %arrayidx17 = getelementptr inbounds [100 x float], ptr @D, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %add15, ptr %arrayidx17, align 4, !tbaa !2
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
