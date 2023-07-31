; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Dead Store Elimination ***
;<0>       BEGIN REGION { }
;<21>            + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;<4>             |   if (i1 >u 5)
;<4>             |   {
;<8>             |      (@a)[0][i1] = 3;
;<4>             |   }
;<12>            |   (@a)[0][i1] = i1;
;<13>            |   %add = %s.017  +  i1;
;<16>            |   %s.017 = %add;
;<21>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK-NEXT:   |   (@a)[0][i1] = i1;
; CHECK:        |   %add = %s.017  +  i1;
; CHECK:        |   %s.017 = %add;
; CHECK:        + END LOOP
; CHECK:  END REGION

; ModuleID = 'test.ll'
source_filename = "test.ll"

@a = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end ]
  %s.017 = phi i32 [ 0, %for.body.preheader ], [ %add, %if.end ]
  %cmp1 = icmp ugt i64 %indvars.iv, 5
  %arrayidx = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %indvars.iv
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  store i32 3, ptr %arrayidx, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4, !tbaa !2
  %add = add nuw nsw i32 %s.017, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.end.loopexit ]
  ret i32 %s.0.lcssa
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f60752aa03c2809f5b861db02cfa7e2e3be7d8bd)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
