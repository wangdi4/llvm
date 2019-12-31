;RUN: opt -VPlanDriver -vplan-force-vf=4 -S %s | FileCheck %s

; TODO: This test needs support to generate scalar stores for uniform stores in VPlan. It will be fixed
; in https://git-amr-2.devtools.intel.com/gerrit/#/c/215460/
; XFAIL: *

; CHECK-LABEL: @foo(
; CHECK:       [[PRIVATE_MEM:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:  [[MIN:%.*]] = alloca i32, align 4
; CHECK:       [[DOTPRE:%.*]] = load i32, i32* [[MIN]], align 4
; CHECK:     vector.ph:
; CHECK-NEXT:  [[BROADCAST_SPLATINSERT:%.*]] = insertelement <4 x i32> undef, i32 [[DOTPRE]], i32 0
; CHECK-NEXT:  [[BROADCAST_SPLAT:%.*]] = shufflevector <4 x i32> [[BROADCAST_SPLATINSERT]], <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK-NEXT:    store <4 x i32> [[BROADCAST_SPLAT]], <4 x i32>* [[PRIVATE_MEM]], align 16
; CHECK:     vector.body:
; CHECK:       [[VEC_PHI:%.*]] = phi <4 x i32> [ [[BROADCAST_SPLAT]], [[VECTOR_PH:%.*]] ], [ [[PREDPHI:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK:       [[WIDE_LOAD:%.*]] = load <4 x i32>, <4 x i32>* [[TMP2:%.*]], align 4
; CHECK-NEXT:  [[TMP3:%.*]] = icmp sgt <4 x i32> [[VEC_PHI]], [[WIDE_LOAD]]
; CHECK-NEXT:  call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> [[WIDE_LOAD]], <4 x i32>* [[PRIVATE_MEM]], i32 16, <4 x i1> [[TMP3]])
; CHECK-NEXT:  [[PREDPHI]] = select <4 x i1> [[TMP3]], <4 x i32> [[WIDE_LOAD]], <4 x i32> [[VEC_PHI]]
; CHECK:     VPlannedBB:
; CHECK-NEXT:  [[TMP9:%.*]] = call i32 @llvm.experimental.vector.reduce.smin.v4i32(<4 x i32> [[PREDPHI]])
; CHECK-NEXT:  store i32 [[TMP9]], i32* [[MIN]]
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %ip) {
  %min = alloca i32, align 4
  %1 = bitcast i32* %min to i8*
  store i32 2147483647, i32* %min, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %0
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MIN"(i32* %min) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  %.pre = load i32, i32* %min, align 4
  br label %for.body

for.body:                                      ; preds = %6, %DIR.QUAL.LIST.END.2
  %Tmp = phi i32 [ %.pre, %DIR.QUAL.LIST.END.2 ], [ %Res, %for.inc ]
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  %Val = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %Tmp, %Val
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                      ; preds = %2
  store i32 %Val, i32* %min, align 4
  br label %for.inc

for.inc:                                      ; preds = %5, %2
  %Res = phi i32 [ %Val, %if.then ], [ %Tmp, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                      ; preds = %6
  %.lcssa = phi i32 [ %Res, %for.inc ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %8
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %DIR.OMP.END.SIMD.3
  ret i32 %.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
