; Check that TBAA removes egdes that do not alias. There are three vars int *%x, float *%y, char *%z.
; All three vars will share the same symbase because %z can alias both %x and %y.
; However, %x and %y could not alias to each other. TBAA could remove these dependency.

; int foo(int *x, float *y, char *z) {
;   int i;
;   for (i=0;i<100;i++) {
;     x[i] = (int)y[i];
;     *z = i;
;   }
;   return x[0];
; }

; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -tbaa < %s | FileCheck %s 
; RUN: opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze < %s | FileCheck %s --check-prefix=NOTBAA

; CHECK: DD graph for function:
; CHECK-NOT: {{.*}}%x{{.*}} --> {{.*}}%y{{.*}}
; CHECK-NOT: {{.*}}%y{{.*}} --> {{.*}}%x{{.*}}

; NOTBAA: DD graph for function:
; NOTBAA-DAG: {{.*}}%x{{.*}} --> {{.*}}%y{{.*}}
; NOTBAA-DAG: {{.*}}%y{{.*}} --> {{.*}}%x{{.*}}

; ModuleID = 'tbaa.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* nocapture %x, float* nocapture readonly %y, i8* nocapture %z) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %i.011 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %y, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %conv = fptosi float %0 to i32
  %arrayidx2 = getelementptr inbounds i32, i32* %x, i64 %indvars.iv
  store i32 %conv, i32* %arrayidx2, align 4, !tbaa !5
  %conv3 = trunc i32 %i.011 to i8
  store i8 %conv3, i8* %z, align 1, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.011, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %1 = load i32, i32* %x, align 4, !tbaa !5
  ret i32 %1
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 9676)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!3, !3, i64 0}
