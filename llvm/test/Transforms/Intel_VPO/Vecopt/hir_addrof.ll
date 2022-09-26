; LLVM IR generated from testcase below using icx -O1 -S -emit-llvm
; int *ip[1024];
; int arr[1024];
;
; void foo()
; {
;   int index;
;
;   for (index = 0; index < 1024; index++) {
;     ip[index] = &arr[index];
;   }
; }
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 < %s 2>&1 -disable-output | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 < %s 2>&1 -disable-output | FileCheck %s

; CHECK:      DO i1 = 0, 1023, 4   <DO_LOOP>  <auto-vectorized> <novectorize>
; CHECK-NEXT:     (<4 x i32*>*)(@ip)[0][i1] = &((<4 x i32*>)(@arr)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]);
; CHECK-NEXT: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common global [1024 x i32] zeroinitializer, align 16
@ip = common local_unnamed_addr global [1024 x i32*] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  %arrayidx2 = getelementptr inbounds [1024 x i32*], [1024 x i32*]* @ip, i64 0, i64 %indvars.iv
  store i32* %arrayidx, i32** %arrayidx2, align 8, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_Pi", !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
