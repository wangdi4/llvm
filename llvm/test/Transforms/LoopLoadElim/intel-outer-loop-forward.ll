; RUN: opt -loop-load-elim -S < %s | FileCheck %s

;  Do not apply transformation when Load Ptr induction variable
;    is incremeted in outer loop. 
;
;    for (int i = 0; i <= 1; i++)
;      for (int j = i; j <= 2; j += 1)
;        A[i + 1] = A[i] + A[0];
;
; CHECK-NOT:   %load-initial =
; CHECK-NOT:   %store_forwarded = phi

target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

; Function Attrs: noinline nounwind readnone uwtable
define i32 @main() local_unnamed_addr #0 {
  %A = alloca [3 x i32], align 4
  %1 = bitcast [3 x i32]* %A to i8*
  %2 = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 1
  %3 = bitcast i32* %2 to i64*
  store i64 0, i64* %3, align 4
  %arrayidx = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 0
  store i32 1, i32* %arrayidx, align 4
  br label %.lr.ph

; <label>:4:                                      ; preds = %._crit_edge
  %5 = load i32, i32* %arrayidx, align 4
  %arrayidx9 = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 1
  %6 = load i32, i32* %arrayidx9, align 4
  %mul = mul nsw i32 %6, %5
  %arrayidx10 = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 2
  %7 = load i32, i32* %arrayidx10, align 4
  %mul11 = mul nsw i32 %mul, %7
  %sub = add nsw i32 %mul11, -6
  ret i32 %sub

.lr.ph:                                           ; preds = %0, %._crit_edge
  %indvars.iv = phi i64 [ 0, %0 ], [ %indvars.iv.next, %._crit_edge ]
  %arrayidx2 = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 %indvars.iv
  %8 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds [3 x i32], [3 x i32]* %A, i64 0, i64 %8
  %9 = trunc i64 %indvars.iv to i32
  br label %10

._crit_edge:                                      ; preds = %10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond23, label %4, label %.lr.ph

; <label>:10:                                     ; preds = %10, %.lr.ph
  %j.020 = phi i32 [ %9, %.lr.ph ], [ %add7, %10 ]
  %11 = load i32, i32* %arrayidx2, align 4
  %12 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %12, %11
  store i32 %add, i32* %arrayidx6, align 4
  %add7 = add nuw nsw i32 %j.020, 1
  %exitcond = icmp eq i32 %add7, 3
  br i1 %exitcond, label %._crit_edge, label %10
}
