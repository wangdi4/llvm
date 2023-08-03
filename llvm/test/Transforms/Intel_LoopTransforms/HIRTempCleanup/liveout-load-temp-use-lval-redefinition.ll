; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that the region liveout load temp %i32 is not eliminated by propagating
; its rval to the copy: (%.lcssa348 = %i32) and making %.lcssa348 liveout
; instead. Since %.lcssa348 is redefined later in the region, it cannot be used
; as the substitute liveout temp.

; Note that %i32 is considered liveout since it is used in this loop exit phi-
; %.lcssa384 = phi i32 [ %i32, %for.end83 ]

; CHECK: LiveOuts: 
; CHECK-SAME: %i32

; CHECK: + DO i1 = 0, 77, 1   <DO_LOOP>
; CHECK: |   %indvars.iv376.out = -1 * i1 + 81;
; CHECK: |   %i32 = (%pu7)[0][-1 * i1 + 80];
; CHECK: |   %.lcssa348 = %i32;
; CHECK: |
; CHECK: |   + DO i2 = 0, i1 + -57, 1   <DO_LOOP>  <MAX_TC_EST = 21>  <LEGAL_MAX_TC = 21>
; CHECK: |   |   %i38 = (%k4)[0][-1 * i1 + 80][-1 * i2 + 25];
; CHECK: |   |   (%ma7)[0][-1 * i2 + 24][-1 * i2 + 24] = 14;
; CHECK: |   + END LOOP
; CHECK: |      %.lcssa348 = %i38;
; CHECK: + END LOOP

define void @foo(ptr %k4, ptr %ma7, ptr %pu7) {
entry:
  br label %for.body32

for.body32:                                       ; preds = %for.end83, %entry
  %indvars.iv376 = phi i64 [ 81, %entry ], [ %indvars.iv.next377, %for.end83 ]
  %indvars.iv374 = phi i64 [ 0, %entry ], [ %indvars.iv.next375, %for.end83 ]
  %indvars.iv361 = phi i32 [ -57, %entry ], [ %indvars.iv.next362, %for.end83 ]
  %i31 = sub i32 24, %indvars.iv361
  %indvars.iv.next377 = add nsw i64 %indvars.iv376, -1
  %arrayidx35 = getelementptr inbounds [100 x i32], ptr %pu7, i64 0, i64 %indvars.iv.next377
  %i32 = load i32, ptr %arrayidx35, align 4
  %cmp37339 = icmp ult i64 %indvars.iv376, 25
  br i1 %cmp37339, label %for.body38.preheader, label %for.cond63.preheader.preheader

for.body38.preheader:                             ; preds = %for.body32
  br label %for.body38

for.body38:                                       ; preds = %for.body38, %for.body38.preheader
  %indvars.iv359 = phi i64 [ 25, %for.body38.preheader ], [ %indvars.iv.next360, %for.body38 ]
  %arrayidx43 = getelementptr inbounds [100 x [100 x i32]], ptr %k4, i64 0, i64 %indvars.iv.next377, i64 %indvars.iv359
  %i38 = load i32, ptr %arrayidx43, align 4
  %indvars.iv.next360 = add nsw i64 %indvars.iv359, -1
  %arrayidx49 = getelementptr inbounds [100 x [100 x i32]], ptr %ma7, i64 0, i64 %indvars.iv.next360, i64 %indvars.iv.next360
  store i32 14, ptr %arrayidx49, align 4
  %cmp37 = icmp ugt i64 %indvars.iv.next360, %indvars.iv376
  br i1 %cmp37, label %for.body38, label %for.cond63.preheader.preheader.loopexit

for.cond63.preheader.preheader.loopexit:          ; preds = %for.body38
  %.lcssa = phi i32 [ %i38, %for.body38 ]
  br label %for.cond63.preheader.preheader

for.cond63.preheader.preheader:                   ; preds = %for.cond63.preheader.preheader.loopexit, %for.body32
  %.lcssa348 = phi i32 [ %i32, %for.body32 ], [ %.lcssa, %for.cond63.preheader.preheader.loopexit ]
  br label %for.end83

for.end83:                                        ; preds = %for.inc81
  %indvars.iv.next375 = add nuw nsw i64 %indvars.iv374, 1
  %indvars.iv.next362 = add nsw i32 %indvars.iv361, 1
  %exitcond378.not = icmp eq i64 %indvars.iv.next375, 78
  br i1 %exitcond378.not, label %if.end.loopexit, label %for.body32

if.end.loopexit:                                  ; preds = %for.end83
  %.lcssa348.lcssa = phi i32 [ %.lcssa348, %for.end83 ]
  %indvars.iv376.lcssa = phi i64 [ %indvars.iv376, %for.end83 ]
  %.lcssa384 = phi i32 [ %i32, %for.end83 ]
  ret void
}

