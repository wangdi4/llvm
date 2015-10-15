; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlDCT.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@DCT_cllocal_inter = internal addrspace(3) global [64 x float] zeroinitializer		; <[64 x float] addrspace(3)*> [#uses=2]
@sgv = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [1 x i8*] [i8* addrspacecast ([64 x float] addrspace(3)* @DCT_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@DCT_VECTOR_cllocal_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer		; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@sgv1 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [1 x i8*] [i8* addrspacecast ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@DCT_VECTOR_DOT_cllocal_inter = internal addrspace(3) global [8 x <8 x float>] zeroinitializer		; <[8 x <8 x float>] addrspace(3)*> [#uses=9]
@sgv4 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv5 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv6 = internal constant [1 x i8*] [i8* addrspacecast ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter to i8*)]		; <[1 x i8*]*> [#uses=1]
@sgv7 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv8 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv9 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv10 = internal constant [5 x i8] c"2220\00"		; <[5 x i8]*> [#uses=1]
@fgv11 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv12 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @DCT
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @DCT_VECTOR
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @DCT_VECTOR_DOT
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @DCT_CPU
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @DCT_CPU_VECTOR
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @DCT(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
  %acc = alloca [8 x float], align 4
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind
  %call1 = call i32 @_Z13get_global_idj(i32 1) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %call3 = call i32 @get_group_id(i32 1) nounwind
  %call4 = call i32 @_Z12get_local_idj(i32 1) nounwind
  %.array = getelementptr [8 x float]* %acc, i32 0, i32 0
  %.array5 = getelementptr [8 x float]* %acc, i32 0, i32 1
  %.array6 = getelementptr [8 x float]* %acc, i32 0, i32 2
  %.array7 = getelementptr [8 x float]* %acc, i32 0, i32 3
  %.array8 = getelementptr [8 x float]* %acc, i32 0, i32 4
  %.array9 = getelementptr [8 x float]* %acc, i32 0, i32 5
  %.array10 = getelementptr [8 x float]* %acc, i32 0, i32 6
  %.array11 = getelementptr [8 x float]* %acc, i32 0, i32 7
  %mul = shl i32 %call4, 3
  %mul14 = shl i32 %call3, 3
  %mul16 = mul i32 %mul14, %width
  %mul18 = shl i32 %call2, 3
  %add = add i32 %mul16, %mul18
  %arrayidx31 = getelementptr float addrspace(1)* %dct, i32 %mul
  %0 = bitcast [8 x float]* %acc to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 32, i32 4, i1 false)
  %tmp32.pre = load float addrspace(1)* %arrayidx31, align 4
  br label %for.cond21.preheader

for.cond21.preheader:                             ; preds = %for.cond21.preheader.for.cond21.preheader_crit_edge, %entry
  %arrayidx.promoted = phi float [ 0.000000e+00, %entry ], [ %arrayidx.promoted.pre, %for.cond21.preheader.for.cond21.preheader_crit_edge ]
  %storemerge322 = phi i32 [ 0, %entry ], [ %inc47, %for.cond21.preheader.for.cond21.preheader_crit_edge ]
  %tmp134621 = phi i32 [ %add, %entry ], [ %add44, %for.cond21.preheader.for.cond21.preheader_crit_edge ]
  %arrayidx = getelementptr [8 x float]* %acc, i32 0, i32 %storemerge322
  %arrayidx37 = getelementptr float addrspace(1)* %input, i32 %tmp134621
  %tmp38 = load float addrspace(1)* %arrayidx37, align 4
  %mul39 = fmul float %tmp32.pre, %tmp38
  %add40 = fadd float %arrayidx.promoted, %mul39
  %add29.135 = or i32 %mul, 1
  %arrayidx31.1 = getelementptr float addrspace(1)* %dct, i32 %add29.135
  %tmp32.1 = load float addrspace(1)* %arrayidx31.1, align 4
  %add35.1 = add i32 %tmp134621, 1
  %arrayidx37.1 = getelementptr float addrspace(1)* %input, i32 %add35.1
  %tmp38.1 = load float addrspace(1)* %arrayidx37.1, align 4
  %mul39.1 = fmul float %tmp32.1, %tmp38.1
  %add40.1 = fadd float %add40, %mul39.1
  %add29.236 = or i32 %mul, 2
  %arrayidx31.2 = getelementptr float addrspace(1)* %dct, i32 %add29.236
  %tmp32.2 = load float addrspace(1)* %arrayidx31.2, align 4
  %add35.2 = add i32 %tmp134621, 2
  %arrayidx37.2 = getelementptr float addrspace(1)* %input, i32 %add35.2
  %tmp38.2 = load float addrspace(1)* %arrayidx37.2, align 4
  %mul39.2 = fmul float %tmp32.2, %tmp38.2
  %add40.2 = fadd float %add40.1, %mul39.2
  %add29.337 = or i32 %mul, 3
  %arrayidx31.3 = getelementptr float addrspace(1)* %dct, i32 %add29.337
  %tmp32.3 = load float addrspace(1)* %arrayidx31.3, align 4
  %add35.3 = add i32 %tmp134621, 3
  %arrayidx37.3 = getelementptr float addrspace(1)* %input, i32 %add35.3
  %tmp38.3 = load float addrspace(1)* %arrayidx37.3, align 4
  %mul39.3 = fmul float %tmp32.3, %tmp38.3
  %add40.3 = fadd float %add40.2, %mul39.3
  %add29.438 = or i32 %mul, 4
  %arrayidx31.4 = getelementptr float addrspace(1)* %dct, i32 %add29.438
  %tmp32.4 = load float addrspace(1)* %arrayidx31.4, align 4
  %add35.4 = add i32 %tmp134621, 4
  %arrayidx37.4 = getelementptr float addrspace(1)* %input, i32 %add35.4
  %tmp38.4 = load float addrspace(1)* %arrayidx37.4, align 4
  %mul39.4 = fmul float %tmp32.4, %tmp38.4
  %add40.4 = fadd float %add40.3, %mul39.4
  %add29.539 = or i32 %mul, 5
  %arrayidx31.5 = getelementptr float addrspace(1)* %dct, i32 %add29.539
  %tmp32.5 = load float addrspace(1)* %arrayidx31.5, align 4
  %add35.5 = add i32 %tmp134621, 5
  %arrayidx37.5 = getelementptr float addrspace(1)* %input, i32 %add35.5
  %tmp38.5 = load float addrspace(1)* %arrayidx37.5, align 4
  %mul39.5 = fmul float %tmp32.5, %tmp38.5
  %add40.5 = fadd float %add40.4, %mul39.5
  %add29.640 = or i32 %mul, 6
  %arrayidx31.6 = getelementptr float addrspace(1)* %dct, i32 %add29.640
  %tmp32.6 = load float addrspace(1)* %arrayidx31.6, align 4
  %add35.6 = add i32 %tmp134621, 6
  %arrayidx37.6 = getelementptr float addrspace(1)* %input, i32 %add35.6
  %tmp38.6 = load float addrspace(1)* %arrayidx37.6, align 4
  %mul39.6 = fmul float %tmp32.6, %tmp38.6
  %add40.6 = fadd float %add40.5, %mul39.6
  %add29.741 = or i32 %mul, 7
  %arrayidx31.7 = getelementptr float addrspace(1)* %dct, i32 %add29.741
  %tmp32.7 = load float addrspace(1)* %arrayidx31.7, align 4
  %add35.7 = add i32 %tmp134621, 7
  %arrayidx37.7 = getelementptr float addrspace(1)* %input, i32 %add35.7
  %tmp38.7 = load float addrspace(1)* %arrayidx37.7, align 4
  %mul39.7 = fmul float %tmp32.7, %tmp38.7
  %add40.7 = fadd float %add40.6, %mul39.7
  store float %add40.7, float* %arrayidx, align 4
  %inc47 = add i32 %storemerge322, 1
  %cmp = icmp slt i32 %inc47, 8
  br i1 %cmp, label %for.cond21.preheader.for.cond21.preheader_crit_edge, label %for.end48

for.cond21.preheader.for.cond21.preheader_crit_edge: ; preds = %for.cond21.preheader
  %add44 = add i32 %tmp134621, %width
  %arrayidx.phi.trans.insert = getelementptr [8 x float]* %acc, i32 0, i32 %inc47
  %arrayidx.promoted.pre = load float* %arrayidx.phi.trans.insert, align 4
  br label %for.cond21.preheader

for.end48:                                        ; preds = %for.cond21.preheader
  %acc23 = bitcast [8 x float]* %acc to i8*
  %arrayidx53 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %mul
  %tmp56 = load float* %.array, align 4
  store float %tmp56, float addrspace(3)* %arrayidx53, align 4
  %add587 = or i32 %mul, 1
  %arrayidx59 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add587
  %tmp62 = load float* %.array5, align 4
  store float %tmp62, float addrspace(3)* %arrayidx59, align 4
  %add648 = or i32 %mul, 2
  %arrayidx65 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add648
  %tmp68 = load float* %.array6, align 4
  store float %tmp68, float addrspace(3)* %arrayidx65, align 4
  %add709 = or i32 %mul, 3
  %arrayidx71 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add709
  %tmp74 = load float* %.array7, align 4
  store float %tmp74, float addrspace(3)* %arrayidx71, align 4
  %add7610 = or i32 %mul, 4
  %arrayidx77 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add7610
  %tmp80 = load float* %.array8, align 4
  store float %tmp80, float addrspace(3)* %arrayidx77, align 4
  %add8211 = or i32 %mul, 5
  %arrayidx83 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add8211
  %tmp86 = load float* %.array9, align 4
  store float %tmp86, float addrspace(3)* %arrayidx83, align 4
  %add8812 = or i32 %mul, 6
  %arrayidx89 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add8812
  %tmp92 = load float* %.array10, align 4
  store float %tmp92, float addrspace(3)* %arrayidx89, align 4
  %add9413 = or i32 %mul, 7
  %arrayidx95 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add9413
  %tmp98 = load float* %.array11, align 4
  store float %tmp98, float addrspace(3)* %arrayidx95, align 4
  call void @_Z7barrierm(i32 1) nounwind
  call void @llvm.memset.p0i8.i32(i8* %acc23, i8 0, i32 32, i32 4, i1 false)
  %tmp139.pre = load float addrspace(1)* %arrayidx31, align 4
  %tmp139.1.pre = load float addrspace(1)* %arrayidx31.1, align 4
  %tmp139.2.pre = load float addrspace(1)* %arrayidx31.2, align 4
  %tmp139.3.pre = load float addrspace(1)* %arrayidx31.3, align 4
  %tmp139.4.pre = load float addrspace(1)* %arrayidx31.4, align 4
  %tmp139.5.pre = load float addrspace(1)* %arrayidx31.5, align 4
  %tmp139.6.pre = load float addrspace(1)* %arrayidx31.6, align 4
  %tmp139.7.pre = load float addrspace(1)* %arrayidx31.7, align 4
  br label %for.cond121.preheader

for.cond121.preheader:                            ; preds = %for.cond121.preheader.for.cond121.preheader_crit_edge, %for.end48
  %arrayidx127.promoted = phi float [ 0.000000e+00, %for.end48 ], [ %arrayidx127.promoted.pre, %for.cond121.preheader.for.cond121.preheader_crit_edge ]
  %storemerge17 = phi i32 [ 0, %for.end48 ], [ %inc150, %for.cond121.preheader.for.cond121.preheader_crit_edge ]
  %add147516 = phi i32 [ 0, %for.end48 ], [ %add147, %for.cond121.preheader.for.cond121.preheader_crit_edge ]
  %arrayidx127 = getelementptr [8 x float]* %acc, i32 0, i32 %storemerge17
  %arrayidx132 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add147516
  %tmp133 = load float addrspace(3)* %arrayidx132, align 4
  %mul140 = fmul float %tmp133, %tmp139.pre
  %add141 = fadd float %arrayidx127.promoted, %mul140
  %add131.142 = or i32 %add147516, 1
  %arrayidx132.1 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.142
  %tmp133.1 = load float addrspace(3)* %arrayidx132.1, align 4
  %mul140.1 = fmul float %tmp133.1, %tmp139.1.pre
  %add141.1 = fadd float %add141, %mul140.1
  %add131.243 = or i32 %add147516, 2
  %arrayidx132.2 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.243
  %tmp133.2 = load float addrspace(3)* %arrayidx132.2, align 4
  %mul140.2 = fmul float %tmp133.2, %tmp139.2.pre
  %add141.2 = fadd float %add141.1, %mul140.2
  %add131.344 = or i32 %add147516, 3
  %arrayidx132.3 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.344
  %tmp133.3 = load float addrspace(3)* %arrayidx132.3, align 4
  %mul140.3 = fmul float %tmp133.3, %tmp139.3.pre
  %add141.3 = fadd float %add141.2, %mul140.3
  %add131.445 = or i32 %add147516, 4
  %arrayidx132.4 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.445
  %tmp133.4 = load float addrspace(3)* %arrayidx132.4, align 4
  %mul140.4 = fmul float %tmp133.4, %tmp139.4.pre
  %add141.4 = fadd float %add141.3, %mul140.4
  %add131.546 = or i32 %add147516, 5
  %arrayidx132.5 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.546
  %tmp133.5 = load float addrspace(3)* %arrayidx132.5, align 4
  %mul140.5 = fmul float %tmp133.5, %tmp139.5.pre
  %add141.5 = fadd float %add141.4, %mul140.5
  %add131.647 = or i32 %add147516, 6
  %arrayidx132.6 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.647
  %tmp133.6 = load float addrspace(3)* %arrayidx132.6, align 4
  %mul140.6 = fmul float %tmp133.6, %tmp139.6.pre
  %add141.6 = fadd float %add141.5, %mul140.6
  %add131.748 = or i32 %add147516, 7
  %arrayidx132.7 = getelementptr [64 x float] addrspace(3)* @DCT_cllocal_inter, i32 0, i32 %add131.748
  %tmp133.7 = load float addrspace(3)* %arrayidx132.7, align 4
  %mul140.7 = fmul float %tmp133.7, %tmp139.7.pre
  %add141.7 = fadd float %add141.6, %mul140.7
  store float %add141.7, float* %arrayidx127, align 4
  %inc150 = add i32 %storemerge17, 1
  %cmp119 = icmp slt i32 %inc150, 8
  br i1 %cmp119, label %for.cond121.preheader.for.cond121.preheader_crit_edge, label %for.end151

for.cond121.preheader.for.cond121.preheader_crit_edge: ; preds = %for.cond121.preheader
  %add147 = add i32 %add147516, 8
  %arrayidx127.phi.trans.insert = getelementptr [8 x float]* %acc, i32 0, i32 %inc150
  %arrayidx127.promoted.pre = load float* %arrayidx127.phi.trans.insert, align 4
  br label %for.cond121.preheader

for.end151:                                       ; preds = %for.cond121.preheader
  %add155 = add i32 %mul14, %call4
  %mul157 = mul i32 %add155, %width
  %add160 = add i32 %mul157, %mul18
  %arrayidx164 = getelementptr float addrspace(1)* %output, i32 %add160
  %tmp167 = load float* %.array, align 4
  store float %tmp167, float addrspace(1)* %arrayidx164, align 4
  %add169 = add i32 %add160, 1
  %arrayidx171 = getelementptr float addrspace(1)* %output, i32 %add169
  %tmp174 = load float* %.array5, align 4
  store float %tmp174, float addrspace(1)* %arrayidx171, align 4
  %add176 = add i32 %add160, 2
  %arrayidx178 = getelementptr float addrspace(1)* %output, i32 %add176
  %tmp181 = load float* %.array6, align 4
  store float %tmp181, float addrspace(1)* %arrayidx178, align 4
  %add183 = add i32 %add160, 3
  %arrayidx185 = getelementptr float addrspace(1)* %output, i32 %add183
  %tmp188 = load float* %.array7, align 4
  store float %tmp188, float addrspace(1)* %arrayidx185, align 4
  %add190 = add i32 %add160, 4
  %arrayidx192 = getelementptr float addrspace(1)* %output, i32 %add190
  %tmp195 = load float* %.array8, align 4
  store float %tmp195, float addrspace(1)* %arrayidx192, align 4
  %add197 = add i32 %add160, 5
  %arrayidx199 = getelementptr float addrspace(1)* %output, i32 %add197
  %tmp202 = load float* %.array9, align 4
  store float %tmp202, float addrspace(1)* %arrayidx199, align 4
  %add204 = add i32 %add160, 6
  %arrayidx206 = getelementptr float addrspace(1)* %output, i32 %add204
  %tmp209 = load float* %.array10, align 4
  store float %tmp209, float addrspace(1)* %arrayidx206, align 4
  %add211 = add i32 %add160, 7
  %arrayidx213 = getelementptr float addrspace(1)* %output, i32 %add211
  %tmp216 = load float* %.array11, align 4
  store float %tmp216, float addrspace(1)* %arrayidx213, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

declare i32 @get_group_id(i32)

declare i32 @_Z12get_local_idj(i32)

declare void @_Z7barrierm(i32)

define void @DCT_VECTOR(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind
  %call1 = call i32 @_Z13get_global_idj(i32 1) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %call3 = call i32 @get_group_id(i32 1) nounwind
  %call4 = call i32 @_Z12get_local_idj(i32 1) nounwind
  %div = lshr i32 %width, 3
  %mul = shl i32 %call3, 3
  %mul11 = mul i32 %mul, %div
  %add = add i32 %mul11, %call2
  %arrayidx = getelementptr <8 x float> addrspace(1)* %dct, i32 %call4
  %tmp15 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %arrayidx18 = getelementptr <8 x float> addrspace(1)* %input, i32 %add
  %tmp19 = load <8 x float> addrspace(1)* %arrayidx18, align 32
  %mul20 = fmul <8 x float> %tmp15, %tmp19
  %tmp22 = extractelement <8 x float> %mul20, i32 0
  %tmp24 = extractelement <8 x float> %mul20, i32 1
  %add25 = fadd float %tmp22, %tmp24
  %tmp27 = extractelement <8 x float> %mul20, i32 2
  %add28 = fadd float %add25, %tmp27
  %tmp30 = extractelement <8 x float> %mul20, i32 3
  %add31 = fadd float %add28, %tmp30
  %tmp33 = extractelement <8 x float> %mul20, i32 4
  %add34 = fadd float %add31, %tmp33
  %tmp36 = extractelement <8 x float> %mul20, i32 5
  %add37 = fadd float %add34, %tmp36
  %tmp39 = extractelement <8 x float> %mul20, i32 6
  %add40 = fadd float %add37, %tmp39
  %tmp42 = extractelement <8 x float> %mul20, i32 7
  %add43 = fadd float %add40, %tmp42
  %tmp45 = insertelement <8 x float> undef, float %add43, i32 0
  %add48 = add i32 %add, %div
  %arrayidx55 = getelementptr <8 x float> addrspace(1)* %input, i32 %add48
  %tmp56 = load <8 x float> addrspace(1)* %arrayidx55, align 32
  %mul57 = fmul <8 x float> %tmp15, %tmp56
  %tmp59 = extractelement <8 x float> %mul57, i32 0
  %tmp61 = extractelement <8 x float> %mul57, i32 1
  %add62 = fadd float %tmp59, %tmp61
  %tmp64 = extractelement <8 x float> %mul57, i32 2
  %add65 = fadd float %add62, %tmp64
  %tmp67 = extractelement <8 x float> %mul57, i32 3
  %add68 = fadd float %add65, %tmp67
  %tmp70 = extractelement <8 x float> %mul57, i32 4
  %add71 = fadd float %add68, %tmp70
  %tmp73 = extractelement <8 x float> %mul57, i32 5
  %add74 = fadd float %add71, %tmp73
  %tmp76 = extractelement <8 x float> %mul57, i32 6
  %add77 = fadd float %add74, %tmp76
  %tmp79 = extractelement <8 x float> %mul57, i32 7
  %add80 = fadd float %add77, %tmp79
  %tmp82 = insertelement <8 x float> %tmp45, float %add80, i32 1
  %add85 = add i32 %add48, %div
  %arrayidx92 = getelementptr <8 x float> addrspace(1)* %input, i32 %add85
  %tmp93 = load <8 x float> addrspace(1)* %arrayidx92, align 32
  %mul94 = fmul <8 x float> %tmp15, %tmp93
  %tmp96 = extractelement <8 x float> %mul94, i32 0
  %tmp98 = extractelement <8 x float> %mul94, i32 1
  %add99 = fadd float %tmp96, %tmp98
  %tmp101 = extractelement <8 x float> %mul94, i32 2
  %add102 = fadd float %add99, %tmp101
  %tmp104 = extractelement <8 x float> %mul94, i32 3
  %add105 = fadd float %add102, %tmp104
  %tmp107 = extractelement <8 x float> %mul94, i32 4
  %add108 = fadd float %add105, %tmp107
  %tmp110 = extractelement <8 x float> %mul94, i32 5
  %add111 = fadd float %add108, %tmp110
  %tmp113 = extractelement <8 x float> %mul94, i32 6
  %add114 = fadd float %add111, %tmp113
  %tmp116 = extractelement <8 x float> %mul94, i32 7
  %add117 = fadd float %add114, %tmp116
  %tmp119 = insertelement <8 x float> %tmp82, float %add117, i32 2
  %add122 = add i32 %add85, %div
  %arrayidx129 = getelementptr <8 x float> addrspace(1)* %input, i32 %add122
  %tmp130 = load <8 x float> addrspace(1)* %arrayidx129, align 32
  %mul131 = fmul <8 x float> %tmp15, %tmp130
  %tmp133 = extractelement <8 x float> %mul131, i32 0
  %tmp135 = extractelement <8 x float> %mul131, i32 1
  %add136 = fadd float %tmp133, %tmp135
  %tmp138 = extractelement <8 x float> %mul131, i32 2
  %add139 = fadd float %add136, %tmp138
  %tmp141 = extractelement <8 x float> %mul131, i32 3
  %add142 = fadd float %add139, %tmp141
  %tmp144 = extractelement <8 x float> %mul131, i32 4
  %add145 = fadd float %add142, %tmp144
  %tmp147 = extractelement <8 x float> %mul131, i32 5
  %add148 = fadd float %add145, %tmp147
  %tmp150 = extractelement <8 x float> %mul131, i32 6
  %add151 = fadd float %add148, %tmp150
  %tmp153 = extractelement <8 x float> %mul131, i32 7
  %add154 = fadd float %add151, %tmp153
  %tmp156 = insertelement <8 x float> %tmp119, float %add154, i32 3
  %add159 = add i32 %add122, %div
  %arrayidx166 = getelementptr <8 x float> addrspace(1)* %input, i32 %add159
  %tmp167 = load <8 x float> addrspace(1)* %arrayidx166, align 32
  %mul168 = fmul <8 x float> %tmp15, %tmp167
  %tmp170 = extractelement <8 x float> %mul168, i32 0
  %tmp172 = extractelement <8 x float> %mul168, i32 1
  %add173 = fadd float %tmp170, %tmp172
  %tmp175 = extractelement <8 x float> %mul168, i32 2
  %add176 = fadd float %add173, %tmp175
  %tmp178 = extractelement <8 x float> %mul168, i32 3
  %add179 = fadd float %add176, %tmp178
  %tmp181 = extractelement <8 x float> %mul168, i32 4
  %add182 = fadd float %add179, %tmp181
  %tmp184 = extractelement <8 x float> %mul168, i32 5
  %add185 = fadd float %add182, %tmp184
  %tmp187 = extractelement <8 x float> %mul168, i32 6
  %add188 = fadd float %add185, %tmp187
  %tmp190 = extractelement <8 x float> %mul168, i32 7
  %add191 = fadd float %add188, %tmp190
  %tmp193 = insertelement <8 x float> %tmp156, float %add191, i32 4
  %add196 = add i32 %add159, %div
  %arrayidx203 = getelementptr <8 x float> addrspace(1)* %input, i32 %add196
  %tmp204 = load <8 x float> addrspace(1)* %arrayidx203, align 32
  %mul205 = fmul <8 x float> %tmp15, %tmp204
  %tmp207 = extractelement <8 x float> %mul205, i32 0
  %tmp209 = extractelement <8 x float> %mul205, i32 1
  %add210 = fadd float %tmp207, %tmp209
  %tmp212 = extractelement <8 x float> %mul205, i32 2
  %add213 = fadd float %add210, %tmp212
  %tmp215 = extractelement <8 x float> %mul205, i32 3
  %add216 = fadd float %add213, %tmp215
  %tmp218 = extractelement <8 x float> %mul205, i32 4
  %add219 = fadd float %add216, %tmp218
  %tmp221 = extractelement <8 x float> %mul205, i32 5
  %add222 = fadd float %add219, %tmp221
  %tmp224 = extractelement <8 x float> %mul205, i32 6
  %add225 = fadd float %add222, %tmp224
  %tmp227 = extractelement <8 x float> %mul205, i32 7
  %add228 = fadd float %add225, %tmp227
  %tmp230 = insertelement <8 x float> %tmp193, float %add228, i32 5
  %add233 = add i32 %add196, %div
  %arrayidx240 = getelementptr <8 x float> addrspace(1)* %input, i32 %add233
  %tmp241 = load <8 x float> addrspace(1)* %arrayidx240, align 32
  %mul242 = fmul <8 x float> %tmp15, %tmp241
  %tmp244 = extractelement <8 x float> %mul242, i32 0
  %tmp246 = extractelement <8 x float> %mul242, i32 1
  %add247 = fadd float %tmp244, %tmp246
  %tmp249 = extractelement <8 x float> %mul242, i32 2
  %add250 = fadd float %add247, %tmp249
  %tmp252 = extractelement <8 x float> %mul242, i32 3
  %add253 = fadd float %add250, %tmp252
  %tmp255 = extractelement <8 x float> %mul242, i32 4
  %add256 = fadd float %add253, %tmp255
  %tmp258 = extractelement <8 x float> %mul242, i32 5
  %add259 = fadd float %add256, %tmp258
  %tmp261 = extractelement <8 x float> %mul242, i32 6
  %add262 = fadd float %add259, %tmp261
  %tmp264 = extractelement <8 x float> %mul242, i32 7
  %add265 = fadd float %add262, %tmp264
  %tmp267 = insertelement <8 x float> %tmp230, float %add265, i32 6
  %add270 = add i32 %add233, %div
  %arrayidx277 = getelementptr <8 x float> addrspace(1)* %input, i32 %add270
  %tmp278 = load <8 x float> addrspace(1)* %arrayidx277, align 32
  %mul279 = fmul <8 x float> %tmp15, %tmp278
  %tmp281 = extractelement <8 x float> %mul279, i32 0
  %tmp283 = extractelement <8 x float> %mul279, i32 1
  %add284 = fadd float %tmp281, %tmp283
  %tmp286 = extractelement <8 x float> %mul279, i32 2
  %add287 = fadd float %add284, %tmp286
  %tmp289 = extractelement <8 x float> %mul279, i32 3
  %add290 = fadd float %add287, %tmp289
  %tmp292 = extractelement <8 x float> %mul279, i32 4
  %add293 = fadd float %add290, %tmp292
  %tmp295 = extractelement <8 x float> %mul279, i32 5
  %add296 = fadd float %add293, %tmp295
  %tmp298 = extractelement <8 x float> %mul279, i32 6
  %add299 = fadd float %add296, %tmp298
  %tmp301 = extractelement <8 x float> %mul279, i32 7
  %add302 = fadd float %add299, %tmp301
  %tmp304 = insertelement <8 x float> %tmp267, float %add302, i32 7
  %arrayidx307 = getelementptr [8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 %call4
  store <8 x float> %tmp304, <8 x float> addrspace(3)* %arrayidx307, align 32
  call void @_Z7barrierm(i32 1) nounwind
  %tmp311 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 0), align 32
  %tmp315 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %mul316 = fmul <8 x float> %tmp311, %tmp315
  %tmp318 = extractelement <8 x float> %mul316, i32 0
  %tmp320 = extractelement <8 x float> %mul316, i32 1
  %add321 = fadd float %tmp318, %tmp320
  %tmp323 = extractelement <8 x float> %mul316, i32 2
  %add324 = fadd float %add321, %tmp323
  %tmp326 = extractelement <8 x float> %mul316, i32 3
  %add327 = fadd float %add324, %tmp326
  %tmp329 = extractelement <8 x float> %mul316, i32 4
  %add330 = fadd float %add327, %tmp329
  %tmp332 = extractelement <8 x float> %mul316, i32 5
  %add333 = fadd float %add330, %tmp332
  %tmp335 = extractelement <8 x float> %mul316, i32 6
  %add336 = fadd float %add333, %tmp335
  %tmp338 = extractelement <8 x float> %mul316, i32 7
  %add339 = fadd float %add336, %tmp338
  %tmp341 = insertelement <8 x float> undef, float %add339, i32 0
  %tmp342 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 1), align 32
  %mul347 = fmul <8 x float> %tmp342, %tmp315
  %tmp349 = extractelement <8 x float> %mul347, i32 0
  %tmp351 = extractelement <8 x float> %mul347, i32 1
  %add352 = fadd float %tmp349, %tmp351
  %tmp354 = extractelement <8 x float> %mul347, i32 2
  %add355 = fadd float %add352, %tmp354
  %tmp357 = extractelement <8 x float> %mul347, i32 3
  %add358 = fadd float %add355, %tmp357
  %tmp360 = extractelement <8 x float> %mul347, i32 4
  %add361 = fadd float %add358, %tmp360
  %tmp363 = extractelement <8 x float> %mul347, i32 5
  %add364 = fadd float %add361, %tmp363
  %tmp366 = extractelement <8 x float> %mul347, i32 6
  %add367 = fadd float %add364, %tmp366
  %tmp369 = extractelement <8 x float> %mul347, i32 7
  %add370 = fadd float %add367, %tmp369
  %tmp372 = insertelement <8 x float> %tmp341, float %add370, i32 1
  %tmp373 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 2), align 32
  %mul378 = fmul <8 x float> %tmp373, %tmp315
  %tmp380 = extractelement <8 x float> %mul378, i32 0
  %tmp382 = extractelement <8 x float> %mul378, i32 1
  %add383 = fadd float %tmp380, %tmp382
  %tmp385 = extractelement <8 x float> %mul378, i32 2
  %add386 = fadd float %add383, %tmp385
  %tmp388 = extractelement <8 x float> %mul378, i32 3
  %add389 = fadd float %add386, %tmp388
  %tmp391 = extractelement <8 x float> %mul378, i32 4
  %add392 = fadd float %add389, %tmp391
  %tmp394 = extractelement <8 x float> %mul378, i32 5
  %add395 = fadd float %add392, %tmp394
  %tmp397 = extractelement <8 x float> %mul378, i32 6
  %add398 = fadd float %add395, %tmp397
  %tmp400 = extractelement <8 x float> %mul378, i32 7
  %add401 = fadd float %add398, %tmp400
  %tmp403 = insertelement <8 x float> %tmp372, float %add401, i32 2
  %tmp404 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 3), align 32
  %mul409 = fmul <8 x float> %tmp404, %tmp315
  %tmp411 = extractelement <8 x float> %mul409, i32 0
  %tmp413 = extractelement <8 x float> %mul409, i32 1
  %add414 = fadd float %tmp411, %tmp413
  %tmp416 = extractelement <8 x float> %mul409, i32 2
  %add417 = fadd float %add414, %tmp416
  %tmp419 = extractelement <8 x float> %mul409, i32 3
  %add420 = fadd float %add417, %tmp419
  %tmp422 = extractelement <8 x float> %mul409, i32 4
  %add423 = fadd float %add420, %tmp422
  %tmp425 = extractelement <8 x float> %mul409, i32 5
  %add426 = fadd float %add423, %tmp425
  %tmp428 = extractelement <8 x float> %mul409, i32 6
  %add429 = fadd float %add426, %tmp428
  %tmp431 = extractelement <8 x float> %mul409, i32 7
  %add432 = fadd float %add429, %tmp431
  %tmp434 = insertelement <8 x float> %tmp403, float %add432, i32 3
  %tmp435 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 4), align 32
  %mul440 = fmul <8 x float> %tmp435, %tmp315
  %tmp442 = extractelement <8 x float> %mul440, i32 0
  %tmp444 = extractelement <8 x float> %mul440, i32 1
  %add445 = fadd float %tmp442, %tmp444
  %tmp447 = extractelement <8 x float> %mul440, i32 2
  %add448 = fadd float %add445, %tmp447
  %tmp450 = extractelement <8 x float> %mul440, i32 3
  %add451 = fadd float %add448, %tmp450
  %tmp453 = extractelement <8 x float> %mul440, i32 4
  %add454 = fadd float %add451, %tmp453
  %tmp456 = extractelement <8 x float> %mul440, i32 5
  %add457 = fadd float %add454, %tmp456
  %tmp459 = extractelement <8 x float> %mul440, i32 6
  %add460 = fadd float %add457, %tmp459
  %tmp462 = extractelement <8 x float> %mul440, i32 7
  %add463 = fadd float %add460, %tmp462
  %tmp465 = insertelement <8 x float> %tmp434, float %add463, i32 4
  %tmp466 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 5), align 32
  %mul471 = fmul <8 x float> %tmp466, %tmp315
  %tmp473 = extractelement <8 x float> %mul471, i32 0
  %tmp475 = extractelement <8 x float> %mul471, i32 1
  %add476 = fadd float %tmp473, %tmp475
  %tmp478 = extractelement <8 x float> %mul471, i32 2
  %add479 = fadd float %add476, %tmp478
  %tmp481 = extractelement <8 x float> %mul471, i32 3
  %add482 = fadd float %add479, %tmp481
  %tmp484 = extractelement <8 x float> %mul471, i32 4
  %add485 = fadd float %add482, %tmp484
  %tmp487 = extractelement <8 x float> %mul471, i32 5
  %add488 = fadd float %add485, %tmp487
  %tmp490 = extractelement <8 x float> %mul471, i32 6
  %add491 = fadd float %add488, %tmp490
  %tmp493 = extractelement <8 x float> %mul471, i32 7
  %add494 = fadd float %add491, %tmp493
  %tmp496 = insertelement <8 x float> %tmp465, float %add494, i32 5
  %tmp497 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 6), align 32
  %mul502 = fmul <8 x float> %tmp497, %tmp315
  %tmp504 = extractelement <8 x float> %mul502, i32 0
  %tmp506 = extractelement <8 x float> %mul502, i32 1
  %add507 = fadd float %tmp504, %tmp506
  %tmp509 = extractelement <8 x float> %mul502, i32 2
  %add510 = fadd float %add507, %tmp509
  %tmp512 = extractelement <8 x float> %mul502, i32 3
  %add513 = fadd float %add510, %tmp512
  %tmp515 = extractelement <8 x float> %mul502, i32 4
  %add516 = fadd float %add513, %tmp515
  %tmp518 = extractelement <8 x float> %mul502, i32 5
  %add519 = fadd float %add516, %tmp518
  %tmp521 = extractelement <8 x float> %mul502, i32 6
  %add522 = fadd float %add519, %tmp521
  %tmp524 = extractelement <8 x float> %mul502, i32 7
  %add525 = fadd float %add522, %tmp524
  %tmp527 = insertelement <8 x float> %tmp496, float %add525, i32 6
  %tmp528 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_cllocal_inter, i32 0, i32 7), align 32
  %mul533 = fmul <8 x float> %tmp528, %tmp315
  %tmp535 = extractelement <8 x float> %mul533, i32 0
  %tmp537 = extractelement <8 x float> %mul533, i32 1
  %add538 = fadd float %tmp535, %tmp537
  %tmp540 = extractelement <8 x float> %mul533, i32 2
  %add541 = fadd float %add538, %tmp540
  %tmp543 = extractelement <8 x float> %mul533, i32 3
  %add544 = fadd float %add541, %tmp543
  %tmp546 = extractelement <8 x float> %mul533, i32 4
  %add547 = fadd float %add544, %tmp546
  %tmp549 = extractelement <8 x float> %mul533, i32 5
  %add550 = fadd float %add547, %tmp549
  %tmp552 = extractelement <8 x float> %mul533, i32 6
  %add553 = fadd float %add550, %tmp552
  %tmp555 = extractelement <8 x float> %mul533, i32 7
  %add556 = fadd float %add553, %tmp555
  %tmp558 = insertelement <8 x float> %tmp527, float %add556, i32 7
  %add562 = add i32 %mul, %call4
  %mul564 = mul i32 %add562, %div
  %add566 = add i32 %mul564, %call2
  %arrayidx569 = getelementptr <8 x float> addrspace(1)* %output, i32 %add566
  store <8 x float> %tmp558, <8 x float> addrspace(1)* %arrayidx569, align 32
  ret void
}

define void @DCT_VECTOR_DOT(<8 x float> addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind
  %call1 = call i32 @_Z13get_global_idj(i32 1) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %call3 = call i32 @get_group_id(i32 1) nounwind
  %call4 = call i32 @_Z12get_local_idj(i32 1) nounwind
  %div = lshr i32 %width, 3
  %mul = shl i32 %call3, 3
  %mul11 = mul i32 %mul, %div
  %add = add i32 %mul11, %call2
  %arrayidx = getelementptr <8 x float> addrspace(1)* %dct, i32 %call4
  %tmp15 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp16 = shufflevector <8 x float> %tmp15, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx19 = getelementptr <8 x float> addrspace(1)* %input, i32 %add
  %tmp20 = load <8 x float> addrspace(1)* %arrayidx19, align 32
  %tmp21 = shufflevector <8 x float> %tmp20, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call22 = call float @__dotf4(<4 x float> %tmp16, <4 x float> %tmp21) nounwind
  %tmp26 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp27 = shufflevector <8 x float> %tmp26, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp31 = load <8 x float> addrspace(1)* %arrayidx19, align 32
  %tmp32 = shufflevector <8 x float> %tmp31, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call33 = call float @__dotf4(<4 x float> %tmp27, <4 x float> %tmp32) nounwind
  %add34 = fadd float %call22, %call33
  %tmp36 = insertelement <8 x float> undef, float %add34, i32 0
  %add39 = add i32 %add, %div
  %tmp43 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp44 = shufflevector <8 x float> %tmp43, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx47 = getelementptr <8 x float> addrspace(1)* %input, i32 %add39
  %tmp48 = load <8 x float> addrspace(1)* %arrayidx47, align 32
  %tmp49 = shufflevector <8 x float> %tmp48, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call50 = call float @__dotf4(<4 x float> %tmp44, <4 x float> %tmp49) nounwind
  %tmp54 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp55 = shufflevector <8 x float> %tmp54, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp59 = load <8 x float> addrspace(1)* %arrayidx47, align 32
  %tmp60 = shufflevector <8 x float> %tmp59, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call61 = call float @__dotf4(<4 x float> %tmp55, <4 x float> %tmp60) nounwind
  %add62 = fadd float %call50, %call61
  %tmp64 = insertelement <8 x float> %tmp36, float %add62, i32 1
  %add67 = add i32 %add39, %div
  %tmp71 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp72 = shufflevector <8 x float> %tmp71, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx75 = getelementptr <8 x float> addrspace(1)* %input, i32 %add67
  %tmp76 = load <8 x float> addrspace(1)* %arrayidx75, align 32
  %tmp77 = shufflevector <8 x float> %tmp76, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call78 = call float @__dotf4(<4 x float> %tmp72, <4 x float> %tmp77) nounwind
  %tmp82 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp83 = shufflevector <8 x float> %tmp82, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp87 = load <8 x float> addrspace(1)* %arrayidx75, align 32
  %tmp88 = shufflevector <8 x float> %tmp87, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call89 = call float @__dotf4(<4 x float> %tmp83, <4 x float> %tmp88) nounwind
  %add90 = fadd float %call78, %call89
  %tmp92 = insertelement <8 x float> %tmp64, float %add90, i32 2
  %add95 = add i32 %add67, %div
  %tmp99 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp100 = shufflevector <8 x float> %tmp99, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx103 = getelementptr <8 x float> addrspace(1)* %input, i32 %add95
  %tmp104 = load <8 x float> addrspace(1)* %arrayidx103, align 32
  %tmp105 = shufflevector <8 x float> %tmp104, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call106 = call float @__dotf4(<4 x float> %tmp100, <4 x float> %tmp105) nounwind
  %tmp110 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp111 = shufflevector <8 x float> %tmp110, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp115 = load <8 x float> addrspace(1)* %arrayidx103, align 32
  %tmp116 = shufflevector <8 x float> %tmp115, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call117 = call float @__dotf4(<4 x float> %tmp111, <4 x float> %tmp116) nounwind
  %add118 = fadd float %call106, %call117
  %tmp120 = insertelement <8 x float> %tmp92, float %add118, i32 3
  %add123 = add i32 %add95, %div
  %tmp127 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp128 = shufflevector <8 x float> %tmp127, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx131 = getelementptr <8 x float> addrspace(1)* %input, i32 %add123
  %tmp132 = load <8 x float> addrspace(1)* %arrayidx131, align 32
  %tmp133 = shufflevector <8 x float> %tmp132, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call134 = call float @__dotf4(<4 x float> %tmp128, <4 x float> %tmp133) nounwind
  %tmp138 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp139 = shufflevector <8 x float> %tmp138, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp143 = load <8 x float> addrspace(1)* %arrayidx131, align 32
  %tmp144 = shufflevector <8 x float> %tmp143, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call145 = call float @__dotf4(<4 x float> %tmp139, <4 x float> %tmp144) nounwind
  %add146 = fadd float %call134, %call145
  %tmp148 = insertelement <8 x float> %tmp120, float %add146, i32 4
  %add151 = add i32 %add123, %div
  %tmp155 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp156 = shufflevector <8 x float> %tmp155, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx159 = getelementptr <8 x float> addrspace(1)* %input, i32 %add151
  %tmp160 = load <8 x float> addrspace(1)* %arrayidx159, align 32
  %tmp161 = shufflevector <8 x float> %tmp160, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call162 = call float @__dotf4(<4 x float> %tmp156, <4 x float> %tmp161) nounwind
  %tmp166 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp167 = shufflevector <8 x float> %tmp166, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp171 = load <8 x float> addrspace(1)* %arrayidx159, align 32
  %tmp172 = shufflevector <8 x float> %tmp171, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call173 = call float @__dotf4(<4 x float> %tmp167, <4 x float> %tmp172) nounwind
  %add174 = fadd float %call162, %call173
  %tmp176 = insertelement <8 x float> %tmp148, float %add174, i32 5
  %add179 = add i32 %add151, %div
  %tmp183 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp184 = shufflevector <8 x float> %tmp183, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx187 = getelementptr <8 x float> addrspace(1)* %input, i32 %add179
  %tmp188 = load <8 x float> addrspace(1)* %arrayidx187, align 32
  %tmp189 = shufflevector <8 x float> %tmp188, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call190 = call float @__dotf4(<4 x float> %tmp184, <4 x float> %tmp189) nounwind
  %tmp194 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp195 = shufflevector <8 x float> %tmp194, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp199 = load <8 x float> addrspace(1)* %arrayidx187, align 32
  %tmp200 = shufflevector <8 x float> %tmp199, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call201 = call float @__dotf4(<4 x float> %tmp195, <4 x float> %tmp200) nounwind
  %add202 = fadd float %call190, %call201
  %tmp204 = insertelement <8 x float> %tmp176, float %add202, i32 6
  %add207 = add i32 %add179, %div
  %tmp211 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp212 = shufflevector <8 x float> %tmp211, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx215 = getelementptr <8 x float> addrspace(1)* %input, i32 %add207
  %tmp216 = load <8 x float> addrspace(1)* %arrayidx215, align 32
  %tmp217 = shufflevector <8 x float> %tmp216, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call218 = call float @__dotf4(<4 x float> %tmp212, <4 x float> %tmp217) nounwind
  %tmp222 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp223 = shufflevector <8 x float> %tmp222, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp227 = load <8 x float> addrspace(1)* %arrayidx215, align 32
  %tmp228 = shufflevector <8 x float> %tmp227, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call229 = call float @__dotf4(<4 x float> %tmp223, <4 x float> %tmp228) nounwind
  %add230 = fadd float %call218, %call229
  %tmp232 = insertelement <8 x float> %tmp204, float %add230, i32 7
  %arrayidx235 = getelementptr [8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 %call4
  store <8 x float> %tmp232, <8 x float> addrspace(3)* %arrayidx235, align 32
  call void @_Z7barrierm(i32 2) nounwind
  %tmp239 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 0), align 32
  %tmp240 = shufflevector <8 x float> %tmp239, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp244 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp245 = shufflevector <8 x float> %tmp244, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call246 = call float @__dotf4(<4 x float> %tmp240, <4 x float> %tmp245) nounwind
  %tmp247 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 0), align 32
  %tmp248 = shufflevector <8 x float> %tmp247, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp252 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp253 = shufflevector <8 x float> %tmp252, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call254 = call float @__dotf4(<4 x float> %tmp248, <4 x float> %tmp253) nounwind
  %add255 = fadd float %call246, %call254
  %tmp257 = insertelement <8 x float> undef, float %add255, i32 0
  %tmp258 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 1), align 32
  %tmp259 = shufflevector <8 x float> %tmp258, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp263 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp264 = shufflevector <8 x float> %tmp263, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call265 = call float @__dotf4(<4 x float> %tmp259, <4 x float> %tmp264) nounwind
  %tmp266 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 1), align 32
  %tmp267 = shufflevector <8 x float> %tmp266, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp271 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp272 = shufflevector <8 x float> %tmp271, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call273 = call float @__dotf4(<4 x float> %tmp267, <4 x float> %tmp272) nounwind
  %add274 = fadd float %call265, %call273
  %tmp276 = insertelement <8 x float> %tmp257, float %add274, i32 1
  %tmp277 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 2), align 32
  %tmp278 = shufflevector <8 x float> %tmp277, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp282 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp283 = shufflevector <8 x float> %tmp282, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call284 = call float @__dotf4(<4 x float> %tmp278, <4 x float> %tmp283) nounwind
  %tmp285 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 2), align 32
  %tmp286 = shufflevector <8 x float> %tmp285, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp290 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp291 = shufflevector <8 x float> %tmp290, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call292 = call float @__dotf4(<4 x float> %tmp286, <4 x float> %tmp291) nounwind
  %add293 = fadd float %call284, %call292
  %tmp295 = insertelement <8 x float> %tmp276, float %add293, i32 2
  %tmp296 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 3), align 32
  %tmp297 = shufflevector <8 x float> %tmp296, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp301 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp302 = shufflevector <8 x float> %tmp301, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call303 = call float @__dotf4(<4 x float> %tmp297, <4 x float> %tmp302) nounwind
  %tmp304 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 3), align 32
  %tmp305 = shufflevector <8 x float> %tmp304, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp309 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp310 = shufflevector <8 x float> %tmp309, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call311 = call float @__dotf4(<4 x float> %tmp305, <4 x float> %tmp310) nounwind
  %add312 = fadd float %call303, %call311
  %tmp314 = insertelement <8 x float> %tmp295, float %add312, i32 3
  %tmp315 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 4), align 32
  %tmp316 = shufflevector <8 x float> %tmp315, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp320 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp321 = shufflevector <8 x float> %tmp320, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call322 = call float @__dotf4(<4 x float> %tmp316, <4 x float> %tmp321) nounwind
  %tmp323 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 4), align 32
  %tmp324 = shufflevector <8 x float> %tmp323, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp328 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp329 = shufflevector <8 x float> %tmp328, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call330 = call float @__dotf4(<4 x float> %tmp324, <4 x float> %tmp329) nounwind
  %add331 = fadd float %call322, %call330
  %tmp333 = insertelement <8 x float> %tmp314, float %add331, i32 4
  %tmp334 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 5), align 32
  %tmp335 = shufflevector <8 x float> %tmp334, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp339 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp340 = shufflevector <8 x float> %tmp339, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call341 = call float @__dotf4(<4 x float> %tmp335, <4 x float> %tmp340) nounwind
  %tmp342 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 5), align 32
  %tmp343 = shufflevector <8 x float> %tmp342, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp347 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp348 = shufflevector <8 x float> %tmp347, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call349 = call float @__dotf4(<4 x float> %tmp343, <4 x float> %tmp348) nounwind
  %add350 = fadd float %call341, %call349
  %tmp352 = insertelement <8 x float> %tmp333, float %add350, i32 5
  %tmp353 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 6), align 32
  %tmp354 = shufflevector <8 x float> %tmp353, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp358 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp359 = shufflevector <8 x float> %tmp358, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call360 = call float @__dotf4(<4 x float> %tmp354, <4 x float> %tmp359) nounwind
  %tmp361 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 6), align 32
  %tmp362 = shufflevector <8 x float> %tmp361, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp366 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp367 = shufflevector <8 x float> %tmp366, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call368 = call float @__dotf4(<4 x float> %tmp362, <4 x float> %tmp367) nounwind
  %add369 = fadd float %call360, %call368
  %tmp371 = insertelement <8 x float> %tmp352, float %add369, i32 6
  %tmp372 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 7), align 32
  %tmp373 = shufflevector <8 x float> %tmp372, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %tmp377 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp378 = shufflevector <8 x float> %tmp377, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %call379 = call float @__dotf4(<4 x float> %tmp373, <4 x float> %tmp378) nounwind
  %tmp380 = load <8 x float> addrspace(3)* getelementptr inbounds ([8 x <8 x float>] addrspace(3)* @DCT_VECTOR_DOT_cllocal_inter, i32 0, i32 7), align 32
  %tmp381 = shufflevector <8 x float> %tmp380, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %tmp385 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %tmp386 = shufflevector <8 x float> %tmp385, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %call387 = call float @__dotf4(<4 x float> %tmp381, <4 x float> %tmp386) nounwind
  %add388 = fadd float %call379, %call387
  %tmp390 = insertelement <8 x float> %tmp371, float %add388, i32 7
  %add394 = add i32 %mul, %call4
  %mul396 = mul i32 %add394, %div
  %add398 = add i32 %mul396, %call2
  %arrayidx401 = getelementptr <8 x float> addrspace(1)* %output, i32 %add398
  store <8 x float> %tmp390, <8 x float> addrspace(1)* %arrayidx401, align 32
  ret void
}

declare float @__dotf4(<4 x float>, <4 x float>)

define void @DCT_CPU(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
  %inter = alloca [64 x float], align 4
  %call = call i32 @get_group_id(i32 0) nounwind
  %call1 = call i32 @get_group_id(i32 1) nounwind
  %mul = shl i32 %width, 3
  %mul71 = mul i32 %mul, %call1
  %mul73 = shl i32 %call, 3
  %add = add i32 %mul71, %mul73
  %0 = bitcast [64 x float]* %inter to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 256, i32 4, i1 false)
  br label %for.cond75.preheader

for.cond75.preheader:                             ; preds = %entry, %for.end109
  %storemerge324 = phi i32 [ 0, %entry ], [ %inc115, %for.end109 ]
  %add112623 = phi i32 [ %add, %entry ], [ %add112, %for.end109 ]
  %arrayidx97.phi.trans.insert = getelementptr float addrspace(1)* %input, i32 %add112623
  %tmp98.pre = load float addrspace(1)* %arrayidx97.phi.trans.insert, align 4
  br label %for.cond79.preheader

for.cond79.preheader:                             ; preds = %for.cond75.preheader, %for.cond79.preheader
  %storemerge422 = phi i32 [ 0, %for.cond75.preheader ], [ %inc108, %for.cond79.preheader ]
  %add172821 = phi i32 [ 0, %for.cond75.preheader ], [ %add103, %for.cond79.preheader ]
  %add85 = add i32 %add172821, %storemerge324
  %arrayidx = getelementptr [64 x float]* %inter, i32 0, i32 %add85
  %arrayidx.promoted = load float* %arrayidx, align 4
  %arrayidx91 = getelementptr float addrspace(1)* %dct, i32 %add172821
  %tmp92 = load float addrspace(1)* %arrayidx91, align 4
  %mul99 = fmul float %tmp92, %tmp98.pre
  %add100 = fadd float %arrayidx.promoted, %mul99
  %add89.127 = or i32 %add172821, 1
  %arrayidx91.1 = getelementptr float addrspace(1)* %dct, i32 %add89.127
  %tmp92.1 = load float addrspace(1)* %arrayidx91.1, align 4
  %add95.1 = add i32 %add112623, 1
  %arrayidx97.1 = getelementptr float addrspace(1)* %input, i32 %add95.1
  %tmp98.1 = load float addrspace(1)* %arrayidx97.1, align 4
  %mul99.1 = fmul float %tmp92.1, %tmp98.1
  %add100.1 = fadd float %add100, %mul99.1
  %add89.228 = or i32 %add172821, 2
  %arrayidx91.2 = getelementptr float addrspace(1)* %dct, i32 %add89.228
  %tmp92.2 = load float addrspace(1)* %arrayidx91.2, align 4
  %add95.2 = add i32 %add112623, 2
  %arrayidx97.2 = getelementptr float addrspace(1)* %input, i32 %add95.2
  %tmp98.2 = load float addrspace(1)* %arrayidx97.2, align 4
  %mul99.2 = fmul float %tmp92.2, %tmp98.2
  %add100.2 = fadd float %add100.1, %mul99.2
  %add89.329 = or i32 %add172821, 3
  %arrayidx91.3 = getelementptr float addrspace(1)* %dct, i32 %add89.329
  %tmp92.3 = load float addrspace(1)* %arrayidx91.3, align 4
  %add95.3 = add i32 %add112623, 3
  %arrayidx97.3 = getelementptr float addrspace(1)* %input, i32 %add95.3
  %tmp98.3 = load float addrspace(1)* %arrayidx97.3, align 4
  %mul99.3 = fmul float %tmp92.3, %tmp98.3
  %add100.3 = fadd float %add100.2, %mul99.3
  %add89.430 = or i32 %add172821, 4
  %arrayidx91.4 = getelementptr float addrspace(1)* %dct, i32 %add89.430
  %tmp92.4 = load float addrspace(1)* %arrayidx91.4, align 4
  %add95.4 = add i32 %add112623, 4
  %arrayidx97.4 = getelementptr float addrspace(1)* %input, i32 %add95.4
  %tmp98.4 = load float addrspace(1)* %arrayidx97.4, align 4
  %mul99.4 = fmul float %tmp92.4, %tmp98.4
  %add100.4 = fadd float %add100.3, %mul99.4
  %add89.531 = or i32 %add172821, 5
  %arrayidx91.5 = getelementptr float addrspace(1)* %dct, i32 %add89.531
  %tmp92.5 = load float addrspace(1)* %arrayidx91.5, align 4
  %add95.5 = add i32 %add112623, 5
  %arrayidx97.5 = getelementptr float addrspace(1)* %input, i32 %add95.5
  %tmp98.5 = load float addrspace(1)* %arrayidx97.5, align 4
  %mul99.5 = fmul float %tmp92.5, %tmp98.5
  %add100.5 = fadd float %add100.4, %mul99.5
  %add89.632 = or i32 %add172821, 6
  %arrayidx91.6 = getelementptr float addrspace(1)* %dct, i32 %add89.632
  %tmp92.6 = load float addrspace(1)* %arrayidx91.6, align 4
  %add95.6 = add i32 %add112623, 6
  %arrayidx97.6 = getelementptr float addrspace(1)* %input, i32 %add95.6
  %tmp98.6 = load float addrspace(1)* %arrayidx97.6, align 4
  %mul99.6 = fmul float %tmp92.6, %tmp98.6
  %add100.6 = fadd float %add100.5, %mul99.6
  %add89.733 = or i32 %add172821, 7
  %arrayidx91.7 = getelementptr float addrspace(1)* %dct, i32 %add89.733
  %tmp92.7 = load float addrspace(1)* %arrayidx91.7, align 4
  %add95.7 = add i32 %add112623, 7
  %arrayidx97.7 = getelementptr float addrspace(1)* %input, i32 %add95.7
  %tmp98.7 = load float addrspace(1)* %arrayidx97.7, align 4
  %mul99.7 = fmul float %tmp92.7, %tmp98.7
  %add100.7 = fadd float %add100.6, %mul99.7
  store float %add100.7, float* %arrayidx, align 4
  %add103 = add i32 %add172821, 8
  %inc108 = add i32 %storemerge422, 1
  %cmp77 = icmp ult i32 %inc108, 8
  br i1 %cmp77, label %for.cond79.preheader, label %for.end109

for.end109:                                       ; preds = %for.cond79.preheader
  %add112 = add i32 %add112623, %width
  %inc115 = add i32 %storemerge324, 1
  %cmp = icmp ult i32 %inc115, 8
  br i1 %cmp, label %for.cond75.preheader, label %for.cond128.preheader.preheader

for.cond128.preheader.preheader:                  ; preds = %for.end109
  br label %for.cond128.preheader

for.cond128.preheader:                            ; preds = %for.cond128.preheader.preheader, %for.end170
  %storemerge17 = phi i32 [ %inc178, %for.end170 ], [ 0, %for.cond128.preheader.preheader ]
  %add172716 = phi i32 [ %add172, %for.end170 ], [ 0, %for.cond128.preheader.preheader ]
  %add1751115 = phi i32 [ %add175, %for.end170 ], [ %add, %for.cond128.preheader.preheader ]
  br label %for.body131

for.body131:                                      ; preds = %for.cond128.preheader, %for.body131
  %storemerge114 = phi i32 [ 0, %for.cond128.preheader ], [ %inc169, %for.body131 ]
  %add166913 = phi i32 [ 0, %for.cond128.preheader ], [ %add166, %for.body131 ]
  %add134 = add i32 %storemerge114, %add1751115
  %arrayidx136 = getelementptr float addrspace(1)* %output, i32 %add134
  store float 0.000000e+00, float addrspace(1)* %arrayidx136, align 4
  %arrayidx151 = getelementptr float addrspace(1)* %dct, i32 %add172716
  %tmp152 = load float addrspace(1)* %arrayidx151, align 4
  %arrayidx157 = getelementptr [64 x float]* %inter, i32 0, i32 %add166913
  %tmp158 = load float* %arrayidx157, align 4
  %mul159 = fmul float %tmp152, %tmp158
  %add160 = fadd float %mul159, 0.000000e+00
  store float %add160, float addrspace(1)* %arrayidx136, align 4
  %add149.134 = or i32 %add172716, 1
  %arrayidx151.1 = getelementptr float addrspace(1)* %dct, i32 %add149.134
  %tmp152.1 = load float addrspace(1)* %arrayidx151.1, align 4
  %add155.135 = or i32 %add166913, 1
  %arrayidx157.1 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.135
  %tmp158.1 = load float* %arrayidx157.1, align 4
  %mul159.1 = fmul float %tmp152.1, %tmp158.1
  %add160.1 = fadd float %add160, %mul159.1
  store float %add160.1, float addrspace(1)* %arrayidx136, align 4
  %add149.236 = or i32 %add172716, 2
  %arrayidx151.2 = getelementptr float addrspace(1)* %dct, i32 %add149.236
  %tmp152.2 = load float addrspace(1)* %arrayidx151.2, align 4
  %add155.237 = or i32 %add166913, 2
  %arrayidx157.2 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.237
  %tmp158.2 = load float* %arrayidx157.2, align 4
  %mul159.2 = fmul float %tmp152.2, %tmp158.2
  %add160.2 = fadd float %add160.1, %mul159.2
  store float %add160.2, float addrspace(1)* %arrayidx136, align 4
  %add149.338 = or i32 %add172716, 3
  %arrayidx151.3 = getelementptr float addrspace(1)* %dct, i32 %add149.338
  %tmp152.3 = load float addrspace(1)* %arrayidx151.3, align 4
  %add155.339 = or i32 %add166913, 3
  %arrayidx157.3 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.339
  %tmp158.3 = load float* %arrayidx157.3, align 4
  %mul159.3 = fmul float %tmp152.3, %tmp158.3
  %add160.3 = fadd float %add160.2, %mul159.3
  store float %add160.3, float addrspace(1)* %arrayidx136, align 4
  %add149.440 = or i32 %add172716, 4
  %arrayidx151.4 = getelementptr float addrspace(1)* %dct, i32 %add149.440
  %tmp152.4 = load float addrspace(1)* %arrayidx151.4, align 4
  %add155.441 = or i32 %add166913, 4
  %arrayidx157.4 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.441
  %tmp158.4 = load float* %arrayidx157.4, align 4
  %mul159.4 = fmul float %tmp152.4, %tmp158.4
  %add160.4 = fadd float %add160.3, %mul159.4
  store float %add160.4, float addrspace(1)* %arrayidx136, align 4
  %add149.542 = or i32 %add172716, 5
  %arrayidx151.5 = getelementptr float addrspace(1)* %dct, i32 %add149.542
  %tmp152.5 = load float addrspace(1)* %arrayidx151.5, align 4
  %add155.543 = or i32 %add166913, 5
  %arrayidx157.5 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.543
  %tmp158.5 = load float* %arrayidx157.5, align 4
  %mul159.5 = fmul float %tmp152.5, %tmp158.5
  %add160.5 = fadd float %add160.4, %mul159.5
  store float %add160.5, float addrspace(1)* %arrayidx136, align 4
  %add149.644 = or i32 %add172716, 6
  %arrayidx151.6 = getelementptr float addrspace(1)* %dct, i32 %add149.644
  %tmp152.6 = load float addrspace(1)* %arrayidx151.6, align 4
  %add155.645 = or i32 %add166913, 6
  %arrayidx157.6 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.645
  %tmp158.6 = load float* %arrayidx157.6, align 4
  %mul159.6 = fmul float %tmp152.6, %tmp158.6
  %add160.6 = fadd float %add160.5, %mul159.6
  store float %add160.6, float addrspace(1)* %arrayidx136, align 4
  %add149.746 = or i32 %add172716, 7
  %arrayidx151.7 = getelementptr float addrspace(1)* %dct, i32 %add149.746
  %tmp152.7 = load float addrspace(1)* %arrayidx151.7, align 4
  %add155.747 = or i32 %add166913, 7
  %arrayidx157.7 = getelementptr [64 x float]* %inter, i32 0, i32 %add155.747
  %tmp158.7 = load float* %arrayidx157.7, align 4
  %mul159.7 = fmul float %tmp152.7, %tmp158.7
  %add160.7 = fadd float %add160.6, %mul159.7
  store float %add160.7, float addrspace(1)* %arrayidx136, align 4
  %add166 = add i32 %add166913, 8
  %inc169 = add i32 %storemerge114, 1
  %cmp130 = icmp ult i32 %inc169, 8
  br i1 %cmp130, label %for.body131, label %for.end170

for.end170:                                       ; preds = %for.body131
  %add172 = add i32 %add172716, 8
  %add175 = add i32 %add1751115, %width
  %inc178 = add i32 %storemerge17, 1
  %cmp126 = icmp ult i32 %inc178, 8
  br i1 %cmp126, label %for.cond128.preheader, label %for.end179

for.end179:                                       ; preds = %for.end170
  ret void
}

define void @DCT_CPU_VECTOR(float addrspace(1)* %output, <8 x float> addrspace(1)* %input, <8 x float> addrspace(1)* %dct, i32 %width, ...) nounwind {
entry:
  %acc = alloca [64 x float], align 4
  %inter = alloca [8 x <8 x float>], align 32
  %call = call i32 @get_group_id(i32 0) nounwind
  %call1 = call i32 @get_group_id(i32 1) nounwind
  %div = lshr i32 %width, 3
  %mul = shl i32 %call1, 3
  %mul8 = mul i32 %mul, %width
  %div9 = lshr exact i32 %mul8, 3
  %add = add i32 %div9, %call
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %entry, %for.end
  %storemerge315 = phi i32 [ 0, %entry ], [ %inc59, %for.end ]
  %add56514 = phi i32 [ %add, %entry ], [ %add56, %for.end ]
  %arrayidx21 = getelementptr <8 x float> addrspace(1)* %input, i32 %add56514
  %tmp22 = load <8 x float> addrspace(1)* %arrayidx21, align 32
  br label %for.body15

for.cond62.preheader:                             ; preds = %for.end
  %arraydecay70 = getelementptr [64 x float]* %acc, i32 0, i32 0
  %arrayidx68 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 0
  %call72 = call <8 x float> @__vload8_f8(i32 0, float* %arraydecay70) nounwind
  store <8 x float> %call72, <8 x float>* %arrayidx68, align 32
  %arrayidx68.1 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 1
  %call72.1 = call <8 x float> @__vload8_f8(i32 1, float* %arraydecay70) nounwind
  store <8 x float> %call72.1, <8 x float>* %arrayidx68.1, align 32
  %arrayidx68.2 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 2
  %call72.2 = call <8 x float> @__vload8_f8(i32 2, float* %arraydecay70) nounwind
  store <8 x float> %call72.2, <8 x float>* %arrayidx68.2, align 32
  %arrayidx68.3 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 3
  %call72.3 = call <8 x float> @__vload8_f8(i32 3, float* %arraydecay70) nounwind
  store <8 x float> %call72.3, <8 x float>* %arrayidx68.3, align 32
  %arrayidx68.4 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 4
  %call72.4 = call <8 x float> @__vload8_f8(i32 4, float* %arraydecay70) nounwind
  store <8 x float> %call72.4, <8 x float>* %arrayidx68.4, align 32
  %arrayidx68.5 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 5
  %call72.5 = call <8 x float> @__vload8_f8(i32 5, float* %arraydecay70) nounwind
  store <8 x float> %call72.5, <8 x float>* %arrayidx68.5, align 32
  %arrayidx68.6 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 6
  %call72.6 = call <8 x float> @__vload8_f8(i32 6, float* %arraydecay70) nounwind
  store <8 x float> %call72.6, <8 x float>* %arrayidx68.6, align 32
  %arrayidx68.7 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 7
  %call72.7 = call <8 x float> @__vload8_f8(i32 7, float* %arraydecay70) nounwind
  store <8 x float> %call72.7, <8 x float>* %arrayidx68.7, align 32
  %mul82 = shl i32 %call, 3
  %add83 = add i32 %mul8, %mul82
  br label %for.cond88.preheader

for.body15:                                       ; preds = %for.cond12.preheader, %for.body15
  %storemerge413 = phi i32 [ 0, %for.cond12.preheader ], [ %inc, %for.body15 ]
  %add52612 = phi i32 [ 0, %for.cond12.preheader ], [ %add52, %for.body15 ]
  %arrayidx = getelementptr <8 x float> addrspace(1)* %dct, i32 %storemerge413
  %tmp18 = load <8 x float> addrspace(1)* %arrayidx, align 32
  %mul23 = fmul <8 x float> %tmp18, %tmp22
  %add26 = add i32 %add52612, %storemerge315
  %arrayidx27 = getelementptr [64 x float]* %acc, i32 0, i32 %add26
  %tmp29 = extractelement <8 x float> %mul23, i32 0
  %tmp31 = extractelement <8 x float> %mul23, i32 1
  %add32 = fadd float %tmp29, %tmp31
  %tmp34 = extractelement <8 x float> %mul23, i32 2
  %add35 = fadd float %add32, %tmp34
  %tmp37 = extractelement <8 x float> %mul23, i32 3
  %add38 = fadd float %add35, %tmp37
  %tmp40 = extractelement <8 x float> %mul23, i32 4
  %add41 = fadd float %add38, %tmp40
  %tmp43 = extractelement <8 x float> %mul23, i32 5
  %add44 = fadd float %add41, %tmp43
  %tmp46 = extractelement <8 x float> %mul23, i32 6
  %add47 = fadd float %add44, %tmp46
  %tmp49 = extractelement <8 x float> %mul23, i32 7
  %add50 = fadd float %add47, %tmp49
  store float %add50, float* %arrayidx27, align 4
  %add52 = add i32 %add52612, 8
  %inc = add i32 %storemerge413, 1
  %cmp14 = icmp ult i32 %inc, 8
  br i1 %cmp14, label %for.body15, label %for.end

for.end:                                          ; preds = %for.body15
  %add56 = add i32 %add56514, %div
  %inc59 = add i32 %storemerge315, 1
  %cmp = icmp ult i32 %inc59, 8
  br i1 %cmp, label %for.cond12.preheader, label %for.cond62.preheader

for.cond88.preheader:                             ; preds = %for.cond62.preheader, %for.end132
  %storemerge10 = phi i32 [ 0, %for.cond62.preheader ], [ %inc138, %for.end132 ]
  %add13579 = phi i32 [ %add83, %for.cond62.preheader ], [ %add135, %for.end132 ]
  %arrayidx94 = getelementptr <8 x float> addrspace(1)* %dct, i32 %storemerge10
  br label %for.body91

for.body91:                                       ; preds = %for.body91, %for.cond88.preheader
  %storemerge18 = phi i32 [ 0, %for.cond88.preheader ], [ %inc131, %for.body91 ]
  %tmp95 = load <8 x float> addrspace(1)* %arrayidx94, align 32
  %arrayidx98 = getelementptr [8 x <8 x float>]* %inter, i32 0, i32 %storemerge18
  %tmp99 = load <8 x float>* %arrayidx98, align 32
  %mul100 = fmul <8 x float> %tmp95, %tmp99
  %add103 = add i32 %storemerge18, %add13579
  %arrayidx105 = getelementptr float addrspace(1)* %output, i32 %add103
  %tmp107 = extractelement <8 x float> %mul100, i32 0
  %tmp109 = extractelement <8 x float> %mul100, i32 1
  %add110 = fadd float %tmp107, %tmp109
  %tmp112 = extractelement <8 x float> %mul100, i32 2
  %add113 = fadd float %add110, %tmp112
  %tmp115 = extractelement <8 x float> %mul100, i32 3
  %add116 = fadd float %add113, %tmp115
  %tmp118 = extractelement <8 x float> %mul100, i32 4
  %add119 = fadd float %add116, %tmp118
  %tmp121 = extractelement <8 x float> %mul100, i32 5
  %add122 = fadd float %add119, %tmp121
  %tmp124 = extractelement <8 x float> %mul100, i32 6
  %add125 = fadd float %add122, %tmp124
  %tmp127 = extractelement <8 x float> %mul100, i32 7
  %add128 = fadd float %add125, %tmp127
  store float %add128, float addrspace(1)* %arrayidx105, align 4
  %inc131 = add i32 %storemerge18, 1
  %cmp90 = icmp ult i32 %inc131, 8
  br i1 %cmp90, label %for.body91, label %for.end132

for.end132:                                       ; preds = %for.body91
  %add135 = add i32 %add13579, %width
  %inc138 = add i32 %storemerge10, 1
  %cmp86 = icmp ult i32 %inc138, 8
  br i1 %cmp86, label %for.cond88.preheader, label %for.end139

for.end139:                                       ; preds = %for.end132
  ret void
}

declare <8 x float> @__vload8_f8(i32, float*)

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind
