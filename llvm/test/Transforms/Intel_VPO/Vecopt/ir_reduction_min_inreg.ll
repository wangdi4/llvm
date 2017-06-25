;RUN: opt -VPlanDriver -S %s | FileCheck %s

; CHECK:   %min.vec = alloca <4 x i32>
; CHECK: vector.ph: 
; CHECK:   %minInitVal = load i32, i32* %min
; CHECK:   %minInitVec.splatinsert = insertelement <4 x i32> undef, i32 %minInitVal, i32 0
; CHECK:   %minInitVec.splat = shufflevector <4 x i32> %minInitVec.splatinsert, <4 x i32> undef, <4 x i32> zeroinitializer
; CHECK:   store <4 x i32> %minInitVec.splat, <4 x i32>* %min.vec

; CHECK: vector.body: 
; CHECK:   %vec.phi = phi <4 x i32> [ %minInitVec.splat, %vector.ph ], [ %predphi, %vector.body ]
; CHECK:   call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> 
; CHECK:   %predphi = select

; CHECK: middle.block:
; CHECK:   %Red.vec = load <4 x i32>, <4 x i32>* %min.vec
; CHECK:   %[[RES:.*]] = extractelement <4 x i32> 
; CHECK:   store i32 %[[RES]], i32* %min

; CHECK:  scalar.ph: 
; CHECK:    %bc.merge.rdx = phi i32 {{.*}}, [ %[[RES]], %middle.block ]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %ip) {
  %min = alloca i32, align 4
  %1 = bitcast i32* %min to i8*
  store i32 2147483647, i32* %min, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %0
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.MIN", i32* nonnull %min)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
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
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.4

DIR.QUAL.LIST.END.4:                              ; preds = %DIR.OMP.END.SIMD.3
  ret i32 %.lcssa
}

declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opndlist(metadata , ...)
