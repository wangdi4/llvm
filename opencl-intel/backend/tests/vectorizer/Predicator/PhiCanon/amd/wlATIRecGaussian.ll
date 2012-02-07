; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIRecGaussian.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [7 x i8] c"229000\00"		; <[7 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [13 x i8] c"120000000000\00"		; <[13 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @RecursiveGaussian_kernel
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb:                                     ; preds = %for.end, %return.loopexit
; CHECK: ret

define void @transpose_kernel(<4 x i8> addrspace(1)* %output, <4 x i8> addrspace(1)* %input, <4 x i8> addrspace(3)* %block, i32 %width, i32 %height, i32 %blockSize, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 1) nounwind
  %call2 = call i32 @get_local_id(i32 0) nounwind
  %call3 = call i32 @get_local_id(i32 1) nounwind
  %mul = mul i32 %call3, %blockSize
  %add = add i32 %mul, %call2
  %arrayidx = getelementptr <4 x i8> addrspace(3)* %block, i32 %add
  %mul9 = mul i32 %call1, %width
  %add11 = add i32 %mul9, %call
  %arrayidx13 = getelementptr <4 x i8> addrspace(1)* %input, i32 %add11
  %tmp14 = load <4 x i8> addrspace(1)* %arrayidx13, align 4
  store <4 x i8> %tmp14, <4 x i8> addrspace(3)* %arrayidx, align 4
  call void @barrier(i32 1) nounwind
  %mul25 = mul i32 %call, %height
  %add26 = add i32 %call1, %mul25
  %arrayidx29 = getelementptr <4 x i8> addrspace(1)* %output, i32 %add26
  %tmp33 = load <4 x i8> addrspace(3)* %arrayidx, align 4
  store <4 x i8> %tmp33, <4 x i8> addrspace(1)* %arrayidx29, align 4
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)

declare void @barrier(i32)

define void @RecursiveGaussian_kernel(<4 x i8> addrspace(1)* %input, <4 x i8> addrspace(1)* %output, i32 %width, i32 %height, float %a0, float %a1, float %a2, float %a3, float %b1, float %b2, float %coefp, float %coefn, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %cmp = icmp ult i32 %call, %width
  br i1 %cmp, label %for.cond.preheader, label %return

for.cond.preheader:                               ; preds = %entry
  %cmp816 = icmp sgt i32 %height, 0
  br i1 %cmp816, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %tmp39 = insertelement <4 x float> undef, float %a0, i32 0
  %splat = shufflevector <4 x float> %tmp39, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp43 = insertelement <4 x float> undef, float %a1, i32 0
  %splat44 = shufflevector <4 x float> %tmp43, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp49 = insertelement <4 x float> undef, float %b1, i32 0
  %splat50 = shufflevector <4 x float> %tmp49, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp54 = insertelement <4 x float> undef, float %b2, i32 0
  %splat55 = shufflevector <4 x float> %tmp54, <4 x float> undef, <4 x i32> zeroinitializer
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %storemerge120 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %tmp76219 = phi <4 x float> [ zeroinitializer, %for.body.lr.ph ], [ %3, %for.body ]
  %tmp78318 = phi <4 x float> [ zeroinitializer, %for.body.lr.ph ], [ %sub58, %for.body ]
  %tmp783417 = phi <4 x float> [ zeroinitializer, %for.body.lr.ph ], [ %tmp78318, %for.body ]
  %mul = mul i32 %storemerge120, %width
  %add = add i32 %mul, %call
  %arrayidx = getelementptr <4 x i8> addrspace(1)* %input, i32 %add
  %tmp16 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %tmp17 = extractelement <4 x i8> %tmp16, i32 0
  %conv = uitofp i8 %tmp17 to float
  %0 = insertelement <4 x float> undef, float %conv, i32 0
  %tmp22 = extractelement <4 x i8> %tmp16, i32 1
  %conv23 = uitofp i8 %tmp22 to float
  %1 = insertelement <4 x float> %0, float %conv23, i32 1
  %tmp28 = extractelement <4 x i8> %tmp16, i32 2
  %conv29 = uitofp i8 %tmp28 to float
  %2 = insertelement <4 x float> %1, float %conv29, i32 2
  %tmp34 = extractelement <4 x i8> %tmp16, i32 3
  %conv35 = uitofp i8 %tmp34 to float
  %3 = insertelement <4 x float> %2, float %conv35, i32 3
  %mul41 = fmul <4 x float> %splat, %3
  %mul46 = fmul <4 x float> %splat44, %tmp76219
  %add47 = fadd <4 x float> %mul41, %mul46
  %mul52 = fmul <4 x float> %splat50, %tmp78318
  %sub = fsub <4 x float> %add47, %mul52
  %mul57 = fmul <4 x float> %splat55, %tmp783417
  %sub58 = fsub <4 x float> %sub, %mul57
  %arrayidx61 = getelementptr <4 x i8> addrspace(1)* %output, i32 %add
  %tmp64 = extractelement <4 x float> %sub58, i32 0
  %conv65 = fptoui float %tmp64 to i8
  %4 = insertelement <4 x i8> undef, i8 %conv65, i32 0
  %tmp67 = extractelement <4 x float> %sub58, i32 1
  %conv68 = fptoui float %tmp67 to i8
  %5 = insertelement <4 x i8> %4, i8 %conv68, i32 1
  %tmp70 = extractelement <4 x float> %sub58, i32 2
  %conv71 = fptoui float %tmp70 to i8
  %6 = insertelement <4 x i8> %5, i8 %conv71, i32 2
  %tmp73 = extractelement <4 x float> %sub58, i32 3
  %conv74 = fptoui float %tmp73 to i8
  %7 = insertelement <4 x i8> %6, i8 %conv74, i32 3
  store <4 x i8> %7, <4 x i8> addrspace(1)* %arrayidx61, align 4
  %inc = add i32 %storemerge120, 1
  %cmp8 = icmp slt i32 %inc, %height
  br i1 %cmp8, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.cond.preheader
  call void @barrier(i32 2) nounwind
  %storemerge9 = add i32 %height, -1
  %cmp9010 = icmp sgt i32 %storemerge9, -1
  br i1 %cmp9010, label %for.body92.lr.ph, label %return

for.body92.lr.ph:                                 ; preds = %for.end
  %tmp131 = insertelement <4 x float> undef, float %a2, i32 0
  %splat132 = shufflevector <4 x float> %tmp131, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp136 = insertelement <4 x float> undef, float %a3, i32 0
  %splat137 = shufflevector <4 x float> %tmp136, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp142 = insertelement <4 x float> undef, float %b1, i32 0
  %splat143 = shufflevector <4 x float> %tmp142, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp148 = insertelement <4 x float> undef, float %b2, i32 0
  %splat149 = shufflevector <4 x float> %tmp148, <4 x float> undef, <4 x i32> zeroinitializer
  br label %for.body92

for.body92:                                       ; preds = %for.body92.lr.ph, %for.body92
  %storemerge15 = phi i32 [ %storemerge9, %for.body92.lr.ph ], [ %storemerge, %for.body92 ]
  %tmp154514 = phi <4 x float> [ zeroinitializer, %for.body92.lr.ph ], [ %11, %for.body92 ]
  %tmp1545613 = phi <4 x float> [ zeroinitializer, %for.body92.lr.ph ], [ %tmp154514, %for.body92 ]
  %tmp156712 = phi <4 x float> [ zeroinitializer, %for.body92.lr.ph ], [ %sub152, %for.body92 ]
  %tmp1567811 = phi <4 x float> [ zeroinitializer, %for.body92.lr.ph ], [ %tmp156712, %for.body92 ]
  %mul98 = mul i32 %storemerge15, %width
  %add99 = add i32 %mul98, %call
  %arrayidx105 = getelementptr <4 x i8> addrspace(1)* %input, i32 %add99
  %tmp106 = load <4 x i8> addrspace(1)* %arrayidx105, align 4
  %tmp107 = extractelement <4 x i8> %tmp106, i32 0
  %conv108 = uitofp i8 %tmp107 to float
  %8 = insertelement <4 x float> undef, float %conv108, i32 0
  %tmp113 = extractelement <4 x i8> %tmp106, i32 1
  %conv114 = uitofp i8 %tmp113 to float
  %9 = insertelement <4 x float> %8, float %conv114, i32 1
  %tmp119 = extractelement <4 x i8> %tmp106, i32 2
  %conv120 = uitofp i8 %tmp119 to float
  %10 = insertelement <4 x float> %9, float %conv120, i32 2
  %tmp125 = extractelement <4 x i8> %tmp106, i32 3
  %conv126 = uitofp i8 %tmp125 to float
  %11 = insertelement <4 x float> %10, float %conv126, i32 3
  %mul134 = fmul <4 x float> %splat132, %tmp154514
  %mul139 = fmul <4 x float> %splat137, %tmp1545613
  %add140 = fadd <4 x float> %mul134, %mul139
  %mul145 = fmul <4 x float> %splat143, %tmp156712
  %sub146 = fsub <4 x float> %add140, %mul145
  %mul151 = fmul <4 x float> %splat149, %tmp1567811
  %sub152 = fsub <4 x float> %sub146, %mul151
  %arrayidx161 = getelementptr <4 x i8> addrspace(1)* %output, i32 %add99
  %tmp162 = load <4 x i8> addrspace(1)* %arrayidx161, align 4
  %tmp163 = extractelement <4 x i8> %tmp162, i32 0
  %conv164 = uitofp i8 %tmp163 to float
  %12 = insertelement <4 x float> undef, float %conv164, i32 0
  %tmp169 = extractelement <4 x i8> %tmp162, i32 1
  %conv170 = uitofp i8 %tmp169 to float
  %13 = insertelement <4 x float> %12, float %conv170, i32 1
  %tmp175 = extractelement <4 x i8> %tmp162, i32 2
  %conv176 = uitofp i8 %tmp175 to float
  %14 = insertelement <4 x float> %13, float %conv176, i32 2
  %tmp181 = extractelement <4 x i8> %tmp162, i32 3
  %conv182 = uitofp i8 %tmp181 to float
  %15 = insertelement <4 x float> %14, float %conv182, i32 3
  %add185 = fadd <4 x float> %15, %sub152
  %tmp191 = extractelement <4 x float> %add185, i32 0
  %conv192 = fptoui float %tmp191 to i8
  %16 = insertelement <4 x i8> undef, i8 %conv192, i32 0
  %tmp194 = extractelement <4 x float> %add185, i32 1
  %conv195 = fptoui float %tmp194 to i8
  %17 = insertelement <4 x i8> %16, i8 %conv195, i32 1
  %tmp197 = extractelement <4 x float> %add185, i32 2
  %conv198 = fptoui float %tmp197 to i8
  %18 = insertelement <4 x i8> %17, i8 %conv198, i32 2
  %tmp200 = extractelement <4 x float> %add185, i32 3
  %conv201 = fptoui float %tmp200 to i8
  %19 = insertelement <4 x i8> %18, i8 %conv201, i32 3
  store <4 x i8> %19, <4 x i8> addrspace(1)* %arrayidx161, align 4
  %storemerge = add i32 %storemerge15, -1
  %cmp90 = icmp sgt i32 %storemerge, -1
  br i1 %cmp90, label %for.body92, label %return.loopexit

return.loopexit:                                  ; preds = %for.body92
  br label %return

return:                                           ; preds = %return.loopexit, %for.end, %entry
  ret void
}
