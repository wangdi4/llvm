; This tests check that we correctly widen non-induction phi's and the select
; instruction when their original operands are vector-types to begin with.

; RUN: opt -S -vplan-vec -vplan-enable-all-zero-bypass-loops=false -vplan-force-vf=2 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


;;;;;;;;;;; Test the generation of correct vec-phis.
;     BB0 (s/D/U) <---+
;    /   \            |
;   /     |           |
;  BB1    |           |
;    \   /            |
;     BB2             |
;      |              |
;    Latch------------+

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
; CHECK:      [[ADD_VP1:%.*]] = add <2 x i32> %avec, <i32 7, i32 3>
; CHECK-NEXT: [[WIDE_A_VEC_VP:%.*]] = shufflevector <2 x i32> [[ADD_VP1]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
  %vec2 = add <2 x i32> %bvec, <i32 4, i32 5>
; CHECK:      [[ADD_VP2:%.*]] = add <2 x i32> %bvec, <i32 4, i32 5>
; CHECK-NEXT: [[WIDE_A_VEC_VP1:%.*]] = shufflevector <2 x i32> [[ADD_VP2]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
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
; CHECK:       vector.body:
; CHECK:         [[VEC_PHI0:%.*]] = phi <2 x i64> [ <i64 0, i64 1>, [[VECTOR_PH0:%.*]] ], [ [[TMP24:%.*]], [[VPLANNEDBB230:%.*]] ]
; CHECK-NEXT:    [[TMP3:%.*]] = icmp ult <2 x i64> [[VEC_PHI0]], [[BROADCAST_SPLAT0:%.*]]
; CHECK-NEXT:    br label [[VPLANNEDBB40:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB3:
; CHECK-NEXT:    [[TMP4:%.*]] = trunc <2 x i64> [[VEC_PHI0]] to <2 x i32>
; CHECK-NEXT:    br label [[VPLANNEDBB50:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB4:
; CHECK-NEXT:    [[VEC_PHI60:%.*]] = phi <8 x float> [ <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 0.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 0.000000e+00>, [[VPLANNEDBB40]] ], [ [[TMP15:%.*]], [[VPLANNEDBB160:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI70:%.*]] = phi <2 x i64> [ [[VEC_PHI0]], [[VPLANNEDBB40]] ], [ [[TMP16:%.*]], [[VPLANNEDBB160]] ]
; CHECK-NEXT:    [[VEC_PHI80:%.*]] = phi <2 x i1> [ [[TMP3]], [[VPLANNEDBB40]] ], [ [[TMP19:%.*]], [[VPLANNEDBB160]] ]
; CHECK-NEXT:    [[VEC_PHI90:%.*]] = phi <8 x float> [ undef, [[VPLANNEDBB40]] ], [ [[TMP18:%.*]], [[VPLANNEDBB160]] ]
; CHECK-NEXT:    [[EXTRACTSUBVEC_140:%.*]] = shufflevector <8 x float> [[VEC_PHI60]], <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[EXTRACTSUBVEC_0:%.*]] = shufflevector <8 x float> [[VEC_PHI60]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    br label [[VPLANNEDBB100:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB9:
; CHECK-NEXT:    [[TMP5:%.*]] = and <2 x i1> [[TMP3]], [[VEC_PHI80]]
; CHECK-NEXT:    br label [[VPLANNEDBB110:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB10:
; CHECK-NEXT:    [[TMP6:%.*]] = fadd <8 x float> [[VEC_PHI60]], [[VEC_PHI60]]
; CHECK-NEXT:    [[EXTRACTSUBVEC_150:%.*]] = shufflevector <8 x float> [[TMP6]], <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[EXTRACTSUBVEC_120:%.*]] = shufflevector <8 x float> [[TMP6]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[PREDICATE0:%.*]] = extractelement <2 x i1> [[TMP5]], i64 0
; CHECK-NEXT:    [[TMP7:%.*]] = icmp eq i1 [[PREDICATE0]], true
; CHECK-NEXT:    br i1 [[TMP7]], label [[PRED_CALL_IF0:%.*]], label [[TMP9:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  pred.call.if:
; CHECK-NEXT:    [[TMP8:%.*]] = call <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> [[EXTRACTSUBVEC_0]], <4 x float> [[EXTRACTSUBVEC_120]])
; CHECK-NEXT:    br label [[TMP9]]
; CHECK-EMPTY:
; CHECK-NEXT:  9:
; CHECK-NEXT:    [[TMP10:%.*]] = phi <4 x float> [ undef, [[VPLANNEDBB110]] ], [ [[TMP8]], [[PRED_CALL_IF0]] ]
; CHECK-NEXT:    br label [[PRED_CALL_CONTINUE0:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  pred.call.continue:
; CHECK-NEXT:    [[PREDICATE130:%.*]] = extractelement <2 x i1> [[TMP5]], i64 1
; CHECK-NEXT:    [[TMP11:%.*]] = icmp eq i1 [[PREDICATE130]], true
; CHECK-NEXT:    br i1 [[TMP11]], label [[PRED_CALL_IF300:%.*]], label [[TMP13:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  pred.call.if24:
; CHECK-NEXT:    [[TMP12:%.*]] = call <4 x float> @_Z11fmax_commonDv4_fS_(<4 x float> [[EXTRACTSUBVEC_140]], <4 x float> [[EXTRACTSUBVEC_150]])
; CHECK-NEXT:    br label [[TMP13]]
; CHECK-EMPTY:
; CHECK-NEXT:  13:
; CHECK-NEXT:    [[TMP14:%.*]] = phi <4 x float> [ undef, [[PRED_CALL_CONTINUE0]] ], [ [[TMP12]], [[PRED_CALL_IF300]] ]
; CHECK-NEXT:    br label [[PRED_CALL_CONTINUE310:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  pred.call.continue25:
; CHECK-NEXT:    [[TMP15]] = shufflevector <4 x float> [[TMP10]], <4 x float> [[TMP14]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[TMP16]] = add <2 x i64> [[VEC_PHI70]], [[VEC_PHI0]]
; CHECK-NEXT:    [[TMP17:%.*]] = icmp ult <2 x i64> [[TMP16]], [[BROADCAST_SPLAT0]]
; CHECK-NEXT:    br label [[VPLANNEDBB160]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB15:
; CHECK-NEXT:    [[VEC_PHI8170:%.*]] = shufflevector <2 x i1> [[VEC_PHI80]], <2 x i1> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[TMP18]] = select <8 x i1> [[VEC_PHI8170]], <8 x float> [[TMP15]], <8 x float> [[VEC_PHI90]]
; CHECK:         br i1 [[BROADCAST_SPLAT19_EXTRACT_0_0:%.*]], label [[VPLANNEDBB200:%.*]], label [[VPLANNEDBB50]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB17:
; CHECK-NEXT:    [[VEC_PHI210:%.*]] = phi <8 x float> [ [[TMP18]], [[VPLANNEDBB160]] ]
; CHECK-NEXT:    br label [[VPLANNEDBB220:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB19:
; CHECK-NEXT:    [[TMP23:%.*]] = shufflevector <2 x i1> [[TMP3]], <2 x i1> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[PREDBLEND0:%.*]] = select <8 x i1> [[TMP23]], <8 x float> [[VEC_PHI210]], <8 x float> <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 0.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 0.000000e+00>
; CHECK-NEXT:    br label [[VPLANNEDBB230]]


; This is a minimalist IR which is identical to the control-flow from one of the Embree kernels.
; There is a massaging/canonicalization of control-flow in merge-loop exit transformation which generates
; the IR on which the codegen originally choked. In the future, if merge-loop exit transform is modified and
; the author of the fix deems it appropriate, this function can be dropped.

; Function Attrs: nounwind
define void @test_embree(i64 %0, %"GPU_AABB" addrspace(1)* %1) {
  %alloca. = alloca i64
  br label %simd.begin.region

simd.begin.region:                                ; preds = %4
%entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM:TYPED"(i64* %alloca., i64 0, i32 1) ]
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
