;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

; ModuleID = 'opt.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.cb_params_struct = type { double, double, double, double }

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare <16 x double> @__ocl_svml_b2_exp16(<16 x double>) nounwind readnone

declare <16 x double> @__ocl_svml_b2_fabs16(<16 x double>) nounwind readnone

declare double @__ocl_svml_b2_log1(double) nounwind readnone

declare double @__ocl_svml_b2_fabs1(double) nounwind readnone

declare <8 x double> @llvm.x86.mic.mask.max.pd(<8 x double>, i8, <8 x double>, <8 x double>) nounwind readnone

declare double @__ocl_svml_b2_exp1(double) nounwind readnone

define void @evalConvertibleBond(i8* nocapture %pBuffer) nounwind {
entry:
; KNC: evalConvertibleBond:
  %tmp_buff.i = alloca [805 x double], align 16
  %tmp_rslt.i = alloca [301 x double], align 16
  %a56 = bitcast [301 x double]* %tmp_rslt.i to i8*
  %0 = alloca [805 x double], align 16
  %a1 = alloca [805 x double], align 16
  %a2 = alloca [805 x double], align 16
  %a3 = alloca [805 x double], align 16
  %a4 = alloca [805 x double], align 16
  %a5 = alloca [805 x double], align 16
  %a6 = alloca [805 x double], align 16
  %a7 = alloca [805 x double], align 16
  %a8 = alloca [805 x double], align 16
  %a9 = alloca [805 x double], align 16
  %a10 = alloca [805 x double], align 16
  %a11 = alloca [805 x double], align 16
  %a12 = alloca [805 x double], align 16
  %a13 = alloca [805 x double], align 16
  %a14 = alloca [805 x double], align 16
  %a15 = alloca [805 x double], align 16
  %a16 = alloca [301 x double], align 16
  %a17 = alloca [301 x double], align 16
  %a18 = alloca [301 x double], align 16
  %a19 = alloca [301 x double], align 16
  %a20 = alloca [301 x double], align 16
  %a21 = alloca [301 x double], align 16
  %a22 = alloca [301 x double], align 16
  %a23 = alloca [301 x double], align 16
  %a24 = alloca [301 x double], align 16
  %a25 = alloca [301 x double], align 16
  %a26 = alloca [301 x double], align 16
  %a27 = alloca [301 x double], align 16
  %a28 = alloca [301 x double], align 16
  %a29 = alloca [301 x double], align 16
  %a30 = alloca [301 x double], align 16
  %a31 = alloca [301 x double], align 16
  %a32 = bitcast i8* %pBuffer to %struct.cb_params_struct addrspace(1)**
  %a33 = load %struct.cb_params_struct addrspace(1)** %a32, align 8
  %a34 = getelementptr i8* %pBuffer, i64 8
  %a35 = bitcast i8* %a34 to double addrspace(1)**
  %a36 = load double addrspace(1)** %a35, align 8
  %a37 = getelementptr i8* %pBuffer, i64 16
  %a38 = bitcast i8* %a37 to double addrspace(1)**
  %a39 = load double addrspace(1)** %a38, align 8
  %a40 = getelementptr i8* %pBuffer, i64 24
  %a41 = bitcast i8* %a40 to double addrspace(1)**
  %a42 = load double addrspace(1)** %a41, align 8
  %a43 = getelementptr i8* %pBuffer, i64 32
  %a44 = bitcast i8* %a43 to double addrspace(1)**
  %a45 = load double addrspace(1)** %a44, align 8
  %a46 = getelementptr i8* %pBuffer, i64 40
  %a47 = bitcast i8* %a46 to double addrspace(1)**
  %a48 = load double addrspace(1)** %a47, align 8
  %a49 = getelementptr i8* %pBuffer, i64 56
  %a50 = bitcast i8* %a49 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %a51 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %a50, align 8
  %a52 = getelementptr i8* %pBuffer, i64 72
  %a53 = bitcast i8* %a52 to <{ [4 x i64] }>**
  %a54 = load <{ [4 x i64] }>** %a53, align 8
  %a55 = bitcast [805 x double]* %tmp_buff.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a55) nounwind
  call void @llvm.lifetime.start(i64 -1, i8* %a56) nounwind
  %a57 = bitcast [805 x double]* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a57) nounwind
  %a58 = bitcast [805 x double]* %a1 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a58) nounwind
  %a59 = bitcast [805 x double]* %a2 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a59) nounwind
  %a60 = bitcast [805 x double]* %a3 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a60) nounwind
  %a61 = bitcast [805 x double]* %a4 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a61) nounwind
  %a62 = bitcast [805 x double]* %a5 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a62) nounwind
  %a63 = bitcast [805 x double]* %a6 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a63) nounwind
  %a64 = bitcast [805 x double]* %a7 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a64) nounwind
  %a65 = bitcast [805 x double]* %a8 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a65) nounwind
  %a66 = bitcast [805 x double]* %a9 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a66) nounwind
  %a67 = bitcast [805 x double]* %a10 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a67) nounwind
  %a68 = bitcast [805 x double]* %a11 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a68) nounwind
  %a69 = bitcast [805 x double]* %a12 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a69) nounwind
  %a70 = bitcast [805 x double]* %a13 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a70) nounwind
  %a71 = bitcast [805 x double]* %a14 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a71) nounwind
  %a72 = bitcast [805 x double]* %a15 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a72) nounwind
  %a73 = bitcast [301 x double]* %a16 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a73) nounwind
  %a74 = bitcast [301 x double]* %a17 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a74) nounwind
  %a75 = bitcast [301 x double]* %a18 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a75) nounwind
  %a76 = bitcast [301 x double]* %a19 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a76) nounwind
  %a77 = bitcast [301 x double]* %a20 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a77) nounwind
  %a78 = bitcast [301 x double]* %a21 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a78) nounwind
  %a79 = bitcast [301 x double]* %a22 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a79) nounwind
  %a80 = bitcast [301 x double]* %a23 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a80) nounwind
  %a81 = bitcast [301 x double]* %a24 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a81) nounwind
  %a82 = bitcast [301 x double]* %a25 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a82) nounwind
  %a83 = bitcast [301 x double]* %a26 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a83) nounwind
  %a84 = bitcast [301 x double]* %a27 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a84) nounwind
  %a85 = bitcast [301 x double]* %a28 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a85) nounwind
  %a86 = bitcast [301 x double]* %a29 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a86) nounwind
  %a87 = bitcast [301 x double]* %a30 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a87) nounwind
  %a88 = bitcast [301 x double]* %a31 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %a88) nounwind
  %a89 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %a51, i64 0, i32 3, i64 0
  %a90 = load i64* %a89, align 8
  %a91 = getelementptr <{ [4 x i64] }>* %a54, i64 0, i32 0, i64 0
  %a92 = load i64* %a91, align 8
  %vector.size.i = ashr i64 %a90, 4
  %num.vector.wi.i = and i64 %a90, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %a92
  %scalar.size.i = sub i64 %a90, %num.vector.wi.i
  %a93 = icmp eq i64 %vector.size.i, 0
  br i1 %a93, label %scalarIf.i, label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %entry
  %call.i2.i = call x86_svmlcc double @__ocl_svml_b2_log1(double 1.000000e-02) nounwind readnone
  %call.i3.i = call x86_svmlcc double @__ocl_svml_b2_log1(double 1.000000e+04) nounwind readnone
  %a190 = getelementptr [805 x double]* %a15, i64 0, i64 802
  %a191 = getelementptr [805 x double]* %a14, i64 0, i64 802
  %a192 = getelementptr [805 x double]* %a13, i64 0, i64 802
  %a193 = getelementptr [805 x double]* %a12, i64 0, i64 802
  %a194 = getelementptr [805 x double]* %a11, i64 0, i64 802
  %a195 = getelementptr [805 x double]* %a10, i64 0, i64 802
  %a196 = getelementptr [805 x double]* %a9, i64 0, i64 802
  %a197 = getelementptr [805 x double]* %a8, i64 0, i64 802
  %a198 = getelementptr [805 x double]* %a7, i64 0, i64 802
  %a199 = getelementptr [805 x double]* %a6, i64 0, i64 802
  %a200 = getelementptr [805 x double]* %a5, i64 0, i64 802
  %a201 = getelementptr [805 x double]* %a4, i64 0, i64 802
  %a202 = getelementptr [805 x double]* %a3, i64 0, i64 802
  %a203 = getelementptr [805 x double]* %a2, i64 0, i64 802
  %a204 = getelementptr [805 x double]* %a1, i64 0, i64 802
  %a205 = getelementptr [805 x double]* %0, i64 0, i64 802
  %a206 = getelementptr [805 x double]* %a15, i64 0, i64 601
  %a207 = getelementptr [805 x double]* %a14, i64 0, i64 601
  %a208 = getelementptr [805 x double]* %a13, i64 0, i64 601
  %a209 = getelementptr [805 x double]* %a12, i64 0, i64 601
  %a210 = getelementptr [805 x double]* %a11, i64 0, i64 601
  %a211 = getelementptr [805 x double]* %a10, i64 0, i64 601
  %a212 = getelementptr [805 x double]* %a9, i64 0, i64 601
  %a213 = getelementptr [805 x double]* %a8, i64 0, i64 601
  %a214 = getelementptr [805 x double]* %a7, i64 0, i64 601
  %a215 = getelementptr [805 x double]* %a6, i64 0, i64 601
  %a216 = getelementptr [805 x double]* %a5, i64 0, i64 601
  %a217 = getelementptr [805 x double]* %a4, i64 0, i64 601
  %a218 = getelementptr [805 x double]* %a3, i64 0, i64 601
  %a219 = getelementptr [805 x double]* %a2, i64 0, i64 601
  %a220 = getelementptr [805 x double]* %a1, i64 0, i64 601
  %a221 = getelementptr [805 x double]* %0, i64 0, i64 601
  %a222 = getelementptr [301 x double]* %a31, i64 0, i64 199
  %a223 = getelementptr [301 x double]* %a30, i64 0, i64 199
  %a224 = getelementptr [301 x double]* %a29, i64 0, i64 199
  %a225 = getelementptr [301 x double]* %a28, i64 0, i64 199
  %a226 = getelementptr [301 x double]* %a27, i64 0, i64 199
  %a227 = getelementptr [301 x double]* %a26, i64 0, i64 199
  %a228 = getelementptr [301 x double]* %a25, i64 0, i64 199
  %a229 = getelementptr [301 x double]* %a24, i64 0, i64 199
  %a230 = getelementptr [301 x double]* %a23, i64 0, i64 199
  %a231 = getelementptr [301 x double]* %a22, i64 0, i64 199
  %a232 = getelementptr [301 x double]* %a21, i64 0, i64 199
  %a233 = getelementptr [301 x double]* %a20, i64 0, i64 199
  %a234 = getelementptr [301 x double]* %a19, i64 0, i64 199
  %a235 = getelementptr [301 x double]* %a18, i64 0, i64 199
  %a236 = getelementptr [301 x double]* %a17, i64 0, i64 199
  %a237 = getelementptr [301 x double]* %a16, i64 0, i64 199
  %a238 = getelementptr [805 x double]* %0, i64 0, i64 299
  %a239 = getelementptr [805 x double]* %a1, i64 0, i64 299
  %a240 = getelementptr [805 x double]* %a2, i64 0, i64 299
  %a241 = getelementptr [805 x double]* %a3, i64 0, i64 299
  %a242 = getelementptr [805 x double]* %a4, i64 0, i64 299
  %a243 = getelementptr [805 x double]* %a5, i64 0, i64 299
  %a244 = getelementptr [805 x double]* %a6, i64 0, i64 299
  %a245 = getelementptr [805 x double]* %a7, i64 0, i64 299
  %a246 = getelementptr [805 x double]* %a8, i64 0, i64 299
  %a247 = getelementptr [805 x double]* %a9, i64 0, i64 299
  %a248 = getelementptr [805 x double]* %a10, i64 0, i64 299
  %a249 = getelementptr [805 x double]* %a11, i64 0, i64 299
  %a250 = getelementptr [805 x double]* %a12, i64 0, i64 299
  %a251 = getelementptr [805 x double]* %a13, i64 0, i64 299
  %a252 = getelementptr [805 x double]* %a14, i64 0, i64 299
  %a253 = getelementptr [805 x double]* %a15, i64 0, i64 299
  %broadcast1vector_func.i = insertelement <16 x i64> undef, i64 %a92, i32 0
  %broadcast2vector_func.i = shufflevector <16 x i64> %broadcast1vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %a254 = add <16 x i64> %broadcast2vector_func.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %conv392vector_func.i = trunc <16 x i64> %a254 to <16 x i32>
  %idxprom393vector_func.i = sext <16 x i32> %conv392vector_func.i to <16 x i64>
  %mul160605vector_func.i = mul nsw <16 x i32> %conv392vector_func.i, <i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201, i32 201>
  %a256 = sext <16 x i32> %mul160605vector_func.i to <16 x i64>
  %extract.0149.i = extractelement <16 x i64> %a256, i32 0
  %extract.0150.i = extractelement <16 x i64> %a256, i32 1
  %sub.delta151.i = sub i64 %extract.0150.i, %extract.0149.i
  %mul.delta152.i = shl i64 %sub.delta151.i, 4
  %insert.delta153.i = insertelement <16 x i64> undef, i64 %mul.delta152.i, i32 0
  %broadcast.delta154.i = shufflevector <16 x i64> %insert.delta153.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.body170vector_func.i.loopexit, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.body170vector_func.i.loopexit ]
  %a257 = phi <16 x i64> [ %idxprom393vector_func.i, %dim_0_vector_pre_head.i ], [ %Strided.add.i, %for.body170vector_func.i.loopexit ]
  %a259 = phi <16 x i64> [ %a256, %dim_0_vector_pre_head.i ], [ %Strided.add155.i, %for.body170vector_func.i.loopexit ]
  %extractvector_func.i = extractelement <16 x i64> %a257, i32 0
  %extract394vector_func.i = extractelement <16 x i64> %a257, i32 1
  %extract395vector_func.i = extractelement <16 x i64> %a257, i32 2
  %extract396vector_func.i = extractelement <16 x i64> %a257, i32 3
  %extract397vector_func.i = extractelement <16 x i64> %a257, i32 4
  %extract398vector_func.i = extractelement <16 x i64> %a257, i32 5
  %extract399vector_func.i = extractelement <16 x i64> %a257, i32 6
  %extract400vector_func.i = extractelement <16 x i64> %a257, i32 7
  %extract401vector_func.i = extractelement <16 x i64> %a257, i32 8
  %extract402vector_func.i = extractelement <16 x i64> %a257, i32 9
  %extract403vector_func.i = extractelement <16 x i64> %a257, i32 10
  %extract404vector_func.i = extractelement <16 x i64> %a257, i32 11
  %extract405vector_func.i = extractelement <16 x i64> %a257, i32 12
  %extract406vector_func.i = extractelement <16 x i64> %a257, i32 13
  %extract407vector_func.i = extractelement <16 x i64> %a257, i32 14
  %extract408vector_func.i = extractelement <16 x i64> %a257, i32 15
  %a260 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extractvector_func.i, i32 0
  %a261 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract394vector_func.i, i32 0
  %a262 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract395vector_func.i, i32 0
  %a263 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract396vector_func.i, i32 0
  %a264 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract397vector_func.i, i32 0
  %a265 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract398vector_func.i, i32 0
  %a266 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract399vector_func.i, i32 0
  %a267 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract400vector_func.i, i32 0
  %a268 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract401vector_func.i, i32 0
  %a269 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract402vector_func.i, i32 0
  %a270 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract403vector_func.i, i32 0
  %a271 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract404vector_func.i, i32 0
  %a272 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract405vector_func.i, i32 0
  %a273 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract406vector_func.i, i32 0
  %a274 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract407vector_func.i, i32 0
  %a275 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract408vector_func.i, i32 0
  %a276 = load double addrspace(1)* %a260, align 8
  %a277 = load double addrspace(1)* %a261, align 8
  %a278 = load double addrspace(1)* %a262, align 8
  %a279 = load double addrspace(1)* %a263, align 8
  %a280 = load double addrspace(1)* %a264, align 8
  %a281 = load double addrspace(1)* %a265, align 8
  %a282 = load double addrspace(1)* %a266, align 8
  %a283 = load double addrspace(1)* %a267, align 8
  %a284 = load double addrspace(1)* %a268, align 8
  %a285 = load double addrspace(1)* %a269, align 8
  %a286 = load double addrspace(1)* %a270, align 8
  %a287 = load double addrspace(1)* %a271, align 8
  %a288 = load double addrspace(1)* %a272, align 8
  %a289 = load double addrspace(1)* %a273, align 8
  %a290 = load double addrspace(1)* %a274, align 8
  %a291 = load double addrspace(1)* %a275, align 8
  %temp.vectvector_func.i = insertelement <16 x double> undef, double %a276, i32 0
  %temp.vect409vector_func.i = insertelement <16 x double> %temp.vectvector_func.i, double %a277, i32 1
  %temp.vect410vector_func.i = insertelement <16 x double> %temp.vect409vector_func.i, double %a278, i32 2
  %temp.vect411vector_func.i = insertelement <16 x double> %temp.vect410vector_func.i, double %a279, i32 3
  %temp.vect412vector_func.i = insertelement <16 x double> %temp.vect411vector_func.i, double %a280, i32 4
  %temp.vect413vector_func.i = insertelement <16 x double> %temp.vect412vector_func.i, double %a281, i32 5
  %temp.vect414vector_func.i = insertelement <16 x double> %temp.vect413vector_func.i, double %a282, i32 6
  %temp.vect415vector_func.i = insertelement <16 x double> %temp.vect414vector_func.i, double %a283, i32 7
  %temp.vect416vector_func.i = insertelement <16 x double> %temp.vect415vector_func.i, double %a284, i32 8
  %temp.vect417vector_func.i = insertelement <16 x double> %temp.vect416vector_func.i, double %a285, i32 9
  %temp.vect418vector_func.i = insertelement <16 x double> %temp.vect417vector_func.i, double %a286, i32 10
  %temp.vect419vector_func.i = insertelement <16 x double> %temp.vect418vector_func.i, double %a287, i32 11
  %temp.vect420vector_func.i = insertelement <16 x double> %temp.vect419vector_func.i, double %a288, i32 12
  %temp.vect421vector_func.i = insertelement <16 x double> %temp.vect420vector_func.i, double %a289, i32 13
  %temp.vect422vector_func.i = insertelement <16 x double> %temp.vect421vector_func.i, double %a290, i32 14
  %temp.vect423vector_func.i = insertelement <16 x double> %temp.vect422vector_func.i, double %a291, i32 15
  %div424vector_func.i = fdiv <16 x double> %temp.vect423vector_func.i, <double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02, double 1.000000e+02>
  br label %for.cond25.preheadervector_func.i

for.cond25.preheadervector_func.i:                ; preds = %for.cond25.preheadervector_func.i, %entryvector_func.i
  %vectorPHIvector_func.i = phi <16 x i1> [ %local_edge1071398vector_func.i, %for.cond25.preheadervector_func.i ], [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %entryvector_func.i ]
  %indvars.iv18vector_func.i = phi i64 [ %indvars.iv.next19vector_func.i, %for.cond25.preheadervector_func.i ], [ 0, %entryvector_func.i ]
  %indvars.iv.next19vector_func.i = add i64 %indvars.iv18vector_func.i, 1
  %lftr.wideiv20vector_func.i = trunc i64 %indvars.iv.next19vector_func.i to i32
  %notCond105vector_func.i = icmp ne i32 %lftr.wideiv20vector_func.i, 101
  %temp1396vector_func.i = insertelement <16 x i1> undef, i1 %notCond105vector_func.i, i32 0
  %vector1397vector_func.i = shufflevector <16 x i1> %temp1396vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %local_edge1071398vector_func.i = and <16 x i1> %vectorPHIvector_func.i, %vector1397vector_func.i
  %ipred.i.i = bitcast <16 x i1> %local_edge1071398vector_func.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %a940 = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %a940, 0
  br i1 %res.i.i, label %for.cond25.preheadervector_func.i, label %for.end42vector_func.i

for.end42vector_func.i:                           ; preds = %for.cond25.preheadervector_func.i
  %div14425vector_func.i = fdiv <16 x double> %div424vector_func.i, <double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01, double 5.000000e+01>
  %mul46451vector_func.i = fmul <16 x double> %div14425vector_func.i, <double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02, double 9.000000e-02>
  %mul49455vector_func.i = fmul <16 x double> %div14425vector_func.i, <double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE, double 0xBF9E69AD42C3C9EE>
  %div51458vector_func.i = fdiv <16 x double> %mul49455vector_func.i, zeroinitializer
  %mul54460vector_func.i = fmul <16 x double> %div14425vector_func.i, <double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02, double -9.000000e-02>
  %div56463vector_func.i = fdiv <16 x double> %mul54460vector_func.i, zeroinitializer
  %sub57464vector_func.i = fadd <16 x double> %div56463vector_func.i, <double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00, double -1.000000e+00>
  %mul58465vector_func.i = fmul <16 x double> %div14425vector_func.i, <double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02, double 2.530000e-02>
  %mul59466vector_func.i = fmul <16 x double> %mul58465vector_func.i, <double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01, double 5.000000e-01>
  %sub60467vector_func.i = fsub <16 x double> %sub57464vector_func.i, %mul59466vector_func.i
  %a325 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extractvector_func.i, i32 1
  %a326 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract394vector_func.i, i32 1
  %a327 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract395vector_func.i, i32 1
  %a328 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract396vector_func.i, i32 1
  %a329 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract397vector_func.i, i32 1
  %a330 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract398vector_func.i, i32 1
  %a331 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract399vector_func.i, i32 1
  %a332 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract400vector_func.i, i32 1
  %a333 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract401vector_func.i, i32 1
  %a334 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract402vector_func.i, i32 1
  %a335 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract403vector_func.i, i32 1
  %a336 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract404vector_func.i, i32 1
  %a337 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract405vector_func.i, i32 1
  %a338 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract406vector_func.i, i32 1
  %a339 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract407vector_func.i, i32 1
  %a340 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract408vector_func.i, i32 1
  %a341 = load double addrspace(1)* %a325, align 8
  %a342 = load double addrspace(1)* %a326, align 8
  %a343 = load double addrspace(1)* %a327, align 8
  %a344 = load double addrspace(1)* %a328, align 8
  %a345 = load double addrspace(1)* %a329, align 8
  %a346 = load double addrspace(1)* %a330, align 8
  %a347 = load double addrspace(1)* %a331, align 8
  %a348 = load double addrspace(1)* %a332, align 8
  %a349 = load double addrspace(1)* %a333, align 8
  %a350 = load double addrspace(1)* %a334, align 8
  %a351 = load double addrspace(1)* %a335, align 8
  %a352 = load double addrspace(1)* %a336, align 8
  %a353 = load double addrspace(1)* %a337, align 8
  %a354 = load double addrspace(1)* %a338, align 8
  %a355 = load double addrspace(1)* %a339, align 8
  %a356 = load double addrspace(1)* %a340, align 8
  %temp.vect510vector_func.i = insertelement <16 x double> undef, double %a341, i32 0
  %temp.vect511vector_func.i = insertelement <16 x double> %temp.vect510vector_func.i, double %a342, i32 1
  %temp.vect512vector_func.i = insertelement <16 x double> %temp.vect511vector_func.i, double %a343, i32 2
  %temp.vect513vector_func.i = insertelement <16 x double> %temp.vect512vector_func.i, double %a344, i32 3
  %temp.vect514vector_func.i = insertelement <16 x double> %temp.vect513vector_func.i, double %a345, i32 4
  %temp.vect515vector_func.i = insertelement <16 x double> %temp.vect514vector_func.i, double %a346, i32 5
  %temp.vect516vector_func.i = insertelement <16 x double> %temp.vect515vector_func.i, double %a347, i32 6
  %temp.vect517vector_func.i = insertelement <16 x double> %temp.vect516vector_func.i, double %a348, i32 7
  %temp.vect518vector_func.i = insertelement <16 x double> %temp.vect517vector_func.i, double %a349, i32 8
  %temp.vect519vector_func.i = insertelement <16 x double> %temp.vect518vector_func.i, double %a350, i32 9
  %temp.vect520vector_func.i = insertelement <16 x double> %temp.vect519vector_func.i, double %a351, i32 10
  %temp.vect521vector_func.i = insertelement <16 x double> %temp.vect520vector_func.i, double %a352, i32 11
  %temp.vect522vector_func.i = insertelement <16 x double> %temp.vect521vector_func.i, double %a353, i32 12
  %temp.vect523vector_func.i = insertelement <16 x double> %temp.vect522vector_func.i, double %a354, i32 13
  %temp.vect524vector_func.i = insertelement <16 x double> %temp.vect523vector_func.i, double %a355, i32 14
  %a357 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extractvector_func.i, i32 2
  %a358 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract394vector_func.i, i32 2
  %a359 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract395vector_func.i, i32 2
  %a360 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract396vector_func.i, i32 2
  %a361 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract397vector_func.i, i32 2
  %a362 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract398vector_func.i, i32 2
  %a363 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract399vector_func.i, i32 2
  %a364 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract400vector_func.i, i32 2
  %a365 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract401vector_func.i, i32 2
  %a366 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract402vector_func.i, i32 2
  %a367 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract403vector_func.i, i32 2
  %a368 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract404vector_func.i, i32 2
  %a369 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract405vector_func.i, i32 2
  %a370 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract406vector_func.i, i32 2
  %a371 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract407vector_func.i, i32 2
  %a372 = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %extract408vector_func.i, i32 2
  %a373 = load double addrspace(1)* %a357, align 8
  %a374 = load double addrspace(1)* %a358, align 8
  %a375 = load double addrspace(1)* %a359, align 8
  %a376 = load double addrspace(1)* %a360, align 8
  %a377 = load double addrspace(1)* %a361, align 8
  %a378 = load double addrspace(1)* %a362, align 8
  %a379 = load double addrspace(1)* %a363, align 8
  %a380 = load double addrspace(1)* %a364, align 8
  %a381 = load double addrspace(1)* %a365, align 8
  %a382 = load double addrspace(1)* %a366, align 8
  %a383 = load double addrspace(1)* %a367, align 8
  %a384 = load double addrspace(1)* %a368, align 8
  %a385 = load double addrspace(1)* %a369, align 8
  %a386 = load double addrspace(1)* %a370, align 8
  %a387 = load double addrspace(1)* %a371, align 8
  %a388 = load double addrspace(1)* %a372, align 8
  %temp.vect493vector_func.i = insertelement <16 x double> undef, double %a373, i32 0
  %temp.vect494vector_func.i = insertelement <16 x double> %temp.vect493vector_func.i, double %a374, i32 1
  %temp.vect495vector_func.i = insertelement <16 x double> %temp.vect494vector_func.i, double %a375, i32 2
  %temp.vect496vector_func.i = insertelement <16 x double> %temp.vect495vector_func.i, double %a376, i32 3
  %temp.vect497vector_func.i = insertelement <16 x double> %temp.vect496vector_func.i, double %a377, i32 4
  %temp.vect498vector_func.i = insertelement <16 x double> %temp.vect497vector_func.i, double %a378, i32 5
  %temp.vect499vector_func.i = insertelement <16 x double> %temp.vect498vector_func.i, double %a379, i32 6
  %temp.vect500vector_func.i = insertelement <16 x double> %temp.vect499vector_func.i, double %a380, i32 7
  %temp.vect501vector_func.i = insertelement <16 x double> %temp.vect500vector_func.i, double %a381, i32 8
  %temp.vect502vector_func.i = insertelement <16 x double> %temp.vect501vector_func.i, double %a382, i32 9
  %temp.vect503vector_func.i = insertelement <16 x double> %temp.vect502vector_func.i, double %a383, i32 10
  %temp.vect504vector_func.i = insertelement <16 x double> %temp.vect503vector_func.i, double %a384, i32 11
  %temp.vect505vector_func.i = insertelement <16 x double> %temp.vect504vector_func.i, double %a385, i32 12
  %temp.vect506vector_func.i = insertelement <16 x double> %temp.vect505vector_func.i, double %a386, i32 13
  %temp.vect507vector_func.i = insertelement <16 x double> %temp.vect506vector_func.i, double %a387, i32 14
  %div48454vector_func.i = fdiv <16 x double> %mul46451vector_func.i, zeroinitializer
  %sub52459vector_func.i = fsub <16 x double> %div48454vector_func.i, %div51458vector_func.i
  %temp.vect525vector_func.i = insertelement <16 x double> %temp.vect524vector_func.i, double %a356, i32 15
  %temp.vect508vector_func.i = insertelement <16 x double> %temp.vect507vector_func.i, double %a388, i32 15
  %div.i566vector_func.i = fdiv <16 x double> %temp.vect525vector_func.i, %temp.vect508vector_func.i
  %sub18.i912vector_func.i = fsub <16 x double> <double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00>, %sub52459vector_func.i
  br label %for.body105vector_func.preheader.i

for.body105vector_func.preheader.i:               ; preds = %header337vector_func.i.for.body105vector_func.preheader.i_crit_edge, %for.end42vector_func.i
  %indvars.iv = phi i32 [ 2, %for.end42vector_func.i ], [ %phitmp, %header337vector_func.i.for.body105vector_func.preheader.i_crit_edge ]
  %indvars.iv15vector_func.i = phi i64 [ 99, %for.end42vector_func.i ], [ %indvars.iv.next16vector_func.i, %header337vector_func.i.for.body105vector_func.preheader.i_crit_edge ]
  %a476 = trunc i64 %indvars.iv15vector_func.i to i32
  %sub1.ivector_func.i = sub nsw i32 100, %a476
  %sub2.ivector_func.i = add nsw i32 %sub1.ivector_func.i, -1
  %cmp4.ivector_func.i = icmp slt i32 %sub2.ivector_func.i, 0
  %Mneg19vector_func.i = xor i1 %cmp4.ivector_func.i, true
  %masked_load1590vector_func.i = load double* %a205, align 16
  %masked_load1591vector_func.i = load double* %a204, align 16
  %masked_load1592vector_func.i = load double* %a203, align 16
  %masked_load1593vector_func.i = load double* %a202, align 16
  %masked_load1594vector_func.i = load double* %a201, align 16
  %masked_load1595vector_func.i = load double* %a200, align 16
  %masked_load1596vector_func.i = load double* %a199, align 16
  %masked_load1597vector_func.i = load double* %a198, align 16
  %masked_load1598vector_func.i = load double* %a197, align 16
  %masked_load1599vector_func.i = load double* %a196, align 16
  %masked_load1600vector_func.i = load double* %a195, align 16
  %masked_load1601vector_func.i = load double* %a194, align 16
  %masked_load1602vector_func.i = load double* %a193, align 16
  %masked_load1603vector_func.i = load double* %a192, align 16
  %masked_load1604vector_func.i = load double* %a191, align 16
  %masked_load1605vector_func.i = load double* %a190, align 16
  %temp.vect949vector_func.i = insertelement <16 x double> undef, double %masked_load1590vector_func.i, i32 0
  %temp.vect950vector_func.i = insertelement <16 x double> %temp.vect949vector_func.i, double %masked_load1591vector_func.i, i32 1
  %temp.vect951vector_func.i = insertelement <16 x double> %temp.vect950vector_func.i, double %masked_load1592vector_func.i, i32 2
  %temp.vect952vector_func.i = insertelement <16 x double> %temp.vect951vector_func.i, double %masked_load1593vector_func.i, i32 3
  %temp.vect953vector_func.i = insertelement <16 x double> %temp.vect952vector_func.i, double %masked_load1594vector_func.i, i32 4
  %temp.vect954vector_func.i = insertelement <16 x double> %temp.vect953vector_func.i, double %masked_load1595vector_func.i, i32 5
  %temp.vect955vector_func.i = insertelement <16 x double> %temp.vect954vector_func.i, double %masked_load1596vector_func.i, i32 6
  %temp.vect956vector_func.i = insertelement <16 x double> %temp.vect955vector_func.i, double %masked_load1597vector_func.i, i32 7
  %temp.vect957vector_func.i = insertelement <16 x double> %temp.vect956vector_func.i, double %masked_load1598vector_func.i, i32 8
  %temp.vect958vector_func.i = insertelement <16 x double> %temp.vect957vector_func.i, double %masked_load1599vector_func.i, i32 9
  %temp.vect959vector_func.i = insertelement <16 x double> %temp.vect958vector_func.i, double %masked_load1600vector_func.i, i32 10
  %temp.vect960vector_func.i = insertelement <16 x double> %temp.vect959vector_func.i, double %masked_load1601vector_func.i, i32 11
  %temp.vect961vector_func.i = insertelement <16 x double> %temp.vect960vector_func.i, double %masked_load1602vector_func.i, i32 12
  %temp.vect962vector_func.i = insertelement <16 x double> %temp.vect961vector_func.i, double %masked_load1603vector_func.i, i32 13
  %temp.vect963vector_func.i = insertelement <16 x double> %temp.vect962vector_func.i, double %masked_load1604vector_func.i, i32 14
  %temp.vect964vector_func.i = insertelement <16 x double> %temp.vect963vector_func.i, double %masked_load1605vector_func.i, i32 15
  %mul40.i965vector_func.i = fmul <16 x double> %temp.vect964vector_func.i, %sub18.i912vector_func.i
  %masked_load1606vector_func.i = load double* %a238, align 8
  %masked_load1607vector_func.i = load double* %a239, align 8
  %masked_load1608vector_func.i = load double* %a240, align 8
  %masked_load1609vector_func.i = load double* %a241, align 8
  %masked_load1610vector_func.i = load double* %a242, align 8
  %masked_load1611vector_func.i = load double* %a243, align 8
  %masked_load1612vector_func.i = load double* %a244, align 8
  %masked_load1613vector_func.i = load double* %a245, align 8
  %masked_load1614vector_func.i = load double* %a246, align 8
  %masked_load1615vector_func.i = load double* %a247, align 8
  %masked_load1616vector_func.i = load double* %a248, align 8
  %masked_load1617vector_func.i = load double* %a249, align 8
  %masked_load1618vector_func.i = load double* %a250, align 8
  %masked_load1619vector_func.i = load double* %a251, align 8
  %masked_load1620vector_func.i = load double* %a252, align 8
  %masked_load1621vector_func.i = load double* %a253, align 8
  %temp.vect966vector_func.i = insertelement <16 x double> undef, double %masked_load1606vector_func.i, i32 0
  %temp.vect967vector_func.i = insertelement <16 x double> %temp.vect966vector_func.i, double %masked_load1607vector_func.i, i32 1
  %temp.vect968vector_func.i = insertelement <16 x double> %temp.vect967vector_func.i, double %masked_load1608vector_func.i, i32 2
  %temp.vect969vector_func.i = insertelement <16 x double> %temp.vect968vector_func.i, double %masked_load1609vector_func.i, i32 3
  %temp.vect970vector_func.i = insertelement <16 x double> %temp.vect969vector_func.i, double %masked_load1610vector_func.i, i32 4
  %temp.vect971vector_func.i = insertelement <16 x double> %temp.vect970vector_func.i, double %masked_load1611vector_func.i, i32 5
  %temp.vect972vector_func.i = insertelement <16 x double> %temp.vect971vector_func.i, double %masked_load1612vector_func.i, i32 6
  %temp.vect973vector_func.i = insertelement <16 x double> %temp.vect972vector_func.i, double %masked_load1613vector_func.i, i32 7
  %temp.vect974vector_func.i = insertelement <16 x double> %temp.vect973vector_func.i, double %masked_load1614vector_func.i, i32 8
  %temp.vect975vector_func.i = insertelement <16 x double> %temp.vect974vector_func.i, double %masked_load1615vector_func.i, i32 9
  %temp.vect976vector_func.i = insertelement <16 x double> %temp.vect975vector_func.i, double %masked_load1616vector_func.i, i32 10
  %temp.vect977vector_func.i = insertelement <16 x double> %temp.vect976vector_func.i, double %masked_load1617vector_func.i, i32 11
  %temp.vect978vector_func.i = insertelement <16 x double> %temp.vect977vector_func.i, double %masked_load1618vector_func.i, i32 12
  %temp.vect979vector_func.i = insertelement <16 x double> %temp.vect978vector_func.i, double %masked_load1619vector_func.i, i32 13
  %temp.vect980vector_func.i = insertelement <16 x double> %temp.vect979vector_func.i, double %masked_load1620vector_func.i, i32 14
  %temp.vect981vector_func.i = insertelement <16 x double> %temp.vect980vector_func.i, double %masked_load1621vector_func.i, i32 15
  %add44.i982vector_func.i = fadd <16 x double> %mul40.i965vector_func.i, %temp.vect981vector_func.i
  %masked_load1622vector_func.i = load double* %a221, align 8
  %masked_load1623vector_func.i = load double* %a220, align 8
  %masked_load1624vector_func.i = load double* %a219, align 8
  %masked_load1625vector_func.i = load double* %a218, align 8
  %masked_load1626vector_func.i = load double* %a217, align 8
  %masked_load1627vector_func.i = load double* %a216, align 8
  %masked_load1628vector_func.i = load double* %a215, align 8
  %masked_load1629vector_func.i = load double* %a214, align 8
  %masked_load1630vector_func.i = load double* %a213, align 8
  %masked_load1631vector_func.i = load double* %a212, align 8
  %masked_load1632vector_func.i = load double* %a211, align 8
  %masked_load1633vector_func.i = load double* %a210, align 8
  %masked_load1634vector_func.i = load double* %a209, align 8
  %masked_load1635vector_func.i = load double* %a208, align 8
  %masked_load1636vector_func.i = load double* %a207, align 8
  %masked_load1637vector_func.i = load double* %a206, align 8
  %temp.vect983vector_func.i = insertelement <16 x double> undef, double %masked_load1622vector_func.i, i32 0
  %temp.vect984vector_func.i = insertelement <16 x double> %temp.vect983vector_func.i, double %masked_load1623vector_func.i, i32 1
  %temp.vect985vector_func.i = insertelement <16 x double> %temp.vect984vector_func.i, double %masked_load1624vector_func.i, i32 2
  %temp.vect986vector_func.i = insertelement <16 x double> %temp.vect985vector_func.i, double %masked_load1625vector_func.i, i32 3
  %temp.vect987vector_func.i = insertelement <16 x double> %temp.vect986vector_func.i, double %masked_load1626vector_func.i, i32 4
  %temp.vect988vector_func.i = insertelement <16 x double> %temp.vect987vector_func.i, double %masked_load1627vector_func.i, i32 5
  %temp.vect989vector_func.i = insertelement <16 x double> %temp.vect988vector_func.i, double %masked_load1628vector_func.i, i32 6
  %temp.vect990vector_func.i = insertelement <16 x double> %temp.vect989vector_func.i, double %masked_load1629vector_func.i, i32 7
  %temp.vect991vector_func.i = insertelement <16 x double> %temp.vect990vector_func.i, double %masked_load1630vector_func.i, i32 8
  %temp.vect992vector_func.i = insertelement <16 x double> %temp.vect991vector_func.i, double %masked_load1631vector_func.i, i32 9
  %temp.vect993vector_func.i = insertelement <16 x double> %temp.vect992vector_func.i, double %masked_load1632vector_func.i, i32 10
  %temp.vect994vector_func.i = insertelement <16 x double> %temp.vect993vector_func.i, double %masked_load1633vector_func.i, i32 11
  %temp.vect995vector_func.i = insertelement <16 x double> %temp.vect994vector_func.i, double %masked_load1634vector_func.i, i32 12
  %temp.vect996vector_func.i = insertelement <16 x double> %temp.vect995vector_func.i, double %masked_load1635vector_func.i, i32 13
  %temp.vect997vector_func.i = insertelement <16 x double> %temp.vect996vector_func.i, double %masked_load1636vector_func.i, i32 14
  %temp.vect998vector_func.i = insertelement <16 x double> %temp.vect997vector_func.i, double %masked_load1637vector_func.i, i32 15
  %mul50.i999vector_func.i = fmul <16 x double> %sub52459vector_func.i, %temp.vect998vector_func.i
  %add51.i1000vector_func.i = fadd <16 x double> %sub60467vector_func.i, %mul50.i999vector_func.i
  %div52.i1001vector_func.i = fdiv <16 x double> %add44.i982vector_func.i, %add51.i1000vector_func.i
  %extract1017vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 15
  %extract1015vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 13
  %extract1013vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 11
  %extract1011vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 9
  %extract1009vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 7
  %extract1007vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 5
  %extract1005vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 3
  %extract1003vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 1
  br label %vdExp.exit.i46vector_func.i

vdExp.exit.i46vector_func.i:                      ; preds = %evalBoundaryCondBottom.exitvector_func.i, %for.body105vector_func.preheader.i
  %p.04vector_func.i = phi i32 [ %decvector_func.i, %evalBoundaryCondBottom.exitvector_func.i ], [ 50, %for.body105vector_func.preheader.i ]
  br i1 %cmp4.ivector_func.i, label %evalBoundaryCondBottom.exitvector_func.i, label %for.body10.ivector_func.i

for.body10.ivector_func.i:                        ; preds = %for.body10.ivector_func.i, %vdExp.exit.i46vector_func.i
  %for.body10.i_in_mask.0vector_func.i = phi i1 [ true, %for.body10.ivector_func.i ], [ %Mneg19vector_func.i, %vdExp.exit.i46vector_func.i ]
  %indvars.iv.i48vector_func.i = phi i64 [ %indvars.iv.next.i49vector_func.i, %for.body10.ivector_func.i ], [ 0, %vdExp.exit.i46vector_func.i ]
  %indvars.iv.next.i49vector_func.i = add i64 %indvars.iv.i48vector_func.i, 1
  %lftr.wideiv.i50vector_func.i = trunc i64 %indvars.iv.next.i49vector_func.i to i32
  %notCond34vector_func.i = icmp ne i32 %lftr.wideiv.i50vector_func.i, %sub1.ivector_func.i
  %local_edge36vector_func.i = and i1 %for.body10.i_in_mask.0vector_func.i, %notCond34vector_func.i
  br i1 %local_edge36vector_func.i, label %for.body10.ivector_func.i, label %evalBoundaryCondBottom.exitvector_func.i

evalBoundaryCondBottom.exitvector_func.i:         ; preds = %for.body10.ivector_func.i, %vdExp.exit.i46vector_func.i
  %decvector_func.i = add nsw i32 %p.04vector_func.i, -1
  %cmp103vector_func.i = icmp sgt i32 %p.04vector_func.i, 0
  br i1 %cmp103vector_func.i, label %vdExp.exit.i46vector_func.i, label %header322vector_func.i

header322vector_func.i:                           ; preds = %evalBoundaryCondBottom.exitvector_func.i
  %extract1016vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 14
  %extract1014vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 12
  %extract1012vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 10
  %extract1010vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 8
  %extract1008vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 6
  %extract1006vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 4
  %extract1004vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 2
  %extract1002vector_func.i = extractelement <16 x double> %div52.i1001vector_func.i, i32 0
  store double %extract1017vector_func.i, double* %a222, align 8
  store double %extract1016vector_func.i, double* %a223, align 8
  store double %extract1015vector_func.i, double* %a224, align 8
  store double %extract1014vector_func.i, double* %a225, align 8
  store double %extract1013vector_func.i, double* %a226, align 8
  store double %extract1012vector_func.i, double* %a227, align 8
  store double %extract1011vector_func.i, double* %a228, align 8
  store double %extract1010vector_func.i, double* %a229, align 8
  store double %extract1009vector_func.i, double* %a230, align 8
  store double %extract1008vector_func.i, double* %a231, align 8
  store double %extract1007vector_func.i, double* %a232, align 8
  store double %extract1006vector_func.i, double* %a233, align 8
  store double %extract1005vector_func.i, double* %a234, align 8
  store double %extract1004vector_func.i, double* %a235, align 8
  store double %extract1003vector_func.i, double* %a236, align 8
  store double %extract1002vector_func.i, double* %a237, align 8
  %a673 = getelementptr [805 x double]* %0, i64 0, i64 %indvars.iv15vector_func.i
  %a674 = getelementptr [805 x double]* %a1, i64 0, i64 %indvars.iv15vector_func.i
  %a675 = getelementptr [805 x double]* %a2, i64 0, i64 %indvars.iv15vector_func.i
  %a676 = getelementptr [805 x double]* %a3, i64 0, i64 %indvars.iv15vector_func.i
  %a677 = getelementptr [805 x double]* %a4, i64 0, i64 %indvars.iv15vector_func.i
  %a678 = getelementptr [805 x double]* %a5, i64 0, i64 %indvars.iv15vector_func.i
  %a679 = getelementptr [805 x double]* %a6, i64 0, i64 %indvars.iv15vector_func.i
  %a680 = getelementptr [805 x double]* %a7, i64 0, i64 %indvars.iv15vector_func.i
  %a681 = getelementptr [805 x double]* %a8, i64 0, i64 %indvars.iv15vector_func.i
  %a682 = getelementptr [805 x double]* %a9, i64 0, i64 %indvars.iv15vector_func.i
  %a683 = getelementptr [805 x double]* %a10, i64 0, i64 %indvars.iv15vector_func.i
  %a684 = getelementptr [805 x double]* %a11, i64 0, i64 %indvars.iv15vector_func.i
  %a685 = getelementptr [805 x double]* %a12, i64 0, i64 %indvars.iv15vector_func.i
  %a686 = getelementptr [805 x double]* %a13, i64 0, i64 %indvars.iv15vector_func.i
  %a687 = getelementptr [805 x double]* %a14, i64 0, i64 %indvars.iv15vector_func.i
  %a688 = getelementptr [805 x double]* %a15, i64 0, i64 %indvars.iv15vector_func.i
  %masked_load1670vector_func.i = load double* %a673, align 8
  %masked_load1671vector_func.i = load double* %a674, align 8
  %masked_load1672vector_func.i = load double* %a675, align 8
  %masked_load1673vector_func.i = load double* %a676, align 8
  %masked_load1674vector_func.i = load double* %a677, align 8
  %masked_load1675vector_func.i = load double* %a678, align 8
  %masked_load1676vector_func.i = load double* %a679, align 8
  %masked_load1677vector_func.i = load double* %a680, align 8
  %masked_load1678vector_func.i = load double* %a681, align 8
  %masked_load1679vector_func.i = load double* %a682, align 8
  %masked_load1680vector_func.i = load double* %a683, align 8
  %masked_load1681vector_func.i = load double* %a684, align 8
  %masked_load1682vector_func.i = load double* %a685, align 8
  %masked_load1683vector_func.i = load double* %a686, align 8
  %masked_load1684vector_func.i = load double* %a687, align 8
  %masked_load1685vector_func.i = load double* %a688, align 8
  %temp.vect1074vector_func.i = insertelement <16 x double> undef, double %masked_load1670vector_func.i, i32 0
  %temp.vect1075vector_func.i = insertelement <16 x double> %temp.vect1074vector_func.i, double %masked_load1671vector_func.i, i32 1
  %temp.vect1076vector_func.i = insertelement <16 x double> %temp.vect1075vector_func.i, double %masked_load1672vector_func.i, i32 2
  %temp.vect1077vector_func.i = insertelement <16 x double> %temp.vect1076vector_func.i, double %masked_load1673vector_func.i, i32 3
  %temp.vect1078vector_func.i = insertelement <16 x double> %temp.vect1077vector_func.i, double %masked_load1674vector_func.i, i32 4
  %temp.vect1079vector_func.i = insertelement <16 x double> %temp.vect1078vector_func.i, double %masked_load1675vector_func.i, i32 5
  %temp.vect1080vector_func.i = insertelement <16 x double> %temp.vect1079vector_func.i, double %masked_load1676vector_func.i, i32 6
  %temp.vect1081vector_func.i = insertelement <16 x double> %temp.vect1080vector_func.i, double %masked_load1677vector_func.i, i32 7
  %temp.vect1082vector_func.i = insertelement <16 x double> %temp.vect1081vector_func.i, double %masked_load1678vector_func.i, i32 8
  %temp.vect1083vector_func.i = insertelement <16 x double> %temp.vect1082vector_func.i, double %masked_load1679vector_func.i, i32 9
  %temp.vect1084vector_func.i = insertelement <16 x double> %temp.vect1083vector_func.i, double %masked_load1680vector_func.i, i32 10
  %temp.vect1085vector_func.i = insertelement <16 x double> %temp.vect1084vector_func.i, double %masked_load1681vector_func.i, i32 11
  %temp.vect1086vector_func.i = insertelement <16 x double> %temp.vect1085vector_func.i, double %masked_load1682vector_func.i, i32 12
  %temp.vect1087vector_func.i = insertelement <16 x double> %temp.vect1086vector_func.i, double %masked_load1683vector_func.i, i32 13
  %temp.vect1088vector_func.i = insertelement <16 x double> %temp.vect1087vector_func.i, double %masked_load1684vector_func.i, i32 14
  br label %for.body.i79vector_func.i

for.body.i79vector_func.i:                        ; preds = %for.body.i79vector_func.i, %header322vector_func.i
  %indvars.iv7.i73vector_func.i = phi i64 [ %indvars.iv.next8.i78vector_func.i, %for.body.i79vector_func.i ], [ 0, %header322vector_func.i ]
  %a705 = trunc i64 %indvars.iv7.i73vector_func.i to i32
  %conv.i74vector_func.i = sitofp i32 %a705 to double
  %mul.i75vector_func.i = fmul double %conv.i74vector_func.i, 0.000000e+00
  %add.i76vector_func.i = fadd double %mul.i75vector_func.i, %call.i2.i
  %add.ptr7.sum136vector_func.i = add i64 %indvars.iv7.i73vector_func.i, 403
  %a706 = getelementptr [805 x double]* %a15, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a707 = getelementptr [805 x double]* %a14, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a708 = getelementptr [805 x double]* %a13, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a709 = getelementptr [805 x double]* %a12, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a710 = getelementptr [805 x double]* %a11, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a711 = getelementptr [805 x double]* %a10, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a712 = getelementptr [805 x double]* %a9, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a713 = getelementptr [805 x double]* %a8, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a714 = getelementptr [805 x double]* %a7, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a715 = getelementptr [805 x double]* %a6, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a716 = getelementptr [805 x double]* %a5, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a717 = getelementptr [805 x double]* %a4, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a718 = getelementptr [805 x double]* %a3, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a719 = getelementptr [805 x double]* %a2, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a720 = getelementptr [805 x double]* %a1, i64 0, i64 %add.ptr7.sum136vector_func.i
  %a721 = getelementptr [805 x double]* %0, i64 0, i64 %add.ptr7.sum136vector_func.i
  store double %add.i76vector_func.i, double* %a721, align 8
  store double %add.i76vector_func.i, double* %a720, align 8
  store double %add.i76vector_func.i, double* %a719, align 8
  store double %add.i76vector_func.i, double* %a718, align 8
  store double %add.i76vector_func.i, double* %a717, align 8
  store double %add.i76vector_func.i, double* %a716, align 8
  store double %add.i76vector_func.i, double* %a715, align 8
  store double %add.i76vector_func.i, double* %a714, align 8
  store double %add.i76vector_func.i, double* %a713, align 8
  store double %add.i76vector_func.i, double* %a712, align 8
  store double %add.i76vector_func.i, double* %a711, align 8
  store double %add.i76vector_func.i, double* %a710, align 8
  store double %add.i76vector_func.i, double* %a709, align 8
  store double %add.i76vector_func.i, double* %a708, align 8
  store double %add.i76vector_func.i, double* %a707, align 8
  store double %add.i76vector_func.i, double* %a706, align 8
  %indvars.iv.next8.i78vector_func.i = add i64 %indvars.iv7.i73vector_func.i, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next8.i78vector_func.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 200
  br i1 %exitcond, label %for.body.i.i87vector_func.i, label %for.body.i79vector_func.i

for.body.i.i87vector_func.i:                      ; preds = %for.body.i.i87vector_func.i, %for.body.i79vector_func.i
  %indvars.iv.i.i81vector_func.i = phi i64 [ %indvars.iv.next.i.i84vector_func.i, %for.body.i.i87vector_func.i ], [ 0, %for.body.i79vector_func.i ]
  %add.ptr7.sum137vector_func.i = add i64 %indvars.iv.i.i81vector_func.i, 403
  %a722 = getelementptr [805 x double]* %0, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a723 = getelementptr [805 x double]* %a1, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a724 = getelementptr [805 x double]* %a2, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a725 = getelementptr [805 x double]* %a3, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a726 = getelementptr [805 x double]* %a4, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a727 = getelementptr [805 x double]* %a5, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a728 = getelementptr [805 x double]* %a6, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a729 = getelementptr [805 x double]* %a7, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a730 = getelementptr [805 x double]* %a8, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a731 = getelementptr [805 x double]* %a9, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a732 = getelementptr [805 x double]* %a10, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a733 = getelementptr [805 x double]* %a11, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a734 = getelementptr [805 x double]* %a12, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a735 = getelementptr [805 x double]* %a13, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a736 = getelementptr [805 x double]* %a14, i64 0, i64 %add.ptr7.sum137vector_func.i
  %a737 = getelementptr [805 x double]* %a15, i64 0, i64 %add.ptr7.sum137vector_func.i
  %masked_load1686vector_func.i = load double* %a722, align 8
  %masked_load1687vector_func.i = load double* %a723, align 8
  %masked_load1688vector_func.i = load double* %a724, align 8
  %masked_load1689vector_func.i = load double* %a725, align 8
  %masked_load1690vector_func.i = load double* %a726, align 8
  %masked_load1691vector_func.i = load double* %a727, align 8
  %masked_load1692vector_func.i = load double* %a728, align 8
  %masked_load1693vector_func.i = load double* %a729, align 8
  %masked_load1694vector_func.i = load double* %a730, align 8
  %masked_load1695vector_func.i = load double* %a731, align 8
  %masked_load1696vector_func.i = load double* %a732, align 8
  %masked_load1697vector_func.i = load double* %a733, align 8
  %masked_load1698vector_func.i = load double* %a734, align 8
  %masked_load1699vector_func.i = load double* %a735, align 8
  %masked_load1700vector_func.i = load double* %a736, align 8
  %masked_load1701vector_func.i = load double* %a737, align 8
  %temp.vect1090vector_func.i = insertelement <16 x double> undef, double %masked_load1686vector_func.i, i32 0
  %temp.vect1091vector_func.i = insertelement <16 x double> %temp.vect1090vector_func.i, double %masked_load1687vector_func.i, i32 1
  %temp.vect1092vector_func.i = insertelement <16 x double> %temp.vect1091vector_func.i, double %masked_load1688vector_func.i, i32 2
  %temp.vect1093vector_func.i = insertelement <16 x double> %temp.vect1092vector_func.i, double %masked_load1689vector_func.i, i32 3
  %temp.vect1094vector_func.i = insertelement <16 x double> %temp.vect1093vector_func.i, double %masked_load1690vector_func.i, i32 4
  %temp.vect1095vector_func.i = insertelement <16 x double> %temp.vect1094vector_func.i, double %masked_load1691vector_func.i, i32 5
  %temp.vect1096vector_func.i = insertelement <16 x double> %temp.vect1095vector_func.i, double %masked_load1692vector_func.i, i32 6
  %temp.vect1097vector_func.i = insertelement <16 x double> %temp.vect1096vector_func.i, double %masked_load1693vector_func.i, i32 7
  %temp.vect1098vector_func.i = insertelement <16 x double> %temp.vect1097vector_func.i, double %masked_load1694vector_func.i, i32 8
  %temp.vect1099vector_func.i = insertelement <16 x double> %temp.vect1098vector_func.i, double %masked_load1695vector_func.i, i32 9
  %temp.vect1100vector_func.i = insertelement <16 x double> %temp.vect1099vector_func.i, double %masked_load1696vector_func.i, i32 10
  %temp.vect1101vector_func.i = insertelement <16 x double> %temp.vect1100vector_func.i, double %masked_load1697vector_func.i, i32 11
  %temp.vect1102vector_func.i = insertelement <16 x double> %temp.vect1101vector_func.i, double %masked_load1698vector_func.i, i32 12
  %temp.vect1103vector_func.i = insertelement <16 x double> %temp.vect1102vector_func.i, double %masked_load1699vector_func.i, i32 13
  %temp.vect1104vector_func.i = insertelement <16 x double> %temp.vect1103vector_func.i, double %masked_load1700vector_func.i, i32 14
  %temp.vect1105vector_func.i = insertelement <16 x double> %temp.vect1104vector_func.i, double %masked_load1701vector_func.i, i32 15
  %call.i58.i = call x86_svmlcc <16 x double> @__ocl_svml_b2_exp16(<16 x double> %temp.vect1105vector_func.i) nounwind readnone
  %extract1121vector_func.i = extractelement <16 x double> %call.i58.i, i32 15
  %extract1120vector_func.i = extractelement <16 x double> %call.i58.i, i32 14
  %extract1119vector_func.i = extractelement <16 x double> %call.i58.i, i32 13
  %extract1118vector_func.i = extractelement <16 x double> %call.i58.i, i32 12
  %extract1117vector_func.i = extractelement <16 x double> %call.i58.i, i32 11
  %extract1116vector_func.i = extractelement <16 x double> %call.i58.i, i32 10
  %extract1115vector_func.i = extractelement <16 x double> %call.i58.i, i32 9
  %extract1114vector_func.i = extractelement <16 x double> %call.i58.i, i32 8
  %extract1113vector_func.i = extractelement <16 x double> %call.i58.i, i32 7
  %extract1112vector_func.i = extractelement <16 x double> %call.i58.i, i32 6
  %extract1111vector_func.i = extractelement <16 x double> %call.i58.i, i32 5
  %extract1110vector_func.i = extractelement <16 x double> %call.i58.i, i32 4
  %extract1109vector_func.i = extractelement <16 x double> %call.i58.i, i32 3
  %extract1108vector_func.i = extractelement <16 x double> %call.i58.i, i32 2
  %extract1107vector_func.i = extractelement <16 x double> %call.i58.i, i32 1
  %extract1106vector_func.i = extractelement <16 x double> %call.i58.i, i32 0
  store double %extract1106vector_func.i, double* %a722, align 8
  store double %extract1107vector_func.i, double* %a723, align 8
  store double %extract1108vector_func.i, double* %a724, align 8
  store double %extract1109vector_func.i, double* %a725, align 8
  store double %extract1110vector_func.i, double* %a726, align 8
  store double %extract1111vector_func.i, double* %a727, align 8
  store double %extract1112vector_func.i, double* %a728, align 8
  store double %extract1113vector_func.i, double* %a729, align 8
  store double %extract1114vector_func.i, double* %a730, align 8
  store double %extract1115vector_func.i, double* %a731, align 8
  store double %extract1116vector_func.i, double* %a732, align 8
  store double %extract1117vector_func.i, double* %a733, align 8
  store double %extract1118vector_func.i, double* %a734, align 8
  store double %extract1119vector_func.i, double* %a735, align 8
  store double %extract1120vector_func.i, double* %a736, align 8
  store double %extract1121vector_func.i, double* %a737, align 8
  %indvars.iv.next.i.i84vector_func.i = add i64 %indvars.iv.i.i81vector_func.i, 1
  %lftr.wideiv71 = trunc i64 %indvars.iv.next.i.i84vector_func.i to i32
  %exitcond72 = icmp eq i32 %lftr.wideiv71, 200
  br i1 %exitcond72, label %header332vector_func.i, label %for.body.i.i87vector_func.i

header332vector_func.i:                           ; preds = %for.body.i.i87vector_func.i
  %temp.vect1089vector_func.i = insertelement <16 x double> %temp.vect1088vector_func.i, double %masked_load1685vector_func.i, i32 15
  %mul5.i891122vector_func.i = fmul <16 x double> %temp.vect525vector_func.i, %temp.vect1089vector_func.i
  br label %preload2269vector_func.i

preload2269vector_func.i:                         ; preds = %preload2269vector_func.i, %header332vector_func.i
  %indvars.iv.i91vector_func.i = phi i64 [ %indvars.iv.next.i97vector_func.i, %preload2269vector_func.i ], [ 0, %header332vector_func.i ]
  %vectorPHI1125vector_func.i = phi <16 x double> [ %minv.1.i1166vector_func.i, %preload2269vector_func.i ], [ <double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10, double 1.000000e+10>, %header332vector_func.i ]
  %vectorPHI1126vector_func.i = phi <16 x i32> [ %__j.1.i1164vector_func.i, %preload2269vector_func.i ], [ undef, %header332vector_func.i ]
  %a738 = getelementptr [301 x double]* %a31, i64 0, i64 %indvars.iv.i91vector_func.i
  %a739 = getelementptr [301 x double]* %a30, i64 0, i64 %indvars.iv.i91vector_func.i
  %a740 = getelementptr [301 x double]* %a29, i64 0, i64 %indvars.iv.i91vector_func.i
  %a741 = getelementptr [301 x double]* %a28, i64 0, i64 %indvars.iv.i91vector_func.i
  %a742 = getelementptr [301 x double]* %a27, i64 0, i64 %indvars.iv.i91vector_func.i
  %a743 = getelementptr [301 x double]* %a26, i64 0, i64 %indvars.iv.i91vector_func.i
  %a744 = getelementptr [301 x double]* %a25, i64 0, i64 %indvars.iv.i91vector_func.i
  %a745 = getelementptr [301 x double]* %a24, i64 0, i64 %indvars.iv.i91vector_func.i
  %a746 = getelementptr [301 x double]* %a23, i64 0, i64 %indvars.iv.i91vector_func.i
  %a747 = getelementptr [301 x double]* %a22, i64 0, i64 %indvars.iv.i91vector_func.i
  %a748 = getelementptr [301 x double]* %a21, i64 0, i64 %indvars.iv.i91vector_func.i
  %a749 = getelementptr [301 x double]* %a20, i64 0, i64 %indvars.iv.i91vector_func.i
  %a750 = getelementptr [301 x double]* %a19, i64 0, i64 %indvars.iv.i91vector_func.i
  %a751 = getelementptr [301 x double]* %a18, i64 0, i64 %indvars.iv.i91vector_func.i
  %a752 = getelementptr [301 x double]* %a17, i64 0, i64 %indvars.iv.i91vector_func.i
  %a753 = getelementptr [301 x double]* %a16, i64 0, i64 %indvars.iv.i91vector_func.i
  %masked_load1702vector_func.i = load double* %a753, align 8
  %masked_load1703vector_func.i = load double* %a752, align 8
  %masked_load1704vector_func.i = load double* %a751, align 8
  %masked_load1705vector_func.i = load double* %a750, align 8
  %masked_load1706vector_func.i = load double* %a749, align 8
  %masked_load1707vector_func.i = load double* %a748, align 8
  %masked_load1708vector_func.i = load double* %a747, align 8
  %masked_load1709vector_func.i = load double* %a746, align 8
  %masked_load1710vector_func.i = load double* %a745, align 8
  %masked_load1711vector_func.i = load double* %a744, align 8
  %masked_load1712vector_func.i = load double* %a743, align 8
  %masked_load1713vector_func.i = load double* %a742, align 8
  %masked_load1714vector_func.i = load double* %a741, align 8
  %masked_load1715vector_func.i = load double* %a740, align 8
  %masked_load1716vector_func.i = load double* %a739, align 8
  %masked_load1717vector_func.i = load double* %a738, align 8
  %temp.vect1127vector_func.i = insertelement <16 x double> undef, double %masked_load1702vector_func.i, i32 0
  %temp.vect1128vector_func.i = insertelement <16 x double> %temp.vect1127vector_func.i, double %masked_load1703vector_func.i, i32 1
  %temp.vect1129vector_func.i = insertelement <16 x double> %temp.vect1128vector_func.i, double %masked_load1704vector_func.i, i32 2
  %temp.vect1130vector_func.i = insertelement <16 x double> %temp.vect1129vector_func.i, double %masked_load1705vector_func.i, i32 3
  %temp.vect1131vector_func.i = insertelement <16 x double> %temp.vect1130vector_func.i, double %masked_load1706vector_func.i, i32 4
  %temp.vect1132vector_func.i = insertelement <16 x double> %temp.vect1131vector_func.i, double %masked_load1707vector_func.i, i32 5
  %temp.vect1133vector_func.i = insertelement <16 x double> %temp.vect1132vector_func.i, double %masked_load1708vector_func.i, i32 6
  %temp.vect1134vector_func.i = insertelement <16 x double> %temp.vect1133vector_func.i, double %masked_load1709vector_func.i, i32 7
  %temp.vect1135vector_func.i = insertelement <16 x double> %temp.vect1134vector_func.i, double %masked_load1710vector_func.i, i32 8
  %temp.vect1136vector_func.i = insertelement <16 x double> %temp.vect1135vector_func.i, double %masked_load1711vector_func.i, i32 9
  %temp.vect1137vector_func.i = insertelement <16 x double> %temp.vect1136vector_func.i, double %masked_load1712vector_func.i, i32 10
  %temp.vect1138vector_func.i = insertelement <16 x double> %temp.vect1137vector_func.i, double %masked_load1713vector_func.i, i32 11
  %temp.vect1139vector_func.i = insertelement <16 x double> %temp.vect1138vector_func.i, double %masked_load1714vector_func.i, i32 12
  %temp.vect1140vector_func.i = insertelement <16 x double> %temp.vect1139vector_func.i, double %masked_load1715vector_func.i, i32 13
  %temp.vect1141vector_func.i = insertelement <16 x double> %temp.vect1140vector_func.i, double %masked_load1716vector_func.i, i32 14
  %temp.vect1142vector_func.i = insertelement <16 x double> %temp.vect1141vector_func.i, double %masked_load1717vector_func.i, i32 15
  %add8.i1143vector_func.i = fadd <16 x double> %mul5.i891122vector_func.i, %temp.vect1142vector_func.i
  %add.ptr7.sum138vector_func.i = add i64 %indvars.iv.i91vector_func.i, 403
  %a754 = getelementptr [805 x double]* %a15, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a755 = getelementptr [805 x double]* %a14, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a756 = getelementptr [805 x double]* %a13, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a757 = getelementptr [805 x double]* %a12, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a758 = getelementptr [805 x double]* %a11, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a759 = getelementptr [805 x double]* %a10, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a760 = getelementptr [805 x double]* %a9, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a761 = getelementptr [805 x double]* %a8, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a762 = getelementptr [805 x double]* %a7, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a763 = getelementptr [805 x double]* %a6, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a764 = getelementptr [805 x double]* %a5, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a765 = getelementptr [805 x double]* %a4, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a766 = getelementptr [805 x double]* %a3, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a767 = getelementptr [805 x double]* %a2, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a768 = getelementptr [805 x double]* %a1, i64 0, i64 %add.ptr7.sum138vector_func.i
  %a769 = getelementptr [805 x double]* %0, i64 0, i64 %add.ptr7.sum138vector_func.i
  %masked_load1718vector_func.i = load double* %a769, align 8
  %masked_load1719vector_func.i = load double* %a768, align 8
  %masked_load1720vector_func.i = load double* %a767, align 8
  %masked_load1721vector_func.i = load double* %a766, align 8
  %masked_load1722vector_func.i = load double* %a765, align 8
  %masked_load1723vector_func.i = load double* %a764, align 8
  %masked_load1724vector_func.i = load double* %a763, align 8
  %masked_load1725vector_func.i = load double* %a762, align 8
  %masked_load1726vector_func.i = load double* %a761, align 8
  %masked_load1727vector_func.i = load double* %a760, align 8
  %masked_load1728vector_func.i = load double* %a759, align 8
  %masked_load1729vector_func.i = load double* %a758, align 8
  %masked_load1730vector_func.i = load double* %a757, align 8
  %masked_load1731vector_func.i = load double* %a756, align 8
  %masked_load1732vector_func.i = load double* %a755, align 8
  %masked_load1733vector_func.i = load double* %a754, align 8
  %temp.vect1144vector_func.i = insertelement <16 x double> undef, double %masked_load1718vector_func.i, i32 0
  %temp.vect1145vector_func.i = insertelement <16 x double> %temp.vect1144vector_func.i, double %masked_load1719vector_func.i, i32 1
  %temp.vect1146vector_func.i = insertelement <16 x double> %temp.vect1145vector_func.i, double %masked_load1720vector_func.i, i32 2
  %temp.vect1147vector_func.i = insertelement <16 x double> %temp.vect1146vector_func.i, double %masked_load1721vector_func.i, i32 3
  %temp.vect1148vector_func.i = insertelement <16 x double> %temp.vect1147vector_func.i, double %masked_load1722vector_func.i, i32 4
  %temp.vect1149vector_func.i = insertelement <16 x double> %temp.vect1148vector_func.i, double %masked_load1723vector_func.i, i32 5
  %temp.vect1150vector_func.i = insertelement <16 x double> %temp.vect1149vector_func.i, double %masked_load1724vector_func.i, i32 6
  %temp.vect1151vector_func.i = insertelement <16 x double> %temp.vect1150vector_func.i, double %masked_load1725vector_func.i, i32 7
  %temp.vect1152vector_func.i = insertelement <16 x double> %temp.vect1151vector_func.i, double %masked_load1726vector_func.i, i32 8
  %temp.vect1153vector_func.i = insertelement <16 x double> %temp.vect1152vector_func.i, double %masked_load1727vector_func.i, i32 9
  %temp.vect1154vector_func.i = insertelement <16 x double> %temp.vect1153vector_func.i, double %masked_load1728vector_func.i, i32 10
  %temp.vect1155vector_func.i = insertelement <16 x double> %temp.vect1154vector_func.i, double %masked_load1729vector_func.i, i32 11
  %temp.vect1156vector_func.i = insertelement <16 x double> %temp.vect1155vector_func.i, double %masked_load1730vector_func.i, i32 12
  %temp.vect1157vector_func.i = insertelement <16 x double> %temp.vect1156vector_func.i, double %masked_load1731vector_func.i, i32 13
  %temp.vect1158vector_func.i = insertelement <16 x double> %temp.vect1157vector_func.i, double %masked_load1732vector_func.i, i32 14
  %temp.vect1159vector_func.i = insertelement <16 x double> %temp.vect1158vector_func.i, double %masked_load1733vector_func.i, i32 15
  %mul11.i941160vector_func.i = fmul <16 x double> %div.i566vector_func.i, %temp.vect1159vector_func.i
  %sub.i951161vector_func.i = fsub <16 x double> %add8.i1143vector_func.i, %mul11.i941160vector_func.i
  %call.i54.i = call x86_svmlcc <16 x double> @__ocl_svml_b2_fabs16(<16 x double> %sub.i951161vector_func.i) nounwind readnone
  %cmp12.ivector_func.i = fcmp olt <16 x double> %call.i54.i, %vectorPHI1125vector_func.i
  %a770 = trunc i64 %indvars.iv.i91vector_func.i to i32
  %temp1162vector_func.i = insertelement <16 x i32> undef, i32 %a770, i32 0
  %vector1163vector_func.i = shufflevector <16 x i32> %temp1162vector_func.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %__j.1.i1164vector_func.i = select <16 x i1> %cmp12.ivector_func.i, <16 x i32> %vector1163vector_func.i, <16 x i32> %vectorPHI1126vector_func.i
  %minv.1.i1166vector_func.i = select <16 x i1> %cmp12.ivector_func.i, <16 x double> %call.i54.i, <16 x double> %vectorPHI1125vector_func.i
  %indvars.iv.next.i97vector_func.i = add i64 %indvars.iv.i91vector_func.i, 1
  %lftr.wideiv73 = trunc i64 %indvars.iv.next.i97vector_func.i to i32
  %exitcond74 = icmp eq i32 %lftr.wideiv73, 200
  br i1 %exitcond74, label %header337vector_func.i, label %preload2269vector_func.i

header337vector_func.i:                           ; preds = %preload2269vector_func.i
  %phitmp.i1167vector_func.i = sext <16 x i32> %__j.1.i1164vector_func.i to <16 x i64>
  %add.ptr7.sum1391168vector_func.i = add <16 x i64> %phitmp.i1167vector_func.i, <i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403, i64 403>
  %extract1184vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 15
  %a771 = getelementptr [805 x double]* %a15, i64 0, i64 %extract1184vector_func.i
  %extract1183vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 14
  %a772 = getelementptr [805 x double]* %a14, i64 0, i64 %extract1183vector_func.i
  %extract1182vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 13
  %a773 = getelementptr [805 x double]* %a13, i64 0, i64 %extract1182vector_func.i
  %extract1181vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 12
  %a774 = getelementptr [805 x double]* %a12, i64 0, i64 %extract1181vector_func.i
  %extract1180vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 11
  %a775 = getelementptr [805 x double]* %a11, i64 0, i64 %extract1180vector_func.i
  %extract1179vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 10
  %a776 = getelementptr [805 x double]* %a10, i64 0, i64 %extract1179vector_func.i
  %extract1178vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 9
  %a777 = getelementptr [805 x double]* %a9, i64 0, i64 %extract1178vector_func.i
  %extract1177vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 8
  %a778 = getelementptr [805 x double]* %a8, i64 0, i64 %extract1177vector_func.i
  %extract1176vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 7
  %a779 = getelementptr [805 x double]* %a7, i64 0, i64 %extract1176vector_func.i
  %extract1175vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 6
  %a780 = getelementptr [805 x double]* %a6, i64 0, i64 %extract1175vector_func.i
  %extract1174vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 5
  %a781 = getelementptr [805 x double]* %a5, i64 0, i64 %extract1174vector_func.i
  %extract1173vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 4
  %a782 = getelementptr [805 x double]* %a4, i64 0, i64 %extract1173vector_func.i
  %extract1172vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 3
  %a783 = getelementptr [805 x double]* %a3, i64 0, i64 %extract1172vector_func.i
  %extract1171vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 2
  %a784 = getelementptr [805 x double]* %a2, i64 0, i64 %extract1171vector_func.i
  %extract1170vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 1
  %a785 = getelementptr [805 x double]* %a1, i64 0, i64 %extract1170vector_func.i
  %extract1169vector_func.i = extractelement <16 x i64> %add.ptr7.sum1391168vector_func.i, i32 0
  %a786 = getelementptr [805 x double]* %0, i64 0, i64 %extract1169vector_func.i
  %masked_load1734vector_func.i = load double* %a786, align 8
  %masked_load1735vector_func.i = load double* %a785, align 8
  %masked_load1736vector_func.i = load double* %a784, align 8
  %masked_load1737vector_func.i = load double* %a783, align 8
  %masked_load1738vector_func.i = load double* %a782, align 8
  %masked_load1739vector_func.i = load double* %a781, align 8
  %masked_load1740vector_func.i = load double* %a780, align 8
  %masked_load1741vector_func.i = load double* %a779, align 8
  %masked_load1742vector_func.i = load double* %a778, align 8
  %masked_load1743vector_func.i = load double* %a777, align 8
  %masked_load1744vector_func.i = load double* %a776, align 8
  %masked_load1745vector_func.i = load double* %a775, align 8
  %masked_load1746vector_func.i = load double* %a774, align 8
  %masked_load1747vector_func.i = load double* %a773, align 8
  %masked_load1748vector_func.i = load double* %a772, align 8
  %masked_load1749vector_func.i = load double* %a771, align 8
  %add.ptr13.sum1vector_func.i = add i64 %indvars.iv15vector_func.i, 201
  %a787 = getelementptr [301 x double]* %a31, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a788 = getelementptr [301 x double]* %a30, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a789 = getelementptr [301 x double]* %a29, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a790 = getelementptr [301 x double]* %a28, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a791 = getelementptr [301 x double]* %a27, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a792 = getelementptr [301 x double]* %a26, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a793 = getelementptr [301 x double]* %a25, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a794 = getelementptr [301 x double]* %a24, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a795 = getelementptr [301 x double]* %a23, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a796 = getelementptr [301 x double]* %a22, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a797 = getelementptr [301 x double]* %a21, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a798 = getelementptr [301 x double]* %a20, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a799 = getelementptr [301 x double]* %a19, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a800 = getelementptr [301 x double]* %a18, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a801 = getelementptr [301 x double]* %a17, i64 0, i64 %add.ptr13.sum1vector_func.i
  %a802 = getelementptr [301 x double]* %a16, i64 0, i64 %add.ptr13.sum1vector_func.i
  store double %masked_load1734vector_func.i, double* %a802, align 8
  store double %masked_load1735vector_func.i, double* %a801, align 8
  store double %masked_load1736vector_func.i, double* %a800, align 8
  store double %masked_load1737vector_func.i, double* %a799, align 8
  store double %masked_load1738vector_func.i, double* %a798, align 8
  store double %masked_load1739vector_func.i, double* %a797, align 8
  store double %masked_load1740vector_func.i, double* %a796, align 8
  store double %masked_load1741vector_func.i, double* %a795, align 8
  store double %masked_load1742vector_func.i, double* %a794, align 8
  store double %masked_load1743vector_func.i, double* %a793, align 8
  store double %masked_load1744vector_func.i, double* %a792, align 8
  store double %masked_load1745vector_func.i, double* %a791, align 8
  store double %masked_load1746vector_func.i, double* %a790, align 8
  store double %masked_load1747vector_func.i, double* %a789, align 8
  store double %masked_load1748vector_func.i, double* %a788, align 8
  store double %masked_load1749vector_func.i, double* %a787, align 8
  %exitcond81 = icmp eq i32 %indvars.iv, 101
  br i1 %exitcond81, label %for.body156vector_func.i, label %header337vector_func.i.for.body105vector_func.preheader.i_crit_edge

header337vector_func.i.for.body105vector_func.preheader.i_crit_edge: ; preds = %header337vector_func.i
  %indvars.iv.next16vector_func.i = add i64 %indvars.iv15vector_func.i, -1
  %phitmp = add i32 %indvars.iv, 1
  br label %for.body105vector_func.preheader.i

for.body156vector_func.i:                         ; preds = %for.body156vector_func.i, %header337vector_func.i
  %indvars.iv10vector_func.i = phi i64 [ %indvars.iv.next11vector_func.i, %for.body156vector_func.i ], [ 0, %header337vector_func.i ]
  %temp1289vector_func.i = insertelement <16 x i64> undef, i64 %indvars.iv10vector_func.i, i32 0
  %vector1290vector_func.i = shufflevector <16 x i64> %temp1289vector_func.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %a874 = getelementptr [301 x double]* %a31, i64 0, i64 %indvars.iv10vector_func.i
  %a875 = getelementptr [301 x double]* %a30, i64 0, i64 %indvars.iv10vector_func.i
  %a876 = getelementptr [301 x double]* %a29, i64 0, i64 %indvars.iv10vector_func.i
  %a877 = getelementptr [301 x double]* %a28, i64 0, i64 %indvars.iv10vector_func.i
  %a878 = getelementptr [301 x double]* %a27, i64 0, i64 %indvars.iv10vector_func.i
  %a879 = getelementptr [301 x double]* %a26, i64 0, i64 %indvars.iv10vector_func.i
  %a880 = getelementptr [301 x double]* %a25, i64 0, i64 %indvars.iv10vector_func.i
  %a881 = getelementptr [301 x double]* %a24, i64 0, i64 %indvars.iv10vector_func.i
  %a882 = getelementptr [301 x double]* %a23, i64 0, i64 %indvars.iv10vector_func.i
  %a883 = getelementptr [301 x double]* %a22, i64 0, i64 %indvars.iv10vector_func.i
  %a884 = getelementptr [301 x double]* %a21, i64 0, i64 %indvars.iv10vector_func.i
  %a885 = getelementptr [301 x double]* %a20, i64 0, i64 %indvars.iv10vector_func.i
  %a886 = getelementptr [301 x double]* %a19, i64 0, i64 %indvars.iv10vector_func.i
  %a887 = getelementptr [301 x double]* %a18, i64 0, i64 %indvars.iv10vector_func.i
  %a888 = getelementptr [301 x double]* %a17, i64 0, i64 %indvars.iv10vector_func.i
  %a889 = getelementptr [301 x double]* %a16, i64 0, i64 %indvars.iv10vector_func.i
  %masked_load1814vector_func.i = load double* %a889, align 8
  %masked_load1815vector_func.i = load double* %a888, align 8
  %masked_load1816vector_func.i = load double* %a887, align 8
  %masked_load1817vector_func.i = load double* %a886, align 8
  %masked_load1818vector_func.i = load double* %a885, align 8
  %masked_load1819vector_func.i = load double* %a884, align 8
  %masked_load1820vector_func.i = load double* %a883, align 8
  %masked_load1821vector_func.i = load double* %a882, align 8
  %masked_load1822vector_func.i = load double* %a881, align 8
  %masked_load1823vector_func.i = load double* %a880, align 8
  %masked_load1824vector_func.i = load double* %a879, align 8
  %masked_load1825vector_func.i = load double* %a878, align 8
  %masked_load1826vector_func.i = load double* %a877, align 8
  %masked_load1827vector_func.i = load double* %a876, align 8
  %masked_load1828vector_func.i = load double* %a875, align 8
  %masked_load1829vector_func.i = load double* %a874, align 8
  %a89038 = add nsw <16 x i64> %vector1290vector_func.i, %a259
  %extract1306vector_func.i = extractelement <16 x i64> %a89038, i32 15
  %extract1305vector_func.i = extractelement <16 x i64> %a89038, i32 14
  %extract1304vector_func.i = extractelement <16 x i64> %a89038, i32 13
  %extract1303vector_func.i = extractelement <16 x i64> %a89038, i32 12
  %extract1302vector_func.i = extractelement <16 x i64> %a89038, i32 11
  %extract1301vector_func.i = extractelement <16 x i64> %a89038, i32 10
  %extract1300vector_func.i = extractelement <16 x i64> %a89038, i32 9
  %extract1299vector_func.i = extractelement <16 x i64> %a89038, i32 8
  %extract1298vector_func.i = extractelement <16 x i64> %a89038, i32 7
  %extract1297vector_func.i = extractelement <16 x i64> %a89038, i32 6
  %extract1296vector_func.i = extractelement <16 x i64> %a89038, i32 5
  %extract1295vector_func.i = extractelement <16 x i64> %a89038, i32 4
  %extract1294vector_func.i = extractelement <16 x i64> %a89038, i32 3
  %extract1293vector_func.i = extractelement <16 x i64> %a89038, i32 2
  %extract1292vector_func.i = extractelement <16 x i64> %a89038, i32 1
  %extract1291vector_func.i = extractelement <16 x i64> %a89038, i32 0
  %a891 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1306vector_func.i
  %a892 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1305vector_func.i
  %a893 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1304vector_func.i
  %a894 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1303vector_func.i
  %a895 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1302vector_func.i
  %a896 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1301vector_func.i
  %a897 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1300vector_func.i
  %a898 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1299vector_func.i
  %a899 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1298vector_func.i
  %a900 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1297vector_func.i
  %a901 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1296vector_func.i
  %a902 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1295vector_func.i
  %a903 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1294vector_func.i
  %a904 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1293vector_func.i
  %a905 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1292vector_func.i
  %a906 = getelementptr inbounds double addrspace(1)* %a45, i64 %extract1291vector_func.i
  store double %masked_load1814vector_func.i, double addrspace(1)* %a906, align 8
  store double %masked_load1815vector_func.i, double addrspace(1)* %a905, align 8
  store double %masked_load1816vector_func.i, double addrspace(1)* %a904, align 8
  store double %masked_load1817vector_func.i, double addrspace(1)* %a903, align 8
  store double %masked_load1818vector_func.i, double addrspace(1)* %a902, align 8
  store double %masked_load1819vector_func.i, double addrspace(1)* %a901, align 8
  store double %masked_load1820vector_func.i, double addrspace(1)* %a900, align 8
  store double %masked_load1821vector_func.i, double addrspace(1)* %a899, align 8
  store double %masked_load1822vector_func.i, double addrspace(1)* %a898, align 8
  store double %masked_load1823vector_func.i, double addrspace(1)* %a897, align 8
  store double %masked_load1824vector_func.i, double addrspace(1)* %a896, align 8
  store double %masked_load1825vector_func.i, double addrspace(1)* %a895, align 8
  store double %masked_load1826vector_func.i, double addrspace(1)* %a894, align 8
  store double %masked_load1827vector_func.i, double addrspace(1)* %a893, align 8
  store double %masked_load1828vector_func.i, double addrspace(1)* %a892, align 8
  store double %masked_load1829vector_func.i, double addrspace(1)* %a891, align 8
  %indvars.iv.next11vector_func.i = add i64 %indvars.iv10vector_func.i, 1
  %lftr.wideiv82 = trunc i64 %indvars.iv.next11vector_func.i to i32
  %exitcond83 = icmp eq i32 %lftr.wideiv82, 201
  br i1 %exitcond83, label %for.body170vector_func.i.loopexit, label %for.body156vector_func.i

for.body170vector_func.i.loopexit:                ; preds = %for.body156vector_func.i
  store double %call.i2.i, double addrspace(1)* %a36, align 8
  store double %call.i3.i, double addrspace(1)* %a39, align 8
  store double 0.000000e+00, double addrspace(1)* %a42, align 8
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %Strided.add.i = add <16 x i64> %a257, <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
  %Strided.add155.i = add <16 x i64> %a259, %broadcast.delta154.i
  br i1 %dim_0_vector_cmp.to.max.i, label %scalarIf.i, label %entryvector_func.i

scalarIf.i:                                       ; preds = %for.body170vector_func.i.loopexit, %entry
  %a941 = icmp eq i64 %a90, %num.vector.wi.i
  br i1 %a941, label %__evalConvertibleBond_separated_args.exit, label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %scalarIf.i
  %call.i31.i = call x86_svmlcc double @__ocl_svml_b2_log1(double 1.000000e-02) nounwind readnone
  %arrayidx20.100.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 100
  %add.ptr.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 101
  %call.i29.i = call x86_svmlcc double @__ocl_svml_b2_log1(double 1.000000e+04) nounwind readnone
  %sub43.i = fsub double %call.i29.i, %call.i31.i
  %div44.i = fdiv double %sub43.i, 2.000000e+02
  %mul45.i = fmul double %div44.i, %div44.i
  %mul47.i = fmul double %mul45.i, 4.000000e+00
  %mul50.i = fmul double %div44.i, 4.000000e+00
  %mul55.i = fmul double %mul45.i, 2.000000e+00
  %arrayidx122.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 299
  %arrayidx1.i.phi.trans.insert.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 0
  %arrayidx4.i.phi.trans.insert.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 1
  %arrayidx2.i63.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 404
  %arrayidx6.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 605
  %arrayidx39.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 802
  %arrayidx49.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 601
  %arrayidx55.i.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 199
  %arrayidx72.i.i68 = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 198
  %1 = add i64 %a92, %num.vector.wi.i
  %2 = trunc i64 %1 to i32
  %3 = mul i32 %2, 201
  %4 = zext i32 %3 to i64
  %5 = mul i32 %2, 100
  %6 = zext i32 %5 to i64
  %scevgep130 = getelementptr [301 x double]* %tmp_rslt.i, i64 0, i64 201
  %scevgep130131 = bitcast double* %scevgep130 to i8*
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.cond153.preheader.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.cond153.preheader.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %for.cond153.preheader.i ]
  %7 = mul i64 %dim_0_ind_var.i, 100
  %8 = add i64 %6, %7
  %sext = shl i64 %8, 32
  %9 = ashr exact i64 %sext, 32
  %scevgep128 = getelementptr double addrspace(1)* %a48, i64 %9
  %scevgep128129 = bitcast double addrspace(1)* %scevgep128 to i8 addrspace(1)*
  %10 = mul i64 %dim_0_ind_var.i, 201
  %11 = add i64 %4, %10
  %sext132 = shl i64 %11, 32
  %12 = ashr exact i64 %sext132, 32
  %scevgep = getelementptr double addrspace(1)* %a45, i64 %12
  %scevgep125 = bitcast double addrspace(1)* %scevgep to i8 addrspace(1)*
  %sext133 = shl i64 %dim_0_tid.i, 32
  %idxprom.i = ashr exact i64 %sext133, 32
  %t.i = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %idxprom.i, i32 0
  %a942 = load double addrspace(1)* %t.i, align 8
  %div.i = fdiv double %a942, 1.000000e+02
  %rk.i = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %idxprom.i, i32 3
  call void @llvm.memset.p0i8.i64(i8* %a55, i8 0, i64 808, i32 16, i1 false) nounwind
  br label %for.cond25.preheader.i

for.cond25.preheader.i:                           ; preds = %for.inc37.4.i, %scalar_kernel_entry.i
  %indvars.iv18.i = phi i64 [ 0, %scalar_kernel_entry.i ], [ %indvars.iv.next19.i, %for.inc37.4.i ]
  %a943 = trunc i64 %indvars.iv18.i to i32
  %conv29.i = sitofp i32 %a943 to double
  %mul.i = fmul double %conv29.i, %div.i
  %arrayidx36.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %indvars.iv18.i
  %sub.i = fadd double %mul.i, -1.000000e+00
  %call.i27.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.i) nounwind readnone
  %cmp32.i = fcmp olt double %call.i27.i, 1.000000e-15
  br i1 %cmp32.i, label %if.then.i, label %for.inc37.i

if.then.i:                                        ; preds = %for.cond25.preheader.i
  %a944 = load double addrspace(1)* %rk.i, align 8
  store double %a944, double* %arrayidx36.i, align 8
  br label %for.inc37.i

for.inc37.i:                                      ; preds = %if.then.i, %for.cond25.preheader.i
  %sub.1.i = fadd double %mul.i, -2.000000e+00
  %call.i25.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.1.i) nounwind readnone
  %cmp32.1.i = fcmp olt double %call.i25.i, 1.000000e-15
  br i1 %cmp32.1.i, label %if.then.1.i, label %for.inc37.1.i

for.end42.i:                                      ; preds = %for.inc37.4.i
  %div14.i = fdiv double %div.i, 5.000000e+01
  %mul46.i = fmul double %div14.i, 9.000000e-02
  %mul49.i = fmul double %div14.i, 0xBF9E69AD42C3C9EE
  %div51.i = fdiv double %mul49.i, %mul50.i
  %mul54.i = fmul double %div14.i, -9.000000e-02
  %div56.i = fdiv double %mul54.i, %mul55.i
  %sub57.i = fadd double %div56.i, -1.000000e+00
  %mul58.i = fmul double %div14.i, 2.530000e-02
  %mul59.i = fmul double %mul58.i, 5.000000e-01
  %sub60.i = fsub double %sub57.i, %mul59.i
  %div71.i = fdiv double %mul54.i, %mul47.i
  %div79.i = fdiv double %mul46.i, %mul55.i
  %sub80.i = fadd double %div79.i, -1.000000e+00
  %add83.i = fadd double %sub80.i, %mul59.i
  %k94.i = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %idxprom.i, i32 1
  %a945 = load double addrspace(1)* %k94.i, align 8
  %Ic.i = getelementptr inbounds %struct.cb_params_struct addrspace(1)* %a33, i64 %idxprom.i, i32 2
  %a946 = load double addrspace(1)* %Ic.i, align 8
  %a947 = load double* %arrayidx20.100.i, align 16
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.end42.i
  %indvars.iv6.i.i = phi i64 [ %indvars.iv.next7.i.i, %for.body.i.i ], [ 0, %for.end42.i ]
  %a948 = trunc i64 %indvars.iv6.i.i to i32
  %conv.i.i = sitofp i32 %a948 to double
  %mul.i.i = fmul double %conv.i.i, %div44.i
  %add.i.i = fadd double %mul.i.i, %call.i31.i
  %add.ptr7.sum.i = add i64 %indvars.iv6.i.i, 403
  %arrayidx.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum.i
  store double %add.i.i, double* %arrayidx.i.i, align 8
  %indvars.iv.next7.i.i = add i64 %indvars.iv6.i.i, 1
  %lftr.wideiv88 = trunc i64 %indvars.iv.next7.i.i to i32
  %exitcond89 = icmp eq i32 %lftr.wideiv88, 201
  br i1 %exitcond89, label %for.body.i.i.i.loopexit, label %for.body.i.i

for.body.i.i.i.loopexit:                          ; preds = %for.body.i.i
  %div48.i = fdiv double %mul46.i, %mul47.i
  %sub52.i = fsub double %div48.i, %div51.i
  %add75.i = fadd double %div71.i, %div51.i
  br label %for.body.i.i.i

for.body.i.i.i:                                   ; preds = %for.body.i.i.i, %for.body.i.i.i.loopexit
  %indvars.iv.i.i.i = phi i64 [ %indvars.iv.next.i.i.i, %for.body.i.i.i ], [ 0, %for.body.i.i.i.loopexit ]
  %add.ptr7.sum126.i = add i64 %indvars.iv.i.i.i, 403
  %arrayidx.i.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum126.i
  %a949 = load double* %arrayidx.i.i.i, align 8
  %call.i.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %a949) nounwind readnone
  store double %call.i.i, double* %arrayidx.i.i.i, align 8
  %indvars.iv.next.i.i.i = add i64 %indvars.iv.i.i.i, 1
  %lftr.wideiv90 = trunc i64 %indvars.iv.next.i.i.i to i32
  %exitcond91 = icmp eq i32 %lftr.wideiv90, 201
  br i1 %exitcond91, label %vdExp.exit.i.i, label %for.body.i.i.i

vdExp.exit.i.i:                                   ; preds = %for.body.i.i.i
  %add.i = fadd double %div48.i, %div51.i
  %add6.i.i = fadd double %a947, 1.000000e+00
  %mul7.i.i = fmul double %add6.i.i, %a945
  %div.i.i = fdiv double %a945, %a946
  %a951 = insertelement <8 x double> undef, double %mul7.i.i, i32 0
  br label %for.body5.i.i

for.body5.i.i:                                    ; preds = %for.body5.i.i, %vdExp.exit.i.i
  %indvars.iv.i.i = phi i64 [ 0, %vdExp.exit.i.i ], [ %indvars.iv.next.i.i, %for.body5.i.i ]
  %add.ptr7.sum127.i = add i64 %indvars.iv.i.i, 403
  %arrayidx9.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum127.i
  %a950 = load double* %arrayidx9.i.i, align 8
  %mul10.i.i = fmul double %div.i.i, %a950
  %a952 = insertelement <8 x double> undef, double %mul10.i.i, i32 0
  %a953 = call <8 x double> @llvm.x86.mic.mask.max.pd(<8 x double> %a951, i8 1, <8 x double> %a951, <8 x double> %a952) nounwind
  %a954 = extractelement <8 x double> %a953, i32 0
  %arrayidx12.i.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %indvars.iv.i.i
  store double %a954, double* %arrayidx12.i.i, align 8
  %indvars.iv.next.i.i = add i64 %indvars.iv.i.i, 1
  %lftr.wideiv92 = trunc i64 %indvars.iv.next.i.i to i32
  %exitcond93 = icmp eq i32 %lftr.wideiv92, 201
  br i1 %exitcond93, label %evalTermCondLastStep.exit.i, label %for.body5.i.i

evalTermCondLastStep.exit.i:                      ; preds = %for.body5.i.i
  %sub91.i = fsub double %div71.i, %div51.i
  %mul.i34.i = fmul double %div.i, -2.530000e-02
  %sub.i60.i = fsub double -0.000000e+00, %add.i
  %div.i62.i = fdiv double %sub.i60.i, %sub60.i
  %sub18.i.i = fsub double -0.000000e+00, %sub52.i
  br label %for.cond102.preheader.i

for.cond102.preheader.i:                          ; preds = %evalTermCond.exit.i, %evalTermCondLastStep.exit.i
  %indvars.iv106 = phi i32 [ %indvars.iv.next107, %evalTermCond.exit.i ], [ 1, %evalTermCondLastStep.exit.i ]
  %indvars.iv15.i = phi i64 [ %indvars.iv.next16.i, %evalTermCond.exit.i ], [ 99, %evalTermCondLastStep.exit.i ]
  %a955 = trunc i64 %indvars.iv15.i to i32
  %sub1.i.i = sub nsw i32 100, %a955
  %sub2.i.i = add nsw i32 %sub1.i.i, -1
  %cmp4.i.i = icmp slt i32 %sub2.i.i, 0
  %cmp1.i.i.i = icmp sgt i32 %sub1.i.i, 0
  %conv29.i.i = sitofp i32 %a955 to double
  %sub30.i.i = fsub double 1.000000e+02, %conv29.i.i
  %sub31.i.i = fadd double %sub30.i.i, -1.000000e+00
  %mul32.i.i = fmul double %sub31.i.i, -2.530000e-02
  %mul33.i.i = fmul double %mul32.i.i, %div.i
  %call.i21.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %mul33.i.i) nounwind readnone
  %add.i47.i = add i32 %a955, 1
  br label %for.body105.i

for.cond153.preheader.i:                          ; preds = %evalTermCond.exit.i
  call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %scevgep125, i8* %a56, i64 1608, i32 8, i1 false)
  call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %scevgep128129, i8* %scevgep130131, i64 800, i32 8, i1 false)
  store double %call.i31.i, double addrspace(1)* %a36, align 8
  store double %call.i29.i, double addrspace(1)* %a39, align 8
  store double %div44.i, double addrspace(1)* %a42, align 8
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %__evalConvertibleBond_separated_args.exit, label %scalar_kernel_entry.i

for.body105.i:                                    ; preds = %Sweep.exit.i, %for.cond102.preheader.i
  %p.04.i = phi i32 [ 50, %for.cond102.preheader.i ], [ %dec.i, %Sweep.exit.i ]
  %.pre.i = load double* %arrayidx1.i.phi.trans.insert.i, align 16
  %.pre144.i = load double* %arrayidx4.i.phi.trans.insert.i, align 8
  br label %for.body.i32.i

for.body.i32.i:                                   ; preds = %for.body.i32.i, %for.body105.i
  %a957 = phi double [ %.pre144.i, %for.body105.i ], [ %a960, %for.body.i32.i ]
  %a958 = phi double [ %.pre.i, %for.body105.i ], [ %a957, %for.body.i32.i ]
  %indvars.iv.i26.i = phi i64 [ 0, %for.body105.i ], [ %indvars.iv.next.i28.i, %for.body.i32.i ]
  %mul.i27.i = fmul double %add75.i, %a958
  %indvars.iv.next.i28.i = add i64 %indvars.iv.i26.i, 1
  %mul5.i.i = fmul double %add83.i, %a957
  %add6.i29.i = fadd double %mul.i27.i, %mul5.i.i
  %a959 = add nsw i64 %indvars.iv.i26.i, 2
  %arrayidx10.i.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %a959
  %a960 = load double* %arrayidx10.i.i, align 8
  %mul11.i.i = fmul double %sub91.i, %a960
  %add12.i.i = fadd double %add6.i29.i, %mul11.i.i
  %add.ptr.sum.i = add i64 %indvars.iv.i26.i, 101
  %arrayidx14.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr.sum.i
  store double %add12.i.i, double* %arrayidx14.i.i, align 8
  %lftr.wideiv94 = trunc i64 %indvars.iv.next.i28.i to i32
  %exitcond95 = icmp eq i32 %lftr.wideiv94, 199
  br i1 %exitcond95, label %tridiag_mvMultiply.exit.i, label %for.body.i32.i

tridiag_mvMultiply.exit.i:                        ; preds = %for.body.i32.i
  br i1 %cmp4.i.i, label %for.end.i38.i, label %for.body.i37.i

for.body.i37.i:                                   ; preds = %for.body.i37.i, %tridiag_mvMultiply.exit.i
  %indvars.iv8.i.i = phi i64 [ %indvars.iv.next9.i.i, %for.body.i37.i ], [ 0, %tridiag_mvMultiply.exit.i ]
  %a961 = trunc i64 %indvars.iv8.i.i to i32
  %conv.i35.i = sitofp i32 %a961 to double
  %mul3.i.i = fmul double %mul.i34.i, %conv.i35.i
  %add.ptr4.sum.i = add i64 %indvars.iv8.i.i, 302
  %arrayidx.i36.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr4.sum.i
  store double %mul3.i.i, double* %arrayidx.i36.i, align 8
  %indvars.iv.next9.i.i = add i64 %indvars.iv8.i.i, 1
  %lftr.wideiv108 = trunc i64 %indvars.iv.next9.i.i to i32
  %exitcond109 = icmp eq i32 %lftr.wideiv108, %indvars.iv106
  br i1 %exitcond109, label %for.end.i38.i, label %for.body.i37.i

for.end.i38.i:                                    ; preds = %for.body.i37.i, %tridiag_mvMultiply.exit.i
  br i1 %cmp1.i.i.i, label %for.body.i.i45.i, label %vdExp.exit.i46.i

for.body.i.i45.i:                                 ; preds = %for.body.i.i45.i, %for.end.i38.i
  %indvars.iv.i.i39.i = phi i64 [ %indvars.iv.next.i.i42.i, %for.body.i.i45.i ], [ 0, %for.end.i38.i ]
  %add.ptr4.sum143.i = add i64 %indvars.iv.i.i39.i, 302
  %arrayidx.i.i40.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr4.sum143.i
  %a962 = load double* %arrayidx.i.i40.i, align 8
  %call.i19.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %a962) nounwind readnone
  store double %call.i19.i, double* %arrayidx.i.i40.i, align 8
  %indvars.iv.next.i.i42.i = add i64 %indvars.iv.i.i39.i, 1
  %lftr.wideiv104 = trunc i64 %indvars.iv.next.i.i42.i to i32
  %exitcond105 = icmp eq i32 %lftr.wideiv104, %indvars.iv106
  br i1 %exitcond105, label %vdExp.exit.i46.i, label %for.body.i.i45.i

vdExp.exit.i46.i:                                 ; preds = %for.body.i.i45.i, %for.end.i38.i
  br i1 %cmp4.i.i, label %evalBoundaryCondBottom.exit.i, label %for.body10.i.i

for.body10.i.i:                                   ; preds = %for.body10.i.i, %vdExp.exit.i46.i
  %indvars.iv.i48.i = phi i64 [ %indvars.iv.next.i49.i, %for.body10.i.i ], [ 0, %vdExp.exit.i46.i ]
  %sum.02.i.i = phi double [ %add17.i.i, %for.body10.i.i ], [ 0.000000e+00, %vdExp.exit.i46.i ]
  %a963 = trunc i64 %indvars.iv.i48.i to i32
  %add11.i.i = add i32 %add.i47.i, %a963
  %idxprom12.i.i = sext i32 %add11.i.i to i64
  %arrayidx13.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %idxprom12.i.i
  %a964 = load double* %arrayidx13.i.i, align 8
  %add.ptr4.sum128.i = add i64 %indvars.iv.i48.i, 302
  %arrayidx15.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr4.sum128.i
  %a965 = load double* %arrayidx15.i.i, align 8
  %mul16.i.i = fmul double %a964, %a965
  %add17.i.i = fadd double %sum.02.i.i, %mul16.i.i
  %indvars.iv.next.i49.i = add i64 %indvars.iv.i48.i, 1
  %lftr.wideiv100 = trunc i64 %indvars.iv.next.i49.i to i32
  %exitcond101 = icmp eq i32 %lftr.wideiv100, %indvars.iv106
  br i1 %exitcond101, label %evalBoundaryCondBottom.exit.i, label %for.body10.i.i

evalBoundaryCondBottom.exit.i:                    ; preds = %for.body10.i.i, %vdExp.exit.i46.i
  %sum.0.lcssa.i.i = phi double [ 0.000000e+00, %vdExp.exit.i46.i ], [ %add17.i.i, %for.body10.i.i ]
  %conv22.i.i = sitofp i32 %p.04.i to double
  %mul23.i.i = fmul double %conv22.i.i, %div14.i
  %sub24.i.i = fsub double %div.i, %mul23.i.i
  %mul25.i.i = fmul double %sub24.i.i, -2.530000e-02
  %call.i17.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %mul25.i.i) nounwind readnone
  %mul26.i.i = fmul double %call.i17.i, %a945
  %add35.i.i = fadd double %sum.0.lcssa.i.i, %call.i21.i
  %mul36.i.i = fmul double %mul26.i.i, %add35.i.i
  %mul113.i = fmul double %sub52.i, %mul36.i.i
  %a966 = load double* %add.ptr.i, align 8
  %sub116.i = fsub double %a966, %mul113.i
  store double %sub116.i, double* %add.ptr.i, align 8
  %mul1.i.i = fmul double %sub24.i.i, 1.000000e-02
  %sub2.i56.i = fsub double %call.i29.i, %mul1.i.i
  %call.i16.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %sub2.i56.i) nounwind readnone
  %mul3.i58.i = fmul double %div.i.i, %call.i16.i
  %mul120.i = fmul double %add.i, %mul3.i58.i
  %a967 = load double* %arrayidx122.i, align 8
  %sub123.i = fsub double %a967, %mul120.i
  store double %sub123.i, double* %arrayidx122.i, align 8
  store double %div.i62.i, double* %arrayidx2.i63.i, align 16
  %div5.i.i = fdiv double %sub116.i, %sub60.i
  store double %div5.i.i, double* %arrayidx6.i.i, align 8
  br label %for.body.i69.i

for.body.i69.i:                                   ; preds = %for.body.i69.i, %evalBoundaryCondBottom.exit.i
  %a968 = phi double [ %div31.i.i, %for.body.i69.i ], [ %div5.i.i, %evalBoundaryCondBottom.exit.i ]
  %a969 = phi double [ %div13.i.i, %for.body.i69.i ], [ %div.i62.i, %evalBoundaryCondBottom.exit.i ]
  %indvars.iv7.i.i = phi i64 [ %indvars.iv.next8.i.i, %for.body.i69.i ], [ 1, %evalBoundaryCondBottom.exit.i ]
  %mul.i65.i = fmul double %sub52.i, %a969
  %add.i66.i = fadd double %sub60.i, %mul.i65.i
  %div13.i.i = fdiv double %sub.i60.i, %add.i66.i
  %indvars.iv.next8.i.i = add i64 %indvars.iv7.i.i, 1
  %add.ptr7.sum130.i = add i64 %indvars.iv7.i.i, 404
  %arrayidx16.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum130.i
  store double %div13.i.i, double* %arrayidx16.i.i, align 8
  %mul21.i.i = fmul double %a968, %sub18.i.i
  %add.ptr.sum131.i = add i64 %indvars.iv7.i.i, 101
  %arrayidx23.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr.sum131.i
  %a970 = load double* %arrayidx23.i.i, align 8
  %add24.i.i = fadd double %mul21.i.i, %a970
  %div31.i.i = fdiv double %add24.i.i, %add.i66.i
  %add.ptr10.sum132.i = add i64 %indvars.iv7.i.i, 605
  %arrayidx34.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr10.sum132.i
  store double %div31.i.i, double* %arrayidx34.i.i, align 8
  %lftr.wideiv96 = trunc i64 %indvars.iv.next8.i.i to i32
  %exitcond97 = icmp eq i32 %lftr.wideiv96, 198
  br i1 %exitcond97, label %for.end.i70.i, label %for.body.i69.i

for.end.i70.i:                                    ; preds = %for.body.i69.i
  %a971 = load double* %arrayidx39.i.i, align 16
  %mul40.i.i = fmul double %a971, %sub18.i.i
  %a972 = load double* %arrayidx122.i, align 8
  %add44.i.i = fadd double %mul40.i.i, %a972
  %a973 = load double* %arrayidx49.i.i, align 8
  %mul50.i.i = fmul double %sub52.i, %a973
  %add51.i.i = fadd double %sub60.i, %mul50.i.i
  %div52.i.i = fdiv double %add44.i.i, %add51.i.i
  store double %div52.i.i, double* %arrayidx55.i.i, align 8
  %mul66.i.i64 = fmul double %a973, %div52.i.i
  %add70.i.i67 = fadd double %mul66.i.i64, %a971
  store double %add70.i.i67, double* %arrayidx72.i.i68, align 16
  br label %for.body59.i.for.body59.i_crit_edge.i

for.body59.i.for.body59.i_crit_edge.i:            ; preds = %for.body59.i.for.body59.i_crit_edge.i, %for.end.i70.i
  %add70.i.i70 = phi double [ %add70.i.i67, %for.end.i70.i ], [ %add70.i.i, %for.body59.i.for.body59.i_crit_edge.i ]
  %indvars.iv.i71.i69 = phi i64 [ 197, %for.end.i70.i ], [ %indvars.iv.next.i72.i, %for.body59.i.for.body59.i_crit_edge.i ]
  %indvars.iv.next.i72.i = add i64 %indvars.iv.i71.i69, -1
  %add.ptr7.sum133.i = add i64 %indvars.iv.i71.i69, 403
  %arrayidx62.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum133.i
  %a975 = load double* %arrayidx62.i.i, align 8
  %mul66.i.i = fmul double %a975, %add70.i.i70
  %add.ptr10.sum134.i = add i64 %indvars.iv.i71.i69, 604
  %arrayidx69.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr10.sum134.i
  %a976 = load double* %arrayidx69.i.i, align 8
  %add70.i.i = fadd double %mul66.i.i, %a976
  %arrayidx72.i.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %indvars.iv.i71.i69
  store double %add70.i.i, double* %arrayidx72.i.i, align 8
  %a977 = trunc i64 %indvars.iv.next.i72.i to i32
  %cmp58.i.i = icmp sgt i32 %a977, 0
  br i1 %cmp58.i.i, label %for.body59.i.for.body59.i_crit_edge.i, label %Sweep.exit.i

Sweep.exit.i:                                     ; preds = %for.body59.i.for.body59.i_crit_edge.i
  %dec.i = add nsw i32 %p.04.i, -1
  %cmp103.i = icmp sgt i32 %p.04.i, 0
  br i1 %cmp103.i, label %for.body105.i, label %for.end131.i

for.end131.i:                                     ; preds = %Sweep.exit.i
  %arrayidx137.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %indvars.iv15.i
  %a978 = load double* %arrayidx137.i, align 8
  br label %for.body.i79.i

for.body.i79.i:                                   ; preds = %for.body.i79.i, %for.end131.i
  %indvars.iv7.i73.i = phi i64 [ %indvars.iv.next8.i78.i, %for.body.i79.i ], [ 0, %for.end131.i ]
  %a979 = trunc i64 %indvars.iv7.i73.i to i32
  %conv.i74.i = sitofp i32 %a979 to double
  %mul.i75.i = fmul double %conv.i74.i, %div44.i
  %add.i76.i = fadd double %mul.i75.i, %call.i31.i
  %add.ptr7.sum136.i = add i64 %indvars.iv7.i73.i, 403
  %arrayidx.i77.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum136.i
  store double %add.i76.i, double* %arrayidx.i77.i, align 8
  %indvars.iv.next8.i78.i = add i64 %indvars.iv7.i73.i, 1
  %lftr.wideiv110 = trunc i64 %indvars.iv.next8.i78.i to i32
  %exitcond111 = icmp eq i32 %lftr.wideiv110, 200
  br i1 %exitcond111, label %for.body.i.i87.i, label %for.body.i79.i

for.body.i.i87.i:                                 ; preds = %for.body.i.i87.i, %for.body.i79.i
  %indvars.iv.i.i81.i = phi i64 [ %indvars.iv.next.i.i84.i, %for.body.i.i87.i ], [ 0, %for.body.i79.i ]
  %add.ptr7.sum137.i = add i64 %indvars.iv.i.i81.i, 403
  %arrayidx.i.i82.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum137.i
  %a980 = load double* %arrayidx.i.i82.i, align 8
  %call.i14.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %a980) nounwind readnone
  store double %call.i14.i, double* %arrayidx.i.i82.i, align 8
  %indvars.iv.next.i.i84.i = add i64 %indvars.iv.i.i81.i, 1
  %lftr.wideiv112 = trunc i64 %indvars.iv.next.i.i84.i to i32
  %exitcond113 = icmp eq i32 %lftr.wideiv112, 200
  br i1 %exitcond113, label %vdExp.exit.i88.i, label %for.body.i.i87.i

vdExp.exit.i88.i:                                 ; preds = %for.body.i.i87.i
  %mul5.i89.i = fmul double %a945, %a978
  br label %for.body4.i.i

for.body4.i.i:                                    ; preds = %for.body4.i.i, %vdExp.exit.i88.i
  %indvars.iv.i91.i = phi i64 [ 0, %vdExp.exit.i88.i ], [ %indvars.iv.next.i97.i, %for.body4.i.i ]
  %minv.04.i.i = phi double [ 1.000000e+10, %vdExp.exit.i88.i ], [ %minv.1.i.i, %for.body4.i.i ]
  %__j.03.i.i = phi i32 [ undef, %vdExp.exit.i88.i ], [ %__j.1.i.i, %for.body4.i.i ]
  %arrayidx7.i92.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %indvars.iv.i91.i
  %a981 = load double* %arrayidx7.i92.i, align 8
  %add8.i.i = fadd double %mul5.i89.i, %a981
  %add.ptr7.sum138.i = add i64 %indvars.iv.i91.i, 403
  %arrayidx10.i93.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum138.i
  %a982 = load double* %arrayidx10.i93.i, align 8
  %mul11.i94.i = fmul double %div.i.i, %a982
  %sub.i95.i = fsub double %add8.i.i, %mul11.i94.i
  %call.i11.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.i95.i) nounwind readnone
  %cmp12.i.i = fcmp olt double %call.i11.i, %minv.04.i.i
  %a983 = trunc i64 %indvars.iv.i91.i to i32
  %__j.1.i.i = select i1 %cmp12.i.i, i32 %a983, i32 %__j.03.i.i
  %minv.1.i.i = select i1 %cmp12.i.i, double %call.i11.i, double %minv.04.i.i
  %indvars.iv.next.i97.i = add i64 %indvars.iv.i91.i, 1
  %lftr.wideiv114 = trunc i64 %indvars.iv.next.i97.i to i32
  %exitcond115 = icmp eq i32 %lftr.wideiv114, 200
  br i1 %exitcond115, label %for.cond1.for.end16_crit_edge.i.i, label %for.body4.i.i

for.cond1.for.end16_crit_edge.i.i:                ; preds = %for.body4.i.i
  %phitmp.i.i = sext i32 %__j.1.i.i to i64
  %add.ptr7.sum139.i = add i64 %phitmp.i.i, 403
  %arrayidx18.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum139.i
  %a984 = load double* %arrayidx18.i.i, align 8
  %add.ptr13.sum1.i = add i64 %indvars.iv15.i, 201
  %arrayidx141.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %add.ptr13.sum1.i
  store double %a984, double* %arrayidx141.i, align 8
  %a985 = load double* %arrayidx137.i, align 8
  br label %for.body.i108.i

for.body.i108.i:                                  ; preds = %for.body.i108.i, %for.cond1.for.end16_crit_edge.i.i
  %indvars.iv6.i100.i = phi i64 [ %indvars.iv.next7.i105.i, %for.body.i108.i ], [ 0, %for.cond1.for.end16_crit_edge.i.i ]
  %a986 = trunc i64 %indvars.iv6.i100.i to i32
  %conv.i101.i = sitofp i32 %a986 to double
  %mul.i102.i = fmul double %conv.i101.i, %div44.i
  %add.i103.i = fadd double %mul.i102.i, %call.i31.i
  %add.ptr7.sum140.i = add i64 %indvars.iv6.i100.i, 403
  %arrayidx.i104.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum140.i
  store double %add.i103.i, double* %arrayidx.i104.i, align 8
  %indvars.iv.next7.i105.i = add i64 %indvars.iv6.i100.i, 1
  %lftr.wideiv116 = trunc i64 %indvars.iv.next7.i105.i to i32
  %exitcond117 = icmp eq i32 %lftr.wideiv116, 201
  br i1 %exitcond117, label %for.body.i.i116.i, label %for.body.i108.i

for.body.i.i116.i:                                ; preds = %for.body.i.i116.i, %for.body.i108.i
  %indvars.iv.i.i110.i = phi i64 [ %indvars.iv.next.i.i113.i, %for.body.i.i116.i ], [ 0, %for.body.i108.i ]
  %add.ptr7.sum141.i = add i64 %indvars.iv.i.i110.i, 403
  %arrayidx.i.i111.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum141.i
  %a987 = load double* %arrayidx.i.i111.i, align 8
  %call.i9.i = call x86_svmlcc double @__ocl_svml_b2_exp1(double %a987) nounwind readnone
  store double %call.i9.i, double* %arrayidx.i.i111.i, align 8
  %indvars.iv.next.i.i113.i = add i64 %indvars.iv.i.i110.i, 1
  %lftr.wideiv118 = trunc i64 %indvars.iv.next.i.i113.i to i32
  %exitcond119 = icmp eq i32 %lftr.wideiv118, 201
  br i1 %exitcond119, label %vdExp.exit.i117.i, label %for.body.i.i116.i

vdExp.exit.i117.i:                                ; preds = %for.body.i.i116.i
  %mul6.i.i = fmul double %a945, %a985
  br label %for.body5.i125.i

for.body5.i125.i:                                 ; preds = %for.body5.i125.i, %vdExp.exit.i117.i
  %indvars.iv.i119.i = phi i64 [ 0, %vdExp.exit.i117.i ], [ %indvars.iv.next.i122.i, %for.body5.i125.i ]
  %arrayidx8.i.i = getelementptr inbounds [301 x double]* %tmp_rslt.i, i64 0, i64 %indvars.iv.i119.i
  %a988 = load double* %arrayidx8.i.i, align 8
  %add9.i.i = fadd double %mul6.i.i, %a988
  %add.ptr7.sum142.i = add i64 %indvars.iv.i119.i, 403
  %arrayidx11.i.i = getelementptr inbounds [805 x double]* %tmp_buff.i, i64 0, i64 %add.ptr7.sum142.i
  %a989 = load double* %arrayidx11.i.i, align 8
  %mul12.i.i = fmul double %div.i.i, %a989
  %a990 = insertelement <8 x double> undef, double %add9.i.i, i32 0
  %a991 = insertelement <8 x double> undef, double %mul12.i.i, i32 0
  %a992 = call <8 x double> @llvm.x86.mic.mask.max.pd(<8 x double> %a990, i8 1, <8 x double> %a990, <8 x double> %a991) nounwind
  %a993 = extractelement <8 x double> %a992, i32 0
  store double %a993, double* %arrayidx8.i.i, align 8
  %indvars.iv.next.i122.i = add i64 %indvars.iv.i119.i, 1
  %lftr.wideiv120 = trunc i64 %indvars.iv.next.i122.i to i32
  %exitcond121 = icmp eq i32 %lftr.wideiv120, 201
  br i1 %exitcond121, label %evalTermCond.exit.i, label %for.body5.i125.i

evalTermCond.exit.i:                              ; preds = %for.body5.i125.i
  %indvars.iv.next16.i = add i64 %indvars.iv15.i, -1
  %indvars.iv.next107 = add i32 %indvars.iv106, 1
  %exitcond122 = icmp eq i32 %indvars.iv.next107, 101
  br i1 %exitcond122, label %for.cond153.preheader.i, label %for.cond102.preheader.i

if.then.1.i:                                      ; preds = %for.inc37.i
  %a999 = load double addrspace(1)* %rk.i, align 8
  store double %a999, double* %arrayidx36.i, align 8
  br label %for.inc37.1.i

for.inc37.1.i:                                    ; preds = %if.then.1.i, %for.inc37.i
  %sub.2.i = fadd double %mul.i, -3.000000e+00
  %call.i7.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.2.i) nounwind readnone
  %cmp32.2.i = fcmp olt double %call.i7.i, 1.000000e-15
  br i1 %cmp32.2.i, label %if.then.2.i, label %for.inc37.2.i

if.then.2.i:                                      ; preds = %for.inc37.1.i
  %a1000 = load double addrspace(1)* %rk.i, align 8
  store double %a1000, double* %arrayidx36.i, align 8
  br label %for.inc37.2.i

for.inc37.2.i:                                    ; preds = %if.then.2.i, %for.inc37.1.i
  %sub.3.i = fadd double %mul.i, -4.000000e+00
  %call.i4.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.3.i) nounwind readnone
  %cmp32.3.i = fcmp olt double %call.i4.i, 1.000000e-15
  br i1 %cmp32.3.i, label %if.then.3.i, label %for.inc37.3.i

if.then.3.i:                                      ; preds = %for.inc37.2.i
  %a1001 = load double addrspace(1)* %rk.i, align 8
  store double %a1001, double* %arrayidx36.i, align 8
  br label %for.inc37.3.i

for.inc37.3.i:                                    ; preds = %if.then.3.i, %for.inc37.2.i
  %sub.4.i = fadd double %mul.i, -5.000000e+00
  %call.i1.i = call x86_svmlcc double @__ocl_svml_b2_fabs1(double %sub.4.i) nounwind readnone
  %cmp32.4.i = fcmp olt double %call.i1.i, 1.000000e-15
  br i1 %cmp32.4.i, label %if.then.4.i, label %for.inc37.4.i

if.then.4.i:                                      ; preds = %for.inc37.3.i
  %a1002 = load double addrspace(1)* %rk.i, align 8
  store double %a1002, double* %arrayidx36.i, align 8
  br label %for.inc37.4.i

for.inc37.4.i:                                    ; preds = %if.then.4.i, %for.inc37.3.i
  %indvars.iv.next19.i = add i64 %indvars.iv18.i, 1
  %lftr.wideiv86 = trunc i64 %indvars.iv.next19.i to i32
  %exitcond87 = icmp eq i32 %lftr.wideiv86, 101
  br i1 %exitcond87, label %for.end42.i, label %for.cond25.preheader.i

__evalConvertibleBond_separated_args.exit:        ; preds = %for.cond153.preheader.i, %scalarIf.i
  ret void
}

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind

declare void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* nocapture, i8* nocapture, i64, i32, i1) nounwind
