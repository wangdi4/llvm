; This tests check that we correctly widen non-induction phi's and the select
; instruction when their original operands are vector-types to begin with.

; RUN: opt -S -VPlanDriver -vplan-force-vf=2 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


;;;;;;;;;;; Test the generation of correct vec-phis.
;     BB0 (s/D/U) <---+
;    /   \        |
;   /     |       |
;  BB1    |       |
;    \   /        |
;     BB2         |
;      |          |
;    Latch--------+

define void @test_vec_phi(i1 %flag1, <3 x i32>* %a, <3 x i32>* %b, <3 x i32>* %c) local_unnamed_addr {
; CHECK-LABEL:@test_vec_phi
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
; CHECK: vector.body
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %latch ]
  %gep1 = getelementptr <3 x i32>, <3 x i32>* %a, i32 %indvars.iv
  %ld1 = load <3 x i32>, <3 x i32>* %gep1, align 4
  %gep2 = getelementptr <3 x i32>, <3 x i32>* %b, i32 %indvars.iv
  %ld2 = load <3 x i32>, <3 x i32>* %gep2, align 4
  %gep3 = getelementptr <3 x i32>, <3 x i32>* %c, i32 %indvars.iv
  %ld3 = load <3 x i32>, <3 x i32>* %gep3, align 4
  %add1 = add <3 x i32> %ld2, %ld3
; CHECK: [[ADD1:%.*]] = add <6 x i32> [[WMG2:%.*]], [[WMG3:%.*]]
  br i1 %flag1, label %if, label %merge

if:
  %add2 = add <3 x i32> %ld1, %add1
; CHECK: [[ADD2:%.*]] = add <6 x i32> [[WMG1:%.*]], [[ADD1]]
  br label %merge

merge:
  %phi_add = phi <3 x i32> [ %add2, %if], [%add1, %for.body]
; CHECK:  [[VEC_PHI:%.*]] = phi <6 x i32> [ [[ADD2]], {{.*}} ], [ [[ADD1]], %vector.body ]
  br label %latch

latch:
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void

}

;;;;;;;;;;; Test the generation of correct blend-phis.

define void @test_blend_phis(i32* %a, i32 %b, <2 x i32> %avec, <2 x i32> %bvec) local_unnamed_addr {
; CHECK-LABEL:@test_blend_phis
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

;     BB0 (D) <------+
;    /   \           |
;   /     \          |
;  BB3   BB2         |
;    \   /           |
;     BB4            |
;      |             |
;    Latch-----------+

for.body:
; CHECK: vector.body
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %latch ]
  %gep = getelementptr i32, i32 * %a, i32 %indvars.iv
  %ld = load i32, i32* %gep, align 4
; CHECK: [[WIDE_LOAD:%.*]] = load <2 x i32>, <2 x i32>* {{.*}}, align 4
  %varying = icmp eq i32 %ld,  42
; CHECK: [[WIDE_ICMP:%.*]] = icmp eq <2 x i32> [[WIDE_LOAD]], <i32 42, i32 42>
  br label %bb0

bb0:
  %vec1 = add <2 x i32> %avec, <i32 7, i32 3>
; CHECK: [[WIDE_A_VEC:%.*]] = shufflevector <2 x i32> %avec, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[ADD1:%.*]] = add <4 x i32> [[WIDE_A_VEC]], <i32 7, i32 3, i32 7, i32 3>
  %vec2 = add <2 x i32> %bvec, <i32 4, i32 5>
; CHECK: [[WIDE_B_VEC:%.*]] = shufflevector <2 x i32> %bvec, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[ADD2:%.*]] = add <4 x i32> [[WIDE_B_VEC]], <i32 4, i32 5, i32 4, i32 5>
  %bb1.varying = or i1 %varying, true
  br i1 %bb1.varying, label %bb3, label %bb2

bb2:
  %def2 = phi <2 x i32> [%vec1, %bb0 ]
  br label %bb4

bb3:
  %def1 = phi <2 x i32> [%vec2, %bb0 ]
  br label %bb4

bb4:
  %phi_use = phi <2 x i32> [ %def1, %bb3 ], [ %def2, %bb2 ]
; CHECK : [[WIDE_SELECTOR:%.*]] = shufflevector <2 x i1> [[WIDE_ICMP]], <2 x i1> undef, <4 x i32> <i32 0, i32 0, i32 1, i32 1>
; CHECK-NEXT : [[PRED_PHI:%.*]] = select <4 x i1> [[WIDE_SELECTOR]], <4 x i32> [[ADD2]], <4 x i32> [[ADD1]]
  %bb4.add = add i32 %ld, 4
  br label %latch

latch:
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}


;;;;;;;;;;; Test based on the original Embree kernel.

%"GPU_AABB" = type { {<4 x float>}, <4 x float> }
%"Globals" = type <{ %"GPU_AABB", %"GPU_AABB", i32, i32, [14 x i32], i32, i32, i32, i32, i32, [11 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, float, i32, float, i32, i32, i32, i32, i32, i32, [8 x i8] }>

; CHECK-LABEL:@test
; CHECK: vector.body:
; CHECK:   [[VEC_IND:%.*]] = phi <2 x i64>
; CHECK:   [[CMP1:%.*]] = icmp ult <2 x i64> [[VEC_IND]], {{.*}}
; CHECK:   br label %[[VPlannedBB1:.*]]

; CHECK: [[VPlannedBB1]]:
; CHECK:   [[VEC_PHI1:%.*]] = phi <8 x float>
; CHECK:   [[VEC_PHI2:%.*]] = phi <8 x float>
; CHECK:   [[VEC_PHI3:%.*]] = phi <2 x i64>
; CHECK:   [[VEC_PHI4:%.*]] = phi <2 x i1>
; CHECK:   [[AND1:%.*]] = and <2 x i1> [[CMP1]], [[VEC_PHI4]]
; CHECK:   [[FADD:%.*]] = fadd <8 x float> [[VEC_PHI2]], [[VEC_PHI2]]
; CHECK:   [[PRED0:%.*]] = extractelement <2 x i1> [[AND1]], i64 0
; CHECK:   [[CMP0:%.*]] = icmp eq i1 [[PRED0]], true
; CHECK:   br i1 [[CMP0]], label %[[PRED_CALL_IF1:.*]], label %[[LBL1:.*]]

; CHECK: [[PRED_CALL_IF1]]:
; CHECK:   [[MAX1:%.*]] = call <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> {{.*}}, <4 x float> {{.*}})
; CHECK:   br label %[[LBL1:.*]]

; CHECK: [[LBL1]]:
; CHECK:   [[PHI1:%.*]] = phi <4 x float>
; CHECK:   br label %[[PRED_CALL_CONTINUE1:.*]]

; CHECK: [[PRED_CALL_CONTINUE1]]:
; CHECK:   [[PRED1:%.*]] = extractelement <2 x i1> [[AND1]], i64 1
; CHECK:   [[CMP2:%.*]] = icmp eq i1 [[PRED1]], true
; CHECK:   br i1 [[CMP2]], label %[[PRED_CALL_IF2:.*]], label %[[LBL2:.*]]

; CHECK: [[PRED_CALL_IF2:[0-9]+]]:
; CHECK:   [[MAX2:%.*]] = call <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> {{.*}}, <4 x float> {{.*}})
; CHECK:   br label %[[LBL2:.*]]

; CHECK: [[LBL2]]:
; CHECK:   [[PHI2:%.*]] = phi <4 x float>
; CHECK:   br label %[[PRED_CALL_CONTINUE2:.*]]

; CHECK: [[PRED_CALL_CONTINUE2]]:
; CHECK:   [[SHUF1:%.*]] = shufflevector <4 x float> [[PHI1]], <4 x float> [[PHI2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK:   [[ADD1:%.*]] = add <2 x i64> [[VEC_PHI3]], {{.*}}
; CHECK:   [[CMP3:%.*]] = icmp ult <2 x i64> [[ADD1]], {{.*}}
; CHECK:   [[AND2:%.*]] = and <2 x i1> [[CMP3]], [[VEC_PHI4]]
; CHECK:   [[VEC_PHI49:%.*]] = shufflevector <2 x i1> [[VEC_PHI4]], <2 x i1> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:   [[WIDE_SELECT1:%.*]] = select <8 x i1> [[VEC_PHI49]], <8 x float> [[SHUF1]], <8 x float> [[VEC_PHI1]]
; CHECK:   br i1 {{.*}}, label %[[VPlannedBB1]], label %[[VPlannedBB2:.*]]

; CHECK: [[VPlannedBB2]]:
; CHECK:   [[SHUF2:%.*]] = shufflevector <2 x i1> [[CMP1]], <2 x i1> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:   [[PRED_PHI:%.*]] = select <8 x i1> [[SHUF2]], <8 x float> [[WIDE_SELECT1]], <8 x float>
; CHECK:   br i1 [[EXIT_COND:%.*]], label [[MIDDLE_BLOCK:%.*]], label %vector.body


; This is a minimalist IR which is identical to the control-flow from one of the Embree kernels.
; There is a massaging/canonicalization of control-flow in merge-loop exit transformation which generates
; the IR on which the codegen originally choked. In the future, if merge-loop exit transform is modified and
; the author of the fix deems it appropriate, this function can be dropped.

; Function Attrs: nounwind
define void @test_embree(i64 %0, %"GPU_AABB" addrspace(1)* %1) {
  %alloca. = alloca i64
  br label %simd.begin.region

simd.begin.region:                                ; preds = %4
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i64* %alloca.) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load. = load i64, i64* %alloca.
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.loop.preheader
  %index = phi i64 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %cmp0 = icmp ult i64 %index, %load.
  br i1 %cmp0, label %ph.i, label %Exit

ph.i:                                         ; preds = %simd.loop
  %trunc1 = trunc i64 %index to i32
  br label %PRE

PRE:                                               ; preds = %PRE, %ph.i
  %sroa.862 = phi <4 x float> [ <float 2.0e+00, float 2.0e+00, float 2.0e+00, float 0.000000e+00>, %ph.i ], [ %max1, %PRE ]
  %idx.next = phi i64 [ %index, %ph.i ], [ %add1, %PRE ]
  %Fadd = fadd <4 x float> %sroa.862, %sroa.862
  %max1 = call <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> %sroa.862, <4 x float> %Fadd)
  %add1 = add i64 %idx.next, %index
  %cmp = icmp ult i64 %add1, %load.
  br i1 %cmp, label %PRE, label %LoopExit

LoopExit: ; preds = %PRE
  %.lcssa8 = phi <4 x float> [ %max1, %PRE ]
  br label %Exit

Exit: ; preds = %LoopExit, %simd.loop
  %.sroa.862.0.i = phi <4 x float> [ <float 2.0e+00, float 2.0e+00, float 2.0e+00, float 0.000000e+00>, %simd.loop ], [ %.lcssa8, %LoopExit ]
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %Exit
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !1

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}


!2 = !{!"llvm.loop.unroll.disable"}
!1 = distinct !{!1, !2}

; Function Attrs: nounwind
declare <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> %0, <4 x float> %1)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)
