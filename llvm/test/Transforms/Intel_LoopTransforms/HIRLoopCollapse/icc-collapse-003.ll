; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; HIR Loop Collapse Sanity Test: testcase is reduced from icc's loopcollapse test - collaps_003.c
; This testcase won't trigger loop collapse.
;
;
; *** Source Code ***
;void init(int *a){
;  int i, j, k;
;
;  for(i=0; i < 10; i++){
;    for(j=0; j < 10; j++){
;      for(k=0; k < 10; k++){
;        a[9*i+j+k] = i+j+k;
;      }
;    }
;  }
;}
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (%a)[9 * i1 + i2 + i3] = i1 + i2 + i3;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; CHECK: Function
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK:        |   |   |   (%a)[9 * i1 + i2 + i3] = i1 + i2 + i3;
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "collaps_003.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.1 = private unnamed_addr constant [14 x i8] c"TEST PASSED \0A\00", align 1
@str = private unnamed_addr constant [13 x i8] c"TEST PASSED \00"

; Function Attrs: norecurse nounwind uwtable
define void @init(ptr nocapture %a) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %entry
  %indvars.iv38 = phi i64 [ 0, %entry ], [ %indvars.iv.next39, %for.inc13 ]
  %0 = mul nuw nsw i64 %indvars.iv38, 9
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc10, %for.cond1.preheader
  %indvars.iv33 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next34, %for.inc10 ]
  %1 = add nuw nsw i64 %indvars.iv33, %indvars.iv38
  %2 = add nuw nsw i64 %indvars.iv33, %0
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %3 = add nuw nsw i64 %1, %indvars.iv
  %4 = add nuw nsw i64 %2, %indvars.iv
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %4
  %5 = trunc i64 %3 to i32
  store i32 %5, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc10, label %for.body6

for.inc10:                                        ; preds = %for.body6
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next34, 10
  br i1 %exitcond37, label %for.inc13, label %for.cond4.preheader

for.inc13:                                        ; preds = %for.inc10
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next39, 10
  br i1 %exitcond41, label %for.end15, label %for.cond1.preheader

for.end15:                                        ; preds = %for.inc13
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.inc13.i, %entry
  %indvars.iv38.i = phi i64 [ 0, %entry ], [ %indvars.iv.next39.i, %for.inc13.i ]
  br label %for.cond4.preheader.i

for.cond4.preheader.i:                            ; preds = %for.inc10.i, %for.cond1.preheader.i
  %indvars.iv33.i = phi i64 [ 0, %for.cond1.preheader.i ], [ %indvars.iv.next34.i, %for.inc10.i ]
  br label %for.body6.i

for.body6.i:                                      ; preds = %for.body6.i, %for.cond4.preheader.i
  %indvars.iv.i = phi i64 [ 0, %for.cond4.preheader.i ], [ %indvars.iv.next.i, %for.body6.i ]
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, 10
  br i1 %exitcond.i, label %for.inc10.i, label %for.body6.i

for.inc10.i:                                      ; preds = %for.body6.i
  %indvars.iv.next34.i = add nuw nsw i64 %indvars.iv33.i, 1
  %exitcond37.i = icmp eq i64 %indvars.iv.next34.i, 10
  br i1 %exitcond37.i, label %for.inc13.i, label %for.cond4.preheader.i

for.inc13.i:                                      ; preds = %for.inc10.i
  %indvars.iv.next39.i = add nuw nsw i64 %indvars.iv38.i, 1
  %exitcond41.i = icmp eq i64 %indvars.iv.next39.i, 10
  br i1 %exitcond41.i, label %for.body.preheader, label %for.cond1.preheader.i

for.body.preheader:                               ; preds = %for.inc13.i
  %puts = tail call i32 @puts(ptr @str)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly) #4

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21316) (llvm/branches/loopopt 21336)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
