;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa"    < %s 2>&1 | FileCheck %s
;
;  Testing for no distribution
;   for (i =0 ; i<n ; i++) {
; #pragma distribute_point
;    a1[i] = a4[i+1] - a4[i+3];
;    a2[i] = a4[i+2] - a4[i+7];
;    a3[i] = a4[i+3] + a4[i+8]; }
;
; CHECK:  BEGIN REGION
; CHECK:    DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:      %1 = (@a4)[0][i1 + 1];
; CHECK:      %3 = (@a4)[0][i1 + 3];
; CHECK:      %sub = %1  -  %3;
; CHECK:      (@a1)[0][i1] = %sub;
; CHECK:      %5 = (@a4)[0][i1 + 2];
; CHECK:      %7 = (@a4)[0][i1 + 7];
; CHECK:      %sub12 = %5  -  %7;
; CHECK:      (@a2)[0][i1] = %sub12;
; CHECK:      %8 = (@a4)[0][i1 + 3];
; CHECK:      %10 = (@a4)[0][i1 + 8];
; CHECK:      %add21 = %8  +  %10;
; CHECK:      (@a3)[0][i1] = %add21;
; CHECK:    END LOOP
; CHECK:  END REGION
;
;Module Before HIR; ModuleID = 'distpoint3.c'
source_filename = "distpoint3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a4 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a1 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a2 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a3 = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp35 = icmp sgt i32 %n, 0
  br i1 %cmp35, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %indvars.iv.next
  %1 = load float, ptr %arrayidx, align 4, !tbaa !2
  %2 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx3 = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %2
  %3 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %sub = fsub float %1, %3
  %arrayidx5 = getelementptr inbounds [1000 x float], ptr @a1, i64 0, i64 %indvars.iv
  store float %sub, ptr %arrayidx5, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %4 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx8 = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %4
  %5 = load float, ptr %arrayidx8, align 4, !tbaa !2
  %6 = add nuw nsw i64 %indvars.iv, 7
  %arrayidx11 = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %6
  %7 = load float, ptr %arrayidx11, align 4, !tbaa !2
  %sub12 = fsub float %5, %7
  %arrayidx14 = getelementptr inbounds [1000 x float], ptr @a2, i64 0, i64 %indvars.iv
  store float %sub12, ptr %arrayidx14, align 4, !tbaa !2
  %8 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %9 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx20 = getelementptr inbounds [1000 x float], ptr @a4, i64 0, i64 %9
  %10 = load float, ptr %arrayidx20, align 4, !tbaa !2
  %add21 = fadd float %8, %10
  %arrayidx23 = getelementptr inbounds [1000 x float], ptr @a3, i64 0, i64 %indvars.iv
  store float %add21, ptr %arrayidx23, align 4, !tbaa !2
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 65e8f9d46b54671e271ba934ab45010c98c98cce) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d5a4f1cbcbff7885d7b00fe044003cf37ba39d4d)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

