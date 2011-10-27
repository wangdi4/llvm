; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/runtime.bc -verify -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'ev.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


declare i32 @get_global_id(i32)
; CHECK: void

define void @recalculateEigenIntervals(float addrspace(1)* %newEigenIntervals, float addrspace(1)* %eigenIntervals, i32 addrspace(1)* %numEigenIntervals, float addrspace(1)* %diagonal, float addrspace(1)* %offDiagonal, i32 %width, float %tolerance) nounwind {
BB0:
  %0 = call fastcc i32 @maskedf_1_get_global_id(i1 true, i32 0) nounwind ; <i32> [#uses=4]
  %mul = shl i32 %0, 1                            ; <i32> [#uses=4]
  %add1 = or i32 %mul, 1                          ; <i32> [#uses=4]
  %pLoad = call fastcc i32 @masked_load10(i1 true, i32 addrspace(1)* %numEigenIntervals) nounwind ; <i32> [#uses=3]
  %cmp33 = icmp ult i32 %0, %pLoad                ; <i1> [#uses=6]
  br label %BB048.preheader

BB048.preheader:                                  ; preds = %BB0
  %Mneg = xor i1 %cmp33, true                     ; <i1> [#uses=1]
  br label %BB048

BB048:                                            ; preds = %BB049, %BB048.preheader
  %BB048_loop_mask.0 = phi i1 [ %cmp33, %BB048.preheader ], [ %loop_mask67, %BB049 ] ; <i1> [#uses=1]
  %BB048_exit_mask.0 = phi i1 [ false, %BB048.preheader ], [ %1, %BB049 ] ; <i1> [#uses=1]
  %tmp9_prev = phi i32 [ undef, %BB048.preheader ], [ %pLoad109, %BB049 ] ; <i32> [#uses=1]
  %sub_prev = phi i32 [ undef, %BB048.preheader ], [ %sub, %BB049 ] ; <i32> [#uses=1]
  %storemerge35_prev = phi i32 [ undef, %BB048.preheader ], [ %storemerge35, %BB049 ] ; <i32> [#uses=1]
  %BB048_Min = phi i1 [ %Mneg, %BB048.preheader ], [ %local_edge, %BB049 ] ; <i1> [#uses=6]
  %tmp13 = phi i32 [ %pLoad, %BB048.preheader ], [ %pLoad109, %BB049 ] ; <i32> [#uses=1]
  %currentIndex.034 = phi i32 [ %0, %BB048.preheader ], [ %sub, %BB049 ] ; <i32> [#uses=1]
  %storemerge35 = phi i32 [ 1, %BB048.preheader ], [ %phitmp, %BB049 ] ; <i32> [#uses=4]
  %out_sel = select i1 %BB048_Min, i32 %storemerge35, i32 %storemerge35_prev ; <i32> [#uses=1]
  %sub = sub i32 %currentIndex.034, %tmp13        ; <i32> [#uses=4]
  %out_sel102 = select i1 %BB048_Min, i32 %sub, i32 %sub_prev ; <i32> [#uses=1]
  %arrayidx = getelementptr i32 addrspace(1)* %numEigenIntervals, i32 %storemerge35 ; <i32 addrspace(1)*> [#uses=1]
  %pLoad109 = call fastcc i32 @masked_load11(i1 %BB048_Min, i32 addrspace(1)* %arrayidx) nounwind ; <i32> [#uses=4]
  %out_sel104 = select i1 %BB048_Min, i32 %pLoad109, i32 %tmp9_prev ; <i32> [#uses=1]
  %cmp = icmp ult i32 %sub, %pLoad109             ; <i1> [#uses=2]
  %who_left_tr = and i1 %BB048_Min, %cmp          ; <i1> [#uses=2]
  %1 = or i1 %BB048_exit_mask.0, %who_left_tr     ; <i1> [#uses=2]
  %loop_mask67 = or i1 %BB048_loop_mask.0, %who_left_tr ; <i1> [#uses=3]
  %shouldexit = call i1 @allOne3(i1 %loop_mask67) nounwind ; <i1> [#uses=0]
  br label %BB049

BB049:                                            ; preds = %BB048
  %notCond = xor i1 %cmp, true                    ; <i1> [#uses=1]
  %local_edge = and i1 %BB048_Min, %notCond       ; <i1> [#uses=1]
  %phitmp = add i32 %storemerge35, 1              ; <i32> [#uses=1]
  %leave = call i1 @allOne3(i1 %loop_mask67) nounwind ; <i1> [#uses=1]
  br i1 %leave, label %BB048, label %BB050.loopexit

BB050.loopexit:                                   ; preds = %BB049
  br label %BB050

BB050:                                            ; preds = %BB050.loopexit
  %merge96 = select i1 %cmp33, i32 %0, i32 %out_sel102 ; <i32> [#uses=1]
  %merge = select i1 %cmp33, i32 %pLoad, i32 %out_sel104 ; <i32> [#uses=2]
  %BB050_Min87 = or i1 %1, %cmp33                 ; <i1> [#uses=4]
  %out_sel.op = shl i32 %out_sel, 1               ; <i32> [#uses=1]
  %mul18 = select i1 %cmp33, i32 0, i32 %out_sel.op ; <i32> [#uses=2]
  %add212 = or i32 %mul18, 1                      ; <i32> [#uses=1]
  %cmp26 = icmp eq i32 %merge, 1                  ; <i1> [#uses=2]
  %arrayidx30 = getelementptr inbounds float addrspace(1)* %eigenIntervals, i32 %add212 ; <float addrspace(1)*> [#uses=3]
  %pLoad110 = call fastcc float @masked_load12(i1 %BB050_Min87, float addrspace(1)* %arrayidx30) nounwind ; <float> [#uses=3]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %eigenIntervals, i32 %mul18 ; <float addrspace(1)*> [#uses=1]
  %pLoad111 = call fastcc float @masked_load13(i1 %BB050_Min87, float addrspace(1)* %arrayidx34) nounwind ; <float> [#uses=9]
  %Mneg68 = xor i1 %cmp26, true                   ; <i1> [#uses=1]
  %BB050_to_BB063 = and i1 %BB050_Min87, %Mneg68  ; <i1> [#uses=2]
  %BB050_to_BB051 = and i1 %BB050_Min87, %cmp26   ; <i1> [#uses=3]
  br label %BB051

BB051:                                            ; preds = %BB050
  %add36 = fadd float %pLoad110, %pLoad111        ; <float> [#uses=1]
  %div = fdiv float %add36, 2.000000e+00          ; <float> [#uses=4]
  %pLoad112 = call fastcc float @masked_load14(i1 %BB050_to_BB051, float addrspace(1)* %diagonal) nounwind ; <float> [#uses=3]
  %sub.i = fsub float %pLoad112, %div             ; <float> [#uses=2]
  %cmp.i = fcmp olt float %sub.i, 0.000000e+00    ; <i1> [#uses=2]
  %cond.i = zext i1 %cmp.i to i32                 ; <i32> [#uses=1]
  %cmp81.i = icmp ugt i32 %width, 1               ; <i1> [#uses=4]
  %Mneg69 = xor i1 %cmp81.i, true                 ; <i1> [#uses=1]
  %BB051_to_BB052 = and i1 %BB050_to_BB051, %Mneg69 ; <i1> [#uses=1]
  %BB051_to_BB053 = and i1 %BB050_to_BB051, %cmp81.i ; <i1> [#uses=2]
  br label %BB052

BB052:                                            ; preds = %BB051
  %conv.i42 = uitofp i1 %cmp.i to float           ; <float> [#uses=1]
  %sub.i443 = fsub float %pLoad112, %pLoad111     ; <float> [#uses=1]
  %cmp.i544 = fcmp olt float %sub.i443, 0.000000e+00 ; <i1> [#uses=1]
  %cond.i645 = zext i1 %cmp.i544 to i32           ; <i32> [#uses=1]
  br label %BB053

BB053:                                            ; preds = %BB052
  %tmp.i = add i32 %width, -1                     ; <i32> [#uses=1]
  %negIncomingLoopMask88 = xor i1 %BB051_to_BB053, true ; <i1> [#uses=1]
  br label %BB054

BB054:                                            ; preds = %BB053, %BB054
  %BB054_loop_mask.0 = phi i1 [ %negIncomingLoopMask88, %BB053 ], [ %loop_mask73, %BB054 ] ; <i1> [#uses=1]
  %BB054_exit_mask.0 = phi i1 [ false, %BB053 ], [ %2, %BB054 ] ; <i1> [#uses=1]
  %add33.i_prev = phi i32 [ undef, %BB053 ], [ %add33.i, %BB054 ] ; <i32> [#uses=1]
  %BB054_Min = phi i1 [ %BB051_to_BB053, %BB053 ], [ %local_edge76, %BB054 ] ; <i1> [#uses=5]
  %indvar.i = phi i32 [ 0, %BB053 ], [ %tmp38, %BB054 ] ; <i32> [#uses=2]
  %prev_diff.03.i = phi float [ %sub.i, %BB053 ], [ %sub28.i, %BB054 ] ; <float> [#uses=1]
  %count.02.i = phi i32 [ %cond.i, %BB053 ], [ %add33.i, %BB054 ] ; <i32> [#uses=1]
  %tmp38 = add i32 %indvar.i, 1                   ; <i32> [#uses=3]
  %arrayidx12.i = getelementptr float addrspace(1)* %diagonal, i32 %tmp38 ; <float addrspace(1)*> [#uses=1]
  %arrayidx24.i = getelementptr float addrspace(1)* %offDiagonal, i32 %indvar.i ; <float addrspace(1)*> [#uses=1]
  %pLoad113 = call fastcc float @masked_load15(i1 %BB054_Min, float addrspace(1)* %arrayidx12.i) nounwind ; <float> [#uses=1]
  %sub15.i = fsub float %pLoad113, %div           ; <float> [#uses=1]
  %pLoad114 = call fastcc float @masked_load16(i1 %BB054_Min, float addrspace(1)* %arrayidx24.i) nounwind ; <float> [#uses=2]
  %mul.i = fmul float %pLoad114, %pLoad114        ; <float> [#uses=1]
  %div.i = fdiv float %mul.i, %prev_diff.03.i     ; <float> [#uses=1]
  %sub28.i = fsub float %sub15.i, %div.i          ; <float> [#uses=2]
  %cmp30.i = fcmp olt float %sub28.i, 0.000000e+00 ; <i1> [#uses=1]
  %cond31.i = zext i1 %cmp30.i to i32             ; <i32> [#uses=1]
  %add33.i = add i32 %cond31.i, %count.02.i       ; <i32> [#uses=3]
  %exitcond37 = icmp eq i32 %tmp38, %tmp.i        ; <i1> [#uses=2]
  %notCond70 = xor i1 %exitcond37, true           ; <i1> [#uses=1]
  %who_left_tr71 = and i1 %BB054_Min, %exitcond37 ; <i1> [#uses=2]
  %2 = or i1 %BB054_exit_mask.0, %who_left_tr71   ; <i1> [#uses=3]
  %loop_mask73 = or i1 %BB054_loop_mask.0, %who_left_tr71 ; <i1> [#uses=2]
  %shouldexit74 = call i1 @allOne3(i1 %loop_mask73) nounwind ; <i1> [#uses=1]
  %local_edge76 = and i1 %BB054_Min, %notCond70   ; <i1> [#uses=1]
  br i1 %shouldexit74, label %BB055, label %BB054

BB055:                                            ; preds = %BB054
  %out_sel106 = select i1 %BB054_Min, i32 %add33.i, i32 %add33.i_prev ; <i32> [#uses=1]
  %conv.i = uitofp i32 %out_sel106 to float       ; <float> [#uses=2]
  %sub.i4 = fsub float %pLoad112, %pLoad111       ; <float> [#uses=2]
  %cmp.i5 = fcmp olt float %sub.i4, 0.000000e+00  ; <i1> [#uses=1]
  %cond.i6 = zext i1 %cmp.i5 to i32               ; <i32> [#uses=2]
  %Mneg77 = xor i1 %cmp81.i, true                 ; <i1> [#uses=1]
  %BB055_to_BB058 = and i1 %2, %Mneg77            ; <i1> [#uses=3]
  %BB055_to_BB056 = and i1 %2, %cmp81.i           ; <i1> [#uses=2]
  br label %BB063

BB056:                                            ; preds = %BB063
  %tmp.i8 = add i32 %width, -1                    ; <i32> [#uses=1]
  %negIncomingLoopMask89 = xor i1 %BB055_to_BB056, true ; <i1> [#uses=1]
  br label %BB057

BB057:                                            ; preds = %BB056, %BB057
  %BB057_loop_mask.0 = phi i1 [ %negIncomingLoopMask89, %BB056 ], [ %loop_mask81, %BB057 ] ; <i1> [#uses=1]
  %BB057_exit_mask.0 = phi i1 [ false, %BB056 ], [ %3, %BB057 ] ; <i1> [#uses=1]
  %add33.i24_prev = phi i32 [ undef, %BB056 ], [ %add33.i24, %BB057 ] ; <i32> [#uses=1]
  %BB057_Min = phi i1 [ %BB055_to_BB056, %BB056 ], [ %local_edge84, %BB057 ] ; <i1> [#uses=5]
  %indvar.i10 = phi i32 [ 0, %BB056 ], [ %tmp, %BB057 ] ; <i32> [#uses=2]
  %prev_diff.03.i11 = phi float [ %sub.i4, %BB056 ], [ %sub28.i21, %BB057 ] ; <float> [#uses=1]
  %count.02.i12 = phi i32 [ %cond.i6, %BB056 ], [ %add33.i24, %BB057 ] ; <i32> [#uses=1]
  %tmp = add i32 %indvar.i10, 1                   ; <i32> [#uses=3]
  %arrayidx12.i14 = getelementptr float addrspace(1)* %diagonal, i32 %tmp ; <float addrspace(1)*> [#uses=1]
  %arrayidx24.i15 = getelementptr float addrspace(1)* %offDiagonal, i32 %indvar.i10 ; <float addrspace(1)*> [#uses=1]
  %pLoad115 = call fastcc float @masked_load17(i1 %BB057_Min, float addrspace(1)* %arrayidx12.i14) nounwind ; <float> [#uses=1]
  %sub15.i17 = fsub float %pLoad115, %pLoad111    ; <float> [#uses=1]
  %pLoad116 = call fastcc float @masked_load18(i1 %BB057_Min, float addrspace(1)* %arrayidx24.i15) nounwind ; <float> [#uses=2]
  %mul.i19 = fmul float %pLoad116, %pLoad116      ; <float> [#uses=1]
  %div.i20 = fdiv float %mul.i19, %prev_diff.03.i11 ; <float> [#uses=1]
  %sub28.i21 = fsub float %sub15.i17, %div.i20    ; <float> [#uses=2]
  %cmp30.i22 = fcmp olt float %sub28.i21, 0.000000e+00 ; <i1> [#uses=1]
  %cond31.i23 = zext i1 %cmp30.i22 to i32         ; <i32> [#uses=1]
  %add33.i24 = add i32 %cond31.i23, %count.02.i12 ; <i32> [#uses=3]
  %out_sel108 = select i1 %BB057_Min, i32 %add33.i24, i32 %add33.i24_prev ; <i32> [#uses=1]
  %exitcond = icmp eq i32 %tmp, %tmp.i8           ; <i1> [#uses=2]
  %notCond78 = xor i1 %exitcond, true             ; <i1> [#uses=1]
  %who_left_tr79 = and i1 %BB057_Min, %exitcond   ; <i1> [#uses=2]
  %3 = or i1 %BB057_exit_mask.0, %who_left_tr79   ; <i1> [#uses=4]
  %loop_mask81 = or i1 %BB057_loop_mask.0, %who_left_tr79 ; <i1> [#uses=2]
  %shouldexit82 = call i1 @allOne3(i1 %loop_mask81) nounwind ; <i1> [#uses=1]
  %local_edge84 = and i1 %BB057_Min, %notCond78   ; <i1> [#uses=1]
  br i1 %shouldexit82, label %BB058.loopexit, label %BB057

BB058.loopexit:                                   ; preds = %BB057
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %BB058.loopexit
  br label %BB058

BB058:                                            ; preds = %phi-split-bb
  %phi-split-bb_Min90 = or i1 %BB051_to_BB052, %3 ; <i1> [#uses=1]
  %merge97 = select i1 %3, i32 %out_sel108, i32 %cond.i645 ; <i32> [#uses=1]
  %merge98 = select i1 %3, float %conv.i, float %conv.i42 ; <float> [#uses=1]
  %merge100 = select i1 %BB055_to_BB058, i32 %cond.i6, i32 %merge97 ; <i32> [#uses=1]
  %merge99 = select i1 %BB055_to_BB058, float %conv.i, float %merge98 ; <float> [#uses=1]
  %BB058_Min91 = or i1 %phi-split-bb_Min90, %BB055_to_BB058 ; <i1> [#uses=2]
  %sub60 = fsub float %pLoad110, %pLoad111        ; <float> [#uses=1]
  %cmp62 = fcmp olt float %sub60, %tolerance      ; <i1> [#uses=2]
  %Mneg85 = xor i1 %cmp62, true                   ; <i1> [#uses=1]
  %BB058_to_BB060 = and i1 %BB058_Min91, %Mneg85  ; <i1> [#uses=2]
  %BB058_to_BB059 = and i1 %BB058_Min91, %cmp62   ; <i1> [#uses=3]
  br label %BB059

BB059:                                            ; preds = %BB058
  %arrayidx70 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %mul ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store1(i1 %BB058_to_BB059, float %pLoad111, float addrspace(1)* %arrayidx70) nounwind
  %pLoad117 = call fastcc float @masked_load19(i1 %BB058_to_BB059, float addrspace(1)* %arrayidx30) nounwind ; <float> [#uses=1]
  %arrayidx77 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %add1 ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store2(i1 %BB058_to_BB059, float %pLoad117, float addrspace(1)* %arrayidx77) nounwind
  br label %BB060

BB060:                                            ; preds = %BB059
  %conv.i28 = uitofp i32 %merge100 to float       ; <float> [#uses=1]
  %sub51 = fsub float %merge99, %conv.i28         ; <float> [#uses=1]
  %cmp79 = fcmp oeq float %sub51, 0.000000e+00    ; <i1> [#uses=2]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %mul ; <float addrspace(1)*> [#uses=2]
  %Mneg86 = xor i1 %cmp79, true                   ; <i1> [#uses=1]
  %BB060_to_BB062 = and i1 %BB058_to_BB060, %Mneg86 ; <i1> [#uses=2]
  %BB060_to_BB061 = and i1 %BB058_to_BB060, %cmp79 ; <i1> [#uses=3]
  br label %BB061

BB061:                                            ; preds = %BB060
  call fastcc void @masked_store3(i1 %BB060_to_BB061, float %div, float addrspace(1)* %arrayidx84) nounwind
  %pLoad118 = call fastcc float @masked_load20(i1 %BB060_to_BB061, float addrspace(1)* %arrayidx30) nounwind ; <float> [#uses=1]
  %arrayidx91 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %add1 ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store4(i1 %BB060_to_BB061, float %pLoad118, float addrspace(1)* %arrayidx91) nounwind
  br label %BB062

BB062:                                            ; preds = %BB061
  call fastcc void @masked_store5(i1 %BB060_to_BB062, float %pLoad111, float addrspace(1)* %arrayidx84) nounwind
  %arrayidx103 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %add1 ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store6(i1 %BB060_to_BB062, float %div, float addrspace(1)* %arrayidx103) nounwind
  br label %phi-split-bb65

BB063:                                            ; preds = %BB055
  %sub115 = fsub float %pLoad110, %pLoad111       ; <float> [#uses=1]
  %conv = uitofp i32 %merge to float              ; <float> [#uses=1]
  %div121 = fdiv float %sub115, %conv             ; <float> [#uses=2]
  %conv128 = uitofp i32 %merge96 to float         ; <float> [#uses=1]
  %mul129 = fmul float %div121, %conv128          ; <float> [#uses=1]
  %add130 = fadd float %pLoad111, %mul129         ; <float> [#uses=2]
  %arrayidx133 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %mul ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store7(i1 %BB050_to_BB063, float %add130, float addrspace(1)* %arrayidx133) nounwind
  %add139 = fadd float %add130, %div121           ; <float> [#uses=1]
  %arrayidx142 = getelementptr inbounds float addrspace(1)* %newEigenIntervals, i32 %add1 ; <float addrspace(1)*> [#uses=1]
  call fastcc void @masked_store8(i1 %BB050_to_BB063, float %add139, float addrspace(1)* %arrayidx142) nounwind
  br label %BB056

phi-split-bb65:                                   ; preds = %BB062
  br label %phi-split-bb66

phi-split-bb66:                                   ; preds = %phi-split-bb65
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %phi-split-bb66
  ret void
}

declare fastcc i1 @allOne(i1)

declare fastcc i1 @allZero(i1)

declare fastcc float @masked_load0(i1, float addrspace(1)*)

declare fastcc float @masked_load1(i1, float addrspace(1)*)

declare fastcc float @masked_load2(i1, float addrspace(1)*)

declare fastcc i1 @allOne1(i1)

declare fastcc i1 @allZero2(i1)

declare fastcc i32 @maskedf_0_get_global_id(i1, i32)

declare fastcc float @masked_load3(i1, float addrspace(1)*)

declare fastcc float @masked_load4(i1, float addrspace(1)*)

declare fastcc float @masked_load5(i1, float addrspace(1)*)

declare fastcc float @masked_load6(i1, float addrspace(1)*)

declare fastcc float @masked_load7(i1, float addrspace(1)*)

declare fastcc float @masked_load8(i1, float addrspace(1)*)

declare fastcc float @masked_load9(i1, float addrspace(1)*)

declare fastcc void @masked_store0(i1, i32, i32 addrspace(1)*)

declare fastcc i1 @allOne3(i1)

declare fastcc i1 @allZero4(i1)

declare fastcc i32 @maskedf_1_get_global_id(i1, i32)

declare fastcc i32 @masked_load10(i1, i32 addrspace(1)*)

declare fastcc i32 @masked_load11(i1, i32 addrspace(1)*)

declare fastcc float @masked_load12(i1, float addrspace(1)*)

declare fastcc float @masked_load13(i1, float addrspace(1)*)

declare fastcc float @masked_load14(i1, float addrspace(1)*)

declare fastcc float @masked_load15(i1, float addrspace(1)*)

declare fastcc float @masked_load16(i1, float addrspace(1)*)

declare fastcc float @masked_load17(i1, float addrspace(1)*)

declare fastcc float @masked_load18(i1, float addrspace(1)*)

declare fastcc void @masked_store1(i1, float, float addrspace(1)*)

declare fastcc float @masked_load19(i1, float addrspace(1)*)

declare fastcc void @masked_store2(i1, float, float addrspace(1)*)

declare fastcc void @masked_store3(i1, float, float addrspace(1)*)

declare fastcc float @masked_load20(i1, float addrspace(1)*)

declare fastcc void @masked_store4(i1, float, float addrspace(1)*)

declare fastcc void @masked_store5(i1, float, float addrspace(1)*)

declare fastcc void @masked_store6(i1, float, float addrspace(1)*)

declare fastcc void @masked_store7(i1, float, float addrspace(1)*)

declare fastcc void @masked_store8(i1, float, float addrspace(1)*)
