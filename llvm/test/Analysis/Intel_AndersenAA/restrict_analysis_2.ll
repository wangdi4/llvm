; Restrict analysis in anders-aa shouldn't compute %b and %arrayidx3
; are NoAlias. %b is marked as noalias (restrict) argument and %arrayidx3
; is a copy of %b.
;
; RUN: opt < %s -anders-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK-NOT:  NoAlias:      i32* %arrayidx3, i32* %b

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* noalias nocapture readonly %a, i32* noalias nocapture %b, i32* nocapture readnone %c, i32 %n) local_unnamed_addr {
entry:
  store i32 0, i32* %b, align 4, !tbaa !2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx1, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add = add i32 %0, 1
  %add4 = add i32 %add, %1
  store i32 %add4, i32* %arrayidx3, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: norecurse nounwind uwtable
define void @bar(i32* noalias nocapture readonly %a, i32* noalias nocapture %b, i32* nocapture readnone %c, i32 %n) local_unnamed_addr  {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 1
  store i32 0, i32* %arrayidx, align 4, !tbaa !2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx1, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add = add i32 %0, 1
  %add4 = add i32 %add, %1
  store i32 %add4, i32* %arrayidx3, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 988c0a4a4fa557da73c722f549beb6a31243207f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d9ce6905d03ca71e892716d49969ebcf03345911)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
