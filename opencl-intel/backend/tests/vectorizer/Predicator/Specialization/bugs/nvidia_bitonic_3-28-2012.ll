; RUN: llvm-as %s -o %t.bc
; RUN: opt -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon passes

; CHECK: @bitonicSortLocal1
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:
; CHECK: ret


@opencl_bitonicSortLocal1_local_l_key = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4
@opencl_bitonicSortLocal1_local_l_val = internal addrspace(3) global [1024 x i32] zeroinitializer, align 4

declare i32 @get_group_id(i32) readnone

declare i32 @_Z12get_local_idj(i32) readnone

declare i32 @_Z13get_global_idj(i32) readnone

declare void @_Z7barrierm(i32)

define void @bitonicSortLocal1(i32 addrspace(1)* nocapture %d_DstKey, i32 addrspace(1)* nocapture %d_DstVal, i32 addrspace(1)* nocapture %d_SrcKey, i32 addrspace(1)* nocapture %d_SrcVal) nounwind {
bb.nph11:
  %call = tail call i32 @get_group_id(i32 0) nounwind readnone
  %mul = shl i32 %call, 10
  %call1 = tail call i32 @_Z12get_local_idj(i32 0) nounwind readnone
  %add = add i32 %mul, %call1
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %d_SrcKey, i32 %add
  %add.ptr7 = getelementptr inbounds i32 addrspace(1)* %d_SrcVal, i32 %add
  %add.ptr13 = getelementptr inbounds i32 addrspace(1)* %d_DstKey, i32 %add
  %add.ptr19 = getelementptr inbounds i32 addrspace(1)* %d_DstVal, i32 %add
  %tmp21 = load i32 addrspace(1)* %add.ptr, align 4
  %arrayidx24 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %call1
  store i32 %tmp21, i32 addrspace(3)* %arrayidx24, align 4
  %tmp27 = load i32 addrspace(1)* %add.ptr7, align 4
  %arrayidx30 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %call1
  store i32 %tmp27, i32 addrspace(3)* %arrayidx30, align 4
  %add.ptr.sum = add i32 %add, 512
  %arrayidx32 = getelementptr inbounds i32 addrspace(1)* %d_SrcKey, i32 %add.ptr.sum
  %tmp33 = load i32 addrspace(1)* %arrayidx32, align 4
  %add35 = add i32 %call1, 512
  %arrayidx36 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %add35
  store i32 %tmp33, i32 addrspace(3)* %arrayidx36, align 4
  %arrayidx38 = getelementptr inbounds i32 addrspace(1)* %d_SrcVal, i32 %add.ptr.sum
  %tmp39 = load i32 addrspace(1)* %arrayidx38, align 4
  %arrayidx42 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %add35
  store i32 %tmp39, i32 addrspace(3)* %arrayidx42, align 4
  %call44 = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %and = and i32 %call44, 511
  br label %for.body

for.body:                                         ; preds = %for.inc83, %bb.nph11
  %indvar12 = phi i32 [ 0, %bb.nph11 ], [ %indvar.next13, %for.inc83 ]
  %size.010 = phi i32 [ 2, %bb.nph11 ], [ %shl, %for.inc83 ]
  %div = lshr i32 %size.010, 1
  %and50 = and i32 %and, %div
  %cmp51 = icmp ne i32 %and50, 0
  %cmp576 = icmp eq i32 %div, 0
  br i1 %cmp576, label %for.inc83, label %bb.nph8

bb.nph8:                                          ; preds = %for.body
  %mul62 = shl i32 %call1, 1
  br label %for.body59

for.body59:                                       ; preds = %ComparatorLocal.exit, %bb.nph8
  %stride.07 = phi i32 [ %div, %bb.nph8 ], [ %stride.0, %ComparatorLocal.exit ]
  tail call void @_Z7barrierm(i32 1) nounwind
  %sub = add i32 %stride.07, -1
  %and65 = and i32 %call1, %sub
  %sub66 = sub i32 %mul62, %and65
  %arrayidx69 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %sub66
  %arrayidx72 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %sub66
  %add75 = add i32 %sub66, %stride.07
  %arrayidx76 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %add75
  %arrayidx80 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %add75
  %tmp1.i = load i32 addrspace(3)* %arrayidx69, align 4
  %tmp3.i = load i32 addrspace(3)* %arrayidx76, align 4
  %cmp.i = icmp ugt i32 %tmp1.i, %tmp3.i
  %cmp5.itmp = xor i1 %cmp.i, %cmp51
  br i1 %cmp5.itmp, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %for.body59
  store i32 %tmp3.i, i32 addrspace(3)* %arrayidx69, align 4
  store i32 %tmp1.i, i32 addrspace(3)* %arrayidx76, align 4
  %tmp16.i = load i32 addrspace(3)* %arrayidx72, align 4
  %tmp18.i = load i32 addrspace(3)* %arrayidx80, align 4
  store i32 %tmp18.i, i32 addrspace(3)* %arrayidx72, align 4
  store i32 %tmp16.i, i32 addrspace(3)* %arrayidx80, align 4
  br label %ComparatorLocal.exit

if.end.i:                                         ; preds = %for.body59
  br label %ComparatorLocal.exit

ComparatorLocal.exit:                             ; preds = %if.then.i, %if.end.i
  %stride.0 = lshr i32 %stride.07, 1
  %cmp57 = icmp eq i32 %stride.0, 0
  br i1 %cmp57, label %for.inc83.loopexit, label %for.body59

for.inc83.loopexit:                               ; preds = %ComparatorLocal.exit
  br label %for.inc83

for.inc83:                                        ; preds = %for.inc83.loopexit, %for.body
  %shl = shl i32 %size.010, 1
  %indvar.next13 = add i32 %indvar12, 1
  %exitcond14 = icmp eq i32 %indvar.next13, 9
  br i1 %exitcond14, label %bb.nph, label %for.body

bb.nph:                                           ; preds = %for.inc83
  %and89 = and i32 %call, 1
  tail call void @_Z7barrierm(i32 1) nounwind
  br label %for.body96

for.body96:                                       ; preds = %ComparatorLocal.exit10, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %indvar.next, %ComparatorLocal.exit10 ]
  %stride91.03 = phi i32 [ 512, %bb.nph ], [ %shr123, %ComparatorLocal.exit10 ]
  %mul100 = shl i32 %call1, 1
  %sub103 = add i32 %stride91.03, -1
  %and104 = and i32 %call1, %sub103
  %sub105 = sub i32 %mul100, %and104
  %arrayidx108 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %sub105
  %arrayidx111 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %sub105
  %add114 = add i32 %sub105, %stride91.03
  %arrayidx115 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_key, i32 0, i32 %add114
  %arrayidx119 = getelementptr inbounds [1024 x i32] addrspace(3)* @opencl_bitonicSortLocal1_local_l_val, i32 0, i32 %add114
  %tmp1.i1 = load i32 addrspace(3)* %arrayidx108, align 4
  %tmp3.i2 = load i32 addrspace(3)* %arrayidx115, align 4
  %cmp.i3 = icmp ugt i32 %tmp1.i1, %tmp3.i2
  %conv.i4 = zext i1 %cmp.i3 to i32
  %cmp5.i5 = icmp eq i32 %conv.i4, %and89
  br i1 %cmp5.i5, label %if.then.i8, label %if.end.i9

if.then.i8:                                       ; preds = %for.body96
  store i32 %tmp3.i2, i32 addrspace(3)* %arrayidx108, align 4
  store i32 %tmp1.i1, i32 addrspace(3)* %arrayidx115, align 4
  %tmp16.i6 = load i32 addrspace(3)* %arrayidx111, align 4
  %tmp18.i7 = load i32 addrspace(3)* %arrayidx119, align 4
  store i32 %tmp18.i7, i32 addrspace(3)* %arrayidx111, align 4
  store i32 %tmp16.i6, i32 addrspace(3)* %arrayidx119, align 4
  br label %ComparatorLocal.exit10

if.end.i9:                                        ; preds = %for.body96
  br label %ComparatorLocal.exit10

ComparatorLocal.exit10:                           ; preds = %if.then.i8, %if.end.i9
  %shr123 = lshr i32 %stride91.03, 1
  tail call void @_Z7barrierm(i32 1) nounwind
  %indvar.next = add i32 %indvar, 1
  %exitcond = icmp eq i32 %indvar.next, 10
  br i1 %exitcond, label %for.end124, label %for.body96

for.end124:                                       ; preds = %ComparatorLocal.exit10
  %tmp128 = load i32 addrspace(3)* %arrayidx24, align 4
  store i32 %tmp128, i32 addrspace(1)* %add.ptr13, align 4
  %tmp134 = load i32 addrspace(3)* %arrayidx30, align 4
  store i32 %tmp134, i32 addrspace(1)* %add.ptr19, align 4
  %tmp140 = load i32 addrspace(3)* %arrayidx36, align 4
  %arrayidx142 = getelementptr inbounds i32 addrspace(1)* %d_DstKey, i32 %add.ptr.sum
  store i32 %tmp140, i32 addrspace(1)* %arrayidx142, align 4
  %tmp146 = load i32 addrspace(3)* %arrayidx42, align 4
  %arrayidx148 = getelementptr inbounds i32 addrspace(1)* %d_DstVal, i32 %add.ptr.sum
  store i32 %tmp146, i32 addrspace(1)* %arrayidx148, align 4
  ret void
}

define <8 x i32> @local.avx256.pcmpeq.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32>, <4 x i32>) nounwind readnone

define <8 x i32> @local.avx256.pcmpgt.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32>, <4 x i32>) nounwind readnone

declare i1 @allOne(i1)

declare i1 @allZero(i1)

!opencl.kernels = !{!0}
!opencl_bitonicSortLocal1_locals_anchor = !{!6, !7}
!opencl.compiler.options = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @bitonicSortLocal1, !1, !1, !"", !"uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *", !"opencl_bitonicSortLocal1_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 1, i32 1, i32 1, i32 1}
!3 = !{i32 3, i32 3, i32 3, i32 3}
!4 = !{!"uint*", !"uint*", !"uint*", !"uint*"}
!5 = !{!"d_DstKey", !"d_DstVal", !"d_SrcKey", !"d_SrcVal"}
!6 = !{!"opencl_bitonicSortLocal1_local_l_key"}
!7 = !{!"opencl_bitonicSortLocal1_local_l_val"}
