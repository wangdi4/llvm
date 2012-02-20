; ModuleID = '/tmp/webcompile/_4159_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @func1(i32 %s0, i32* nocapture %a) nounwind uwtable readonly {
  br label %1

; <label>:1                                       ; preds = %1, %0
  %indvars.iv = phi i64 [ 0, %0 ], [ %indvars.iv.next, %1 ]
  %s.02 = phi i32 [ %s0, %0 ], [ %4, %1 ]
  %2 = getelementptr inbounds i32* %a, i64 %indvars.iv
  %3 = load i32* %2, align 4, !tbaa !0
  %4 = add nsw i32 %3, %s.02
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 100
  br i1 %exitcond, label %5, label %1

; <label>:5                                       ; preds = %1
  ret i32 %4
}

!0 = metadata !{metadata !"int", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}

; source code:
;int func1(int s0, int *a)
;{
;  int i;
;  int s = s0;
;  for (i = 0; i < 100; i++) {
;    s = s+a[i];
;  }
;  return s;
;}
;
