; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlcs.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @Lcs
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %149, %150
; CHECK: phi-split-bb1:                                    ; preds = %116, %117
; CHECK: ret

@Lcs.iterator_cmp = internal constant [32 x i32] [i32 16, i32 7, i32 10, i32 13, i32 17, i32 20, i32 11, i32 14, i32 18, i32 21, i32 24, i32 15, i32 19, i32 22, i32 25, i32 28, i32 32, i32 23, i32 26, i32 29, i32 33, i32 36, i32 27, i32 30, i32 34, i32 37, i32 40, i32 31, i32 35, i32 38, i32 41, i32 44], align 4

define void @Lcs(float addrspace(1)* nocapture %str1, float addrspace(1)* %str2, i32 addrspace(1)* nocapture %C) nounwind {
bb.nph75:
  %cmp = alloca [8 x <4 x i32>], align 16
  %cmp_id = alloca [48 x i32], align 4
  %0 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 0
  %1 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 1
  %2 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 2
  %3 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 3
  %4 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 4
  %5 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 5
  %6 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 6
  %7 = getelementptr inbounds [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 7
  %scevgep317 = bitcast [48 x i32]* %cmp_id to i8*
  br label %bb.nph72

bb.nph72:                                         ; preds = %._crit_edge73, %bb.nph75
  %indvar25 = phi i64 [ %indvar.next26, %._crit_edge73 ], [ 0, %bb.nph75 ]
  %8 = phi <4 x i32> [ %62, %._crit_edge73 ], [ undef, %bb.nph75 ]
  %9 = phi <4 x i32> [ %61, %._crit_edge73 ], [ undef, %bb.nph75 ]
  %10 = phi <4 x i32> [ %60, %._crit_edge73 ], [ undef, %bb.nph75 ]
  %11 = phi <4 x i32> [ %59, %._crit_edge73 ], [ undef, %bb.nph75 ]
  %tmp124 = mul i64 %indvar25, 2080
  %tmp125 = add i64 %tmp124, 2081
  %tmp128 = add i64 %tmp124, 1562
  %tmp131 = add i64 %tmp124, 1043
  %tmp134 = add i64 %tmp124, 524
  %tmp137 = add i64 %tmp124, 2080
  %tmp140168 = or i64 %tmp124, 4
  %tmp143 = add i64 %tmp124, 523
  %tmp146 = add i64 %tmp124, 1042
  %tmp149 = add i64 %tmp124, 1561
  %tmp152169 = or i64 %tmp124, 3
  %tmp155 = add i64 %tmp124, 522
  %tmp158 = add i64 %tmp124, 1041
  %tmp161 = add i64 %tmp124, 1560
  %tmp164 = shl i64 %indvar25, 2
  %tmp165170 = or i64 %tmp164, 1
  %scevgep299 = getelementptr float, float addrspace(1)* %str1, i64 %tmp165170
  %tmp166171 = or i64 %tmp164, 2
  %scevgep297 = getelementptr float, float addrspace(1)* %str1, i64 %tmp166171
  %tmp167172 = or i64 %tmp164, 3
  %scevgep295 = getelementptr float, float addrspace(1)* %str1, i64 %tmp167172
  %scevgep300 = getelementptr float, float addrspace(1)* %str1, i64 %tmp164
  %12 = load float, float addrspace(1)* %scevgep300, align 4
  %13 = insertelement <4 x float> undef, float %12, i32 0
  %14 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> zeroinitializer
  %15 = load float, float addrspace(1)* %scevgep299, align 4
  %16 = insertelement <4 x float> undef, float %15, i32 0
  %17 = shufflevector <4 x float> %16, <4 x float> undef, <4 x i32> zeroinitializer
  %18 = load float, float addrspace(1)* %scevgep297, align 4
  %19 = insertelement <4 x float> undef, float %18, i32 0
  %20 = shufflevector <4 x float> %19, <4 x float> undef, <4 x i32> zeroinitializer
  %21 = load float, float addrspace(1)* %scevgep295, align 4
  %22 = insertelement <4 x float> undef, float %21, i32 0
  %23 = shufflevector <4 x float> %22, <4 x float> undef, <4 x i32> zeroinitializer
  br label %24

; <label>:24                                      ; preds = %._crit_edge, %bb.nph72
  %indvar22 = phi i64 [ %tmp120, %._crit_edge ], [ 0, %bb.nph72 ]
  %25 = phi <4 x i32> [ %62, %._crit_edge ], [ %8, %bb.nph72 ]
  %26 = phi <4 x i32> [ %61, %._crit_edge ], [ %9, %bb.nph72 ]
  %27 = phi <4 x i32> [ %60, %._crit_edge ], [ %10, %bb.nph72 ]
  %28 = phi <4 x i32> [ %59, %._crit_edge ], [ %11, %bb.nph72 ]
  %tmp123 = shl i64 %indvar22, 2
  %tmp126 = add i64 %tmp125, %tmp123
  %tmp129 = add i64 %tmp128, %tmp123
  %tmp132 = add i64 %tmp131, %tmp123
  %tmp135 = add i64 %tmp134, %tmp123
  %tmp138 = add i64 %tmp137, %tmp123
  %tmp141 = add i64 %tmp140168, %tmp123
  %tmp144 = add i64 %tmp143, %tmp123
  %tmp147 = add i64 %tmp146, %tmp123
  %tmp150 = add i64 %tmp149, %tmp123
  %tmp153 = add i64 %tmp152169, %tmp123
  %tmp156 = add i64 %tmp155, %tmp123
  %tmp159 = add i64 %tmp158, %tmp123
  %tmp162 = add i64 %tmp161, %tmp123
  %tmp120 = add i64 %indvar22, 1
  %j.071 = trunc i64 %tmp120 to i32
  %scevgep236 = getelementptr float, float addrspace(1)* %str2, i64 %tmp123
  %29 = call <4 x float> @_Z6vload4jPU3AS1Kf(i32 0, float addrspace(1)* %scevgep236) nounwind
  %30 = icmp eq i32 %j.071, 1
  br i1 %30, label %bb.nph65, label %bb.nph68

bb.nph65:                                         ; preds = %24
  %31 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %14, <4 x float> %29) nounwind
  store <4 x i32> %31, <4 x i32>* %0, align 16
  %32 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %17, <4 x float> %29) nounwind
  store <4 x i32> %32, <4 x i32>* %1, align 16
  %33 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %20, <4 x float> %29) nounwind
  store <4 x i32> %33, <4 x i32>* %2, align 16
  %34 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %23, <4 x float> %29) nounwind
  store <4 x i32> %34, <4 x i32>* %3, align 16
  store <4 x i32> %31, <4 x i32>* %4, align 16
  store <4 x i32> %32, <4 x i32>* %5, align 16
  store <4 x i32> %33, <4 x i32>* %6, align 16
  store <4 x i32> %34, <4 x i32>* %7, align 16
  call void @llvm.memset.p0i8.i64(i8* %scevgep317, i8 0, i64 64, i32 4, i1 false)
  br label %35

; <label>:35                                      ; preds = %._crit_edge315, %bb.nph65
  %indvar13 = phi i64 [ %tmp16, %._crit_edge315 ], [ 0, %bb.nph65 ]
  %36 = phi <4 x i32> [ %.pre, %._crit_edge315 ], [ %31, %bb.nph65 ]
  %tmp17 = shl i64 %indvar13, 2
  %tmp18 = add i64 %tmp17, 19
  %scevgep92 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp18
  %tmp19 = add i64 %tmp17, 18
  %scevgep90 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp19
  %tmp20 = add i64 %tmp17, 17
  %scevgep88 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp20
  %tmp21 = add i64 %tmp17, 16
  %scevgep86 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp21
  %37 = extractelement <4 x i32> %36, i32 0
  %38 = sub i32 0, %37
  store i32 %38, i32* %scevgep86, align 4
  %39 = extractelement <4 x i32> %36, i32 1
  %40 = sub i32 0, %39
  store i32 %40, i32* %scevgep88, align 4
  %41 = extractelement <4 x i32> %36, i32 2
  %42 = sub i32 0, %41
  store i32 %42, i32* %scevgep90, align 4
  %43 = extractelement <4 x i32> %36, i32 3
  %44 = sub i32 0, %43
  store i32 %44, i32* %scevgep92, align 4
  %exitcond15 = icmp eq i64 %indvar13, 7
  br i1 %exitcond15, label %bb.nph70.loopexit, label %._crit_edge315

._crit_edge315:                                   ; preds = %35
  %tmp16 = add i64 %indvar13, 1
  %scevgep93.phi.trans.insert = getelementptr [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 %tmp16
  %.pre = load <4 x i32>, <4 x i32>* %scevgep93.phi.trans.insert, align 16
  br label %35

bb.nph68:                                         ; preds = %24
  store <4 x i32> %28, <4 x i32>* %0, align 16
  store <4 x i32> %27, <4 x i32>* %1, align 16
  store <4 x i32> %26, <4 x i32>* %2, align 16
  store <4 x i32> %25, <4 x i32>* %3, align 16
  %45 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %14, <4 x float> %29) nounwind
  store <4 x i32> %45, <4 x i32>* %4, align 16
  %46 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %17, <4 x float> %29) nounwind
  store <4 x i32> %46, <4 x i32>* %5, align 16
  %47 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %20, <4 x float> %29) nounwind
  store <4 x i32> %47, <4 x i32>* %6, align 16
  %48 = call <4 x i32> @_Z7isequalDv4_fS_(<4 x float> %23, <4 x float> %29) nounwind
  store <4 x i32> %48, <4 x i32>* %7, align 16
  br label %49

; <label>:49                                      ; preds = %._crit_edge314, %bb.nph68
  %indvar5 = phi i64 [ %tmp, %._crit_edge314 ], [ 0, %bb.nph68 ]
  %50 = phi <4 x i32> [ %.pre316, %._crit_edge314 ], [ %28, %bb.nph68 ]
  %tmp8 = shl i64 %indvar5, 2
  %tmp9 = add i64 %tmp8, 19
  %scevgep105 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp9
  %tmp10 = add i64 %tmp8, 18
  %scevgep103 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp10
  %tmp11 = add i64 %tmp8, 17
  %scevgep101 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp11
  %tmp12 = add i64 %tmp8, 16
  %scevgep99 = getelementptr [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %tmp12
  %51 = extractelement <4 x i32> %50, i32 0
  %52 = sub i32 0, %51
  store i32 %52, i32* %scevgep99, align 4
  %53 = extractelement <4 x i32> %50, i32 1
  %54 = sub i32 0, %53
  store i32 %54, i32* %scevgep101, align 4
  %55 = extractelement <4 x i32> %50, i32 2
  %56 = sub i32 0, %55
  store i32 %56, i32* %scevgep103, align 4
  %57 = extractelement <4 x i32> %50, i32 3
  %58 = sub i32 0, %57
  store i32 %58, i32* %scevgep105, align 4
  %exitcond7 = icmp eq i64 %indvar5, 7
  br i1 %exitcond7, label %bb.nph70.loopexit176, label %._crit_edge314

._crit_edge314:                                   ; preds = %49
  %tmp = add i64 %indvar5, 1
  %scevgep106.phi.trans.insert = getelementptr [8 x <4 x i32>], [8 x <4 x i32>]* %cmp, i64 0, i64 %tmp
  %.pre316 = load <4 x i32>, <4 x i32>* %scevgep106.phi.trans.insert, align 16
  br label %49

bb.nph70.loopexit:                                ; preds = %35
  br label %bb.nph70

bb.nph70.loopexit176:                             ; preds = %49
  br label %bb.nph70

bb.nph70:                                         ; preds = %bb.nph70.loopexit176, %bb.nph70.loopexit
  %59 = phi <4 x i32> [ %31, %bb.nph70.loopexit ], [ %45, %bb.nph70.loopexit176 ]
  %60 = phi <4 x i32> [ %32, %bb.nph70.loopexit ], [ %46, %bb.nph70.loopexit176 ]
  %61 = phi <4 x i32> [ %33, %bb.nph70.loopexit ], [ %47, %bb.nph70.loopexit176 ]
  %62 = phi <4 x i32> [ %34, %bb.nph70.loopexit ], [ %48, %bb.nph70.loopexit176 ]
  br label %63

; <label>:63                                      ; preds = %152, %bb.nph70
  %indvar = phi i64 [ %indvar.next, %152 ], [ 0, %bb.nph70 ]
  %tmp127 = add i64 %tmp126, %indvar
  %scevgep181 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp127
  %tmp130 = add i64 %tmp129, %indvar
  %scevgep177 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp130
  %tmp133 = add i64 %tmp132, %indvar
  %scevgep173 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp133
  %tmp136 = add i64 %tmp135, %indvar
  %scevgep169 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp136
  %tmp139 = add i64 %tmp138, %indvar
  %scevgep165 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp139
  %tmp142 = add i64 %tmp141, %indvar
  %scevgep161 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp142
  %tmp145 = add i64 %tmp144, %indvar
  %scevgep157 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp145
  %tmp148 = add i64 %tmp147, %indvar
  %scevgep153 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp148
  %tmp151 = add i64 %tmp150, %indvar
  %scevgep149 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp151
  %tmp154 = add i64 %tmp153, %indvar
  %scevgep145 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp154
  %tmp157 = add i64 %tmp156, %indvar
  %scevgep141 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp157
  %tmp160 = add i64 %tmp159, %indvar
  %scevgep137 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp160
  %tmp163 = add i64 %tmp162, %indvar
  %scevgep133 = getelementptr i32, i32 addrspace(1)* %C, i64 %tmp163
  %tmp67 = shl i64 %indvar, 2
  %tmp69173 = or i64 %tmp67, 3
  %scevgep123 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp69173
  %tmp70174 = or i64 %tmp67, 2
  %scevgep121 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp70174
  %tmp71175 = or i64 %tmp67, 1
  %scevgep119 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp71175
  %tmp72 = add i64 %tmp67, 17
  %scevgep114 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp72
  %tmp73 = add i64 %tmp67, 18
  %scevgep112 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp73
  %tmp74 = add i64 %tmp67, 19
  %scevgep110 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp74
  %64 = load i32, i32 addrspace(1)* %scevgep133, align 4
  %65 = add nsw i32 %64, 1
  %66 = load i32, i32 addrspace(1)* %scevgep137, align 4
  %67 = add nsw i32 %66, 1
  %68 = load i32, i32 addrspace(1)* %scevgep141, align 4
  %69 = add nsw i32 %68, 1
  %70 = load i32, i32 addrspace(1)* %scevgep145, align 4
  %71 = add nsw i32 %70, 1
  %72 = load i32, i32 addrspace(1)* %scevgep149, align 4
  %73 = insertelement <4 x i32> undef, i32 %72, i32 0
  %74 = load i32, i32 addrspace(1)* %scevgep153, align 4
  %75 = insertelement <4 x i32> %73, i32 %74, i32 1
  %76 = load i32, i32 addrspace(1)* %scevgep157, align 4
  %77 = insertelement <4 x i32> %75, i32 %76, i32 2
  %78 = load i32, i32 addrspace(1)* %scevgep161, align 4
  %79 = insertelement <4 x i32> %77, i32 %78, i32 3
  %80 = load i32, i32 addrspace(1)* %scevgep165, align 4
  %81 = insertelement <4 x i32> undef, i32 %80, i32 0
  %82 = insertelement <4 x i32> %81, i32 %72, i32 1
  %83 = insertelement <4 x i32> %82, i32 %74, i32 2
  %84 = insertelement <4 x i32> %83, i32 %76, i32 3
  %85 = call <4 x i32> @_Z3maxDv4_iS_(<4 x i32> %79, <4 x i32> %84) nounwind
  br i1 %30, label %86, label %119

; <label>:86                                      ; preds = %63
  %scevgep117 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp67
  %87 = load i32, i32* %scevgep117, align 4
  %88 = sext i32 %87 to i64
  %89 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %88
  %90 = load i32, i32* %89, align 4
  %91 = icmp eq i32 %90, 0
  br i1 %91, label %92, label %94

; <label>:92                                      ; preds = %86
  %93 = extractelement <4 x i32> %85, i32 3
  br label %94

; <label>:94                                      ; preds = %92, %86
  %storemerge311 = phi i32 [ %93, %92 ], [ %71, %86 ]
  store i32 %storemerge311, i32 addrspace(1)* %scevgep169, align 4
  %95 = load i32, i32* %scevgep119, align 4
  %96 = sext i32 %95 to i64
  %97 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %96
  %98 = load i32, i32* %97, align 4
  %99 = icmp eq i32 %98, 0
  br i1 %99, label %100, label %102

; <label>:100                                     ; preds = %94
  %101 = extractelement <4 x i32> %85, i32 2
  br label %102

; <label>:102                                     ; preds = %100, %94
  %storemerge312 = phi i32 [ %101, %100 ], [ %69, %94 ]
  store i32 %storemerge312, i32 addrspace(1)* %scevgep173, align 4
  %103 = load i32, i32* %scevgep121, align 4
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %104
  %106 = load i32, i32* %105, align 4
  %107 = icmp eq i32 %106, 0
  br i1 %107, label %108, label %110

; <label>:108                                     ; preds = %102
  %109 = extractelement <4 x i32> %85, i32 1
  br label %110

; <label>:110                                     ; preds = %108, %102
  %storemerge313 = phi i32 [ %109, %108 ], [ %67, %102 ]
  store i32 %storemerge313, i32 addrspace(1)* %scevgep177, align 4
  %111 = load i32, i32* %scevgep123, align 4
  %112 = sext i32 %111 to i64
  %113 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %112
  %114 = load i32, i32* %113, align 4
  %115 = icmp eq i32 %114, 0
  br i1 %115, label %117, label %116

; <label>:116                                     ; preds = %110
  store i32 %65, i32 addrspace(1)* %scevgep181, align 4
  br label %152

; <label>:117                                     ; preds = %110
  %118 = extractelement <4 x i32> %85, i32 0
  store i32 %118, i32 addrspace(1)* %scevgep181, align 4
  br label %152

; <label>:119                                     ; preds = %63
  %tmp68 = add i64 %tmp67, 16
  %scevgep116 = getelementptr [32 x i32], [32 x i32]* @Lcs.iterator_cmp, i64 0, i64 %tmp68
  %120 = load i32, i32* %scevgep116, align 4
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %121
  %123 = load i32, i32* %122, align 4
  %124 = icmp eq i32 %123, 0
  br i1 %124, label %125, label %127

; <label>:125                                     ; preds = %119
  %126 = extractelement <4 x i32> %85, i32 3
  br label %127

; <label>:127                                     ; preds = %125, %119
  %storemerge = phi i32 [ %126, %125 ], [ %71, %119 ]
  store i32 %storemerge, i32 addrspace(1)* %scevgep169, align 4
  %128 = load i32, i32* %scevgep114, align 4
  %129 = sext i32 %128 to i64
  %130 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %129
  %131 = load i32, i32* %130, align 4
  %132 = icmp eq i32 %131, 0
  br i1 %132, label %133, label %135

; <label>:133                                     ; preds = %127
  %134 = extractelement <4 x i32> %85, i32 2
  br label %135

; <label>:135                                     ; preds = %133, %127
  %storemerge309 = phi i32 [ %134, %133 ], [ %69, %127 ]
  store i32 %storemerge309, i32 addrspace(1)* %scevgep173, align 4
  %136 = load i32, i32* %scevgep112, align 4
  %137 = sext i32 %136 to i64
  %138 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %137
  %139 = load i32, i32* %138, align 4
  %140 = icmp eq i32 %139, 0
  br i1 %140, label %141, label %143

; <label>:141                                     ; preds = %135
  %142 = extractelement <4 x i32> %85, i32 1
  br label %143

; <label>:143                                     ; preds = %141, %135
  %storemerge310 = phi i32 [ %142, %141 ], [ %67, %135 ]
  store i32 %storemerge310, i32 addrspace(1)* %scevgep177, align 4
  %144 = load i32, i32* %scevgep110, align 4
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds [48 x i32], [48 x i32]* %cmp_id, i64 0, i64 %145
  %147 = load i32, i32* %146, align 4
  %148 = icmp eq i32 %147, 0
  br i1 %148, label %150, label %149

; <label>:149                                     ; preds = %143
  store i32 %65, i32 addrspace(1)* %scevgep181, align 4
  br label %152

; <label>:150                                     ; preds = %143
  %151 = extractelement <4 x i32> %85, i32 0
  store i32 %151, i32 addrspace(1)* %scevgep181, align 4
  br label %152

; <label>:152                                     ; preds = %150, %149, %117, %116
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 4
  br i1 %exitcond, label %._crit_edge, label %63

._crit_edge:                                      ; preds = %152
  %exitcond78 = icmp eq i64 %tmp120, 129
  br i1 %exitcond78, label %._crit_edge73, label %24

._crit_edge73:                                    ; preds = %._crit_edge
  %indvar.next26 = add i64 %indvar25, 1
  %exitcond122 = icmp eq i64 %indvar.next26, 128
  br i1 %exitcond122, label %._crit_edge76, label %bb.nph72

._crit_edge76:                                    ; preds = %._crit_edge73
  ret void
}

declare <4 x float> @_Z6vload4jPU3AS1Kf(i32, float addrspace(1)*)

declare <4 x i32> @_Z7isequalDv4_fS_(<4 x float>, <4 x float>)

declare <4 x i32> @_Z3maxDv4_iS_(<4 x i32>, <4 x i32>)

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind
