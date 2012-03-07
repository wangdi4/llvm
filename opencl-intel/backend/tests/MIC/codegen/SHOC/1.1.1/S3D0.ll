; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__gr_base_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)*, float addrspace(1)*, float, float) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare float @_Z4fmaxff(float, float) nounwind readnone

declare [7 x i64] @__WG.boundaries.gr_base_original(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, float)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare <16 x float> @_Z4fmaxDv16_fS_(<16 x float>, <16 x float>) nounwind readnone

declare void @__gr_base_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)*, float addrspace(1)*, float, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.gr_base(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare <16 x float> @__ocl_svml_b1_fmaxf16(<16 x float>, <16 x float>) nounwind readnone

declare float @__ocl_svml_b1_fmaxf1(float, float) nounwind readnone

define void @gr_base(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(1)**
  %10 = load float addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 36
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %19 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to <{ [4 x i64] }>**
  %22 = load <{ [4 x i64] }>** %21, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 0
  %24 = load i64* %23, align 8
  %25 = getelementptr <{ [4 x i64] }>* %22, i64 0, i32 0, i64 0
  %26 = load i64* %25, align 8
  %27 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 1
  %28 = load i64* %27, align 8
  %29 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %19, i64 0, i32 3, i64 2
  %30 = load i64* %29, align 8
  %vector.size.i = ashr i64 %24, 4
  %num.vector.wi.i = and i64 %24, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %26
  %scalar.size.i = sub i64 %24, %num.vector.wi.i
  %31 = icmp eq i64 %vector.size.i, 0
  br i1 %31, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %temp18vector_func.i = insertelement <16 x float> undef, float %16, i32 0
  %vector19vector_func.i = shufflevector <16 x float> %temp18vector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  %tempvector_func.i = insertelement <16 x float> undef, float %13, i32 0
  %vectorvector_func.i = shufflevector <16 x float> %tempvector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %for.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %for.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %26, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %for.endvector_func.i ]
  %32 = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_vector_tid.i
  %ptrTypeCastvector_func.i = bitcast float addrspace(1)* %32 to <16 x float> addrspace(1)*
  %33 = load <16 x float> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %mul16vector_func.i = fmul <16 x float> %33, %vectorvector_func.i
  %34 = getelementptr inbounds float addrspace(1)* %1, i64 %dim_0_vector_tid.i
  %ptrTypeCast17vector_func.i = bitcast float addrspace(1)* %34 to <16 x float> addrspace(1)*
  %35 = load <16 x float> addrspace(1)* %ptrTypeCast17vector_func.i, align 4
  %mul320vector_func.i = fmul <16 x float> %35, %vector19vector_func.i
  %36 = getelementptr inbounds float addrspace(1)* %7, i64 %dim_0_vector_tid.i
  %ptrTypeCast21vector_func.i = bitcast float addrspace(1)* %36 to <16 x float> addrspace(1)*
  %37 = load <16 x float> addrspace(1)* %ptrTypeCast21vector_func.i, align 4
  %mul622vector_func.i = fmul <16 x float> %37, <float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000, float 0x3FDFBF39E0000000>
  %38 = getelementptr inbounds float addrspace(1)* %10, i64 %dim_0_vector_tid.i
  %ptrTypeCast23vector_func.i = bitcast float addrspace(1)* %38 to <16 x float> addrspace(1)*
  store <16 x float> %mul622vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast23vector_func.i, align 4
  %add1024vector_func.i = fadd <16 x float> %mul622vector_func.i, zeroinitializer
  %extract26vector_func.i = add i64 %dim_0_vector_tid.i, 13824
  %39 = getelementptr inbounds float addrspace(1)* %7, i64 %extract26vector_func.i
  %ptrTypeCast42vector_func.i = bitcast float addrspace(1)* %39 to <16 x float> addrspace(1)*
  %40 = load <16 x float> addrspace(1)* %ptrTypeCast42vector_func.i, align 4
  %mul1443vector_func.i = fmul <16 x float> %40, <float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000, float 0x3FEFBF39E0000000>
  %41 = getelementptr inbounds float addrspace(1)* %10, i64 %extract26vector_func.i
  %ptrTypeCast44vector_func.i = bitcast float addrspace(1)* %41 to <16 x float> addrspace(1)*
  store <16 x float> %mul1443vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast44vector_func.i, align 4
  %add1845vector_func.i = fadd <16 x float> %add1024vector_func.i, %mul1443vector_func.i
  %extract47vector_func.i = add i64 %dim_0_vector_tid.i, 27648
  %42 = getelementptr inbounds float addrspace(1)* %7, i64 %extract47vector_func.i
  %ptrTypeCast63vector_func.i = bitcast float addrspace(1)* %42 to <16 x float> addrspace(1)*
  %43 = load <16 x float> addrspace(1)* %ptrTypeCast63vector_func.i, align 4
  %mul2264vector_func.i = fmul <16 x float> %43, <float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000, float 0x3FB0002760000000>
  %44 = getelementptr inbounds float addrspace(1)* %10, i64 %extract47vector_func.i
  %ptrTypeCast65vector_func.i = bitcast float addrspace(1)* %44 to <16 x float> addrspace(1)*
  store <16 x float> %mul2264vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast65vector_func.i, align 4
  %add2666vector_func.i = fadd <16 x float> %add1845vector_func.i, %mul2264vector_func.i
  %extract68vector_func.i = add i64 %dim_0_vector_tid.i, 41472
  %45 = getelementptr inbounds float addrspace(1)* %7, i64 %extract68vector_func.i
  %ptrTypeCast84vector_func.i = bitcast float addrspace(1)* %45 to <16 x float> addrspace(1)*
  %46 = load <16 x float> addrspace(1)* %ptrTypeCast84vector_func.i, align 4
  %mul3085vector_func.i = fmul <16 x float> %46, <float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000, float 0x3FA0002740000000>
  %47 = getelementptr inbounds float addrspace(1)* %10, i64 %extract68vector_func.i
  %ptrTypeCast86vector_func.i = bitcast float addrspace(1)* %47 to <16 x float> addrspace(1)*
  store <16 x float> %mul3085vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast86vector_func.i, align 4
  %add3487vector_func.i = fadd <16 x float> %add2666vector_func.i, %mul3085vector_func.i
  %extract89vector_func.i = add i64 %dim_0_vector_tid.i, 55296
  %48 = getelementptr inbounds float addrspace(1)* %7, i64 %extract89vector_func.i
  %ptrTypeCast105vector_func.i = bitcast float addrspace(1)* %48 to <16 x float> addrspace(1)*
  %49 = load <16 x float> addrspace(1)* %ptrTypeCast105vector_func.i, align 4
  %mul38106vector_func.i = fmul <16 x float> %49, <float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000, float 0x3FAE1AC6C0000000>
  %50 = getelementptr inbounds float addrspace(1)* %10, i64 %extract89vector_func.i
  %ptrTypeCast107vector_func.i = bitcast float addrspace(1)* %50 to <16 x float> addrspace(1)*
  store <16 x float> %mul38106vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast107vector_func.i, align 4
  %add42108vector_func.i = fadd <16 x float> %add3487vector_func.i, %mul38106vector_func.i
  %extract110vector_func.i = add i64 %dim_0_vector_tid.i, 69120
  %51 = getelementptr inbounds float addrspace(1)* %7, i64 %extract110vector_func.i
  %ptrTypeCast126vector_func.i = bitcast float addrspace(1)* %51 to <16 x float> addrspace(1)*
  %52 = load <16 x float> addrspace(1)* %ptrTypeCast126vector_func.i, align 4
  %mul46127vector_func.i = fmul <16 x float> %52, <float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000, float 0x3FAC6B93C0000000>
  %53 = getelementptr inbounds float addrspace(1)* %10, i64 %extract110vector_func.i
  %ptrTypeCast128vector_func.i = bitcast float addrspace(1)* %53 to <16 x float> addrspace(1)*
  store <16 x float> %mul46127vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast128vector_func.i, align 4
  %add50129vector_func.i = fadd <16 x float> %add42108vector_func.i, %mul46127vector_func.i
  %extract131vector_func.i = add i64 %dim_0_vector_tid.i, 82944
  %54 = getelementptr inbounds float addrspace(1)* %7, i64 %extract131vector_func.i
  %ptrTypeCast147vector_func.i = bitcast float addrspace(1)* %54 to <16 x float> addrspace(1)*
  %55 = load <16 x float> addrspace(1)* %ptrTypeCast147vector_func.i, align 4
  %mul54148vector_func.i = fmul <16 x float> %55, <float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000, float 0x3F9F0620C0000000>
  %56 = getelementptr inbounds float addrspace(1)* %10, i64 %extract131vector_func.i
  %ptrTypeCast149vector_func.i = bitcast float addrspace(1)* %56 to <16 x float> addrspace(1)*
  store <16 x float> %mul54148vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast149vector_func.i, align 4
  %add58150vector_func.i = fadd <16 x float> %add50129vector_func.i, %mul54148vector_func.i
  %extract152vector_func.i = add i64 %dim_0_vector_tid.i, 96768
  %57 = getelementptr inbounds float addrspace(1)* %7, i64 %extract152vector_func.i
  %ptrTypeCast168vector_func.i = bitcast float addrspace(1)* %57 to <16 x float> addrspace(1)*
  %58 = load <16 x float> addrspace(1)* %ptrTypeCast168vector_func.i, align 4
  %mul62169vector_func.i = fmul <16 x float> %58, <float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000, float 0x3F9E1AC6C0000000>
  %59 = getelementptr inbounds float addrspace(1)* %10, i64 %extract152vector_func.i
  %ptrTypeCast170vector_func.i = bitcast float addrspace(1)* %59 to <16 x float> addrspace(1)*
  store <16 x float> %mul62169vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast170vector_func.i, align 4
  %add66171vector_func.i = fadd <16 x float> %add58150vector_func.i, %mul62169vector_func.i
  %extract173vector_func.i = add i64 %dim_0_vector_tid.i, 110592
  %60 = getelementptr inbounds float addrspace(1)* %7, i64 %extract173vector_func.i
  %ptrTypeCast189vector_func.i = bitcast float addrspace(1)* %60 to <16 x float> addrspace(1)*
  %61 = load <16 x float> addrspace(1)* %ptrTypeCast189vector_func.i, align 4
  %mul70190vector_func.i = fmul <16 x float> %61, <float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000, float 0x3FB106E0E0000000>
  %62 = getelementptr inbounds float addrspace(1)* %10, i64 %extract173vector_func.i
  %ptrTypeCast191vector_func.i = bitcast float addrspace(1)* %62 to <16 x float> addrspace(1)*
  store <16 x float> %mul70190vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast191vector_func.i, align 4
  %add74192vector_func.i = fadd <16 x float> %add66171vector_func.i, %mul70190vector_func.i
  %extract194vector_func.i = add i64 %dim_0_vector_tid.i, 124416
  %63 = getelementptr inbounds float addrspace(1)* %7, i64 %extract194vector_func.i
  %ptrTypeCast210vector_func.i = bitcast float addrspace(1)* %63 to <16 x float> addrspace(1)*
  %64 = load <16 x float> addrspace(1)* %ptrTypeCast210vector_func.i, align 4
  %mul78211vector_func.i = fmul <16 x float> %64, <float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000, float 0x3FAFEA0720000000>
  %65 = getelementptr inbounds float addrspace(1)* %10, i64 %extract194vector_func.i
  %ptrTypeCast212vector_func.i = bitcast float addrspace(1)* %65 to <16 x float> addrspace(1)*
  store <16 x float> %mul78211vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast212vector_func.i, align 4
  %add82213vector_func.i = fadd <16 x float> %add74192vector_func.i, %mul78211vector_func.i
  %extract215vector_func.i = add i64 %dim_0_vector_tid.i, 138240
  %66 = getelementptr inbounds float addrspace(1)* %7, i64 %extract215vector_func.i
  %ptrTypeCast231vector_func.i = bitcast float addrspace(1)* %66 to <16 x float> addrspace(1)*
  %67 = load <16 x float> addrspace(1)* %ptrTypeCast231vector_func.i, align 4
  %mul86232vector_func.i = fmul <16 x float> %67, <float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000, float 0x3FA2476140000000>
  %68 = getelementptr inbounds float addrspace(1)* %10, i64 %extract215vector_func.i
  %ptrTypeCast233vector_func.i = bitcast float addrspace(1)* %68 to <16 x float> addrspace(1)*
  store <16 x float> %mul86232vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast233vector_func.i, align 4
  %add90234vector_func.i = fadd <16 x float> %add82213vector_func.i, %mul86232vector_func.i
  %extract236vector_func.i = add i64 %dim_0_vector_tid.i, 152064
  %69 = getelementptr inbounds float addrspace(1)* %7, i64 %extract236vector_func.i
  %ptrTypeCast252vector_func.i = bitcast float addrspace(1)* %69 to <16 x float> addrspace(1)*
  %70 = load <16 x float> addrspace(1)* %ptrTypeCast252vector_func.i, align 4
  %mul94253vector_func.i = fmul <16 x float> %70, <float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000, float 0x3F974478A0000000>
  %71 = getelementptr inbounds float addrspace(1)* %10, i64 %extract236vector_func.i
  %ptrTypeCast254vector_func.i = bitcast float addrspace(1)* %71 to <16 x float> addrspace(1)*
  store <16 x float> %mul94253vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast254vector_func.i, align 4
  %add98255vector_func.i = fadd <16 x float> %add90234vector_func.i, %mul94253vector_func.i
  %extract257vector_func.i = add i64 %dim_0_vector_tid.i, 165888
  %72 = getelementptr inbounds float addrspace(1)* %7, i64 %extract257vector_func.i
  %ptrTypeCast273vector_func.i = bitcast float addrspace(1)* %72 to <16 x float> addrspace(1)*
  %73 = load <16 x float> addrspace(1)* %ptrTypeCast273vector_func.i, align 4
  %mul102274vector_func.i = fmul <16 x float> %73, <float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000, float 0x3FA10D3640000000>
  %74 = getelementptr inbounds float addrspace(1)* %10, i64 %extract257vector_func.i
  %ptrTypeCast275vector_func.i = bitcast float addrspace(1)* %74 to <16 x float> addrspace(1)*
  store <16 x float> %mul102274vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast275vector_func.i, align 4
  %add106276vector_func.i = fadd <16 x float> %add98255vector_func.i, %mul102274vector_func.i
  %extract278vector_func.i = add i64 %dim_0_vector_tid.i, 179712
  %75 = getelementptr inbounds float addrspace(1)* %7, i64 %extract278vector_func.i
  %ptrTypeCast294vector_func.i = bitcast float addrspace(1)* %75 to <16 x float> addrspace(1)*
  %76 = load <16 x float> addrspace(1)* %ptrTypeCast294vector_func.i, align 4
  %mul110295vector_func.i = fmul <16 x float> %76, <float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000, float 0x3FA3A9D3C0000000>
  %77 = getelementptr inbounds float addrspace(1)* %10, i64 %extract278vector_func.i
  %ptrTypeCast296vector_func.i = bitcast float addrspace(1)* %77 to <16 x float> addrspace(1)*
  store <16 x float> %mul110295vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast296vector_func.i, align 4
  %add114297vector_func.i = fadd <16 x float> %add106276vector_func.i, %mul110295vector_func.i
  %extract299vector_func.i = add i64 %dim_0_vector_tid.i, 193536
  %78 = getelementptr inbounds float addrspace(1)* %7, i64 %extract299vector_func.i
  %ptrTypeCast315vector_func.i = bitcast float addrspace(1)* %78 to <16 x float> addrspace(1)*
  %79 = load <16 x float> addrspace(1)* %ptrTypeCast315vector_func.i, align 4
  %mul118316vector_func.i = fmul <16 x float> %79, <float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000, float 0x3FA2401A20000000>
  %80 = getelementptr inbounds float addrspace(1)* %10, i64 %extract299vector_func.i
  %ptrTypeCast317vector_func.i = bitcast float addrspace(1)* %80 to <16 x float> addrspace(1)*
  store <16 x float> %mul118316vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast317vector_func.i, align 4
  %add122318vector_func.i = fadd <16 x float> %add114297vector_func.i, %mul118316vector_func.i
  %extract320vector_func.i = add i64 %dim_0_vector_tid.i, 207360
  %81 = getelementptr inbounds float addrspace(1)* %7, i64 %extract320vector_func.i
  %ptrTypeCast336vector_func.i = bitcast float addrspace(1)* %81 to <16 x float> addrspace(1)*
  %82 = load <16 x float> addrspace(1)* %ptrTypeCast336vector_func.i, align 4
  %mul126337vector_func.i = fmul <16 x float> %82, <float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000, float 0x3FA106E0E0000000>
  %83 = getelementptr inbounds float addrspace(1)* %10, i64 %extract320vector_func.i
  %ptrTypeCast338vector_func.i = bitcast float addrspace(1)* %83 to <16 x float> addrspace(1)*
  store <16 x float> %mul126337vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast338vector_func.i, align 4
  %add130339vector_func.i = fadd <16 x float> %add122318vector_func.i, %mul126337vector_func.i
  %extract341vector_func.i = add i64 %dim_0_vector_tid.i, 221184
  %84 = getelementptr inbounds float addrspace(1)* %7, i64 %extract341vector_func.i
  %ptrTypeCast357vector_func.i = bitcast float addrspace(1)* %84 to <16 x float> addrspace(1)*
  %85 = load <16 x float> addrspace(1)* %ptrTypeCast357vector_func.i, align 4
  %mul134358vector_func.i = fmul <16 x float> %85, <float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000, float 0x3F98F521E0000000>
  %86 = getelementptr inbounds float addrspace(1)* %10, i64 %extract341vector_func.i
  %ptrTypeCast359vector_func.i = bitcast float addrspace(1)* %86 to <16 x float> addrspace(1)*
  store <16 x float> %mul134358vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast359vector_func.i, align 4
  %add138360vector_func.i = fadd <16 x float> %add130339vector_func.i, %mul134358vector_func.i
  %extract362vector_func.i = add i64 %dim_0_vector_tid.i, 235008
  %87 = getelementptr inbounds float addrspace(1)* %7, i64 %extract362vector_func.i
  %ptrTypeCast378vector_func.i = bitcast float addrspace(1)* %87 to <16 x float> addrspace(1)*
  %88 = load <16 x float> addrspace(1)* %ptrTypeCast378vector_func.i, align 4
  %mul142379vector_func.i = fmul <16 x float> %88, <float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000, float 0x3F985BEF60000000>
  %89 = getelementptr inbounds float addrspace(1)* %10, i64 %extract362vector_func.i
  %ptrTypeCast380vector_func.i = bitcast float addrspace(1)* %89 to <16 x float> addrspace(1)*
  store <16 x float> %mul142379vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast380vector_func.i, align 4
  %add146381vector_func.i = fadd <16 x float> %add138360vector_func.i, %mul142379vector_func.i
  %extract383vector_func.i = add i64 %dim_0_vector_tid.i, 248832
  %90 = getelementptr inbounds float addrspace(1)* %7, i64 %extract383vector_func.i
  %ptrTypeCast399vector_func.i = bitcast float addrspace(1)* %90 to <16 x float> addrspace(1)*
  %91 = load <16 x float> addrspace(1)* %ptrTypeCast399vector_func.i, align 4
  %mul150400vector_func.i = fmul <16 x float> %91, <float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000, float 0x3F973E9260000000>
  %92 = getelementptr inbounds float addrspace(1)* %10, i64 %extract383vector_func.i
  %ptrTypeCast401vector_func.i = bitcast float addrspace(1)* %92 to <16 x float> addrspace(1)*
  store <16 x float> %mul150400vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast401vector_func.i, align 4
  %add154402vector_func.i = fadd <16 x float> %add146381vector_func.i, %mul150400vector_func.i
  %extract404vector_func.i = add i64 %dim_0_vector_tid.i, 262656
  %93 = getelementptr inbounds float addrspace(1)* %7, i64 %extract404vector_func.i
  %ptrTypeCast420vector_func.i = bitcast float addrspace(1)* %93 to <16 x float> addrspace(1)*
  %94 = load <16 x float> addrspace(1)* %ptrTypeCast420vector_func.i, align 4
  %mul158421vector_func.i = fmul <16 x float> %94, <float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000, float 0x3F98EE5880000000>
  %95 = getelementptr inbounds float addrspace(1)* %10, i64 %extract404vector_func.i
  %ptrTypeCast422vector_func.i = bitcast float addrspace(1)* %95 to <16 x float> addrspace(1)*
  store <16 x float> %mul158421vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast422vector_func.i, align 4
  %add162423vector_func.i = fadd <16 x float> %add154402vector_func.i, %mul158421vector_func.i
  %extract425vector_func.i = add i64 %dim_0_vector_tid.i, 276480
  %96 = getelementptr inbounds float addrspace(1)* %7, i64 %extract425vector_func.i
  %ptrTypeCast441vector_func.i = bitcast float addrspace(1)* %96 to <16 x float> addrspace(1)*
  %97 = load <16 x float> addrspace(1)* %ptrTypeCast441vector_func.i, align 4
  %mul166442vector_func.i = fmul <16 x float> %97, <float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000, float 0x3F98557840000000>
  %98 = getelementptr inbounds float addrspace(1)* %10, i64 %extract425vector_func.i
  %ptrTypeCast443vector_func.i = bitcast float addrspace(1)* %98 to <16 x float> addrspace(1)*
  store <16 x float> %mul166442vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast443vector_func.i, align 4
  %add170444vector_func.i = fadd <16 x float> %add162423vector_func.i, %mul166442vector_func.i
  %extract446vector_func.i = add i64 %dim_0_vector_tid.i, 290304
  %99 = getelementptr inbounds float addrspace(1)* %7, i64 %extract446vector_func.i
  %ptrTypeCast462vector_func.i = bitcast float addrspace(1)* %99 to <16 x float> addrspace(1)*
  %100 = load <16 x float> addrspace(1)* %ptrTypeCast462vector_func.i, align 4
  %mul174463vector_func.i = fmul <16 x float> %100, <float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000, float 0x3FA246E760000000>
  %101 = getelementptr inbounds float addrspace(1)* %10, i64 %extract446vector_func.i
  %ptrTypeCast464vector_func.i = bitcast float addrspace(1)* %101 to <16 x float> addrspace(1)*
  store <16 x float> %mul174463vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast464vector_func.i, align 4
  %add178465vector_func.i = fadd <16 x float> %add170444vector_func.i, %mul174463vector_func.i
  %mul179466vector_func.i = fmul <16 x float> %add178465vector_func.i, %mul16vector_func.i
  %mul180467vector_func.i = fmul <16 x float> %mul179466vector_func.i, <float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000, float 0x4193D2C640000000>
  %div468vector_func.i = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %mul180467vector_func.i
  %mul181469vector_func.i = fmul <16 x float> %mul320vector_func.i, %div468vector_func.i
  br label %for.bodyvector_func.i

for.bodyvector_func.i:                            ; preds = %for.bodyvector_func.i, %entryvector_func.i
  %indvars.ivvector_func.i = phi i64 [ 1, %entryvector_func.i ], [ %indvars.iv.nextvector_func.i, %for.bodyvector_func.i ]
  %k.01vector_func.i = phi i32 [ 1, %entryvector_func.i ], [ %incvector_func.i, %for.bodyvector_func.i ]
  %102 = mul i32 %k.01vector_func.i, 13824
  %mul182vector_func.i = add i32 %102, -13824
  %convvector_func.i = zext i32 %mul182vector_func.i to i64
  %extract473vector_func.i = add i64 %convvector_func.i, %dim_0_vector_tid.i
  %103 = getelementptr inbounds float addrspace(1)* %10, i64 %extract473vector_func.i
  %ptrTypeCast489vector_func.i = bitcast float addrspace(1)* %103 to <16 x float> addrspace(1)*
  %104 = load <16 x float> addrspace(1)* %ptrTypeCast489vector_func.i, align 4
  %call.i1.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_fmaxf16(<16 x float> %104, <16 x float> <float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000, float 0x3810000000000000>) nounwind readnone
  %mul187490vector_func.i = fmul <16 x float> %call.i1.i, %mul181469vector_func.i
  store <16 x float> %mul187490vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast489vector_func.i, align 4
  %indvars.iv.nextvector_func.i = add i64 %indvars.ivvector_func.i, 1
  %incvector_func.i = add i32 %k.01vector_func.i, 1
  %lftr.wideivvector_func.i = trunc i64 %indvars.iv.nextvector_func.i to i32
  %exitcondvector_func.i = icmp eq i32 %lftr.wideivvector_func.i, 23
  br i1 %exitcondvector_func.i, label %for.endvector_func.i, label %for.bodyvector_func.i

for.endvector_func.i:                             ; preds = %for.bodyvector_func.i
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %for.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %28
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %30
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %105 = icmp eq i64 %24, %num.vector.wi.i
  br i1 %105, label %__gr_base_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %for.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %for.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %for.end.i ]
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_tid.i
  %106 = load float addrspace(1)* %arrayidx.i, align 4
  %mul.i = fmul float %106, %13
  %arrayidx2.i = getelementptr inbounds float addrspace(1)* %1, i64 %dim_0_tid.i
  %107 = load float addrspace(1)* %arrayidx2.i, align 4
  %mul3.i = fmul float %107, %16
  %arrayidx5.i = getelementptr inbounds float addrspace(1)* %7, i64 %dim_0_tid.i
  %108 = load float addrspace(1)* %arrayidx5.i, align 4
  %mul6.i = fmul float %108, 0x3FDFBF39E0000000
  %arrayidx9.i = getelementptr inbounds float addrspace(1)* %10, i64 %dim_0_tid.i
  store float %mul6.i, float addrspace(1)* %arrayidx9.i, align 4
  %add10.i = fadd float %mul6.i, 0.000000e+00
  %add12.i = add i64 %dim_0_tid.i, 13824
  %arrayidx13.i = getelementptr inbounds float addrspace(1)* %7, i64 %add12.i
  %109 = load float addrspace(1)* %arrayidx13.i, align 4
  %mul14.i = fmul float %109, 0x3FEFBF39E0000000
  %arrayidx17.i = getelementptr inbounds float addrspace(1)* %10, i64 %add12.i
  store float %mul14.i, float addrspace(1)* %arrayidx17.i, align 4
  %add18.i = fadd float %add10.i, %mul14.i
  %add20.i = add i64 %dim_0_tid.i, 27648
  %arrayidx21.i = getelementptr inbounds float addrspace(1)* %7, i64 %add20.i
  %110 = load float addrspace(1)* %arrayidx21.i, align 4
  %mul22.i = fmul float %110, 0x3FB0002760000000
  %arrayidx25.i = getelementptr inbounds float addrspace(1)* %10, i64 %add20.i
  store float %mul22.i, float addrspace(1)* %arrayidx25.i, align 4
  %add26.i = fadd float %add18.i, %mul22.i
  %add28.i = add i64 %dim_0_tid.i, 41472
  %arrayidx29.i = getelementptr inbounds float addrspace(1)* %7, i64 %add28.i
  %111 = load float addrspace(1)* %arrayidx29.i, align 4
  %mul30.i = fmul float %111, 0x3FA0002740000000
  %arrayidx33.i = getelementptr inbounds float addrspace(1)* %10, i64 %add28.i
  store float %mul30.i, float addrspace(1)* %arrayidx33.i, align 4
  %add34.i = fadd float %add26.i, %mul30.i
  %add36.i = add i64 %dim_0_tid.i, 55296
  %arrayidx37.i = getelementptr inbounds float addrspace(1)* %7, i64 %add36.i
  %112 = load float addrspace(1)* %arrayidx37.i, align 4
  %mul38.i = fmul float %112, 0x3FAE1AC6C0000000
  %arrayidx41.i = getelementptr inbounds float addrspace(1)* %10, i64 %add36.i
  store float %mul38.i, float addrspace(1)* %arrayidx41.i, align 4
  %add42.i = fadd float %add34.i, %mul38.i
  %add44.i = add i64 %dim_0_tid.i, 69120
  %arrayidx45.i = getelementptr inbounds float addrspace(1)* %7, i64 %add44.i
  %113 = load float addrspace(1)* %arrayidx45.i, align 4
  %mul46.i = fmul float %113, 0x3FAC6B93C0000000
  %arrayidx49.i = getelementptr inbounds float addrspace(1)* %10, i64 %add44.i
  store float %mul46.i, float addrspace(1)* %arrayidx49.i, align 4
  %add50.i = fadd float %add42.i, %mul46.i
  %add52.i = add i64 %dim_0_tid.i, 82944
  %arrayidx53.i = getelementptr inbounds float addrspace(1)* %7, i64 %add52.i
  %114 = load float addrspace(1)* %arrayidx53.i, align 4
  %mul54.i = fmul float %114, 0x3F9F0620C0000000
  %arrayidx57.i = getelementptr inbounds float addrspace(1)* %10, i64 %add52.i
  store float %mul54.i, float addrspace(1)* %arrayidx57.i, align 4
  %add58.i = fadd float %add50.i, %mul54.i
  %add60.i = add i64 %dim_0_tid.i, 96768
  %arrayidx61.i = getelementptr inbounds float addrspace(1)* %7, i64 %add60.i
  %115 = load float addrspace(1)* %arrayidx61.i, align 4
  %mul62.i = fmul float %115, 0x3F9E1AC6C0000000
  %arrayidx65.i = getelementptr inbounds float addrspace(1)* %10, i64 %add60.i
  store float %mul62.i, float addrspace(1)* %arrayidx65.i, align 4
  %add66.i = fadd float %add58.i, %mul62.i
  %add68.i = add i64 %dim_0_tid.i, 110592
  %arrayidx69.i = getelementptr inbounds float addrspace(1)* %7, i64 %add68.i
  %116 = load float addrspace(1)* %arrayidx69.i, align 4
  %mul70.i = fmul float %116, 0x3FB106E0E0000000
  %arrayidx73.i = getelementptr inbounds float addrspace(1)* %10, i64 %add68.i
  store float %mul70.i, float addrspace(1)* %arrayidx73.i, align 4
  %add74.i = fadd float %add66.i, %mul70.i
  %add76.i = add i64 %dim_0_tid.i, 124416
  %arrayidx77.i = getelementptr inbounds float addrspace(1)* %7, i64 %add76.i
  %117 = load float addrspace(1)* %arrayidx77.i, align 4
  %mul78.i = fmul float %117, 0x3FAFEA0720000000
  %arrayidx81.i = getelementptr inbounds float addrspace(1)* %10, i64 %add76.i
  store float %mul78.i, float addrspace(1)* %arrayidx81.i, align 4
  %add82.i = fadd float %add74.i, %mul78.i
  %add84.i = add i64 %dim_0_tid.i, 138240
  %arrayidx85.i = getelementptr inbounds float addrspace(1)* %7, i64 %add84.i
  %118 = load float addrspace(1)* %arrayidx85.i, align 4
  %mul86.i = fmul float %118, 0x3FA2476140000000
  %arrayidx89.i = getelementptr inbounds float addrspace(1)* %10, i64 %add84.i
  store float %mul86.i, float addrspace(1)* %arrayidx89.i, align 4
  %add90.i = fadd float %add82.i, %mul86.i
  %add92.i = add i64 %dim_0_tid.i, 152064
  %arrayidx93.i = getelementptr inbounds float addrspace(1)* %7, i64 %add92.i
  %119 = load float addrspace(1)* %arrayidx93.i, align 4
  %mul94.i = fmul float %119, 0x3F974478A0000000
  %arrayidx97.i = getelementptr inbounds float addrspace(1)* %10, i64 %add92.i
  store float %mul94.i, float addrspace(1)* %arrayidx97.i, align 4
  %add98.i = fadd float %add90.i, %mul94.i
  %add100.i = add i64 %dim_0_tid.i, 165888
  %arrayidx101.i = getelementptr inbounds float addrspace(1)* %7, i64 %add100.i
  %120 = load float addrspace(1)* %arrayidx101.i, align 4
  %mul102.i = fmul float %120, 0x3FA10D3640000000
  %arrayidx105.i = getelementptr inbounds float addrspace(1)* %10, i64 %add100.i
  store float %mul102.i, float addrspace(1)* %arrayidx105.i, align 4
  %add106.i = fadd float %add98.i, %mul102.i
  %add108.i = add i64 %dim_0_tid.i, 179712
  %arrayidx109.i = getelementptr inbounds float addrspace(1)* %7, i64 %add108.i
  %121 = load float addrspace(1)* %arrayidx109.i, align 4
  %mul110.i = fmul float %121, 0x3FA3A9D3C0000000
  %arrayidx113.i = getelementptr inbounds float addrspace(1)* %10, i64 %add108.i
  store float %mul110.i, float addrspace(1)* %arrayidx113.i, align 4
  %add114.i = fadd float %add106.i, %mul110.i
  %add116.i = add i64 %dim_0_tid.i, 193536
  %arrayidx117.i = getelementptr inbounds float addrspace(1)* %7, i64 %add116.i
  %122 = load float addrspace(1)* %arrayidx117.i, align 4
  %mul118.i = fmul float %122, 0x3FA2401A20000000
  %arrayidx121.i = getelementptr inbounds float addrspace(1)* %10, i64 %add116.i
  store float %mul118.i, float addrspace(1)* %arrayidx121.i, align 4
  %add122.i = fadd float %add114.i, %mul118.i
  %add124.i = add i64 %dim_0_tid.i, 207360
  %arrayidx125.i = getelementptr inbounds float addrspace(1)* %7, i64 %add124.i
  %123 = load float addrspace(1)* %arrayidx125.i, align 4
  %mul126.i = fmul float %123, 0x3FA106E0E0000000
  %arrayidx129.i = getelementptr inbounds float addrspace(1)* %10, i64 %add124.i
  store float %mul126.i, float addrspace(1)* %arrayidx129.i, align 4
  %add130.i = fadd float %add122.i, %mul126.i
  %add132.i = add i64 %dim_0_tid.i, 221184
  %arrayidx133.i = getelementptr inbounds float addrspace(1)* %7, i64 %add132.i
  %124 = load float addrspace(1)* %arrayidx133.i, align 4
  %mul134.i = fmul float %124, 0x3F98F521E0000000
  %arrayidx137.i = getelementptr inbounds float addrspace(1)* %10, i64 %add132.i
  store float %mul134.i, float addrspace(1)* %arrayidx137.i, align 4
  %add138.i = fadd float %add130.i, %mul134.i
  %add140.i = add i64 %dim_0_tid.i, 235008
  %arrayidx141.i = getelementptr inbounds float addrspace(1)* %7, i64 %add140.i
  %125 = load float addrspace(1)* %arrayidx141.i, align 4
  %mul142.i = fmul float %125, 0x3F985BEF60000000
  %arrayidx145.i = getelementptr inbounds float addrspace(1)* %10, i64 %add140.i
  store float %mul142.i, float addrspace(1)* %arrayidx145.i, align 4
  %add146.i = fadd float %add138.i, %mul142.i
  %add148.i = add i64 %dim_0_tid.i, 248832
  %arrayidx149.i = getelementptr inbounds float addrspace(1)* %7, i64 %add148.i
  %126 = load float addrspace(1)* %arrayidx149.i, align 4
  %mul150.i = fmul float %126, 0x3F973E9260000000
  %arrayidx153.i = getelementptr inbounds float addrspace(1)* %10, i64 %add148.i
  store float %mul150.i, float addrspace(1)* %arrayidx153.i, align 4
  %add154.i = fadd float %add146.i, %mul150.i
  %add156.i = add i64 %dim_0_tid.i, 262656
  %arrayidx157.i = getelementptr inbounds float addrspace(1)* %7, i64 %add156.i
  %127 = load float addrspace(1)* %arrayidx157.i, align 4
  %mul158.i = fmul float %127, 0x3F98EE5880000000
  %arrayidx161.i = getelementptr inbounds float addrspace(1)* %10, i64 %add156.i
  store float %mul158.i, float addrspace(1)* %arrayidx161.i, align 4
  %add162.i = fadd float %add154.i, %mul158.i
  %add164.i = add i64 %dim_0_tid.i, 276480
  %arrayidx165.i = getelementptr inbounds float addrspace(1)* %7, i64 %add164.i
  %128 = load float addrspace(1)* %arrayidx165.i, align 4
  %mul166.i = fmul float %128, 0x3F98557840000000
  %arrayidx169.i = getelementptr inbounds float addrspace(1)* %10, i64 %add164.i
  store float %mul166.i, float addrspace(1)* %arrayidx169.i, align 4
  %add170.i = fadd float %add162.i, %mul166.i
  %add172.i = add i64 %dim_0_tid.i, 290304
  %arrayidx173.i = getelementptr inbounds float addrspace(1)* %7, i64 %add172.i
  %129 = load float addrspace(1)* %arrayidx173.i, align 4
  %mul174.i = fmul float %129, 0x3FA246E760000000
  %arrayidx177.i = getelementptr inbounds float addrspace(1)* %10, i64 %add172.i
  store float %mul174.i, float addrspace(1)* %arrayidx177.i, align 4
  %add178.i = fadd float %add170.i, %mul174.i
  %mul179.i = fmul float %add178.i, %mul.i
  %mul180.i = fmul float %mul179.i, 0x4193D2C640000000
  %div.i = fdiv float 1.000000e+00, %mul180.i
  %mul181.i = fmul float %mul3.i, %div.i
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %scalar_kernel_entry.i
  %indvars.iv.i = phi i64 [ 1, %scalar_kernel_entry.i ], [ %indvars.iv.next.i, %for.body.i ]
  %k.01.i = phi i32 [ 1, %scalar_kernel_entry.i ], [ %inc.i, %for.body.i ]
  %130 = mul i32 %k.01.i, 13824
  %mul182.i = add i32 %130, -13824
  %conv.i = zext i32 %mul182.i to i64
  %add184.i = add i64 %conv.i, %dim_0_tid.i
  %arrayidx185.i = getelementptr inbounds float addrspace(1)* %10, i64 %add184.i
  %131 = load float addrspace(1)* %arrayidx185.i, align 4
  %call.i.i = call x86_svmlcc float @__ocl_svml_b1_fmaxf1(float %131, float 0x3810000000000000) nounwind readnone
  %mul187.i = fmul float %call.i.i, %mul181.i
  store float %mul187.i, float addrspace(1)* %arrayidx185.i, align 4
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %inc.i = add i32 %k.01.i, 1
  %lftr.wideiv.i = trunc i64 %indvars.iv.next.i to i32
  %exitcond.i = icmp eq i32 %lftr.wideiv.i, 23
  br i1 %exitcond.i, label %for.end.i, label %for.body.i

for.end.i:                                        ; preds = %for.body.i
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %for.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %28
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %30
  br i1 %dim_2_cmp.to.max.i, label %__gr_base_separated_args.exit, label %dim_1_pre_head.i

__gr_base_separated_args.exit:                    ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__gr_base_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"gr_base"}
!4 = metadata !{void (i8*)* @gr_base}
