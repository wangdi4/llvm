; RUN: opt -aa-pipeline=tbaa -passes='print<access-info>' -disable-output < %s 2>&1 | FileCheck %s
;
; We expect there exist only one anti-dep in this loop.
;
;  for (int i=0;i<N;i++) {
;    t->b[i] =t->c[i];
;    t->c[i]=0.0;
;  }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test = type { [1000 x double], [1000 x double], [1000 x double] }

@t = common local_unnamed_addr global ptr null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr {
; CHECK: Dependences:
; CHECK-NEXT:   Forward:
; CHECK-NEXT:       load i64, ptr %arrayidx, align 8, !tbaa !5 ->
; CHECK-NEXT:       store double 0.000000e+00, ptr %arrayidx, align 8, !tbaa !5
; CHECK-NOT:     Forward:

entry:
  %0 = load ptr, ptr @t, align 8, !tbaa !1
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %struct.test, ptr %0, i64 0, i32 2, i64 %indvars.iv
  %1 = load i64, ptr %arrayidx, align 8, !tbaa !5
  %arrayidx2 = getelementptr inbounds %struct.test, ptr %0, i64 0, i32 1, i64 %indvars.iv
  store i64 %1, ptr %arrayidx2, align 8, !tbaa !9
  store double 0.000000e+00, ptr %arrayidx, align 8, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17859)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"pointer@_ZTSP4test", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !8, i64 16000}
!6 = !{!"struct@test", !7, i64 0, !7, i64 8000, !7, i64 16000}
!7 = !{!"array@_ZTSA1000_d", !8, i64 0}
!8 = !{!"double", !3, i64 0}
!9 = !{!6, !8, i64 8000}
