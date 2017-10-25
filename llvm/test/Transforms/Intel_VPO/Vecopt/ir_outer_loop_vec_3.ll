; RUN: opt -S -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator < %s | FileCheck %s

; Outer loop vectorization
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


;#pragma omp simd
;  for (int i=0; i < iCount; i++) {
;    A[i] = c;
;    for (int j=1; i< jCount; i++) {
;      A[i] += B[j] + i;
;    }
;  }

; CHECK-LABEL: foo
; CHECK: vector.body
; CHECK: %[[I:.*]] = trunc <4 x i64> %vec.ind to <4 x i32>
; CHECK: store <4 x i32>
; CHECK: [[VP1:VPlannedBB[0-9]+]]:
; CHECK: phi i64 [{{.*}}, %[[VP1]] ], [ 0, %VPlannedBB{{[0-9]*}} ]
; CHECK: %A_val.vec = phi <4 x i32>
; CHECK: %[[B_j:.*]] = load i32, i32*
; CHECK: %[[B_j_insert:.*]] = insertelement <4 x i32> undef, i32 %[[B_j]], i32 0
; CHECK: %[[B_j_splat:.*]] = shufflevector <4 x i32> %[[B_j_insert]], <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK: add <4 x i32> %[[B_j_splat]], %[[I]]
; CHECK: add <4 x i32> %{{.*}}, %A_val.vec
; CHECK: [[VP2:VPlannedBB[0-9]+]]:
; CHECK:  %add6.lcssa.vec = phi <4 x i32> 
; CHECK:  getelementptr i32, i32* %A, i64 {{.*}}
; CHECK:  store <4 x i32> %add6.lcssa.vec, <4 x i32>*


define void @foo(i32* noalias nocapture %A, i32* noalias nocapture readonly %B, i32 %iCount, i32 %jCount, i32 %c) local_unnamed_addr #0 {
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %0
  %cmp18 = icmp sgt i32 %iCount, 0
  br i1 %cmp18, label %outer_loop.ph, label %.outer

outer_loop.ph:                                         ; preds = %DIR.QUAL.LIST.END.2
  %cmp116 = icmp sgt i32 %jCount, 0
  %wide.trip.count25 = zext i32 %iCount to i64
  %wide.trip.count = zext i32 %jCount to i64
  br label %outer_1

outer_1:                                      ; preds = %outer_2, %outer_loop.ph
  %indvars.iv23 = phi i64 [ 0, %outer_loop.ph ], [ %indvars.iv.next24, %outer_2 ]
  %arrayidx = getelementptr i32, i32* %A, i64 %indvars.iv23
  %iv.i = trunc i64 %indvars.iv23 to i32
  store i32 %c, i32* %arrayidx, align 4
  br i1 %cmp116, label %inner.preheader, label %outer_2

inner.preheader:                                 ; preds = %1
  br label %inner_loop


inner_loop:                                           ; preds = %inner.preheader, %inner_loop
  %indvars.iv = phi i64 [ %indvars.iv.next, %inner_loop ], [ 0, %inner.preheader ]
  %A_val = phi i32 [ %add6, %inner_loop ], [ %c, %inner.preheader ]
  %arrayidx3 = getelementptr i32, i32* %B, i64 %indvars.iv
  %B_val = load i32, i32* %arrayidx3, align 4
  %add = add i32 %B_val, %iv.i
  %add6 = add i32 %add, %A_val
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %inner_loop_exit, label %inner_loop

inner_loop_exit:                                      ; preds = %inner_loop
  %add6.lcssa = phi i32 [ %add6, %inner_loop ]
  store i32 %add6.lcssa, i32* %arrayidx, align 4
  br label %outer_2

outer_2:                                      ; preds = %inner_loop_exit, %1
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next24, %wide.trip.count25
  br i1 %exitcond26, label %.outer.loopexit, label %outer_1

.outer.loopexit:                           ; preds = %2
  br label %.outer

.outer:                                    ; preds = %.outer.loopexit, %DIR.QUAL.LIST.END.2
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %.outer
  ret void

}

declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)


