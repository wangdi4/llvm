; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__rdsmh_kernel_original(float addrspace(1)* nocapture, float addrspace(1)*, float) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare float @_Z3logf(float) nounwind readnone

declare float @_Z3expf(float) nounwind readnone

declare [7 x i64] @__WG.boundaries.rdsmh_kernel_original(float addrspace(1)*, float addrspace(1)*, float)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare float @maskedf_0__Z3expf(i1, float)

declare void @masked_store_align4_0(i1, float, float addrspace(1)*)

declare float @maskedf_1__Z3expf(i1, float)

declare void @masked_store_align4_1(i1, float, float addrspace(1)*)

declare float @maskedf_2__Z3expf(i1, float)

declare void @masked_store_align4_2(i1, float, float addrspace(1)*)

declare float @maskedf_3__Z3expf(i1, float)

declare void @masked_store_align4_3(i1, float, float addrspace(1)*)

declare float @maskedf_4__Z3expf(i1, float)

declare void @masked_store_align4_4(i1, float, float addrspace(1)*)

declare float @maskedf_5__Z3expf(i1, float)

declare void @masked_store_align4_5(i1, float, float addrspace(1)*)

declare float @maskedf_6__Z3expf(i1, float)

declare void @masked_store_align4_6(i1, float, float addrspace(1)*)

declare float @maskedf_7__Z3expf(i1, float)

declare void @masked_store_align4_7(i1, float, float addrspace(1)*)

declare float @maskedf_8__Z3expf(i1, float)

declare void @masked_store_align4_8(i1, float, float addrspace(1)*)

declare float @maskedf_9__Z3expf(i1, float)

declare void @masked_store_align4_9(i1, float, float addrspace(1)*)

declare float @maskedf_10__Z3expf(i1, float)

declare void @masked_store_align4_10(i1, float, float addrspace(1)*)

declare float @maskedf_11__Z3expf(i1, float)

declare void @masked_store_align4_11(i1, float, float addrspace(1)*)

declare float @maskedf_12__Z3expf(i1, float)

declare void @masked_store_align4_12(i1, float, float addrspace(1)*)

declare float @maskedf_13__Z3expf(i1, float)

declare void @masked_store_align4_13(i1, float, float addrspace(1)*)

declare float @maskedf_14__Z3expf(i1, float)

declare void @masked_store_align4_14(i1, float, float addrspace(1)*)

declare float @maskedf_15__Z3expf(i1, float)

declare void @masked_store_align4_15(i1, float, float addrspace(1)*)

declare float @maskedf_16__Z3expf(i1, float)

declare void @masked_store_align4_16(i1, float, float addrspace(1)*)

declare float @maskedf_17__Z3expf(i1, float)

declare void @masked_store_align4_17(i1, float, float addrspace(1)*)

declare float @maskedf_18__Z3expf(i1, float)

declare void @masked_store_align4_18(i1, float, float addrspace(1)*)

declare float @maskedf_19__Z3expf(i1, float)

declare void @masked_store_align4_19(i1, float, float addrspace(1)*)

declare float @maskedf_20__Z3expf(i1, float)

declare void @masked_store_align4_20(i1, float, float addrspace(1)*)

declare float @maskedf_21__Z3expf(i1, float)

declare void @masked_store_align4_21(i1, float, float addrspace(1)*)

declare float @maskedf_22__Z3expf(i1, float)

declare void @masked_store_align4_22(i1, float, float addrspace(1)*)

declare float @maskedf_23__Z3expf(i1, float)

declare void @masked_store_align4_23(i1, float, float addrspace(1)*)

declare float @maskedf_24__Z3expf(i1, float)

declare void @masked_store_align4_24(i1, float, float addrspace(1)*)

declare float @maskedf_25__Z3expf(i1, float)

declare void @masked_store_align4_25(i1, float, float addrspace(1)*)

declare float @maskedf_26__Z3expf(i1, float)

declare void @masked_store_align4_26(i1, float, float addrspace(1)*)

declare float @maskedf_27__Z3expf(i1, float)

declare void @masked_store_align4_27(i1, float, float addrspace(1)*)

declare float @maskedf_28__Z3expf(i1, float)

declare void @masked_store_align4_28(i1, float, float addrspace(1)*)

declare float @maskedf_29__Z3expf(i1, float)

declare void @masked_store_align4_29(i1, float, float addrspace(1)*)

declare float @maskedf_30__Z3expf(i1, float)

declare void @masked_store_align4_30(i1, float, float addrspace(1)*)

declare float @maskedf_31__Z3expf(i1, float)

declare void @masked_store_align4_31(i1, float, float addrspace(1)*)

declare float @maskedf_32__Z3expf(i1, float)

declare void @masked_store_align4_32(i1, float, float addrspace(1)*)

declare float @maskedf_33__Z3expf(i1, float)

declare void @masked_store_align4_33(i1, float, float addrspace(1)*)

declare float @maskedf_34__Z3expf(i1, float)

declare void @masked_store_align4_34(i1, float, float addrspace(1)*)

declare float @maskedf_35__Z3expf(i1, float)

declare void @masked_store_align4_35(i1, float, float addrspace(1)*)

declare float @maskedf_36__Z3expf(i1, float)

declare void @masked_store_align4_36(i1, float, float addrspace(1)*)

declare float @maskedf_37__Z3expf(i1, float)

declare void @masked_store_align4_37(i1, float, float addrspace(1)*)

declare float @maskedf_38__Z3expf(i1, float)

declare void @masked_store_align4_38(i1, float, float addrspace(1)*)

declare float @maskedf_39__Z3expf(i1, float)

declare void @masked_store_align4_39(i1, float, float addrspace(1)*)

declare float @maskedf_40__Z3expf(i1, float)

declare void @masked_store_align4_40(i1, float, float addrspace(1)*)

declare float @maskedf_41__Z3expf(i1, float)

declare void @masked_store_align4_41(i1, float, float addrspace(1)*)

declare float @maskedf_42__Z3expf(i1, float)

declare void @masked_store_align4_42(i1, float, float addrspace(1)*)

declare float @maskedf_43__Z3expf(i1, float)

declare void @masked_store_align4_43(i1, float, float addrspace(1)*)

declare float @maskedf_44__Z3expf(i1, float)

declare void @masked_store_align4_44(i1, float, float addrspace(1)*)

declare float @maskedf_45__Z3expf(i1, float)

declare void @masked_store_align4_45(i1, float, float addrspace(1)*)

declare float @maskedf_46__Z3expf(i1, float)

declare void @masked_store_align4_46(i1, float, float addrspace(1)*)

declare float @maskedf_47__Z3expf(i1, float)

declare void @masked_store_align4_47(i1, float, float addrspace(1)*)

declare float @maskedf_48__Z3expf(i1, float)

declare void @masked_store_align4_48(i1, float, float addrspace(1)*)

declare float @maskedf_49__Z3expf(i1, float)

declare void @masked_store_align4_49(i1, float, float addrspace(1)*)

declare float @maskedf_50__Z3expf(i1, float)

declare void @masked_store_align4_50(i1, float, float addrspace(1)*)

declare float @maskedf_51__Z3expf(i1, float)

declare void @masked_store_align4_51(i1, float, float addrspace(1)*)

declare float @maskedf_52__Z3expf(i1, float)

declare void @masked_store_align4_52(i1, float, float addrspace(1)*)

declare float @maskedf_53__Z3expf(i1, float)

declare void @masked_store_align4_53(i1, float, float addrspace(1)*)

declare float @maskedf_54__Z3expf(i1, float)

declare void @masked_store_align4_54(i1, float, float addrspace(1)*)

declare float @maskedf_55__Z3expf(i1, float)

declare void @masked_store_align4_55(i1, float, float addrspace(1)*)

declare float @maskedf_56__Z3expf(i1, float)

declare void @masked_store_align4_56(i1, float, float addrspace(1)*)

declare float @maskedf_57__Z3expf(i1, float)

declare void @masked_store_align4_57(i1, float, float addrspace(1)*)

declare float @maskedf_58__Z3expf(i1, float)

declare void @masked_store_align4_58(i1, float, float addrspace(1)*)

declare float @maskedf_59__Z3expf(i1, float)

declare void @masked_store_align4_59(i1, float, float addrspace(1)*)

declare float @maskedf_60__Z3expf(i1, float)

declare void @masked_store_align4_60(i1, float, float addrspace(1)*)

declare float @maskedf_61__Z3expf(i1, float)

declare void @masked_store_align4_61(i1, float, float addrspace(1)*)

declare <16 x float> @_Z3logDv16_f(<16 x float>) nounwind readnone

declare <16 x float> @_Z3expDv16_f(<16 x float>) nounwind readnone

declare void @masked_store_align4_62(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_63(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_64(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_65(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_66(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_67(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_68(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_69(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_70(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_71(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_72(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_73(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_74(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_75(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_76(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_77(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_78(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_79(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_80(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_81(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_82(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_83(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_84(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_85(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_86(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_87(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_88(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_89(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_90(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_91(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_92(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_93(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_94(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_95(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_96(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_97(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_98(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_99(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_100(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_101(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_102(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_103(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_104(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_105(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_106(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_107(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_108(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_109(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_110(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_111(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_112(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_113(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_114(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_115(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_116(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_117(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_118(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_119(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_120(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_121(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_122(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @masked_store_align4_123(<16 x i1>, <16 x float>, <16 x float> addrspace(1)*)

declare void @__rdsmh_kernel_separated_args(float addrspace(1)* nocapture, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.rdsmh_kernel(float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare <16 x float> @__ocl_svml_b1_expf16(<16 x float>) nounwind readnone

declare <16 x float> @__ocl_svml_b1_logf16(<16 x float>) nounwind readnone

declare float @__ocl_svml_b1_expf1(float) nounwind readnone

declare float @__ocl_svml_b1_logf1(float) nounwind readnone

define void @rdsmh_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float*
  %7 = load float* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %10 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to <{ [4 x i64] }>**
  %13 = load <{ [4 x i64] }>** %12, align 8
  %14 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 0
  %15 = load i64* %14, align 8
  %16 = getelementptr <{ [4 x i64] }>* %13, i64 0, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 1
  %19 = load i64* %18, align 8
  %20 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %10, i64 0, i32 3, i64 2
  %21 = load i64* %20, align 8
  %vector.size.i = ashr i64 %15, 4
  %num.vector.wi.i = and i64 %15, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %17
  %scalar.size.i = sub i64 %15, %num.vector.wi.i
  %22 = icmp eq i64 %vector.size.i, 0
  br i1 %22, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %tempvector_func.i = insertelement <16 x float> undef, float %7, i32 0
  %vectorvector_func.i = shufflevector <16 x float> %tempvector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %if.endvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %if.endvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %17, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %if.endvector_func.i ]
  %23 = getelementptr inbounds float addrspace(1)* %1, i64 %dim_0_vector_tid.i
  %ptrTypeCastvector_func.i = bitcast float addrspace(1)* %23 to <16 x float> addrspace(1)*
  %24 = load <16 x float> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %mul567vector_func.i = fmul <16 x float> %24, %vectorvector_func.i
  %call.i1.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_logf16(<16 x float> %mul567vector_func.i) nounwind readnone
  %div568vector_func.i = fdiv <16 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, %mul567vector_func.i
  %sub569vector_func.i = fadd <16 x float> %call.i1.i, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  %cmpvector_func.i = fcmp ogt <16 x float> %mul567vector_func.i, <float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03, float 1.000000e+03>
  %Mneg570vector_func.i = xor <16 x i1> %cmpvector_func.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %mul2573vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000, float 0x408DB14580000000>
  %add575vector_func.i = fadd <16 x float> %mul2573vector_func.i, <float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000, float 0xC009A3E340000000>
  %mul3576vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000, float 0x400AB2BF60000000>
  %add4577vector_func.i = fadd <16 x float> %add575vector_func.i, %mul3576vector_func.i
  %mul.i578vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000, float 0x3CD2099320000000>
  %add.i579vector_func.i = fadd <16 x float> %mul.i578vector_func.i, <float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000, float 0xBDB073F440000000>
  %mul1.i580vector_func.i = fmul <16 x float> %add.i579vector_func.i, %mul567vector_func.i
  %add2.i581vector_func.i = fadd <16 x float> %mul1.i580vector_func.i, <float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000, float 0x3E765866C0000000>
  %mul3.i582vector_func.i = fmul <16 x float> %add2.i581vector_func.i, %mul567vector_func.i
  %add4.i583vector_func.i = fadd <16 x float> %mul3.i582vector_func.i, <float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000, float 0xBEF9E6B000000000>
  %mul5.i584vector_func.i = fmul <16 x float> %add4.i583vector_func.i, %mul567vector_func.i
  %add6585vector_func.i = fadd <16 x float> %add4577vector_func.i, %mul5.i584vector_func.i
  %call.i2.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add6585vector_func.i) nounwind readnone
  %exmaskvector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 0
  br i1 %exmaskvector_func.i, label %preload6038vector_func.i, label %postload6039vector_func.i

preload6038vector_func.i:                         ; preds = %entryvector_func.i
  %25 = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_vector_tid.i
  %exDatavector_func.i = extractelement <16 x float> %call.i2.i, i32 0
  store float %exDatavector_func.i, float addrspace(1)* %25, align 4
  br label %postload6039vector_func.i

postload6039vector_func.i:                        ; preds = %preload6038vector_func.i, %entryvector_func.i
  %exmask2402vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 1
  br i1 %exmask2402vector_func.i, label %preload7256vector_func.i, label %postload7257vector_func.i

preload7256vector_func.i:                         ; preds = %postload6039vector_func.i
  %.sum9336vector_func.i = add i64 %dim_0_vector_tid.i, 1
  %26 = getelementptr float addrspace(1)* %4, i64 %.sum9336vector_func.i
  %exData2403vector_func.i = extractelement <16 x float> %call.i2.i, i32 1
  store float %exData2403vector_func.i, float addrspace(1)* %26, align 4
  br label %postload7257vector_func.i

postload7257vector_func.i:                        ; preds = %preload7256vector_func.i, %postload6039vector_func.i
  %exmask2405vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 2
  br i1 %exmask2405vector_func.i, label %preload6935vector_func.i, label %postload6936vector_func.i

preload6935vector_func.i:                         ; preds = %postload7257vector_func.i
  %.sum9335vector_func.i = add i64 %dim_0_vector_tid.i, 2
  %27 = getelementptr float addrspace(1)* %4, i64 %.sum9335vector_func.i
  %exData2406vector_func.i = extractelement <16 x float> %call.i2.i, i32 2
  store float %exData2406vector_func.i, float addrspace(1)* %27, align 4
  br label %postload6936vector_func.i

postload6936vector_func.i:                        ; preds = %preload6935vector_func.i, %postload7257vector_func.i
  %exmask2408vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 3
  br i1 %exmask2408vector_func.i, label %preload6863vector_func.i, label %postload6864vector_func.i

preload6863vector_func.i:                         ; preds = %postload6936vector_func.i
  %.sum9334vector_func.i = add i64 %dim_0_vector_tid.i, 3
  %28 = getelementptr float addrspace(1)* %4, i64 %.sum9334vector_func.i
  %exData2409vector_func.i = extractelement <16 x float> %call.i2.i, i32 3
  store float %exData2409vector_func.i, float addrspace(1)* %28, align 4
  br label %postload6864vector_func.i

postload6864vector_func.i:                        ; preds = %preload6863vector_func.i, %postload6936vector_func.i
  %exmask2411vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 4
  br i1 %exmask2411vector_func.i, label %preload5945vector_func.i, label %postload5946vector_func.i

preload5945vector_func.i:                         ; preds = %postload6864vector_func.i
  %.sum9333vector_func.i = add i64 %dim_0_vector_tid.i, 4
  %29 = getelementptr float addrspace(1)* %4, i64 %.sum9333vector_func.i
  %exData2412vector_func.i = extractelement <16 x float> %call.i2.i, i32 4
  store float %exData2412vector_func.i, float addrspace(1)* %29, align 4
  br label %postload5946vector_func.i

postload5946vector_func.i:                        ; preds = %preload5945vector_func.i, %postload6864vector_func.i
  %exmask2414vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 5
  br i1 %exmask2414vector_func.i, label %preload5783vector_func.i, label %postload5784vector_func.i

preload5783vector_func.i:                         ; preds = %postload5946vector_func.i
  %.sum9332vector_func.i = add i64 %dim_0_vector_tid.i, 5
  %30 = getelementptr float addrspace(1)* %4, i64 %.sum9332vector_func.i
  %exData2415vector_func.i = extractelement <16 x float> %call.i2.i, i32 5
  store float %exData2415vector_func.i, float addrspace(1)* %30, align 4
  br label %postload5784vector_func.i

postload5784vector_func.i:                        ; preds = %preload5783vector_func.i, %postload5946vector_func.i
  %exmask2417vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 6
  br i1 %exmask2417vector_func.i, label %preload6998vector_func.i, label %postload6999vector_func.i

preload6998vector_func.i:                         ; preds = %postload5784vector_func.i
  %.sum9331vector_func.i = add i64 %dim_0_vector_tid.i, 6
  %31 = getelementptr float addrspace(1)* %4, i64 %.sum9331vector_func.i
  %exData2418vector_func.i = extractelement <16 x float> %call.i2.i, i32 6
  store float %exData2418vector_func.i, float addrspace(1)* %31, align 4
  br label %postload6999vector_func.i

postload6999vector_func.i:                        ; preds = %preload6998vector_func.i, %postload5784vector_func.i
  %exmask2420vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 7
  br i1 %exmask2420vector_func.i, label %preload7361vector_func.i, label %postload7362vector_func.i

preload7361vector_func.i:                         ; preds = %postload6999vector_func.i
  %.sum9330vector_func.i = add i64 %dim_0_vector_tid.i, 7
  %32 = getelementptr float addrspace(1)* %4, i64 %.sum9330vector_func.i
  %exData2421vector_func.i = extractelement <16 x float> %call.i2.i, i32 7
  store float %exData2421vector_func.i, float addrspace(1)* %32, align 4
  br label %postload7362vector_func.i

postload7362vector_func.i:                        ; preds = %preload7361vector_func.i, %postload6999vector_func.i
  %exmask2423vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 8
  br i1 %exmask2423vector_func.i, label %preloadvector_func.i, label %postloadvector_func.i

preloadvector_func.i:                             ; preds = %postload7362vector_func.i
  %.sum9329vector_func.i = add i64 %dim_0_vector_tid.i, 8
  %33 = getelementptr float addrspace(1)* %4, i64 %.sum9329vector_func.i
  %exData2424vector_func.i = extractelement <16 x float> %call.i2.i, i32 8
  store float %exData2424vector_func.i, float addrspace(1)* %33, align 4
  br label %postloadvector_func.i

postloadvector_func.i:                            ; preds = %preloadvector_func.i, %postload7362vector_func.i
  %exmask2426vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 9
  br i1 %exmask2426vector_func.i, label %preload5453vector_func.i, label %postload5454vector_func.i

preload5453vector_func.i:                         ; preds = %postloadvector_func.i
  %.sum9328vector_func.i = add i64 %dim_0_vector_tid.i, 9
  %34 = getelementptr float addrspace(1)* %4, i64 %.sum9328vector_func.i
  %exData2427vector_func.i = extractelement <16 x float> %call.i2.i, i32 9
  store float %exData2427vector_func.i, float addrspace(1)* %34, align 4
  br label %postload5454vector_func.i

postload5454vector_func.i:                        ; preds = %preload5453vector_func.i, %postloadvector_func.i
  %exmask2429vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 10
  br i1 %exmask2429vector_func.i, label %preload5435vector_func.i, label %postload5436vector_func.i

preload5435vector_func.i:                         ; preds = %postload5454vector_func.i
  %.sum9327vector_func.i = add i64 %dim_0_vector_tid.i, 10
  %35 = getelementptr float addrspace(1)* %4, i64 %.sum9327vector_func.i
  %exData2430vector_func.i = extractelement <16 x float> %call.i2.i, i32 10
  store float %exData2430vector_func.i, float addrspace(1)* %35, align 4
  br label %postload5436vector_func.i

postload5436vector_func.i:                        ; preds = %preload5435vector_func.i, %postload5454vector_func.i
  %exmask2432vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 11
  br i1 %exmask2432vector_func.i, label %preload5450vector_func.i, label %postload5451vector_func.i

preload5450vector_func.i:                         ; preds = %postload5436vector_func.i
  %.sum9326vector_func.i = add i64 %dim_0_vector_tid.i, 11
  %36 = getelementptr float addrspace(1)* %4, i64 %.sum9326vector_func.i
  %exData2433vector_func.i = extractelement <16 x float> %call.i2.i, i32 11
  store float %exData2433vector_func.i, float addrspace(1)* %36, align 4
  br label %postload5451vector_func.i

postload5451vector_func.i:                        ; preds = %preload5450vector_func.i, %postload5436vector_func.i
  %exmask2435vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 12
  br i1 %exmask2435vector_func.i, label %preload5456vector_func.i, label %postload5457vector_func.i

preload5456vector_func.i:                         ; preds = %postload5451vector_func.i
  %.sum9325vector_func.i = add i64 %dim_0_vector_tid.i, 12
  %37 = getelementptr float addrspace(1)* %4, i64 %.sum9325vector_func.i
  %exData2436vector_func.i = extractelement <16 x float> %call.i2.i, i32 12
  store float %exData2436vector_func.i, float addrspace(1)* %37, align 4
  br label %postload5457vector_func.i

postload5457vector_func.i:                        ; preds = %preload5456vector_func.i, %postload5451vector_func.i
  %exmask2438vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 13
  br i1 %exmask2438vector_func.i, label %preload5447vector_func.i, label %postload5448vector_func.i

preload5447vector_func.i:                         ; preds = %postload5457vector_func.i
  %.sum9324vector_func.i = add i64 %dim_0_vector_tid.i, 13
  %38 = getelementptr float addrspace(1)* %4, i64 %.sum9324vector_func.i
  %exData2439vector_func.i = extractelement <16 x float> %call.i2.i, i32 13
  store float %exData2439vector_func.i, float addrspace(1)* %38, align 4
  br label %postload5448vector_func.i

postload5448vector_func.i:                        ; preds = %preload5447vector_func.i, %postload5457vector_func.i
  %exmask2441vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 14
  br i1 %exmask2441vector_func.i, label %preload5462vector_func.i, label %postload5463vector_func.i

preload5462vector_func.i:                         ; preds = %postload5448vector_func.i
  %.sum9323vector_func.i = add i64 %dim_0_vector_tid.i, 14
  %39 = getelementptr float addrspace(1)* %4, i64 %.sum9323vector_func.i
  %exData2442vector_func.i = extractelement <16 x float> %call.i2.i, i32 14
  store float %exData2442vector_func.i, float addrspace(1)* %39, align 4
  br label %postload5463vector_func.i

postload5463vector_func.i:                        ; preds = %preload5462vector_func.i, %postload5448vector_func.i
  %exmask2444vector_func.i = extractelement <16 x i1> %cmpvector_func.i, i32 15
  br i1 %exmask2444vector_func.i, label %preload5438vector_func.i, label %postload5439vector_func.i

preload5438vector_func.i:                         ; preds = %postload5463vector_func.i
  %.sum9322vector_func.i = add i64 %dim_0_vector_tid.i, 15
  %40 = getelementptr float addrspace(1)* %4, i64 %.sum9322vector_func.i
  %exData2445vector_func.i = extractelement <16 x float> %call.i2.i, i32 15
  store float %exData2445vector_func.i, float addrspace(1)* %40, align 4
  br label %postload5439vector_func.i

postload5439vector_func.i:                        ; preds = %preload5438vector_func.i, %postload5463vector_func.i
  %mul11587vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000, float 0x40D8E06A40000000>
  %sub12588vector_func.i = fsub <16 x float> <float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000, float 0xBFDC9673E0000000>, %mul11587vector_func.i
  %mul13589vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00, float 2.500000e+00>
  %add14590vector_func.i = fadd <16 x float> %sub12588vector_func.i, %mul13589vector_func.i
  %mul.i421591vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000, float 0x3B3E1D3B00000000>
  %add.i422592vector_func.i = fadd <16 x float> %mul.i421591vector_func.i, <float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000, float 0xBC1D1DB540000000>
  %mul1.i423593vector_func.i = fmul <16 x float> %add.i422592vector_func.i, %mul567vector_func.i
  %add2.i424594vector_func.i = fadd <16 x float> %mul1.i423593vector_func.i, <float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000, float 0x3CE840F100000000>
  %mul3.i425595vector_func.i = fmul <16 x float> %add2.i424594vector_func.i, %mul567vector_func.i
  %add4.i426596vector_func.i = fadd <16 x float> %mul3.i425595vector_func.i, <float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000, float 0xBDA961A6E0000000>
  %mul5.i427597vector_func.i = fmul <16 x float> %add4.i426596vector_func.i, %mul567vector_func.i
  %add16598vector_func.i = fadd <16 x float> %add14590vector_func.i, %mul5.i427597vector_func.i
  %call.i3.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add16598vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5441vector_func.i, label %postload5442vector_func.i

preload5441vector_func.i:                         ; preds = %postload5439vector_func.i
  %extract600vector_func.i = add i64 %dim_0_vector_tid.i, 13824
  %41 = getelementptr inbounds float addrspace(1)* %4, i64 %extract600vector_func.i
  %exData2449vector_func.i = extractelement <16 x float> %call.i3.i, i32 0
  store float %exData2449vector_func.i, float addrspace(1)* %41, align 4
  br label %postload5442vector_func.i

postload5442vector_func.i:                        ; preds = %preload5441vector_func.i, %postload5439vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6101vector_func.i, label %postload6102vector_func.i

preload6101vector_func.i:                         ; preds = %postload5442vector_func.i
  %.sum9321vector_func.i = add i64 %dim_0_vector_tid.i, 13825
  %42 = getelementptr float addrspace(1)* %4, i64 %.sum9321vector_func.i
  %exData2452vector_func.i = extractelement <16 x float> %call.i3.i, i32 1
  store float %exData2452vector_func.i, float addrspace(1)* %42, align 4
  br label %postload6102vector_func.i

postload6102vector_func.i:                        ; preds = %preload6101vector_func.i, %postload5442vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6131vector_func.i, label %postload6132vector_func.i

preload6131vector_func.i:                         ; preds = %postload6102vector_func.i
  %.sum9320vector_func.i = add i64 %dim_0_vector_tid.i, 13826
  %43 = getelementptr float addrspace(1)* %4, i64 %.sum9320vector_func.i
  %exData2455vector_func.i = extractelement <16 x float> %call.i3.i, i32 2
  store float %exData2455vector_func.i, float addrspace(1)* %43, align 4
  br label %postload6132vector_func.i

postload6132vector_func.i:                        ; preds = %preload6131vector_func.i, %postload6102vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6134vector_func.i, label %postload6135vector_func.i

preload6134vector_func.i:                         ; preds = %postload6132vector_func.i
  %.sum9319vector_func.i = add i64 %dim_0_vector_tid.i, 13827
  %44 = getelementptr float addrspace(1)* %4, i64 %.sum9319vector_func.i
  %exData2458vector_func.i = extractelement <16 x float> %call.i3.i, i32 3
  store float %exData2458vector_func.i, float addrspace(1)* %44, align 4
  br label %postload6135vector_func.i

postload6135vector_func.i:                        ; preds = %preload6134vector_func.i, %postload6132vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6050vector_func.i, label %postload6051vector_func.i

preload6050vector_func.i:                         ; preds = %postload6135vector_func.i
  %.sum9318vector_func.i = add i64 %dim_0_vector_tid.i, 13828
  %45 = getelementptr float addrspace(1)* %4, i64 %.sum9318vector_func.i
  %exData2461vector_func.i = extractelement <16 x float> %call.i3.i, i32 4
  store float %exData2461vector_func.i, float addrspace(1)* %45, align 4
  br label %postload6051vector_func.i

postload6051vector_func.i:                        ; preds = %preload6050vector_func.i, %postload6135vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6110vector_func.i, label %postload6111vector_func.i

preload6110vector_func.i:                         ; preds = %postload6051vector_func.i
  %.sum9317vector_func.i = add i64 %dim_0_vector_tid.i, 13829
  %46 = getelementptr float addrspace(1)* %4, i64 %.sum9317vector_func.i
  %exData2464vector_func.i = extractelement <16 x float> %call.i3.i, i32 5
  store float %exData2464vector_func.i, float addrspace(1)* %46, align 4
  br label %postload6111vector_func.i

postload6111vector_func.i:                        ; preds = %preload6110vector_func.i, %postload6051vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6056vector_func.i, label %postload6057vector_func.i

preload6056vector_func.i:                         ; preds = %postload6111vector_func.i
  %.sum9316vector_func.i = add i64 %dim_0_vector_tid.i, 13830
  %47 = getelementptr float addrspace(1)* %4, i64 %.sum9316vector_func.i
  %exData2467vector_func.i = extractelement <16 x float> %call.i3.i, i32 6
  store float %exData2467vector_func.i, float addrspace(1)* %47, align 4
  br label %postload6057vector_func.i

postload6057vector_func.i:                        ; preds = %preload6056vector_func.i, %postload6111vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6062vector_func.i, label %postload6063vector_func.i

preload6062vector_func.i:                         ; preds = %postload6057vector_func.i
  %.sum9315vector_func.i = add i64 %dim_0_vector_tid.i, 13831
  %48 = getelementptr float addrspace(1)* %4, i64 %.sum9315vector_func.i
  %exData2470vector_func.i = extractelement <16 x float> %call.i3.i, i32 7
  store float %exData2470vector_func.i, float addrspace(1)* %48, align 4
  br label %postload6063vector_func.i

postload6063vector_func.i:                        ; preds = %preload6062vector_func.i, %postload6057vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6068vector_func.i, label %postload6069vector_func.i

preload6068vector_func.i:                         ; preds = %postload6063vector_func.i
  %.sum9314vector_func.i = add i64 %dim_0_vector_tid.i, 13832
  %49 = getelementptr float addrspace(1)* %4, i64 %.sum9314vector_func.i
  %exData2473vector_func.i = extractelement <16 x float> %call.i3.i, i32 8
  store float %exData2473vector_func.i, float addrspace(1)* %49, align 4
  br label %postload6069vector_func.i

postload6069vector_func.i:                        ; preds = %preload6068vector_func.i, %postload6063vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6074vector_func.i, label %postload6075vector_func.i

preload6074vector_func.i:                         ; preds = %postload6069vector_func.i
  %.sum9313vector_func.i = add i64 %dim_0_vector_tid.i, 13833
  %50 = getelementptr float addrspace(1)* %4, i64 %.sum9313vector_func.i
  %exData2476vector_func.i = extractelement <16 x float> %call.i3.i, i32 9
  store float %exData2476vector_func.i, float addrspace(1)* %50, align 4
  br label %postload6075vector_func.i

postload6075vector_func.i:                        ; preds = %preload6074vector_func.i, %postload6069vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6080vector_func.i, label %postload6081vector_func.i

preload6080vector_func.i:                         ; preds = %postload6075vector_func.i
  %.sum9312vector_func.i = add i64 %dim_0_vector_tid.i, 13834
  %51 = getelementptr float addrspace(1)* %4, i64 %.sum9312vector_func.i
  %exData2479vector_func.i = extractelement <16 x float> %call.i3.i, i32 10
  store float %exData2479vector_func.i, float addrspace(1)* %51, align 4
  br label %postload6081vector_func.i

postload6081vector_func.i:                        ; preds = %preload6080vector_func.i, %postload6075vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6083vector_func.i, label %postload6084vector_func.i

preload6083vector_func.i:                         ; preds = %postload6081vector_func.i
  %.sum9311vector_func.i = add i64 %dim_0_vector_tid.i, 13835
  %52 = getelementptr float addrspace(1)* %4, i64 %.sum9311vector_func.i
  %exData2482vector_func.i = extractelement <16 x float> %call.i3.i, i32 11
  store float %exData2482vector_func.i, float addrspace(1)* %52, align 4
  br label %postload6084vector_func.i

postload6084vector_func.i:                        ; preds = %preload6083vector_func.i, %postload6081vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6089vector_func.i, label %postload6090vector_func.i

preload6089vector_func.i:                         ; preds = %postload6084vector_func.i
  %.sum9310vector_func.i = add i64 %dim_0_vector_tid.i, 13836
  %53 = getelementptr float addrspace(1)* %4, i64 %.sum9310vector_func.i
  %exData2485vector_func.i = extractelement <16 x float> %call.i3.i, i32 12
  store float %exData2485vector_func.i, float addrspace(1)* %53, align 4
  br label %postload6090vector_func.i

postload6090vector_func.i:                        ; preds = %preload6089vector_func.i, %postload6084vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6116vector_func.i, label %postload6117vector_func.i

preload6116vector_func.i:                         ; preds = %postload6090vector_func.i
  %.sum9309vector_func.i = add i64 %dim_0_vector_tid.i, 13837
  %54 = getelementptr float addrspace(1)* %4, i64 %.sum9309vector_func.i
  %exData2488vector_func.i = extractelement <16 x float> %call.i3.i, i32 13
  store float %exData2488vector_func.i, float addrspace(1)* %54, align 4
  br label %postload6117vector_func.i

postload6117vector_func.i:                        ; preds = %preload6116vector_func.i, %postload6090vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6041vector_func.i, label %postload6042vector_func.i

preload6041vector_func.i:                         ; preds = %postload6117vector_func.i
  %.sum9308vector_func.i = add i64 %dim_0_vector_tid.i, 13838
  %55 = getelementptr float addrspace(1)* %4, i64 %.sum9308vector_func.i
  %exData2491vector_func.i = extractelement <16 x float> %call.i3.i, i32 14
  store float %exData2491vector_func.i, float addrspace(1)* %55, align 4
  br label %postload6042vector_func.i

postload6042vector_func.i:                        ; preds = %preload6041vector_func.i, %postload6117vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6095vector_func.i, label %postload6096vector_func.i

preload6095vector_func.i:                         ; preds = %postload6042vector_func.i
  %.sum9307vector_func.i = add i64 %dim_0_vector_tid.i, 13839
  %56 = getelementptr float addrspace(1)* %4, i64 %.sum9307vector_func.i
  %exData2494vector_func.i = extractelement <16 x float> %call.i3.i, i32 15
  store float %exData2494vector_func.i, float addrspace(1)* %56, align 4
  br label %postload6096vector_func.i

postload6096vector_func.i:                        ; preds = %preload6095vector_func.i, %postload6042vector_func.i
  %mul21617vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000, float 0x40DC886500000000>
  %sub22618vector_func.i = fsub <16 x float> <float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000, float 0x40132329A0000000>, %mul21617vector_func.i
  %mul23619vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000, float 0x40048E2C80000000>
  %add24620vector_func.i = fadd <16 x float> %sub22618vector_func.i, %mul23619vector_func.i
  %mul.i414621vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000, float 0x3C91B3C360000000>
  %add.i415622vector_func.i = fadd <16 x float> %mul.i414621vector_func.i, <float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000, float 0xBD6D5F5860000000>
  %mul1.i416623vector_func.i = fmul <16 x float> %add.i415622vector_func.i, %mul567vector_func.i
  %add2.i417624vector_func.i = fadd <16 x float> %mul1.i416623vector_func.i, <float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000, float 0x3E3E0722E0000000>
  %mul3.i418625vector_func.i = fmul <16 x float> %add2.i417624vector_func.i, %mul567vector_func.i
  %add4.i419626vector_func.i = fadd <16 x float> %mul3.i418625vector_func.i, <float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000, float 0xBF0689A000000000>
  %mul5.i420627vector_func.i = fmul <16 x float> %add4.i419626vector_func.i, %mul567vector_func.i
  %add26628vector_func.i = fadd <16 x float> %add24620vector_func.i, %mul5.i420627vector_func.i
  %call.i4.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add26628vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6125vector_func.i, label %postload6126vector_func.i

preload6125vector_func.i:                         ; preds = %postload6096vector_func.i
  %extract630vector_func.i = add i64 %dim_0_vector_tid.i, 27648
  %57 = getelementptr inbounds float addrspace(1)* %4, i64 %extract630vector_func.i
  %exData2498vector_func.i = extractelement <16 x float> %call.i4.i, i32 0
  store float %exData2498vector_func.i, float addrspace(1)* %57, align 4
  br label %postload6126vector_func.i

postload6126vector_func.i:                        ; preds = %preload6125vector_func.i, %postload6096vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7448vector_func.i, label %postload7449vector_func.i

preload7448vector_func.i:                         ; preds = %postload6126vector_func.i
  %.sum9306vector_func.i = add i64 %dim_0_vector_tid.i, 27649
  %58 = getelementptr float addrspace(1)* %4, i64 %.sum9306vector_func.i
  %exData2501vector_func.i = extractelement <16 x float> %call.i4.i, i32 1
  store float %exData2501vector_func.i, float addrspace(1)* %58, align 4
  br label %postload7449vector_func.i

postload7449vector_func.i:                        ; preds = %preload7448vector_func.i, %postload6126vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7127vector_func.i, label %postload7128vector_func.i

preload7127vector_func.i:                         ; preds = %postload7449vector_func.i
  %.sum9305vector_func.i = add i64 %dim_0_vector_tid.i, 27650
  %59 = getelementptr float addrspace(1)* %4, i64 %.sum9305vector_func.i
  %exData2504vector_func.i = extractelement <16 x float> %call.i4.i, i32 2
  store float %exData2504vector_func.i, float addrspace(1)* %59, align 4
  br label %postload7128vector_func.i

postload7128vector_func.i:                        ; preds = %preload7127vector_func.i, %postload7449vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6521vector_func.i, label %postload6522vector_func.i

preload6521vector_func.i:                         ; preds = %postload7128vector_func.i
  %.sum9304vector_func.i = add i64 %dim_0_vector_tid.i, 27651
  %60 = getelementptr float addrspace(1)* %4, i64 %.sum9304vector_func.i
  %exData2507vector_func.i = extractelement <16 x float> %call.i4.i, i32 3
  store float %exData2507vector_func.i, float addrspace(1)* %60, align 4
  br label %postload6522vector_func.i

postload6522vector_func.i:                        ; preds = %preload6521vector_func.i, %postload7128vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6269vector_func.i, label %postload6270vector_func.i

preload6269vector_func.i:                         ; preds = %postload6522vector_func.i
  %.sum9303vector_func.i = add i64 %dim_0_vector_tid.i, 27652
  %61 = getelementptr float addrspace(1)* %4, i64 %.sum9303vector_func.i
  %exData2510vector_func.i = extractelement <16 x float> %call.i4.i, i32 4
  store float %exData2510vector_func.i, float addrspace(1)* %61, align 4
  br label %postload6270vector_func.i

postload6270vector_func.i:                        ; preds = %preload6269vector_func.i, %postload6522vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6128vector_func.i, label %postload6129vector_func.i

preload6128vector_func.i:                         ; preds = %postload6270vector_func.i
  %.sum9302vector_func.i = add i64 %dim_0_vector_tid.i, 27653
  %62 = getelementptr float addrspace(1)* %4, i64 %.sum9302vector_func.i
  %exData2513vector_func.i = extractelement <16 x float> %call.i4.i, i32 5
  store float %exData2513vector_func.i, float addrspace(1)* %62, align 4
  br label %postload6129vector_func.i

postload6129vector_func.i:                        ; preds = %preload6128vector_func.i, %postload6270vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6047vector_func.i, label %postload6048vector_func.i

preload6047vector_func.i:                         ; preds = %postload6129vector_func.i
  %.sum9301vector_func.i = add i64 %dim_0_vector_tid.i, 27654
  %63 = getelementptr float addrspace(1)* %4, i64 %.sum9301vector_func.i
  %exData2516vector_func.i = extractelement <16 x float> %call.i4.i, i32 6
  store float %exData2516vector_func.i, float addrspace(1)* %63, align 4
  br label %postload6048vector_func.i

postload6048vector_func.i:                        ; preds = %preload6047vector_func.i, %postload6129vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6104vector_func.i, label %postload6105vector_func.i

preload6104vector_func.i:                         ; preds = %postload6048vector_func.i
  %.sum9300vector_func.i = add i64 %dim_0_vector_tid.i, 27655
  %64 = getelementptr float addrspace(1)* %4, i64 %.sum9300vector_func.i
  %exData2519vector_func.i = extractelement <16 x float> %call.i4.i, i32 7
  store float %exData2519vector_func.i, float addrspace(1)* %64, align 4
  br label %postload6105vector_func.i

postload6105vector_func.i:                        ; preds = %preload6104vector_func.i, %postload6048vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6107vector_func.i, label %postload6108vector_func.i

preload6107vector_func.i:                         ; preds = %postload6105vector_func.i
  %.sum9299vector_func.i = add i64 %dim_0_vector_tid.i, 27656
  %65 = getelementptr float addrspace(1)* %4, i64 %.sum9299vector_func.i
  %exData2522vector_func.i = extractelement <16 x float> %call.i4.i, i32 8
  store float %exData2522vector_func.i, float addrspace(1)* %65, align 4
  br label %postload6108vector_func.i

postload6108vector_func.i:                        ; preds = %preload6107vector_func.i, %postload6105vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6053vector_func.i, label %postload6054vector_func.i

preload6053vector_func.i:                         ; preds = %postload6108vector_func.i
  %.sum9298vector_func.i = add i64 %dim_0_vector_tid.i, 27657
  %66 = getelementptr float addrspace(1)* %4, i64 %.sum9298vector_func.i
  %exData2525vector_func.i = extractelement <16 x float> %call.i4.i, i32 9
  store float %exData2525vector_func.i, float addrspace(1)* %66, align 4
  br label %postload6054vector_func.i

postload6054vector_func.i:                        ; preds = %preload6053vector_func.i, %postload6108vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6059vector_func.i, label %postload6060vector_func.i

preload6059vector_func.i:                         ; preds = %postload6054vector_func.i
  %.sum9297vector_func.i = add i64 %dim_0_vector_tid.i, 27658
  %67 = getelementptr float addrspace(1)* %4, i64 %.sum9297vector_func.i
  %exData2528vector_func.i = extractelement <16 x float> %call.i4.i, i32 10
  store float %exData2528vector_func.i, float addrspace(1)* %67, align 4
  br label %postload6060vector_func.i

postload6060vector_func.i:                        ; preds = %preload6059vector_func.i, %postload6054vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6065vector_func.i, label %postload6066vector_func.i

preload6065vector_func.i:                         ; preds = %postload6060vector_func.i
  %.sum9296vector_func.i = add i64 %dim_0_vector_tid.i, 27659
  %68 = getelementptr float addrspace(1)* %4, i64 %.sum9296vector_func.i
  %exData2531vector_func.i = extractelement <16 x float> %call.i4.i, i32 11
  store float %exData2531vector_func.i, float addrspace(1)* %68, align 4
  br label %postload6066vector_func.i

postload6066vector_func.i:                        ; preds = %preload6065vector_func.i, %postload6060vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6071vector_func.i, label %postload6072vector_func.i

preload6071vector_func.i:                         ; preds = %postload6066vector_func.i
  %.sum9295vector_func.i = add i64 %dim_0_vector_tid.i, 27660
  %69 = getelementptr float addrspace(1)* %4, i64 %.sum9295vector_func.i
  %exData2534vector_func.i = extractelement <16 x float> %call.i4.i, i32 12
  store float %exData2534vector_func.i, float addrspace(1)* %69, align 4
  br label %postload6072vector_func.i

postload6072vector_func.i:                        ; preds = %preload6071vector_func.i, %postload6066vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6077vector_func.i, label %postload6078vector_func.i

preload6077vector_func.i:                         ; preds = %postload6072vector_func.i
  %.sum9294vector_func.i = add i64 %dim_0_vector_tid.i, 27661
  %70 = getelementptr float addrspace(1)* %4, i64 %.sum9294vector_func.i
  %exData2537vector_func.i = extractelement <16 x float> %call.i4.i, i32 13
  store float %exData2537vector_func.i, float addrspace(1)* %70, align 4
  br label %postload6078vector_func.i

postload6078vector_func.i:                        ; preds = %preload6077vector_func.i, %postload6072vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6113vector_func.i, label %postload6114vector_func.i

preload6113vector_func.i:                         ; preds = %postload6078vector_func.i
  %.sum9293vector_func.i = add i64 %dim_0_vector_tid.i, 27662
  %71 = getelementptr float addrspace(1)* %4, i64 %.sum9293vector_func.i
  %exData2540vector_func.i = extractelement <16 x float> %call.i4.i, i32 14
  store float %exData2540vector_func.i, float addrspace(1)* %71, align 4
  br label %postload6114vector_func.i

postload6114vector_func.i:                        ; preds = %preload6113vector_func.i, %postload6078vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6086vector_func.i, label %postload6087vector_func.i

preload6086vector_func.i:                         ; preds = %postload6114vector_func.i
  %.sum9292vector_func.i = add i64 %dim_0_vector_tid.i, 27663
  %72 = getelementptr float addrspace(1)* %4, i64 %.sum9292vector_func.i
  %exData2543vector_func.i = extractelement <16 x float> %call.i4.i, i32 15
  store float %exData2543vector_func.i, float addrspace(1)* %72, align 4
  br label %postload6087vector_func.i

postload6087vector_func.i:                        ; preds = %preload6086vector_func.i, %postload6114vector_func.i
  %mul31647vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000, float 0x409101D4C0000000>
  %add32648vector_func.i = fadd <16 x float> %mul31647vector_func.i, <float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000, float 0x4015D01BE0000000>
  %mul33649vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000, float 0x400A42A340000000>
  %add34650vector_func.i = fadd <16 x float> %add32648vector_func.i, %mul33649vector_func.i
  %mul.i407651vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000, float 0xBCD3852C00000000>
  %add.i408652vector_func.i = fadd <16 x float> %mul.i407651vector_func.i, <float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000, float 0x3DB33164A0000000>
  %mul1.i409653vector_func.i = fmul <16 x float> %add.i408652vector_func.i, %mul567vector_func.i
  %add2.i410654vector_func.i = fadd <16 x float> %mul1.i409653vector_func.i, <float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000, float 0xBE80F496E0000000>
  %mul3.i411655vector_func.i = fmul <16 x float> %add2.i410654vector_func.i, %mul567vector_func.i
  %add4.i412656vector_func.i = fadd <16 x float> %mul3.i411655vector_func.i, <float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000, float 0x3F484C8520000000>
  %mul5.i413657vector_func.i = fmul <16 x float> %add4.i412656vector_func.i, %mul567vector_func.i
  %add36658vector_func.i = fadd <16 x float> %add34650vector_func.i, %mul5.i413657vector_func.i
  %call.i5.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add36658vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6092vector_func.i, label %postload6093vector_func.i

preload6092vector_func.i:                         ; preds = %postload6087vector_func.i
  %extract660vector_func.i = add i64 %dim_0_vector_tid.i, 41472
  %73 = getelementptr inbounds float addrspace(1)* %4, i64 %extract660vector_func.i
  %exData2547vector_func.i = extractelement <16 x float> %call.i5.i, i32 0
  store float %exData2547vector_func.i, float addrspace(1)* %73, align 4
  br label %postload6093vector_func.i

postload6093vector_func.i:                        ; preds = %preload6092vector_func.i, %postload6087vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6119vector_func.i, label %postload6120vector_func.i

preload6119vector_func.i:                         ; preds = %postload6093vector_func.i
  %.sum9291vector_func.i = add i64 %dim_0_vector_tid.i, 41473
  %74 = getelementptr float addrspace(1)* %4, i64 %.sum9291vector_func.i
  %exData2550vector_func.i = extractelement <16 x float> %call.i5.i, i32 1
  store float %exData2550vector_func.i, float addrspace(1)* %74, align 4
  br label %postload6120vector_func.i

postload6120vector_func.i:                        ; preds = %preload6119vector_func.i, %postload6093vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6122vector_func.i, label %postload6123vector_func.i

preload6122vector_func.i:                         ; preds = %postload6120vector_func.i
  %.sum9290vector_func.i = add i64 %dim_0_vector_tid.i, 41474
  %75 = getelementptr float addrspace(1)* %4, i64 %.sum9290vector_func.i
  %exData2553vector_func.i = extractelement <16 x float> %call.i5.i, i32 2
  store float %exData2553vector_func.i, float addrspace(1)* %75, align 4
  br label %postload6123vector_func.i

postload6123vector_func.i:                        ; preds = %preload6122vector_func.i, %postload6120vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6044vector_func.i, label %postload6045vector_func.i

preload6044vector_func.i:                         ; preds = %postload6123vector_func.i
  %.sum9289vector_func.i = add i64 %dim_0_vector_tid.i, 41475
  %76 = getelementptr float addrspace(1)* %4, i64 %.sum9289vector_func.i
  %exData2556vector_func.i = extractelement <16 x float> %call.i5.i, i32 3
  store float %exData2556vector_func.i, float addrspace(1)* %76, align 4
  br label %postload6045vector_func.i

postload6045vector_func.i:                        ; preds = %preload6044vector_func.i, %postload6123vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6098vector_func.i, label %postload6099vector_func.i

preload6098vector_func.i:                         ; preds = %postload6045vector_func.i
  %.sum9288vector_func.i = add i64 %dim_0_vector_tid.i, 41476
  %77 = getelementptr float addrspace(1)* %4, i64 %.sum9288vector_func.i
  %exData2559vector_func.i = extractelement <16 x float> %call.i5.i, i32 4
  store float %exData2559vector_func.i, float addrspace(1)* %77, align 4
  br label %postload6099vector_func.i

postload6099vector_func.i:                        ; preds = %preload6098vector_func.i, %postload6045vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6203vector_func.i, label %postload6204vector_func.i

preload6203vector_func.i:                         ; preds = %postload6099vector_func.i
  %.sum9287vector_func.i = add i64 %dim_0_vector_tid.i, 41477
  %78 = getelementptr float addrspace(1)* %4, i64 %.sum9287vector_func.i
  %exData2562vector_func.i = extractelement <16 x float> %call.i5.i, i32 5
  store float %exData2562vector_func.i, float addrspace(1)* %78, align 4
  br label %postload6204vector_func.i

postload6204vector_func.i:                        ; preds = %preload6203vector_func.i, %postload6099vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5777vector_func.i, label %postload5778vector_func.i

preload5777vector_func.i:                         ; preds = %postload6204vector_func.i
  %.sum9286vector_func.i = add i64 %dim_0_vector_tid.i, 41478
  %79 = getelementptr float addrspace(1)* %4, i64 %.sum9286vector_func.i
  %exData2565vector_func.i = extractelement <16 x float> %call.i5.i, i32 6
  store float %exData2565vector_func.i, float addrspace(1)* %79, align 4
  br label %postload5778vector_func.i

postload5778vector_func.i:                        ; preds = %preload5777vector_func.i, %postload6204vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5810vector_func.i, label %postload5811vector_func.i

preload5810vector_func.i:                         ; preds = %postload5778vector_func.i
  %.sum9285vector_func.i = add i64 %dim_0_vector_tid.i, 41479
  %80 = getelementptr float addrspace(1)* %4, i64 %.sum9285vector_func.i
  %exData2568vector_func.i = extractelement <16 x float> %call.i5.i, i32 7
  store float %exData2568vector_func.i, float addrspace(1)* %80, align 4
  br label %postload5811vector_func.i

postload5811vector_func.i:                        ; preds = %preload5810vector_func.i, %postload5778vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5828vector_func.i, label %postload5829vector_func.i

preload5828vector_func.i:                         ; preds = %postload5811vector_func.i
  %.sum9284vector_func.i = add i64 %dim_0_vector_tid.i, 41480
  %81 = getelementptr float addrspace(1)* %4, i64 %.sum9284vector_func.i
  %exData2571vector_func.i = extractelement <16 x float> %call.i5.i, i32 8
  store float %exData2571vector_func.i, float addrspace(1)* %81, align 4
  br label %postload5829vector_func.i

postload5829vector_func.i:                        ; preds = %preload5828vector_func.i, %postload5811vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5843vector_func.i, label %postload5844vector_func.i

preload5843vector_func.i:                         ; preds = %postload5829vector_func.i
  %.sum9283vector_func.i = add i64 %dim_0_vector_tid.i, 41481
  %82 = getelementptr float addrspace(1)* %4, i64 %.sum9283vector_func.i
  %exData2574vector_func.i = extractelement <16 x float> %call.i5.i, i32 9
  store float %exData2574vector_func.i, float addrspace(1)* %82, align 4
  br label %postload5844vector_func.i

postload5844vector_func.i:                        ; preds = %preload5843vector_func.i, %postload5829vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5759vector_func.i, label %postload5760vector_func.i

preload5759vector_func.i:                         ; preds = %postload5844vector_func.i
  %.sum9282vector_func.i = add i64 %dim_0_vector_tid.i, 41482
  %83 = getelementptr float addrspace(1)* %4, i64 %.sum9282vector_func.i
  %exData2577vector_func.i = extractelement <16 x float> %call.i5.i, i32 10
  store float %exData2577vector_func.i, float addrspace(1)* %83, align 4
  br label %postload5760vector_func.i

postload5760vector_func.i:                        ; preds = %preload5759vector_func.i, %postload5844vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5768vector_func.i, label %postload5769vector_func.i

preload5768vector_func.i:                         ; preds = %postload5760vector_func.i
  %.sum9281vector_func.i = add i64 %dim_0_vector_tid.i, 41483
  %84 = getelementptr float addrspace(1)* %4, i64 %.sum9281vector_func.i
  %exData2580vector_func.i = extractelement <16 x float> %call.i5.i, i32 11
  store float %exData2580vector_func.i, float addrspace(1)* %84, align 4
  br label %postload5769vector_func.i

postload5769vector_func.i:                        ; preds = %preload5768vector_func.i, %postload5760vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5507vector_func.i, label %postload5508vector_func.i

preload5507vector_func.i:                         ; preds = %postload5769vector_func.i
  %.sum9280vector_func.i = add i64 %dim_0_vector_tid.i, 41484
  %85 = getelementptr float addrspace(1)* %4, i64 %.sum9280vector_func.i
  %exData2583vector_func.i = extractelement <16 x float> %call.i5.i, i32 12
  store float %exData2583vector_func.i, float addrspace(1)* %85, align 4
  br label %postload5508vector_func.i

postload5508vector_func.i:                        ; preds = %preload5507vector_func.i, %postload5769vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5549vector_func.i, label %postload5550vector_func.i

preload5549vector_func.i:                         ; preds = %postload5508vector_func.i
  %.sum9279vector_func.i = add i64 %dim_0_vector_tid.i, 41485
  %86 = getelementptr float addrspace(1)* %4, i64 %.sum9279vector_func.i
  %exData2586vector_func.i = extractelement <16 x float> %call.i5.i, i32 13
  store float %exData2586vector_func.i, float addrspace(1)* %86, align 4
  br label %postload5550vector_func.i

postload5550vector_func.i:                        ; preds = %preload5549vector_func.i, %postload5508vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5636vector_func.i, label %postload5637vector_func.i

preload5636vector_func.i:                         ; preds = %postload5550vector_func.i
  %.sum9278vector_func.i = add i64 %dim_0_vector_tid.i, 41486
  %87 = getelementptr float addrspace(1)* %4, i64 %.sum9278vector_func.i
  %exData2589vector_func.i = extractelement <16 x float> %call.i5.i, i32 14
  store float %exData2589vector_func.i, float addrspace(1)* %87, align 4
  br label %postload5637vector_func.i

postload5637vector_func.i:                        ; preds = %preload5636vector_func.i, %postload5550vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5627vector_func.i, label %postload5628vector_func.i

preload5627vector_func.i:                         ; preds = %postload5637vector_func.i
  %.sum9277vector_func.i = add i64 %dim_0_vector_tid.i, 41487
  %88 = getelementptr float addrspace(1)* %4, i64 %.sum9277vector_func.i
  %exData2592vector_func.i = extractelement <16 x float> %call.i5.i, i32 15
  store float %exData2592vector_func.i, float addrspace(1)* %88, align 4
  br label %postload5628vector_func.i

postload5628vector_func.i:                        ; preds = %preload5627vector_func.i, %postload5637vector_func.i
  %mul41677vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000, float 0x40AE255060000000>
  %sub42678vector_func.i = fsub <16 x float> <float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000, float 0x4011E82300000000>, %mul41677vector_func.i
  %mul43679vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000, float 0x4008BE3BE0000000>
  %add44680vector_func.i = fadd <16 x float> %sub42678vector_func.i, %mul43679vector_func.i
  %mul.i400681vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000, float 0x3CC526B0A0000000>
  %add.i401682vector_func.i = fadd <16 x float> %mul.i400681vector_func.i, <float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000, float 0xBDA01DC620000000>
  %mul1.i402683vector_func.i = fmul <16 x float> %add.i401682vector_func.i, %mul567vector_func.i
  %add2.i403684vector_func.i = fadd <16 x float> %mul1.i402683vector_func.i, <float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000, float 0x3E56A39500000000>
  %mul3.i404685vector_func.i = fmul <16 x float> %add2.i403684vector_func.i, %mul567vector_func.i
  %add4.i405686vector_func.i = fadd <16 x float> %mul3.i404685vector_func.i, <float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000, float 0x3F31F88FE0000000>
  %mul5.i406687vector_func.i = fmul <16 x float> %add4.i405686vector_func.i, %mul567vector_func.i
  %add46688vector_func.i = fadd <16 x float> %add44680vector_func.i, %mul5.i406687vector_func.i
  %call.i6.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add46688vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5732vector_func.i, label %postload5733vector_func.i

preload5732vector_func.i:                         ; preds = %postload5628vector_func.i
  %extract690vector_func.i = add i64 %dim_0_vector_tid.i, 55296
  %89 = getelementptr inbounds float addrspace(1)* %4, i64 %extract690vector_func.i
  %exData2596vector_func.i = extractelement <16 x float> %call.i6.i, i32 0
  store float %exData2596vector_func.i, float addrspace(1)* %89, align 4
  br label %postload5733vector_func.i

postload5733vector_func.i:                        ; preds = %preload5732vector_func.i, %postload5628vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5501vector_func.i, label %postload5502vector_func.i

preload5501vector_func.i:                         ; preds = %postload5733vector_func.i
  %.sum9276vector_func.i = add i64 %dim_0_vector_tid.i, 55297
  %90 = getelementptr float addrspace(1)* %4, i64 %.sum9276vector_func.i
  %exData2599vector_func.i = extractelement <16 x float> %call.i6.i, i32 1
  store float %exData2599vector_func.i, float addrspace(1)* %90, align 4
  br label %postload5502vector_func.i

postload5502vector_func.i:                        ; preds = %preload5501vector_func.i, %postload5733vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5612vector_func.i, label %postload5613vector_func.i

preload5612vector_func.i:                         ; preds = %postload5502vector_func.i
  %.sum9275vector_func.i = add i64 %dim_0_vector_tid.i, 55298
  %91 = getelementptr float addrspace(1)* %4, i64 %.sum9275vector_func.i
  %exData2602vector_func.i = extractelement <16 x float> %call.i6.i, i32 2
  store float %exData2602vector_func.i, float addrspace(1)* %91, align 4
  br label %postload5613vector_func.i

postload5613vector_func.i:                        ; preds = %preload5612vector_func.i, %postload5502vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5495vector_func.i, label %postload5496vector_func.i

preload5495vector_func.i:                         ; preds = %postload5613vector_func.i
  %.sum9274vector_func.i = add i64 %dim_0_vector_tid.i, 55299
  %92 = getelementptr float addrspace(1)* %4, i64 %.sum9274vector_func.i
  %exData2605vector_func.i = extractelement <16 x float> %call.i6.i, i32 3
  store float %exData2605vector_func.i, float addrspace(1)* %92, align 4
  br label %postload5496vector_func.i

postload5496vector_func.i:                        ; preds = %preload5495vector_func.i, %postload5613vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5600vector_func.i, label %postload5601vector_func.i

preload5600vector_func.i:                         ; preds = %postload5496vector_func.i
  %.sum9273vector_func.i = add i64 %dim_0_vector_tid.i, 55300
  %93 = getelementptr float addrspace(1)* %4, i64 %.sum9273vector_func.i
  %exData2608vector_func.i = extractelement <16 x float> %call.i6.i, i32 4
  store float %exData2608vector_func.i, float addrspace(1)* %93, align 4
  br label %postload5601vector_func.i

postload5601vector_func.i:                        ; preds = %preload5600vector_func.i, %postload5496vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5711vector_func.i, label %postload5712vector_func.i

preload5711vector_func.i:                         ; preds = %postload5601vector_func.i
  %.sum9272vector_func.i = add i64 %dim_0_vector_tid.i, 55301
  %94 = getelementptr float addrspace(1)* %4, i64 %.sum9272vector_func.i
  %exData2611vector_func.i = extractelement <16 x float> %call.i6.i, i32 5
  store float %exData2611vector_func.i, float addrspace(1)* %94, align 4
  br label %postload5712vector_func.i

postload5712vector_func.i:                        ; preds = %preload5711vector_func.i, %postload5601vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5483vector_func.i, label %postload5484vector_func.i

preload5483vector_func.i:                         ; preds = %postload5712vector_func.i
  %.sum9271vector_func.i = add i64 %dim_0_vector_tid.i, 55302
  %95 = getelementptr float addrspace(1)* %4, i64 %.sum9271vector_func.i
  %exData2614vector_func.i = extractelement <16 x float> %call.i6.i, i32 6
  store float %exData2614vector_func.i, float addrspace(1)* %95, align 4
  br label %postload5484vector_func.i

postload5484vector_func.i:                        ; preds = %preload5483vector_func.i, %postload5712vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5588vector_func.i, label %postload5589vector_func.i

preload5588vector_func.i:                         ; preds = %postload5484vector_func.i
  %.sum9270vector_func.i = add i64 %dim_0_vector_tid.i, 55303
  %96 = getelementptr float addrspace(1)* %4, i64 %.sum9270vector_func.i
  %exData2617vector_func.i = extractelement <16 x float> %call.i6.i, i32 7
  store float %exData2617vector_func.i, float addrspace(1)* %96, align 4
  br label %postload5589vector_func.i

postload5589vector_func.i:                        ; preds = %preload5588vector_func.i, %postload5484vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5702vector_func.i, label %postload5703vector_func.i

preload5702vector_func.i:                         ; preds = %postload5589vector_func.i
  %.sum9269vector_func.i = add i64 %dim_0_vector_tid.i, 55304
  %97 = getelementptr float addrspace(1)* %4, i64 %.sum9269vector_func.i
  %exData2620vector_func.i = extractelement <16 x float> %call.i6.i, i32 8
  store float %exData2620vector_func.i, float addrspace(1)* %97, align 4
  br label %postload5703vector_func.i

postload5703vector_func.i:                        ; preds = %preload5702vector_func.i, %postload5589vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5579vector_func.i, label %postload5580vector_func.i

preload5579vector_func.i:                         ; preds = %postload5703vector_func.i
  %.sum9268vector_func.i = add i64 %dim_0_vector_tid.i, 55305
  %98 = getelementptr float addrspace(1)* %4, i64 %.sum9268vector_func.i
  %exData2623vector_func.i = extractelement <16 x float> %call.i6.i, i32 9
  store float %exData2623vector_func.i, float addrspace(1)* %98, align 4
  br label %postload5580vector_func.i

postload5580vector_func.i:                        ; preds = %preload5579vector_func.i, %postload5703vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5522vector_func.i, label %postload5523vector_func.i

preload5522vector_func.i:                         ; preds = %postload5580vector_func.i
  %.sum9267vector_func.i = add i64 %dim_0_vector_tid.i, 55306
  %99 = getelementptr float addrspace(1)* %4, i64 %.sum9267vector_func.i
  %exData2626vector_func.i = extractelement <16 x float> %call.i6.i, i32 10
  store float %exData2626vector_func.i, float addrspace(1)* %99, align 4
  br label %postload5523vector_func.i

postload5523vector_func.i:                        ; preds = %preload5522vector_func.i, %postload5580vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5513vector_func.i, label %postload5514vector_func.i

preload5513vector_func.i:                         ; preds = %postload5523vector_func.i
  %.sum9266vector_func.i = add i64 %dim_0_vector_tid.i, 55307
  %100 = getelementptr float addrspace(1)* %4, i64 %.sum9266vector_func.i
  %exData2629vector_func.i = extractelement <16 x float> %call.i6.i, i32 11
  store float %exData2629vector_func.i, float addrspace(1)* %100, align 4
  br label %postload5514vector_func.i

postload5514vector_func.i:                        ; preds = %preload5513vector_func.i, %postload5523vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5504vector_func.i, label %postload5505vector_func.i

preload5504vector_func.i:                         ; preds = %postload5514vector_func.i
  %.sum9265vector_func.i = add i64 %dim_0_vector_tid.i, 55308
  %101 = getelementptr float addrspace(1)* %4, i64 %.sum9265vector_func.i
  %exData2632vector_func.i = extractelement <16 x float> %call.i6.i, i32 12
  store float %exData2632vector_func.i, float addrspace(1)* %101, align 4
  br label %postload5505vector_func.i

postload5505vector_func.i:                        ; preds = %preload5504vector_func.i, %postload5514vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5555vector_func.i, label %postload5556vector_func.i

preload5555vector_func.i:                         ; preds = %postload5505vector_func.i
  %.sum9264vector_func.i = add i64 %dim_0_vector_tid.i, 55309
  %102 = getelementptr float addrspace(1)* %4, i64 %.sum9264vector_func.i
  %exData2635vector_func.i = extractelement <16 x float> %call.i6.i, i32 13
  store float %exData2635vector_func.i, float addrspace(1)* %102, align 4
  br label %postload5556vector_func.i

postload5556vector_func.i:                        ; preds = %preload5555vector_func.i, %postload5505vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5459vector_func.i, label %postload5460vector_func.i

preload5459vector_func.i:                         ; preds = %postload5556vector_func.i
  %.sum9263vector_func.i = add i64 %dim_0_vector_tid.i, 55310
  %103 = getelementptr float addrspace(1)* %4, i64 %.sum9263vector_func.i
  %exData2638vector_func.i = extractelement <16 x float> %call.i6.i, i32 14
  store float %exData2638vector_func.i, float addrspace(1)* %103, align 4
  br label %postload5460vector_func.i

postload5460vector_func.i:                        ; preds = %preload5459vector_func.i, %postload5556vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5564vector_func.i, label %postload5565vector_func.i

preload5564vector_func.i:                         ; preds = %postload5460vector_func.i
  %.sum9262vector_func.i = add i64 %dim_0_vector_tid.i, 55311
  %104 = getelementptr float addrspace(1)* %4, i64 %.sum9262vector_func.i
  %exData2641vector_func.i = extractelement <16 x float> %call.i6.i, i32 15
  store float %exData2641vector_func.i, float addrspace(1)* %104, align 4
  br label %postload5565vector_func.i

postload5565vector_func.i:                        ; preds = %preload5564vector_func.i, %postload5460vector_func.i
  %mul51707vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000, float 0x40DD4D1300000000>
  %add52708vector_func.i = fadd <16 x float> %mul51707vector_func.i, <float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000, float 0x4013DDF900000000>
  %mul53709vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000, float 0x4008459DE0000000>
  %add54710vector_func.i = fadd <16 x float> %add52708vector_func.i, %mul53709vector_func.i
  %mul.i393711vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000, float 0x3CCE4CE6E0000000>
  %add.i394712vector_func.i = fadd <16 x float> %mul.i393711vector_func.i, <float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000, float 0xBDA1C87B60000000>
  %mul1.i395713vector_func.i = fmul <16 x float> %add.i394712vector_func.i, %mul567vector_func.i
  %add2.i396714vector_func.i = fadd <16 x float> %mul1.i395713vector_func.i, <float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000, float 0xBE5D5CA6E0000000>
  %mul3.i397715vector_func.i = fmul <16 x float> %add2.i396714vector_func.i, %mul567vector_func.i
  %add4.i398716vector_func.i = fadd <16 x float> %mul3.i397715vector_func.i, <float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000, float 0x3F51D55400000000>
  %mul5.i399717vector_func.i = fmul <16 x float> %add4.i398716vector_func.i, %mul567vector_func.i
  %add56718vector_func.i = fadd <16 x float> %add54710vector_func.i, %mul5.i399717vector_func.i
  %call.i7.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add56718vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6242vector_func.i, label %postload6243vector_func.i

preload6242vector_func.i:                         ; preds = %postload5565vector_func.i
  %extract720vector_func.i = add i64 %dim_0_vector_tid.i, 69120
  %105 = getelementptr inbounds float addrspace(1)* %4, i64 %extract720vector_func.i
  %exData2645vector_func.i = extractelement <16 x float> %call.i7.i, i32 0
  store float %exData2645vector_func.i, float addrspace(1)* %105, align 4
  br label %postload6243vector_func.i

postload6243vector_func.i:                        ; preds = %preload6242vector_func.i, %postload5565vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6917vector_func.i, label %postload6918vector_func.i

preload6917vector_func.i:                         ; preds = %postload6243vector_func.i
  %.sum9261vector_func.i = add i64 %dim_0_vector_tid.i, 69121
  %106 = getelementptr float addrspace(1)* %4, i64 %.sum9261vector_func.i
  %exData2648vector_func.i = extractelement <16 x float> %call.i7.i, i32 1
  store float %exData2648vector_func.i, float addrspace(1)* %106, align 4
  br label %postload6918vector_func.i

postload6918vector_func.i:                        ; preds = %preload6917vector_func.i, %postload6243vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6881vector_func.i, label %postload6882vector_func.i

preload6881vector_func.i:                         ; preds = %postload6918vector_func.i
  %.sum9260vector_func.i = add i64 %dim_0_vector_tid.i, 69122
  %107 = getelementptr float addrspace(1)* %4, i64 %.sum9260vector_func.i
  %exData2651vector_func.i = extractelement <16 x float> %call.i7.i, i32 2
  store float %exData2651vector_func.i, float addrspace(1)* %107, align 4
  br label %postload6882vector_func.i

postload6882vector_func.i:                        ; preds = %preload6881vector_func.i, %postload6918vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6845vector_func.i, label %postload6846vector_func.i

preload6845vector_func.i:                         ; preds = %postload6882vector_func.i
  %.sum9259vector_func.i = add i64 %dim_0_vector_tid.i, 69123
  %108 = getelementptr float addrspace(1)* %4, i64 %.sum9259vector_func.i
  %exData2654vector_func.i = extractelement <16 x float> %call.i7.i, i32 3
  store float %exData2654vector_func.i, float addrspace(1)* %108, align 4
  br label %postload6846vector_func.i

postload6846vector_func.i:                        ; preds = %preload6845vector_func.i, %postload6882vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5987vector_func.i, label %postload5988vector_func.i

preload5987vector_func.i:                         ; preds = %postload6846vector_func.i
  %.sum9258vector_func.i = add i64 %dim_0_vector_tid.i, 69124
  %109 = getelementptr float addrspace(1)* %4, i64 %.sum9258vector_func.i
  %exData2657vector_func.i = extractelement <16 x float> %call.i7.i, i32 4
  store float %exData2657vector_func.i, float addrspace(1)* %109, align 4
  br label %postload5988vector_func.i

postload5988vector_func.i:                        ; preds = %preload5987vector_func.i, %postload6846vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5465vector_func.i, label %postload5466vector_func.i

preload5465vector_func.i:                         ; preds = %postload5988vector_func.i
  %.sum9257vector_func.i = add i64 %dim_0_vector_tid.i, 69125
  %110 = getelementptr float addrspace(1)* %4, i64 %.sum9257vector_func.i
  %exData2660vector_func.i = extractelement <16 x float> %call.i7.i, i32 5
  store float %exData2660vector_func.i, float addrspace(1)* %110, align 4
  br label %postload5466vector_func.i

postload5466vector_func.i:                        ; preds = %preload5465vector_func.i, %postload5988vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5786vector_func.i, label %postload5787vector_func.i

preload5786vector_func.i:                         ; preds = %postload5466vector_func.i
  %.sum9256vector_func.i = add i64 %dim_0_vector_tid.i, 69126
  %111 = getelementptr float addrspace(1)* %4, i64 %.sum9256vector_func.i
  %exData2663vector_func.i = extractelement <16 x float> %call.i7.i, i32 6
  store float %exData2663vector_func.i, float addrspace(1)* %111, align 4
  br label %postload5787vector_func.i

postload5787vector_func.i:                        ; preds = %preload5786vector_func.i, %postload5466vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5735vector_func.i, label %postload5736vector_func.i

preload5735vector_func.i:                         ; preds = %postload5787vector_func.i
  %.sum9255vector_func.i = add i64 %dim_0_vector_tid.i, 69127
  %112 = getelementptr float addrspace(1)* %4, i64 %.sum9255vector_func.i
  %exData2666vector_func.i = extractelement <16 x float> %call.i7.i, i32 7
  store float %exData2666vector_func.i, float addrspace(1)* %112, align 4
  br label %postload5736vector_func.i

postload5736vector_func.i:                        ; preds = %preload5735vector_func.i, %postload5787vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5537vector_func.i, label %postload5538vector_func.i

preload5537vector_func.i:                         ; preds = %postload5736vector_func.i
  %.sum9254vector_func.i = add i64 %dim_0_vector_tid.i, 69128
  %113 = getelementptr float addrspace(1)* %4, i64 %.sum9254vector_func.i
  %exData2669vector_func.i = extractelement <16 x float> %call.i7.i, i32 8
  store float %exData2669vector_func.i, float addrspace(1)* %113, align 4
  br label %postload5538vector_func.i

postload5538vector_func.i:                        ; preds = %preload5537vector_func.i, %postload5736vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7313vector_func.i, label %postload7314vector_func.i

preload7313vector_func.i:                         ; preds = %postload5538vector_func.i
  %.sum9253vector_func.i = add i64 %dim_0_vector_tid.i, 69129
  %114 = getelementptr float addrspace(1)* %4, i64 %.sum9253vector_func.i
  %exData2672vector_func.i = extractelement <16 x float> %call.i7.i, i32 9
  store float %exData2672vector_func.i, float addrspace(1)* %114, align 4
  br label %postload7314vector_func.i

postload7314vector_func.i:                        ; preds = %preload7313vector_func.i, %postload5538vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5498vector_func.i, label %postload5499vector_func.i

preload5498vector_func.i:                         ; preds = %postload7314vector_func.i
  %.sum9252vector_func.i = add i64 %dim_0_vector_tid.i, 69130
  %115 = getelementptr float addrspace(1)* %4, i64 %.sum9252vector_func.i
  %exData2675vector_func.i = extractelement <16 x float> %call.i7.i, i32 10
  store float %exData2675vector_func.i, float addrspace(1)* %115, align 4
  br label %postload5499vector_func.i

postload5499vector_func.i:                        ; preds = %preload5498vector_func.i, %postload7314vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5444vector_func.i, label %postload5445vector_func.i

preload5444vector_func.i:                         ; preds = %postload5499vector_func.i
  %.sum9251vector_func.i = add i64 %dim_0_vector_tid.i, 69131
  %116 = getelementptr float addrspace(1)* %4, i64 %.sum9251vector_func.i
  %exData2678vector_func.i = extractelement <16 x float> %call.i7.i, i32 11
  store float %exData2678vector_func.i, float addrspace(1)* %116, align 4
  br label %postload5445vector_func.i

postload5445vector_func.i:                        ; preds = %preload5444vector_func.i, %postload5499vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5798vector_func.i, label %postload5799vector_func.i

preload5798vector_func.i:                         ; preds = %postload5445vector_func.i
  %.sum9250vector_func.i = add i64 %dim_0_vector_tid.i, 69132
  %117 = getelementptr float addrspace(1)* %4, i64 %.sum9250vector_func.i
  %exData2681vector_func.i = extractelement <16 x float> %call.i7.i, i32 12
  store float %exData2681vector_func.i, float addrspace(1)* %117, align 4
  br label %postload5799vector_func.i

postload5799vector_func.i:                        ; preds = %preload5798vector_func.i, %postload5445vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5744vector_func.i, label %postload5745vector_func.i

preload5744vector_func.i:                         ; preds = %postload5799vector_func.i
  %.sum9249vector_func.i = add i64 %dim_0_vector_tid.i, 69133
  %118 = getelementptr float addrspace(1)* %4, i64 %.sum9249vector_func.i
  %exData2684vector_func.i = extractelement <16 x float> %call.i7.i, i32 13
  store float %exData2684vector_func.i, float addrspace(1)* %118, align 4
  br label %postload5745vector_func.i

postload5745vector_func.i:                        ; preds = %preload5744vector_func.i, %postload5799vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5474vector_func.i, label %postload5475vector_func.i

preload5474vector_func.i:                         ; preds = %postload5745vector_func.i
  %.sum9248vector_func.i = add i64 %dim_0_vector_tid.i, 69134
  %119 = getelementptr float addrspace(1)* %4, i64 %.sum9248vector_func.i
  %exData2687vector_func.i = extractelement <16 x float> %call.i7.i, i32 14
  store float %exData2687vector_func.i, float addrspace(1)* %119, align 4
  br label %postload5475vector_func.i

postload5475vector_func.i:                        ; preds = %preload5474vector_func.i, %postload5745vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5510vector_func.i, label %postload5511vector_func.i

preload5510vector_func.i:                         ; preds = %postload5475vector_func.i
  %.sum9247vector_func.i = add i64 %dim_0_vector_tid.i, 69135
  %120 = getelementptr float addrspace(1)* %4, i64 %.sum9247vector_func.i
  %exData2690vector_func.i = extractelement <16 x float> %call.i7.i, i32 15
  store float %exData2690vector_func.i, float addrspace(1)* %120, align 4
  br label %postload5511vector_func.i

postload5511vector_func.i:                        ; preds = %preload5510vector_func.i, %postload5475vector_func.i
  %mul61737vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000, float 0x405BF6D460000000>
  %sub62738vector_func.i = fsub <16 x float> <float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000, float 0x400E47E3A0000000>, %mul61737vector_func.i
  %mul63739vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000, float 0x4010119FC0000000>
  %add64740vector_func.i = fadd <16 x float> %sub62738vector_func.i, %mul63739vector_func.i
  %mul.i386741vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000, float 0xBCC3706720000000>
  %add.i387742vector_func.i = fadd <16 x float> %mul.i386741vector_func.i, <float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000, float 0x3DA4EF9520000000>
  %mul1.i388743vector_func.i = fmul <16 x float> %add.i387742vector_func.i, %mul567vector_func.i
  %add2.i389744vector_func.i = fadd <16 x float> %mul1.i388743vector_func.i, <float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000, float 0xBE7C597160000000>
  %mul3.i390745vector_func.i = fmul <16 x float> %add2.i389744vector_func.i, %mul567vector_func.i
  %add4.i391746vector_func.i = fadd <16 x float> %mul3.i390745vector_func.i, <float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000, float 0x3F52593E40000000>
  %mul5.i392747vector_func.i = fmul <16 x float> %add4.i391746vector_func.i, %mul567vector_func.i
  %add66748vector_func.i = fadd <16 x float> %add64740vector_func.i, %mul5.i392747vector_func.i
  %call.i8.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add66748vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5762vector_func.i, label %postload5763vector_func.i

preload5762vector_func.i:                         ; preds = %postload5511vector_func.i
  %extract750vector_func.i = add i64 %dim_0_vector_tid.i, 82944
  %121 = getelementptr inbounds float addrspace(1)* %4, i64 %extract750vector_func.i
  %exData2694vector_func.i = extractelement <16 x float> %call.i8.i, i32 0
  store float %exData2694vector_func.i, float addrspace(1)* %121, align 4
  br label %postload5763vector_func.i

postload5763vector_func.i:                        ; preds = %preload5762vector_func.i, %postload5511vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5765vector_func.i, label %postload5766vector_func.i

preload5765vector_func.i:                         ; preds = %postload5763vector_func.i
  %.sum9246vector_func.i = add i64 %dim_0_vector_tid.i, 82945
  %122 = getelementptr float addrspace(1)* %4, i64 %.sum9246vector_func.i
  %exData2697vector_func.i = extractelement <16 x float> %call.i8.i, i32 1
  store float %exData2697vector_func.i, float addrspace(1)* %122, align 4
  br label %postload5766vector_func.i

postload5766vector_func.i:                        ; preds = %preload5765vector_func.i, %postload5763vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5771vector_func.i, label %postload5772vector_func.i

preload5771vector_func.i:                         ; preds = %postload5766vector_func.i
  %.sum9245vector_func.i = add i64 %dim_0_vector_tid.i, 82946
  %123 = getelementptr float addrspace(1)* %4, i64 %.sum9245vector_func.i
  %exData2700vector_func.i = extractelement <16 x float> %call.i8.i, i32 2
  store float %exData2700vector_func.i, float addrspace(1)* %123, align 4
  br label %postload5772vector_func.i

postload5772vector_func.i:                        ; preds = %preload5771vector_func.i, %postload5766vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5774vector_func.i, label %postload5775vector_func.i

preload5774vector_func.i:                         ; preds = %postload5772vector_func.i
  %.sum9244vector_func.i = add i64 %dim_0_vector_tid.i, 82947
  %124 = getelementptr float addrspace(1)* %4, i64 %.sum9244vector_func.i
  %exData2703vector_func.i = extractelement <16 x float> %call.i8.i, i32 3
  store float %exData2703vector_func.i, float addrspace(1)* %124, align 4
  br label %postload5775vector_func.i

postload5775vector_func.i:                        ; preds = %preload5774vector_func.i, %postload5772vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5780vector_func.i, label %postload5781vector_func.i

preload5780vector_func.i:                         ; preds = %postload5775vector_func.i
  %.sum9243vector_func.i = add i64 %dim_0_vector_tid.i, 82948
  %125 = getelementptr float addrspace(1)* %4, i64 %.sum9243vector_func.i
  %exData2706vector_func.i = extractelement <16 x float> %call.i8.i, i32 4
  store float %exData2706vector_func.i, float addrspace(1)* %125, align 4
  br label %postload5781vector_func.i

postload5781vector_func.i:                        ; preds = %preload5780vector_func.i, %postload5775vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5801vector_func.i, label %postload5802vector_func.i

preload5801vector_func.i:                         ; preds = %postload5781vector_func.i
  %.sum9242vector_func.i = add i64 %dim_0_vector_tid.i, 82949
  %126 = getelementptr float addrspace(1)* %4, i64 %.sum9242vector_func.i
  %exData2709vector_func.i = extractelement <16 x float> %call.i8.i, i32 5
  store float %exData2709vector_func.i, float addrspace(1)* %126, align 4
  br label %postload5802vector_func.i

postload5802vector_func.i:                        ; preds = %preload5801vector_func.i, %postload5781vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5804vector_func.i, label %postload5805vector_func.i

preload5804vector_func.i:                         ; preds = %postload5802vector_func.i
  %.sum9241vector_func.i = add i64 %dim_0_vector_tid.i, 82950
  %127 = getelementptr float addrspace(1)* %4, i64 %.sum9241vector_func.i
  %exData2712vector_func.i = extractelement <16 x float> %call.i8.i, i32 6
  store float %exData2712vector_func.i, float addrspace(1)* %127, align 4
  br label %postload5805vector_func.i

postload5805vector_func.i:                        ; preds = %preload5804vector_func.i, %postload5802vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5807vector_func.i, label %postload5808vector_func.i

preload5807vector_func.i:                         ; preds = %postload5805vector_func.i
  %.sum9240vector_func.i = add i64 %dim_0_vector_tid.i, 82951
  %128 = getelementptr float addrspace(1)* %4, i64 %.sum9240vector_func.i
  %exData2715vector_func.i = extractelement <16 x float> %call.i8.i, i32 7
  store float %exData2715vector_func.i, float addrspace(1)* %128, align 4
  br label %postload5808vector_func.i

postload5808vector_func.i:                        ; preds = %preload5807vector_func.i, %postload5805vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5813vector_func.i, label %postload5814vector_func.i

preload5813vector_func.i:                         ; preds = %postload5808vector_func.i
  %.sum9239vector_func.i = add i64 %dim_0_vector_tid.i, 82952
  %129 = getelementptr float addrspace(1)* %4, i64 %.sum9239vector_func.i
  %exData2718vector_func.i = extractelement <16 x float> %call.i8.i, i32 8
  store float %exData2718vector_func.i, float addrspace(1)* %129, align 4
  br label %postload5814vector_func.i

postload5814vector_func.i:                        ; preds = %preload5813vector_func.i, %postload5808vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5816vector_func.i, label %postload5817vector_func.i

preload5816vector_func.i:                         ; preds = %postload5814vector_func.i
  %.sum9238vector_func.i = add i64 %dim_0_vector_tid.i, 82953
  %130 = getelementptr float addrspace(1)* %4, i64 %.sum9238vector_func.i
  %exData2721vector_func.i = extractelement <16 x float> %call.i8.i, i32 9
  store float %exData2721vector_func.i, float addrspace(1)* %130, align 4
  br label %postload5817vector_func.i

postload5817vector_func.i:                        ; preds = %preload5816vector_func.i, %postload5814vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5819vector_func.i, label %postload5820vector_func.i

preload5819vector_func.i:                         ; preds = %postload5817vector_func.i
  %.sum9237vector_func.i = add i64 %dim_0_vector_tid.i, 82954
  %131 = getelementptr float addrspace(1)* %4, i64 %.sum9237vector_func.i
  %exData2724vector_func.i = extractelement <16 x float> %call.i8.i, i32 10
  store float %exData2724vector_func.i, float addrspace(1)* %131, align 4
  br label %postload5820vector_func.i

postload5820vector_func.i:                        ; preds = %preload5819vector_func.i, %postload5817vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5822vector_func.i, label %postload5823vector_func.i

preload5822vector_func.i:                         ; preds = %postload5820vector_func.i
  %.sum9236vector_func.i = add i64 %dim_0_vector_tid.i, 82955
  %132 = getelementptr float addrspace(1)* %4, i64 %.sum9236vector_func.i
  %exData2727vector_func.i = extractelement <16 x float> %call.i8.i, i32 11
  store float %exData2727vector_func.i, float addrspace(1)* %132, align 4
  br label %postload5823vector_func.i

postload5823vector_func.i:                        ; preds = %preload5822vector_func.i, %postload5820vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5825vector_func.i, label %postload5826vector_func.i

preload5825vector_func.i:                         ; preds = %postload5823vector_func.i
  %.sum9235vector_func.i = add i64 %dim_0_vector_tid.i, 82956
  %133 = getelementptr float addrspace(1)* %4, i64 %.sum9235vector_func.i
  %exData2730vector_func.i = extractelement <16 x float> %call.i8.i, i32 12
  store float %exData2730vector_func.i, float addrspace(1)* %133, align 4
  br label %postload5826vector_func.i

postload5826vector_func.i:                        ; preds = %preload5825vector_func.i, %postload5823vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5831vector_func.i, label %postload5832vector_func.i

preload5831vector_func.i:                         ; preds = %postload5826vector_func.i
  %.sum9234vector_func.i = add i64 %dim_0_vector_tid.i, 82957
  %134 = getelementptr float addrspace(1)* %4, i64 %.sum9234vector_func.i
  %exData2733vector_func.i = extractelement <16 x float> %call.i8.i, i32 13
  store float %exData2733vector_func.i, float addrspace(1)* %134, align 4
  br label %postload5832vector_func.i

postload5832vector_func.i:                        ; preds = %preload5831vector_func.i, %postload5826vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5834vector_func.i, label %postload5835vector_func.i

preload5834vector_func.i:                         ; preds = %postload5832vector_func.i
  %.sum9233vector_func.i = add i64 %dim_0_vector_tid.i, 82958
  %135 = getelementptr float addrspace(1)* %4, i64 %.sum9233vector_func.i
  %exData2736vector_func.i = extractelement <16 x float> %call.i8.i, i32 14
  store float %exData2736vector_func.i, float addrspace(1)* %135, align 4
  br label %postload5835vector_func.i

postload5835vector_func.i:                        ; preds = %preload5834vector_func.i, %postload5832vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5747vector_func.i, label %postload5748vector_func.i

preload5747vector_func.i:                         ; preds = %postload5835vector_func.i
  %.sum9232vector_func.i = add i64 %dim_0_vector_tid.i, 82959
  %136 = getelementptr float addrspace(1)* %4, i64 %.sum9232vector_func.i
  %exData2739vector_func.i = extractelement <16 x float> %call.i8.i, i32 15
  store float %exData2739vector_func.i, float addrspace(1)* %136, align 4
  br label %postload5748vector_func.i

postload5748vector_func.i:                        ; preds = %preload5747vector_func.i, %postload5835vector_func.i
  %mul71767vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000, float 0x40D1717260000000>
  %add72768vector_func.i = fadd <16 x float> %mul71767vector_func.i, <float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000, float 0x40075449E0000000>
  %mul73769vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000, float 0x4010A8F680000000>
  %add74770vector_func.i = fadd <16 x float> %add72768vector_func.i, %mul73769vector_func.i
  %mul.i379771vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000, float 0xBCD9EEB6A0000000>
  %add.i380772vector_func.i = fadd <16 x float> %mul.i379771vector_func.i, <float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000, float 0x3DC10150C0000000>
  %mul1.i381773vector_func.i = fmul <16 x float> %add.i380772vector_func.i, %mul567vector_func.i
  %add2.i382774vector_func.i = fadd <16 x float> %mul1.i381773vector_func.i, <float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000, float 0xBE95444740000000>
  %mul3.i383775vector_func.i = fmul <16 x float> %add2.i382774vector_func.i, %mul567vector_func.i
  %add4.i384776vector_func.i = fadd <16 x float> %mul3.i383775vector_func.i, <float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000, float 0x3F641ABE40000000>
  %mul5.i385777vector_func.i = fmul <16 x float> %add4.i384776vector_func.i, %mul567vector_func.i
  %add76778vector_func.i = fadd <16 x float> %add74770vector_func.i, %mul5.i385777vector_func.i
  %call.i9.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add76778vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5750vector_func.i, label %postload5751vector_func.i

preload5750vector_func.i:                         ; preds = %postload5748vector_func.i
  %extract780vector_func.i = add i64 %dim_0_vector_tid.i, 96768
  %137 = getelementptr inbounds float addrspace(1)* %4, i64 %extract780vector_func.i
  %exData2743vector_func.i = extractelement <16 x float> %call.i9.i, i32 0
  store float %exData2743vector_func.i, float addrspace(1)* %137, align 4
  br label %postload5751vector_func.i

postload5751vector_func.i:                        ; preds = %preload5750vector_func.i, %postload5748vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5837vector_func.i, label %postload5838vector_func.i

preload5837vector_func.i:                         ; preds = %postload5751vector_func.i
  %.sum9231vector_func.i = add i64 %dim_0_vector_tid.i, 96769
  %138 = getelementptr float addrspace(1)* %4, i64 %.sum9231vector_func.i
  %exData2746vector_func.i = extractelement <16 x float> %call.i9.i, i32 1
  store float %exData2746vector_func.i, float addrspace(1)* %138, align 4
  br label %postload5838vector_func.i

postload5838vector_func.i:                        ; preds = %preload5837vector_func.i, %postload5751vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5840vector_func.i, label %postload5841vector_func.i

preload5840vector_func.i:                         ; preds = %postload5838vector_func.i
  %.sum9230vector_func.i = add i64 %dim_0_vector_tid.i, 96770
  %139 = getelementptr float addrspace(1)* %4, i64 %.sum9230vector_func.i
  %exData2749vector_func.i = extractelement <16 x float> %call.i9.i, i32 2
  store float %exData2749vector_func.i, float addrspace(1)* %139, align 4
  br label %postload5841vector_func.i

postload5841vector_func.i:                        ; preds = %preload5840vector_func.i, %postload5838vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5846vector_func.i, label %postload5847vector_func.i

preload5846vector_func.i:                         ; preds = %postload5841vector_func.i
  %.sum9229vector_func.i = add i64 %dim_0_vector_tid.i, 96771
  %140 = getelementptr float addrspace(1)* %4, i64 %.sum9229vector_func.i
  %exData2752vector_func.i = extractelement <16 x float> %call.i9.i, i32 3
  store float %exData2752vector_func.i, float addrspace(1)* %140, align 4
  br label %postload5847vector_func.i

postload5847vector_func.i:                        ; preds = %preload5846vector_func.i, %postload5841vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5849vector_func.i, label %postload5850vector_func.i

preload5849vector_func.i:                         ; preds = %postload5847vector_func.i
  %.sum9228vector_func.i = add i64 %dim_0_vector_tid.i, 96772
  %141 = getelementptr float addrspace(1)* %4, i64 %.sum9228vector_func.i
  %exData2755vector_func.i = extractelement <16 x float> %call.i9.i, i32 4
  store float %exData2755vector_func.i, float addrspace(1)* %141, align 4
  br label %postload5850vector_func.i

postload5850vector_func.i:                        ; preds = %preload5849vector_func.i, %postload5847vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5753vector_func.i, label %postload5754vector_func.i

preload5753vector_func.i:                         ; preds = %postload5850vector_func.i
  %.sum9227vector_func.i = add i64 %dim_0_vector_tid.i, 96773
  %142 = getelementptr float addrspace(1)* %4, i64 %.sum9227vector_func.i
  %exData2758vector_func.i = extractelement <16 x float> %call.i9.i, i32 5
  store float %exData2758vector_func.i, float addrspace(1)* %142, align 4
  br label %postload5754vector_func.i

postload5754vector_func.i:                        ; preds = %preload5753vector_func.i, %postload5850vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5756vector_func.i, label %postload5757vector_func.i

preload5756vector_func.i:                         ; preds = %postload5754vector_func.i
  %.sum9226vector_func.i = add i64 %dim_0_vector_tid.i, 96774
  %143 = getelementptr float addrspace(1)* %4, i64 %.sum9226vector_func.i
  %exData2761vector_func.i = extractelement <16 x float> %call.i9.i, i32 6
  store float %exData2761vector_func.i, float addrspace(1)* %143, align 4
  br label %postload5757vector_func.i

postload5757vector_func.i:                        ; preds = %preload5756vector_func.i, %postload5754vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5609vector_func.i, label %postload5610vector_func.i

preload5609vector_func.i:                         ; preds = %postload5757vector_func.i
  %.sum9225vector_func.i = add i64 %dim_0_vector_tid.i, 96775
  %144 = getelementptr float addrspace(1)* %4, i64 %.sum9225vector_func.i
  %exData2764vector_func.i = extractelement <16 x float> %call.i9.i, i32 7
  store float %exData2764vector_func.i, float addrspace(1)* %144, align 4
  br label %postload5610vector_func.i

postload5610vector_func.i:                        ; preds = %preload5609vector_func.i, %postload5757vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5468vector_func.i, label %postload5469vector_func.i

preload5468vector_func.i:                         ; preds = %postload5610vector_func.i
  %.sum9224vector_func.i = add i64 %dim_0_vector_tid.i, 96776
  %145 = getelementptr float addrspace(1)* %4, i64 %.sum9224vector_func.i
  %exData2767vector_func.i = extractelement <16 x float> %call.i9.i, i32 8
  store float %exData2767vector_func.i, float addrspace(1)* %145, align 4
  br label %postload5469vector_func.i

postload5469vector_func.i:                        ; preds = %preload5468vector_func.i, %postload5610vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5471vector_func.i, label %postload5472vector_func.i

preload5471vector_func.i:                         ; preds = %postload5469vector_func.i
  %.sum9223vector_func.i = add i64 %dim_0_vector_tid.i, 96777
  %146 = getelementptr float addrspace(1)* %4, i64 %.sum9223vector_func.i
  %exData2770vector_func.i = extractelement <16 x float> %call.i9.i, i32 9
  store float %exData2770vector_func.i, float addrspace(1)* %146, align 4
  br label %postload5472vector_func.i

postload5472vector_func.i:                        ; preds = %preload5471vector_func.i, %postload5469vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5585vector_func.i, label %postload5586vector_func.i

preload5585vector_func.i:                         ; preds = %postload5472vector_func.i
  %.sum9222vector_func.i = add i64 %dim_0_vector_tid.i, 96778
  %147 = getelementptr float addrspace(1)* %4, i64 %.sum9222vector_func.i
  %exData2773vector_func.i = extractelement <16 x float> %call.i9.i, i32 10
  store float %exData2773vector_func.i, float addrspace(1)* %147, align 4
  br label %postload5586vector_func.i

postload5586vector_func.i:                        ; preds = %preload5585vector_func.i, %postload5472vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5540vector_func.i, label %postload5541vector_func.i

preload5540vector_func.i:                         ; preds = %postload5586vector_func.i
  %.sum9221vector_func.i = add i64 %dim_0_vector_tid.i, 96779
  %148 = getelementptr float addrspace(1)* %4, i64 %.sum9221vector_func.i
  %exData2776vector_func.i = extractelement <16 x float> %call.i9.i, i32 11
  store float %exData2776vector_func.i, float addrspace(1)* %148, align 4
  br label %postload5541vector_func.i

postload5541vector_func.i:                        ; preds = %preload5540vector_func.i, %postload5586vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5630vector_func.i, label %postload5631vector_func.i

preload5630vector_func.i:                         ; preds = %postload5541vector_func.i
  %.sum9220vector_func.i = add i64 %dim_0_vector_tid.i, 96780
  %149 = getelementptr float addrspace(1)* %4, i64 %.sum9220vector_func.i
  %exData2779vector_func.i = extractelement <16 x float> %call.i9.i, i32 12
  store float %exData2779vector_func.i, float addrspace(1)* %149, align 4
  br label %postload5631vector_func.i

postload5631vector_func.i:                        ; preds = %preload5630vector_func.i, %postload5541vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5633vector_func.i, label %postload5634vector_func.i

preload5633vector_func.i:                         ; preds = %postload5631vector_func.i
  %.sum9219vector_func.i = add i64 %dim_0_vector_tid.i, 96781
  %150 = getelementptr float addrspace(1)* %4, i64 %.sum9219vector_func.i
  %exData2782vector_func.i = extractelement <16 x float> %call.i9.i, i32 13
  store float %exData2782vector_func.i, float addrspace(1)* %150, align 4
  br label %postload5634vector_func.i

postload5634vector_func.i:                        ; preds = %preload5633vector_func.i, %postload5631vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5738vector_func.i, label %postload5739vector_func.i

preload5738vector_func.i:                         ; preds = %postload5634vector_func.i
  %.sum9218vector_func.i = add i64 %dim_0_vector_tid.i, 96782
  %151 = getelementptr float addrspace(1)* %4, i64 %.sum9218vector_func.i
  %exData2785vector_func.i = extractelement <16 x float> %call.i9.i, i32 14
  store float %exData2785vector_func.i, float addrspace(1)* %151, align 4
  br label %postload5739vector_func.i

postload5739vector_func.i:                        ; preds = %preload5738vector_func.i, %postload5634vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5621vector_func.i, label %postload5622vector_func.i

preload5621vector_func.i:                         ; preds = %postload5739vector_func.i
  %.sum9217vector_func.i = add i64 %dim_0_vector_tid.i, 96783
  %152 = getelementptr float addrspace(1)* %4, i64 %.sum9217vector_func.i
  %exData2788vector_func.i = extractelement <16 x float> %call.i9.i, i32 15
  store float %exData2788vector_func.i, float addrspace(1)* %152, align 4
  br label %postload5622vector_func.i

postload5622vector_func.i:                        ; preds = %preload5621vector_func.i, %postload5739vector_func.i
  %mul81797vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000, float 0x40F1564700000000>
  %sub82798vector_func.i = fsub <16 x float> <float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000, float 0x4015F09EA0000000>, %mul81797vector_func.i
  %mul83799vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000, float 0x4007071880000000>
  %add84800vector_func.i = fadd <16 x float> %sub82798vector_func.i, %mul83799vector_func.i
  %mul.i372801vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000, float 0x3CCFB83A80000000>
  %add.i373802vector_func.i = fadd <16 x float> %mul.i372801vector_func.i, <float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000, float 0xBDA7F2E4A0000000>
  %mul1.i374803vector_func.i = fmul <16 x float> %add.i373802vector_func.i, %mul567vector_func.i
  %add2.i375804vector_func.i = fadd <16 x float> %mul1.i374803vector_func.i, <float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000, float 0x3E59D97C80000000>
  %mul3.i376805vector_func.i = fmul <16 x float> %add2.i375804vector_func.i, %mul567vector_func.i
  %add4.i377806vector_func.i = fadd <16 x float> %mul3.i376805vector_func.i, <float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000, float 0x3F3FD09D40000000>
  %mul5.i378807vector_func.i = fmul <16 x float> %add4.i377806vector_func.i, %mul567vector_func.i
  %add86808vector_func.i = fadd <16 x float> %add84800vector_func.i, %mul5.i378807vector_func.i
  %call.i10.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add86808vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5624vector_func.i, label %postload5625vector_func.i

preload5624vector_func.i:                         ; preds = %postload5622vector_func.i
  %extract810vector_func.i = add i64 %dim_0_vector_tid.i, 110592
  %153 = getelementptr inbounds float addrspace(1)* %4, i64 %extract810vector_func.i
  %exData2792vector_func.i = extractelement <16 x float> %call.i10.i, i32 0
  store float %exData2792vector_func.i, float addrspace(1)* %153, align 4
  br label %postload5625vector_func.i

postload5625vector_func.i:                        ; preds = %preload5624vector_func.i, %postload5622vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5729vector_func.i, label %postload5730vector_func.i

preload5729vector_func.i:                         ; preds = %postload5625vector_func.i
  %.sum9216vector_func.i = add i64 %dim_0_vector_tid.i, 110593
  %154 = getelementptr float addrspace(1)* %4, i64 %.sum9216vector_func.i
  %exData2795vector_func.i = extractelement <16 x float> %call.i10.i, i32 1
  store float %exData2795vector_func.i, float addrspace(1)* %154, align 4
  br label %postload5730vector_func.i

postload5730vector_func.i:                        ; preds = %preload5729vector_func.i, %postload5625vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5615vector_func.i, label %postload5616vector_func.i

preload5615vector_func.i:                         ; preds = %postload5730vector_func.i
  %.sum9215vector_func.i = add i64 %dim_0_vector_tid.i, 110594
  %155 = getelementptr float addrspace(1)* %4, i64 %.sum9215vector_func.i
  %exData2798vector_func.i = extractelement <16 x float> %call.i10.i, i32 2
  store float %exData2798vector_func.i, float addrspace(1)* %155, align 4
  br label %postload5616vector_func.i

postload5616vector_func.i:                        ; preds = %preload5615vector_func.i, %postload5730vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5618vector_func.i, label %postload5619vector_func.i

preload5618vector_func.i:                         ; preds = %postload5616vector_func.i
  %.sum9214vector_func.i = add i64 %dim_0_vector_tid.i, 110595
  %156 = getelementptr float addrspace(1)* %4, i64 %.sum9214vector_func.i
  %exData2801vector_func.i = extractelement <16 x float> %call.i10.i, i32 3
  store float %exData2801vector_func.i, float addrspace(1)* %156, align 4
  br label %postload5619vector_func.i

postload5619vector_func.i:                        ; preds = %preload5618vector_func.i, %postload5616vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5720vector_func.i, label %postload5721vector_func.i

preload5720vector_func.i:                         ; preds = %postload5619vector_func.i
  %.sum9213vector_func.i = add i64 %dim_0_vector_tid.i, 110596
  %157 = getelementptr float addrspace(1)* %4, i64 %.sum9213vector_func.i
  %exData2804vector_func.i = extractelement <16 x float> %call.i10.i, i32 4
  store float %exData2804vector_func.i, float addrspace(1)* %157, align 4
  br label %postload5721vector_func.i

postload5721vector_func.i:                        ; preds = %preload5720vector_func.i, %postload5619vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5603vector_func.i, label %postload5604vector_func.i

preload5603vector_func.i:                         ; preds = %postload5721vector_func.i
  %.sum9212vector_func.i = add i64 %dim_0_vector_tid.i, 110597
  %158 = getelementptr float addrspace(1)* %4, i64 %.sum9212vector_func.i
  %exData2807vector_func.i = extractelement <16 x float> %call.i10.i, i32 5
  store float %exData2807vector_func.i, float addrspace(1)* %158, align 4
  br label %postload5604vector_func.i

postload5604vector_func.i:                        ; preds = %preload5603vector_func.i, %postload5721vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5606vector_func.i, label %postload5607vector_func.i

preload5606vector_func.i:                         ; preds = %postload5604vector_func.i
  %.sum9211vector_func.i = add i64 %dim_0_vector_tid.i, 110598
  %159 = getelementptr float addrspace(1)* %4, i64 %.sum9211vector_func.i
  %exData2810vector_func.i = extractelement <16 x float> %call.i10.i, i32 6
  store float %exData2810vector_func.i, float addrspace(1)* %159, align 4
  br label %postload5607vector_func.i

postload5607vector_func.i:                        ; preds = %preload5606vector_func.i, %postload5604vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5492vector_func.i, label %postload5493vector_func.i

preload5492vector_func.i:                         ; preds = %postload5607vector_func.i
  %.sum9210vector_func.i = add i64 %dim_0_vector_tid.i, 110599
  %160 = getelementptr float addrspace(1)* %4, i64 %.sum9210vector_func.i
  %exData2813vector_func.i = extractelement <16 x float> %call.i10.i, i32 7
  store float %exData2813vector_func.i, float addrspace(1)* %160, align 4
  br label %postload5493vector_func.i

postload5493vector_func.i:                        ; preds = %preload5492vector_func.i, %postload5607vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5714vector_func.i, label %postload5715vector_func.i

preload5714vector_func.i:                         ; preds = %postload5493vector_func.i
  %.sum9209vector_func.i = add i64 %dim_0_vector_tid.i, 110600
  %161 = getelementptr float addrspace(1)* %4, i64 %.sum9209vector_func.i
  %exData2816vector_func.i = extractelement <16 x float> %call.i10.i, i32 8
  store float %exData2816vector_func.i, float addrspace(1)* %161, align 4
  br label %postload5715vector_func.i

postload5715vector_func.i:                        ; preds = %preload5714vector_func.i, %postload5493vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5717vector_func.i, label %postload5718vector_func.i

preload5717vector_func.i:                         ; preds = %postload5715vector_func.i
  %.sum9208vector_func.i = add i64 %dim_0_vector_tid.i, 110601
  %162 = getelementptr float addrspace(1)* %4, i64 %.sum9208vector_func.i
  %exData2819vector_func.i = extractelement <16 x float> %call.i10.i, i32 9
  store float %exData2819vector_func.i, float addrspace(1)* %162, align 4
  br label %postload5718vector_func.i

postload5718vector_func.i:                        ; preds = %preload5717vector_func.i, %postload5715vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5597vector_func.i, label %postload5598vector_func.i

preload5597vector_func.i:                         ; preds = %postload5718vector_func.i
  %.sum9207vector_func.i = add i64 %dim_0_vector_tid.i, 110602
  %163 = getelementptr float addrspace(1)* %4, i64 %.sum9207vector_func.i
  %exData2822vector_func.i = extractelement <16 x float> %call.i10.i, i32 10
  store float %exData2822vector_func.i, float addrspace(1)* %163, align 4
  br label %postload5598vector_func.i

postload5598vector_func.i:                        ; preds = %preload5597vector_func.i, %postload5718vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5486vector_func.i, label %postload5487vector_func.i

preload5486vector_func.i:                         ; preds = %postload5598vector_func.i
  %.sum9206vector_func.i = add i64 %dim_0_vector_tid.i, 110603
  %164 = getelementptr float addrspace(1)* %4, i64 %.sum9206vector_func.i
  %exData2825vector_func.i = extractelement <16 x float> %call.i10.i, i32 11
  store float %exData2825vector_func.i, float addrspace(1)* %164, align 4
  br label %postload5487vector_func.i

postload5487vector_func.i:                        ; preds = %preload5486vector_func.i, %postload5598vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5489vector_func.i, label %postload5490vector_func.i

preload5489vector_func.i:                         ; preds = %postload5487vector_func.i
  %.sum9205vector_func.i = add i64 %dim_0_vector_tid.i, 110604
  %165 = getelementptr float addrspace(1)* %4, i64 %.sum9205vector_func.i
  %exData2828vector_func.i = extractelement <16 x float> %call.i10.i, i32 12
  store float %exData2828vector_func.i, float addrspace(1)* %165, align 4
  br label %postload5490vector_func.i

postload5490vector_func.i:                        ; preds = %preload5489vector_func.i, %postload5487vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5708vector_func.i, label %postload5709vector_func.i

preload5708vector_func.i:                         ; preds = %postload5490vector_func.i
  %.sum9204vector_func.i = add i64 %dim_0_vector_tid.i, 110605
  %166 = getelementptr float addrspace(1)* %4, i64 %.sum9204vector_func.i
  %exData2831vector_func.i = extractelement <16 x float> %call.i10.i, i32 13
  store float %exData2831vector_func.i, float addrspace(1)* %166, align 4
  br label %postload5709vector_func.i

postload5709vector_func.i:                        ; preds = %preload5708vector_func.i, %postload5490vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5591vector_func.i, label %postload5592vector_func.i

preload5591vector_func.i:                         ; preds = %postload5709vector_func.i
  %.sum9203vector_func.i = add i64 %dim_0_vector_tid.i, 110606
  %167 = getelementptr float addrspace(1)* %4, i64 %.sum9203vector_func.i
  %exData2834vector_func.i = extractelement <16 x float> %call.i10.i, i32 14
  store float %exData2834vector_func.i, float addrspace(1)* %167, align 4
  br label %postload5592vector_func.i

postload5592vector_func.i:                        ; preds = %preload5591vector_func.i, %postload5709vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5594vector_func.i, label %postload5595vector_func.i

preload5594vector_func.i:                         ; preds = %postload5592vector_func.i
  %.sum9202vector_func.i = add i64 %dim_0_vector_tid.i, 110607
  %168 = getelementptr float addrspace(1)* %4, i64 %.sum9202vector_func.i
  %exData2837vector_func.i = extractelement <16 x float> %call.i10.i, i32 15
  store float %exData2837vector_func.i, float addrspace(1)* %168, align 4
  br label %postload5595vector_func.i

postload5595vector_func.i:                        ; preds = %preload5594vector_func.i, %postload5592vector_func.i
  %mul91827vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000, float 0x40E696F360000000>
  %sub92828vector_func.i = fsub <16 x float> <float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000, float 0x4018AF4D40000000>, %mul91827vector_func.i
  %mul93829vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000, float 0x4006FE28C0000000>
  %add94830vector_func.i = fadd <16 x float> %sub92828vector_func.i, %mul93829vector_func.i
  %mul.i365831vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000, float 0xBCD0E8B400000000>
  %add.i366832vector_func.i = fadd <16 x float> %mul.i365831vector_func.i, <float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000, float 0x3DB7D6D600000000>
  %mul1.i367833vector_func.i = fmul <16 x float> %add.i366832vector_func.i, %mul567vector_func.i
  %add2.i368834vector_func.i = fadd <16 x float> %mul1.i367833vector_func.i, <float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000, float 0xBE8F8480A0000000>
  %mul3.i369835vector_func.i = fmul <16 x float> %add2.i368834vector_func.i, %mul567vector_func.i
  %add4.i370836vector_func.i = fadd <16 x float> %mul3.i369835vector_func.i, <float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000, float 0x3F5DF40300000000>
  %mul5.i371837vector_func.i = fmul <16 x float> %add4.i370836vector_func.i, %mul567vector_func.i
  %add96838vector_func.i = fadd <16 x float> %add94830vector_func.i, %mul5.i371837vector_func.i
  %call.i11.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add96838vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5477vector_func.i, label %postload5478vector_func.i

preload5477vector_func.i:                         ; preds = %postload5595vector_func.i
  %extract840vector_func.i = add i64 %dim_0_vector_tid.i, 124416
  %169 = getelementptr inbounds float addrspace(1)* %4, i64 %extract840vector_func.i
  %exData2841vector_func.i = extractelement <16 x float> %call.i11.i, i32 0
  store float %exData2841vector_func.i, float addrspace(1)* %169, align 4
  br label %postload5478vector_func.i

postload5478vector_func.i:                        ; preds = %preload5477vector_func.i, %postload5595vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5480vector_func.i, label %postload5481vector_func.i

preload5480vector_func.i:                         ; preds = %postload5478vector_func.i
  %.sum9201vector_func.i = add i64 %dim_0_vector_tid.i, 124417
  %170 = getelementptr float addrspace(1)* %4, i64 %.sum9201vector_func.i
  %exData2844vector_func.i = extractelement <16 x float> %call.i11.i, i32 1
  store float %exData2844vector_func.i, float addrspace(1)* %170, align 4
  br label %postload5481vector_func.i

postload5481vector_func.i:                        ; preds = %preload5480vector_func.i, %postload5478vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5705vector_func.i, label %postload5706vector_func.i

preload5705vector_func.i:                         ; preds = %postload5481vector_func.i
  %.sum9200vector_func.i = add i64 %dim_0_vector_tid.i, 124418
  %171 = getelementptr float addrspace(1)* %4, i64 %.sum9200vector_func.i
  %exData2847vector_func.i = extractelement <16 x float> %call.i11.i, i32 2
  store float %exData2847vector_func.i, float addrspace(1)* %171, align 4
  br label %postload5706vector_func.i

postload5706vector_func.i:                        ; preds = %preload5705vector_func.i, %postload5481vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5531vector_func.i, label %postload5532vector_func.i

preload5531vector_func.i:                         ; preds = %postload5706vector_func.i
  %.sum9199vector_func.i = add i64 %dim_0_vector_tid.i, 124419
  %172 = getelementptr float addrspace(1)* %4, i64 %.sum9199vector_func.i
  %exData2850vector_func.i = extractelement <16 x float> %call.i11.i, i32 3
  store float %exData2850vector_func.i, float addrspace(1)* %172, align 4
  br label %postload5532vector_func.i

postload5532vector_func.i:                        ; preds = %preload5531vector_func.i, %postload5706vector_func.i
  br i1 %exmask2411vector_func.i, label %preload5534vector_func.i, label %postload5535vector_func.i

preload5534vector_func.i:                         ; preds = %postload5532vector_func.i
  %.sum9198vector_func.i = add i64 %dim_0_vector_tid.i, 124420
  %173 = getelementptr float addrspace(1)* %4, i64 %.sum9198vector_func.i
  %exData2853vector_func.i = extractelement <16 x float> %call.i11.i, i32 4
  store float %exData2853vector_func.i, float addrspace(1)* %173, align 4
  br label %postload5535vector_func.i

postload5535vector_func.i:                        ; preds = %preload5534vector_func.i, %postload5532vector_func.i
  br i1 %exmask2414vector_func.i, label %preload5582vector_func.i, label %postload5583vector_func.i

preload5582vector_func.i:                         ; preds = %postload5535vector_func.i
  %.sum9197vector_func.i = add i64 %dim_0_vector_tid.i, 124421
  %174 = getelementptr float addrspace(1)* %4, i64 %.sum9197vector_func.i
  %exData2856vector_func.i = extractelement <16 x float> %call.i11.i, i32 5
  store float %exData2856vector_func.i, float addrspace(1)* %174, align 4
  br label %postload5583vector_func.i

postload5583vector_func.i:                        ; preds = %preload5582vector_func.i, %postload5535vector_func.i
  br i1 %exmask2417vector_func.i, label %preload5573vector_func.i, label %postload5574vector_func.i

preload5573vector_func.i:                         ; preds = %postload5583vector_func.i
  %.sum9196vector_func.i = add i64 %dim_0_vector_tid.i, 124422
  %175 = getelementptr float addrspace(1)* %4, i64 %.sum9196vector_func.i
  %exData2859vector_func.i = extractelement <16 x float> %call.i11.i, i32 6
  store float %exData2859vector_func.i, float addrspace(1)* %175, align 4
  br label %postload5574vector_func.i

postload5574vector_func.i:                        ; preds = %preload5573vector_func.i, %postload5583vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5576vector_func.i, label %postload5577vector_func.i

preload5576vector_func.i:                         ; preds = %postload5574vector_func.i
  %.sum9195vector_func.i = add i64 %dim_0_vector_tid.i, 124423
  %176 = getelementptr float addrspace(1)* %4, i64 %.sum9195vector_func.i
  %exData2862vector_func.i = extractelement <16 x float> %call.i11.i, i32 7
  store float %exData2862vector_func.i, float addrspace(1)* %176, align 4
  br label %postload5577vector_func.i

postload5577vector_func.i:                        ; preds = %preload5576vector_func.i, %postload5574vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5570vector_func.i, label %postload5571vector_func.i

preload5570vector_func.i:                         ; preds = %postload5577vector_func.i
  %.sum9194vector_func.i = add i64 %dim_0_vector_tid.i, 124424
  %177 = getelementptr float addrspace(1)* %4, i64 %.sum9194vector_func.i
  %exData2865vector_func.i = extractelement <16 x float> %call.i11.i, i32 8
  store float %exData2865vector_func.i, float addrspace(1)* %177, align 4
  br label %postload5571vector_func.i

postload5571vector_func.i:                        ; preds = %preload5570vector_func.i, %postload5577vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5516vector_func.i, label %postload5517vector_func.i

preload5516vector_func.i:                         ; preds = %postload5571vector_func.i
  %.sum9193vector_func.i = add i64 %dim_0_vector_tid.i, 124425
  %178 = getelementptr float addrspace(1)* %4, i64 %.sum9193vector_func.i
  %exData2868vector_func.i = extractelement <16 x float> %call.i11.i, i32 9
  store float %exData2868vector_func.i, float addrspace(1)* %178, align 4
  br label %postload5517vector_func.i

postload5517vector_func.i:                        ; preds = %preload5516vector_func.i, %postload5571vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5519vector_func.i, label %postload5520vector_func.i

preload5519vector_func.i:                         ; preds = %postload5517vector_func.i
  %.sum9192vector_func.i = add i64 %dim_0_vector_tid.i, 124426
  %179 = getelementptr float addrspace(1)* %4, i64 %.sum9192vector_func.i
  %exData2871vector_func.i = extractelement <16 x float> %call.i11.i, i32 10
  store float %exData2871vector_func.i, float addrspace(1)* %179, align 4
  br label %postload5520vector_func.i

postload5520vector_func.i:                        ; preds = %preload5519vector_func.i, %postload5517vector_func.i
  br i1 %exmask2432vector_func.i, label %preload5567vector_func.i, label %postload5568vector_func.i

preload5567vector_func.i:                         ; preds = %postload5520vector_func.i
  %.sum9191vector_func.i = add i64 %dim_0_vector_tid.i, 124427
  %180 = getelementptr float addrspace(1)* %4, i64 %.sum9191vector_func.i
  %exData2874vector_func.i = extractelement <16 x float> %call.i11.i, i32 11
  store float %exData2874vector_func.i, float addrspace(1)* %180, align 4
  br label %postload5568vector_func.i

postload5568vector_func.i:                        ; preds = %preload5567vector_func.i, %postload5520vector_func.i
  br i1 %exmask2435vector_func.i, label %preload5558vector_func.i, label %postload5559vector_func.i

preload5558vector_func.i:                         ; preds = %postload5568vector_func.i
  %.sum9190vector_func.i = add i64 %dim_0_vector_tid.i, 124428
  %181 = getelementptr float addrspace(1)* %4, i64 %.sum9190vector_func.i
  %exData2877vector_func.i = extractelement <16 x float> %call.i11.i, i32 12
  store float %exData2877vector_func.i, float addrspace(1)* %181, align 4
  br label %postload5559vector_func.i

postload5559vector_func.i:                        ; preds = %preload5558vector_func.i, %postload5568vector_func.i
  br i1 %exmask2438vector_func.i, label %preload5561vector_func.i, label %postload5562vector_func.i

preload5561vector_func.i:                         ; preds = %postload5559vector_func.i
  %.sum9189vector_func.i = add i64 %dim_0_vector_tid.i, 124429
  %182 = getelementptr float addrspace(1)* %4, i64 %.sum9189vector_func.i
  %exData2880vector_func.i = extractelement <16 x float> %call.i11.i, i32 13
  store float %exData2880vector_func.i, float addrspace(1)* %182, align 4
  br label %postload5562vector_func.i

postload5562vector_func.i:                        ; preds = %preload5561vector_func.i, %postload5559vector_func.i
  br i1 %exmask2441vector_func.i, label %preload5552vector_func.i, label %postload5553vector_func.i

preload5552vector_func.i:                         ; preds = %postload5562vector_func.i
  %.sum9188vector_func.i = add i64 %dim_0_vector_tid.i, 124430
  %183 = getelementptr float addrspace(1)* %4, i64 %.sum9188vector_func.i
  %exData2883vector_func.i = extractelement <16 x float> %call.i11.i, i32 14
  store float %exData2883vector_func.i, float addrspace(1)* %183, align 4
  br label %postload5553vector_func.i

postload5553vector_func.i:                        ; preds = %preload5552vector_func.i, %postload5562vector_func.i
  br i1 %exmask2444vector_func.i, label %preload5543vector_func.i, label %postload5544vector_func.i

preload5543vector_func.i:                         ; preds = %postload5553vector_func.i
  %.sum9187vector_func.i = add i64 %dim_0_vector_tid.i, 124431
  %184 = getelementptr float addrspace(1)* %4, i64 %.sum9187vector_func.i
  %exData2886vector_func.i = extractelement <16 x float> %call.i11.i, i32 15
  store float %exData2886vector_func.i, float addrspace(1)* %184, align 4
  br label %postload5544vector_func.i

postload5544vector_func.i:                        ; preds = %preload5543vector_func.i, %postload5553vector_func.i
  %mul101857vector_func.i = fmul <16 x float> %div568vector_func.i, <float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04, float 5.092600e+04>
  %sub102858vector_func.i = fsub <16 x float> <float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000, float 0x402140C4E0000000>, %mul101857vector_func.i
  %mul103859vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000, float 0x4002561840000000>
  %add104860vector_func.i = fadd <16 x float> %sub102858vector_func.i, %mul103859vector_func.i
  %mul.i358861vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000, float 0xBCDE995380000000>
  %add.i359862vector_func.i = fadd <16 x float> %mul.i358861vector_func.i, <float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000, float 0x3DC32540E0000000>
  %mul1.i360863vector_func.i = fmul <16 x float> %add.i359862vector_func.i, %mul567vector_func.i
  %add2.i361864vector_func.i = fadd <16 x float> %mul1.i360863vector_func.i, <float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000, float 0xBE9680C0A0000000>
  %mul3.i362865vector_func.i = fmul <16 x float> %add2.i361864vector_func.i, %mul567vector_func.i
  %add4.i363866vector_func.i = fadd <16 x float> %mul3.i362865vector_func.i, <float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000, float 0x3F63120D00000000>
  %mul5.i364867vector_func.i = fmul <16 x float> %add4.i363866vector_func.i, %mul567vector_func.i
  %add106868vector_func.i = fadd <16 x float> %add104860vector_func.i, %mul5.i364867vector_func.i
  %call.i12.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add106868vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5546vector_func.i, label %postload5547vector_func.i

preload5546vector_func.i:                         ; preds = %postload5544vector_func.i
  %extract870vector_func.i = add i64 %dim_0_vector_tid.i, 138240
  %185 = getelementptr inbounds float addrspace(1)* %4, i64 %extract870vector_func.i
  %exData2890vector_func.i = extractelement <16 x float> %call.i12.i, i32 0
  store float %exData2890vector_func.i, float addrspace(1)* %185, align 4
  br label %postload5547vector_func.i

postload5547vector_func.i:                        ; preds = %preload5546vector_func.i, %postload5544vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5741vector_func.i, label %postload5742vector_func.i

preload5741vector_func.i:                         ; preds = %postload5547vector_func.i
  %.sum9186vector_func.i = add i64 %dim_0_vector_tid.i, 138241
  %186 = getelementptr float addrspace(1)* %4, i64 %.sum9186vector_func.i
  %exData2893vector_func.i = extractelement <16 x float> %call.i12.i, i32 1
  store float %exData2893vector_func.i, float addrspace(1)* %186, align 4
  br label %postload5742vector_func.i

postload5742vector_func.i:                        ; preds = %preload5741vector_func.i, %postload5547vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5723vector_func.i, label %postload5724vector_func.i

preload5723vector_func.i:                         ; preds = %postload5742vector_func.i
  %.sum9185vector_func.i = add i64 %dim_0_vector_tid.i, 138242
  %187 = getelementptr float addrspace(1)* %4, i64 %.sum9185vector_func.i
  %exData2896vector_func.i = extractelement <16 x float> %call.i12.i, i32 2
  store float %exData2896vector_func.i, float addrspace(1)* %187, align 4
  br label %postload5724vector_func.i

postload5724vector_func.i:                        ; preds = %preload5723vector_func.i, %postload5742vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5726vector_func.i, label %postload5727vector_func.i

preload5726vector_func.i:                         ; preds = %postload5724vector_func.i
  %.sum9184vector_func.i = add i64 %dim_0_vector_tid.i, 138243
  %188 = getelementptr float addrspace(1)* %4, i64 %.sum9184vector_func.i
  %exData2899vector_func.i = extractelement <16 x float> %call.i12.i, i32 3
  store float %exData2899vector_func.i, float addrspace(1)* %188, align 4
  br label %postload5727vector_func.i

postload5727vector_func.i:                        ; preds = %preload5726vector_func.i, %postload5724vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6368vector_func.i, label %postload6369vector_func.i

preload6368vector_func.i:                         ; preds = %postload5727vector_func.i
  %.sum9183vector_func.i = add i64 %dim_0_vector_tid.i, 138244
  %189 = getelementptr float addrspace(1)* %4, i64 %.sum9183vector_func.i
  %exData2902vector_func.i = extractelement <16 x float> %call.i12.i, i32 4
  store float %exData2902vector_func.i, float addrspace(1)* %189, align 4
  br label %postload6369vector_func.i

postload6369vector_func.i:                        ; preds = %preload6368vector_func.i, %postload5727vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6371vector_func.i, label %postload6372vector_func.i

preload6371vector_func.i:                         ; preds = %postload6369vector_func.i
  %.sum9182vector_func.i = add i64 %dim_0_vector_tid.i, 138245
  %190 = getelementptr float addrspace(1)* %4, i64 %.sum9182vector_func.i
  %exData2905vector_func.i = extractelement <16 x float> %call.i12.i, i32 5
  store float %exData2905vector_func.i, float addrspace(1)* %190, align 4
  br label %postload6372vector_func.i

postload6372vector_func.i:                        ; preds = %preload6371vector_func.i, %postload6369vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6920vector_func.i, label %postload6921vector_func.i

preload6920vector_func.i:                         ; preds = %postload6372vector_func.i
  %.sum9181vector_func.i = add i64 %dim_0_vector_tid.i, 138246
  %191 = getelementptr float addrspace(1)* %4, i64 %.sum9181vector_func.i
  %exData2908vector_func.i = extractelement <16 x float> %call.i12.i, i32 6
  store float %exData2908vector_func.i, float addrspace(1)* %191, align 4
  br label %postload6921vector_func.i

postload6921vector_func.i:                        ; preds = %preload6920vector_func.i, %postload6372vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6923vector_func.i, label %postload6924vector_func.i

preload6923vector_func.i:                         ; preds = %postload6921vector_func.i
  %.sum9180vector_func.i = add i64 %dim_0_vector_tid.i, 138247
  %192 = getelementptr float addrspace(1)* %4, i64 %.sum9180vector_func.i
  %exData2911vector_func.i = extractelement <16 x float> %call.i12.i, i32 7
  store float %exData2911vector_func.i, float addrspace(1)* %192, align 4
  br label %postload6924vector_func.i

postload6924vector_func.i:                        ; preds = %preload6923vector_func.i, %postload6921vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6902vector_func.i, label %postload6903vector_func.i

preload6902vector_func.i:                         ; preds = %postload6924vector_func.i
  %.sum9179vector_func.i = add i64 %dim_0_vector_tid.i, 138248
  %193 = getelementptr float addrspace(1)* %4, i64 %.sum9179vector_func.i
  %exData2914vector_func.i = extractelement <16 x float> %call.i12.i, i32 8
  store float %exData2914vector_func.i, float addrspace(1)* %193, align 4
  br label %postload6903vector_func.i

postload6903vector_func.i:                        ; preds = %preload6902vector_func.i, %postload6924vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6905vector_func.i, label %postload6906vector_func.i

preload6905vector_func.i:                         ; preds = %postload6903vector_func.i
  %.sum9178vector_func.i = add i64 %dim_0_vector_tid.i, 138249
  %194 = getelementptr float addrspace(1)* %4, i64 %.sum9178vector_func.i
  %exData2917vector_func.i = extractelement <16 x float> %call.i12.i, i32 9
  store float %exData2917vector_func.i, float addrspace(1)* %194, align 4
  br label %postload6906vector_func.i

postload6906vector_func.i:                        ; preds = %preload6905vector_func.i, %postload6903vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6884vector_func.i, label %postload6885vector_func.i

preload6884vector_func.i:                         ; preds = %postload6906vector_func.i
  %.sum9177vector_func.i = add i64 %dim_0_vector_tid.i, 138250
  %195 = getelementptr float addrspace(1)* %4, i64 %.sum9177vector_func.i
  %exData2920vector_func.i = extractelement <16 x float> %call.i12.i, i32 10
  store float %exData2920vector_func.i, float addrspace(1)* %195, align 4
  br label %postload6885vector_func.i

postload6885vector_func.i:                        ; preds = %preload6884vector_func.i, %postload6906vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6887vector_func.i, label %postload6888vector_func.i

preload6887vector_func.i:                         ; preds = %postload6885vector_func.i
  %.sum9176vector_func.i = add i64 %dim_0_vector_tid.i, 138251
  %196 = getelementptr float addrspace(1)* %4, i64 %.sum9176vector_func.i
  %exData2923vector_func.i = extractelement <16 x float> %call.i12.i, i32 11
  store float %exData2923vector_func.i, float addrspace(1)* %196, align 4
  br label %postload6888vector_func.i

postload6888vector_func.i:                        ; preds = %preload6887vector_func.i, %postload6885vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6866vector_func.i, label %postload6867vector_func.i

preload6866vector_func.i:                         ; preds = %postload6888vector_func.i
  %.sum9175vector_func.i = add i64 %dim_0_vector_tid.i, 138252
  %197 = getelementptr float addrspace(1)* %4, i64 %.sum9175vector_func.i
  %exData2926vector_func.i = extractelement <16 x float> %call.i12.i, i32 12
  store float %exData2926vector_func.i, float addrspace(1)* %197, align 4
  br label %postload6867vector_func.i

postload6867vector_func.i:                        ; preds = %preload6866vector_func.i, %postload6888vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6869vector_func.i, label %postload6870vector_func.i

preload6869vector_func.i:                         ; preds = %postload6867vector_func.i
  %.sum9174vector_func.i = add i64 %dim_0_vector_tid.i, 138253
  %198 = getelementptr float addrspace(1)* %4, i64 %.sum9174vector_func.i
  %exData2929vector_func.i = extractelement <16 x float> %call.i12.i, i32 13
  store float %exData2929vector_func.i, float addrspace(1)* %198, align 4
  br label %postload6870vector_func.i

postload6870vector_func.i:                        ; preds = %preload6869vector_func.i, %postload6867vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6830vector_func.i, label %postload6831vector_func.i

preload6830vector_func.i:                         ; preds = %postload6870vector_func.i
  %.sum9173vector_func.i = add i64 %dim_0_vector_tid.i, 138254
  %199 = getelementptr float addrspace(1)* %4, i64 %.sum9173vector_func.i
  %exData2932vector_func.i = extractelement <16 x float> %call.i12.i, i32 14
  store float %exData2932vector_func.i, float addrspace(1)* %199, align 4
  br label %postload6831vector_func.i

postload6831vector_func.i:                        ; preds = %preload6830vector_func.i, %postload6870vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6833vector_func.i, label %postload6834vector_func.i

preload6833vector_func.i:                         ; preds = %postload6831vector_func.i
  %.sum9172vector_func.i = add i64 %dim_0_vector_tid.i, 138255
  %200 = getelementptr float addrspace(1)* %4, i64 %.sum9172vector_func.i
  %exData2935vector_func.i = extractelement <16 x float> %call.i12.i, i32 15
  store float %exData2935vector_func.i, float addrspace(1)* %200, align 4
  br label %postload6834vector_func.i

postload6834vector_func.i:                        ; preds = %preload6833vector_func.i, %postload6831vector_func.i
  %mul111887vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000, float 0x40D061E560000000>
  %sub112888vector_func.i = fsub <16 x float> <float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000, float 0x4020F5CC00000000>, %mul111887vector_func.i
  %mul113889vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000, float 0x4002492660000000>
  %add114890vector_func.i = fadd <16 x float> %sub112888vector_func.i, %mul113889vector_func.i
  %mul.i351891vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000, float 0xBCE509EC60000000>
  %add.i352892vector_func.i = fadd <16 x float> %mul.i351891vector_func.i, <float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000, float 0x3DCB4A4360000000>
  %mul1.i353893vector_func.i = fmul <16 x float> %add.i352892vector_func.i, %mul567vector_func.i
  %add2.i354894vector_func.i = fadd <16 x float> %mul1.i353893vector_func.i, <float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000, float 0xBEA0B48FA0000000>
  %mul3.i355895vector_func.i = fmul <16 x float> %add2.i354894vector_func.i, %mul567vector_func.i
  %add4.i356896vector_func.i = fadd <16 x float> %mul3.i355895vector_func.i, <float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000, float 0x3F6DA79600000000>
  %mul5.i357897vector_func.i = fmul <16 x float> %add4.i356896vector_func.i, %mul567vector_func.i
  %add116898vector_func.i = fadd <16 x float> %add114890vector_func.i, %mul5.i357897vector_func.i
  %call.i13.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add116898vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5948vector_func.i, label %postload5949vector_func.i

preload5948vector_func.i:                         ; preds = %postload6834vector_func.i
  %extract900vector_func.i = add i64 %dim_0_vector_tid.i, 152064
  %201 = getelementptr inbounds float addrspace(1)* %4, i64 %extract900vector_func.i
  %exData2939vector_func.i = extractelement <16 x float> %call.i13.i, i32 0
  store float %exData2939vector_func.i, float addrspace(1)* %201, align 4
  br label %postload5949vector_func.i

postload5949vector_func.i:                        ; preds = %preload5948vector_func.i, %postload6834vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5951vector_func.i, label %postload5952vector_func.i

preload5951vector_func.i:                         ; preds = %postload5949vector_func.i
  %.sum9171vector_func.i = add i64 %dim_0_vector_tid.i, 152065
  %202 = getelementptr float addrspace(1)* %4, i64 %.sum9171vector_func.i
  %exData2942vector_func.i = extractelement <16 x float> %call.i13.i, i32 1
  store float %exData2942vector_func.i, float addrspace(1)* %202, align 4
  br label %postload5952vector_func.i

postload5952vector_func.i:                        ; preds = %preload5951vector_func.i, %postload5949vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6245vector_func.i, label %postload6246vector_func.i

preload6245vector_func.i:                         ; preds = %postload5952vector_func.i
  %.sum9170vector_func.i = add i64 %dim_0_vector_tid.i, 152066
  %203 = getelementptr float addrspace(1)* %4, i64 %.sum9170vector_func.i
  %exData2945vector_func.i = extractelement <16 x float> %call.i13.i, i32 2
  store float %exData2945vector_func.i, float addrspace(1)* %203, align 4
  br label %postload6246vector_func.i

postload6246vector_func.i:                        ; preds = %preload6245vector_func.i, %postload5952vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6248vector_func.i, label %postload6249vector_func.i

preload6248vector_func.i:                         ; preds = %postload6246vector_func.i
  %.sum9169vector_func.i = add i64 %dim_0_vector_tid.i, 152067
  %204 = getelementptr float addrspace(1)* %4, i64 %.sum9169vector_func.i
  %exData2948vector_func.i = extractelement <16 x float> %call.i13.i, i32 3
  store float %exData2948vector_func.i, float addrspace(1)* %204, align 4
  br label %postload6249vector_func.i

postload6249vector_func.i:                        ; preds = %preload6248vector_func.i, %postload6246vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6938vector_func.i, label %postload6939vector_func.i

preload6938vector_func.i:                         ; preds = %postload6249vector_func.i
  %.sum9168vector_func.i = add i64 %dim_0_vector_tid.i, 152068
  %205 = getelementptr float addrspace(1)* %4, i64 %.sum9168vector_func.i
  %exData2951vector_func.i = extractelement <16 x float> %call.i13.i, i32 4
  store float %exData2951vector_func.i, float addrspace(1)* %205, align 4
  br label %postload6939vector_func.i

postload6939vector_func.i:                        ; preds = %preload6938vector_func.i, %postload6249vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6941vector_func.i, label %postload6942vector_func.i

preload6941vector_func.i:                         ; preds = %postload6939vector_func.i
  %.sum9167vector_func.i = add i64 %dim_0_vector_tid.i, 152069
  %206 = getelementptr float addrspace(1)* %4, i64 %.sum9167vector_func.i
  %exData2954vector_func.i = extractelement <16 x float> %call.i13.i, i32 5
  store float %exData2954vector_func.i, float addrspace(1)* %206, align 4
  br label %postload6942vector_func.i

postload6942vector_func.i:                        ; preds = %preload6941vector_func.i, %postload6939vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6272vector_func.i, label %postload6273vector_func.i

preload6272vector_func.i:                         ; preds = %postload6942vector_func.i
  %.sum9166vector_func.i = add i64 %dim_0_vector_tid.i, 152070
  %207 = getelementptr float addrspace(1)* %4, i64 %.sum9166vector_func.i
  %exData2957vector_func.i = extractelement <16 x float> %call.i13.i, i32 6
  store float %exData2957vector_func.i, float addrspace(1)* %207, align 4
  br label %postload6273vector_func.i

postload6273vector_func.i:                        ; preds = %preload6272vector_func.i, %postload6942vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6275vector_func.i, label %postload6276vector_func.i

preload6275vector_func.i:                         ; preds = %postload6273vector_func.i
  %.sum9165vector_func.i = add i64 %dim_0_vector_tid.i, 152071
  %208 = getelementptr float addrspace(1)* %4, i64 %.sum9165vector_func.i
  %exData2960vector_func.i = extractelement <16 x float> %call.i13.i, i32 7
  store float %exData2960vector_func.i, float addrspace(1)* %208, align 4
  br label %postload6276vector_func.i

postload6276vector_func.i:                        ; preds = %preload6275vector_func.i, %postload6273vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6278vector_func.i, label %postload6279vector_func.i

preload6278vector_func.i:                         ; preds = %postload6276vector_func.i
  %.sum9164vector_func.i = add i64 %dim_0_vector_tid.i, 152072
  %209 = getelementptr float addrspace(1)* %4, i64 %.sum9164vector_func.i
  %exData2963vector_func.i = extractelement <16 x float> %call.i13.i, i32 8
  store float %exData2963vector_func.i, float addrspace(1)* %209, align 4
  br label %postload6279vector_func.i

postload6279vector_func.i:                        ; preds = %preload6278vector_func.i, %postload6276vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5525vector_func.i, label %postload5526vector_func.i

preload5525vector_func.i:                         ; preds = %postload6279vector_func.i
  %.sum9163vector_func.i = add i64 %dim_0_vector_tid.i, 152073
  %210 = getelementptr float addrspace(1)* %4, i64 %.sum9163vector_func.i
  %exData2966vector_func.i = extractelement <16 x float> %call.i13.i, i32 9
  store float %exData2966vector_func.i, float addrspace(1)* %210, align 4
  br label %postload5526vector_func.i

postload5526vector_func.i:                        ; preds = %preload5525vector_func.i, %postload6279vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5528vector_func.i, label %postload5529vector_func.i

preload5528vector_func.i:                         ; preds = %postload5526vector_func.i
  %.sum9162vector_func.i = add i64 %dim_0_vector_tid.i, 152074
  %211 = getelementptr float addrspace(1)* %4, i64 %.sum9162vector_func.i
  %exData2969vector_func.i = extractelement <16 x float> %call.i13.i, i32 10
  store float %exData2969vector_func.i, float addrspace(1)* %211, align 4
  br label %postload5529vector_func.i

postload5529vector_func.i:                        ; preds = %preload5528vector_func.i, %postload5526vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7394vector_func.i, label %postload7395vector_func.i

preload7394vector_func.i:                         ; preds = %postload5529vector_func.i
  %.sum9161vector_func.i = add i64 %dim_0_vector_tid.i, 152075
  %212 = getelementptr float addrspace(1)* %4, i64 %.sum9161vector_func.i
  %exData2972vector_func.i = extractelement <16 x float> %call.i13.i, i32 11
  store float %exData2972vector_func.i, float addrspace(1)* %212, align 4
  br label %postload7395vector_func.i

postload7395vector_func.i:                        ; preds = %preload7394vector_func.i, %postload5529vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7397vector_func.i, label %postload7398vector_func.i

preload7397vector_func.i:                         ; preds = %postload7395vector_func.i
  %.sum9160vector_func.i = add i64 %dim_0_vector_tid.i, 152076
  %213 = getelementptr float addrspace(1)* %4, i64 %.sum9160vector_func.i
  %exData2975vector_func.i = extractelement <16 x float> %call.i13.i, i32 12
  store float %exData2975vector_func.i, float addrspace(1)* %213, align 4
  br label %postload7398vector_func.i

postload7398vector_func.i:                        ; preds = %preload7397vector_func.i, %postload7395vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7259vector_func.i, label %postload7260vector_func.i

preload7259vector_func.i:                         ; preds = %postload7398vector_func.i
  %.sum9159vector_func.i = add i64 %dim_0_vector_tid.i, 152077
  %214 = getelementptr float addrspace(1)* %4, i64 %.sum9159vector_func.i
  %exData2978vector_func.i = extractelement <16 x float> %call.i13.i, i32 13
  store float %exData2978vector_func.i, float addrspace(1)* %214, align 4
  br label %postload7260vector_func.i

postload7260vector_func.i:                        ; preds = %preload7259vector_func.i, %postload7398vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7262vector_func.i, label %postload7263vector_func.i

preload7262vector_func.i:                         ; preds = %postload7260vector_func.i
  %.sum9158vector_func.i = add i64 %dim_0_vector_tid.i, 152078
  %215 = getelementptr float addrspace(1)* %4, i64 %.sum9158vector_func.i
  %exData2981vector_func.i = extractelement <16 x float> %call.i13.i, i32 14
  store float %exData2981vector_func.i, float addrspace(1)* %215, align 4
  br label %postload7263vector_func.i

postload7263vector_func.i:                        ; preds = %preload7262vector_func.i, %postload7260vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7265vector_func.i, label %postload7266vector_func.i

preload7265vector_func.i:                         ; preds = %postload7263vector_func.i
  %.sum9157vector_func.i = add i64 %dim_0_vector_tid.i, 152079
  %216 = getelementptr float addrspace(1)* %4, i64 %.sum9157vector_func.i
  %exData2984vector_func.i = extractelement <16 x float> %call.i13.i, i32 15
  store float %exData2984vector_func.i, float addrspace(1)* %216, align 4
  br label %postload7266vector_func.i

postload7266vector_func.i:                        ; preds = %preload7265vector_func.i, %postload7263vector_func.i
  %mul121917vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000, float 0x40C27E2C20000000>
  %add122918vector_func.i = fadd <16 x float> %mul121917vector_func.i, <float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000, float 0x40326FF420000000>
  %mul123919vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000, float 0x3FB32977C0000000>
  %add124920vector_func.i = fadd <16 x float> %add122918vector_func.i, %mul123919vector_func.i
  %mul.i344921vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000, float 0xBCF6ED3FA0000000>
  %add.i345922vector_func.i = fadd <16 x float> %mul.i344921vector_func.i, <float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000, float 0x3DDC034F60000000>
  %mul1.i346923vector_func.i = fmul <16 x float> %add.i345922vector_func.i, %mul567vector_func.i
  %add2.i347924vector_func.i = fadd <16 x float> %mul1.i346923vector_func.i, <float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000, float 0xBEB007BD60000000>
  %mul3.i348925vector_func.i = fmul <16 x float> %add2.i347924vector_func.i, %mul567vector_func.i
  %add4.i349926vector_func.i = fadd <16 x float> %mul3.i348925vector_func.i, <float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000, float 0x3F7B6CB680000000>
  %mul5.i350927vector_func.i = fmul <16 x float> %add4.i349926vector_func.i, %mul567vector_func.i
  %add126928vector_func.i = fadd <16 x float> %add124920vector_func.i, %mul5.i350927vector_func.i
  %call.i14.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add126928vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7181vector_func.i, label %postload7182vector_func.i

preload7181vector_func.i:                         ; preds = %postload7266vector_func.i
  %extract930vector_func.i = add i64 %dim_0_vector_tid.i, 165888
  %217 = getelementptr inbounds float addrspace(1)* %4, i64 %extract930vector_func.i
  %exData2988vector_func.i = extractelement <16 x float> %call.i14.i, i32 0
  store float %exData2988vector_func.i, float addrspace(1)* %217, align 4
  br label %postload7182vector_func.i

postload7182vector_func.i:                        ; preds = %preload7181vector_func.i, %postload7266vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7184vector_func.i, label %postload7185vector_func.i

preload7184vector_func.i:                         ; preds = %postload7182vector_func.i
  %.sum9156vector_func.i = add i64 %dim_0_vector_tid.i, 165889
  %218 = getelementptr float addrspace(1)* %4, i64 %.sum9156vector_func.i
  %exData2991vector_func.i = extractelement <16 x float> %call.i14.i, i32 1
  store float %exData2991vector_func.i, float addrspace(1)* %218, align 4
  br label %postload7185vector_func.i

postload7185vector_func.i:                        ; preds = %preload7184vector_func.i, %postload7182vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7130vector_func.i, label %postload7131vector_func.i

preload7130vector_func.i:                         ; preds = %postload7185vector_func.i
  %.sum9155vector_func.i = add i64 %dim_0_vector_tid.i, 165890
  %219 = getelementptr float addrspace(1)* %4, i64 %.sum9155vector_func.i
  %exData2994vector_func.i = extractelement <16 x float> %call.i14.i, i32 2
  store float %exData2994vector_func.i, float addrspace(1)* %219, align 4
  br label %postload7131vector_func.i

postload7131vector_func.i:                        ; preds = %preload7130vector_func.i, %postload7185vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7133vector_func.i, label %postload7134vector_func.i

preload7133vector_func.i:                         ; preds = %postload7131vector_func.i
  %.sum9154vector_func.i = add i64 %dim_0_vector_tid.i, 165891
  %220 = getelementptr float addrspace(1)* %4, i64 %.sum9154vector_func.i
  %exData2997vector_func.i = extractelement <16 x float> %call.i14.i, i32 3
  store float %exData2997vector_func.i, float addrspace(1)* %220, align 4
  br label %postload7134vector_func.i

postload7134vector_func.i:                        ; preds = %preload7133vector_func.i, %postload7131vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7136vector_func.i, label %postload7137vector_func.i

preload7136vector_func.i:                         ; preds = %postload7134vector_func.i
  %.sum9153vector_func.i = add i64 %dim_0_vector_tid.i, 165892
  %221 = getelementptr float addrspace(1)* %4, i64 %.sum9153vector_func.i
  %exData3000vector_func.i = extractelement <16 x float> %call.i14.i, i32 4
  store float %exData3000vector_func.i, float addrspace(1)* %221, align 4
  br label %postload7137vector_func.i

postload7137vector_func.i:                        ; preds = %preload7136vector_func.i, %postload7134vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7109vector_func.i, label %postload7110vector_func.i

preload7109vector_func.i:                         ; preds = %postload7137vector_func.i
  %.sum9152vector_func.i = add i64 %dim_0_vector_tid.i, 165893
  %222 = getelementptr float addrspace(1)* %4, i64 %.sum9152vector_func.i
  %exData3003vector_func.i = extractelement <16 x float> %call.i14.i, i32 5
  store float %exData3003vector_func.i, float addrspace(1)* %222, align 4
  br label %postload7110vector_func.i

postload7110vector_func.i:                        ; preds = %preload7109vector_func.i, %postload7137vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7112vector_func.i, label %postload7113vector_func.i

preload7112vector_func.i:                         ; preds = %postload7110vector_func.i
  %.sum9151vector_func.i = add i64 %dim_0_vector_tid.i, 165894
  %223 = getelementptr float addrspace(1)* %4, i64 %.sum9151vector_func.i
  %exData3006vector_func.i = extractelement <16 x float> %call.i14.i, i32 6
  store float %exData3006vector_func.i, float addrspace(1)* %223, align 4
  br label %postload7113vector_func.i

postload7113vector_func.i:                        ; preds = %preload7112vector_func.i, %postload7110vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7016vector_func.i, label %postload7017vector_func.i

preload7016vector_func.i:                         ; preds = %postload7113vector_func.i
  %.sum9150vector_func.i = add i64 %dim_0_vector_tid.i, 165895
  %224 = getelementptr float addrspace(1)* %4, i64 %.sum9150vector_func.i
  %exData3009vector_func.i = extractelement <16 x float> %call.i14.i, i32 7
  store float %exData3009vector_func.i, float addrspace(1)* %224, align 4
  br label %postload7017vector_func.i

postload7017vector_func.i:                        ; preds = %preload7016vector_func.i, %postload7113vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7019vector_func.i, label %postload7020vector_func.i

preload7019vector_func.i:                         ; preds = %postload7017vector_func.i
  %.sum9149vector_func.i = add i64 %dim_0_vector_tid.i, 165896
  %225 = getelementptr float addrspace(1)* %4, i64 %.sum9149vector_func.i
  %exData3012vector_func.i = extractelement <16 x float> %call.i14.i, i32 8
  store float %exData3012vector_func.i, float addrspace(1)* %225, align 4
  br label %postload7020vector_func.i

postload7020vector_func.i:                        ; preds = %preload7019vector_func.i, %postload7017vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7022vector_func.i, label %postload7023vector_func.i

preload7022vector_func.i:                         ; preds = %postload7020vector_func.i
  %.sum9148vector_func.i = add i64 %dim_0_vector_tid.i, 165897
  %226 = getelementptr float addrspace(1)* %4, i64 %.sum9148vector_func.i
  %exData3015vector_func.i = extractelement <16 x float> %call.i14.i, i32 9
  store float %exData3015vector_func.i, float addrspace(1)* %226, align 4
  br label %postload7023vector_func.i

postload7023vector_func.i:                        ; preds = %preload7022vector_func.i, %postload7020vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6983vector_func.i, label %postload6984vector_func.i

preload6983vector_func.i:                         ; preds = %postload7023vector_func.i
  %.sum9147vector_func.i = add i64 %dim_0_vector_tid.i, 165898
  %227 = getelementptr float addrspace(1)* %4, i64 %.sum9147vector_func.i
  %exData3018vector_func.i = extractelement <16 x float> %call.i14.i, i32 10
  store float %exData3018vector_func.i, float addrspace(1)* %227, align 4
  br label %postload6984vector_func.i

postload6984vector_func.i:                        ; preds = %preload6983vector_func.i, %postload7023vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6986vector_func.i, label %postload6987vector_func.i

preload6986vector_func.i:                         ; preds = %postload6984vector_func.i
  %.sum9146vector_func.i = add i64 %dim_0_vector_tid.i, 165899
  %228 = getelementptr float addrspace(1)* %4, i64 %.sum9146vector_func.i
  %exData3021vector_func.i = extractelement <16 x float> %call.i14.i, i32 11
  store float %exData3021vector_func.i, float addrspace(1)* %228, align 4
  br label %postload6987vector_func.i

postload6987vector_func.i:                        ; preds = %preload6986vector_func.i, %postload6984vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6965vector_func.i, label %postload6966vector_func.i

preload6965vector_func.i:                         ; preds = %postload6987vector_func.i
  %.sum9145vector_func.i = add i64 %dim_0_vector_tid.i, 165900
  %229 = getelementptr float addrspace(1)* %4, i64 %.sum9145vector_func.i
  %exData3024vector_func.i = extractelement <16 x float> %call.i14.i, i32 12
  store float %exData3024vector_func.i, float addrspace(1)* %229, align 4
  br label %postload6966vector_func.i

postload6966vector_func.i:                        ; preds = %preload6965vector_func.i, %postload6987vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6968vector_func.i, label %postload6969vector_func.i

preload6968vector_func.i:                         ; preds = %postload6966vector_func.i
  %.sum9144vector_func.i = add i64 %dim_0_vector_tid.i, 165901
  %230 = getelementptr float addrspace(1)* %4, i64 %.sum9144vector_func.i
  %exData3027vector_func.i = extractelement <16 x float> %call.i14.i, i32 13
  store float %exData3027vector_func.i, float addrspace(1)* %230, align 4
  br label %postload6969vector_func.i

postload6969vector_func.i:                        ; preds = %preload6968vector_func.i, %postload6966vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6971vector_func.i, label %postload6972vector_func.i

preload6971vector_func.i:                         ; preds = %postload6969vector_func.i
  %.sum9143vector_func.i = add i64 %dim_0_vector_tid.i, 165902
  %231 = getelementptr float addrspace(1)* %4, i64 %.sum9143vector_func.i
  %exData3030vector_func.i = extractelement <16 x float> %call.i14.i, i32 14
  store float %exData3030vector_func.i, float addrspace(1)* %231, align 4
  br label %postload6972vector_func.i

postload6972vector_func.i:                        ; preds = %preload6971vector_func.i, %postload6969vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6524vector_func.i, label %postload6525vector_func.i

preload6524vector_func.i:                         ; preds = %postload6972vector_func.i
  %.sum9142vector_func.i = add i64 %dim_0_vector_tid.i, 165903
  %232 = getelementptr float addrspace(1)* %4, i64 %.sum9142vector_func.i
  %exData3033vector_func.i = extractelement <16 x float> %call.i14.i, i32 15
  store float %exData3033vector_func.i, float addrspace(1)* %232, align 4
  br label %postload6525vector_func.i

postload6525vector_func.i:                        ; preds = %preload6524vector_func.i, %postload6972vector_func.i
  %mul131947vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000, float 0x40CBA3EFA0000000>
  %add132948vector_func.i = fadd <16 x float> %mul131947vector_func.i, <float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000, float 0x401F465620000000>
  %mul133949vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000, float 0x4005B8B340000000>
  %add134950vector_func.i = fadd <16 x float> %add132948vector_func.i, %mul133949vector_func.i
  %mul.i337951vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000, float 0xBCD257CBE0000000>
  %add.i338952vector_func.i = fadd <16 x float> %mul.i337951vector_func.i, <float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000, float 0x3DB5142E40000000>
  %mul1.i339953vector_func.i = fmul <16 x float> %add.i338952vector_func.i, %mul567vector_func.i
  %add2.i340954vector_func.i = fadd <16 x float> %mul1.i339953vector_func.i, <float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000, float 0xBE8657E620000000>
  %mul3.i341955vector_func.i = fmul <16 x float> %add2.i340954vector_func.i, %mul567vector_func.i
  %add4.i342956vector_func.i = fadd <16 x float> %mul3.i341955vector_func.i, <float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000, float 0x3F50E56F00000000>
  %mul5.i343957vector_func.i = fmul <16 x float> %add4.i342956vector_func.i, %mul567vector_func.i
  %add136958vector_func.i = fadd <16 x float> %add134950vector_func.i, %mul5.i343957vector_func.i
  %call.i15.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add136958vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6527vector_func.i, label %postload6528vector_func.i

preload6527vector_func.i:                         ; preds = %postload6525vector_func.i
  %extract960vector_func.i = add i64 %dim_0_vector_tid.i, 179712
  %233 = getelementptr inbounds float addrspace(1)* %4, i64 %extract960vector_func.i
  %exData3037vector_func.i = extractelement <16 x float> %call.i15.i, i32 0
  store float %exData3037vector_func.i, float addrspace(1)* %233, align 4
  br label %postload6528vector_func.i

postload6528vector_func.i:                        ; preds = %preload6527vector_func.i, %postload6525vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6530vector_func.i, label %postload6531vector_func.i

preload6530vector_func.i:                         ; preds = %postload6528vector_func.i
  %.sum9141vector_func.i = add i64 %dim_0_vector_tid.i, 179713
  %234 = getelementptr float addrspace(1)* %4, i64 %.sum9141vector_func.i
  %exData3040vector_func.i = extractelement <16 x float> %call.i15.i, i32 1
  store float %exData3040vector_func.i, float addrspace(1)* %234, align 4
  br label %postload6531vector_func.i

postload6531vector_func.i:                        ; preds = %preload6530vector_func.i, %postload6528vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6467vector_func.i, label %postload6468vector_func.i

preload6467vector_func.i:                         ; preds = %postload6531vector_func.i
  %.sum9140vector_func.i = add i64 %dim_0_vector_tid.i, 179714
  %235 = getelementptr float addrspace(1)* %4, i64 %.sum9140vector_func.i
  %exData3043vector_func.i = extractelement <16 x float> %call.i15.i, i32 2
  store float %exData3043vector_func.i, float addrspace(1)* %235, align 4
  br label %postload6468vector_func.i

postload6468vector_func.i:                        ; preds = %preload6467vector_func.i, %postload6531vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6470vector_func.i, label %postload6471vector_func.i

preload6470vector_func.i:                         ; preds = %postload6468vector_func.i
  %.sum9139vector_func.i = add i64 %dim_0_vector_tid.i, 179715
  %236 = getelementptr float addrspace(1)* %4, i64 %.sum9139vector_func.i
  %exData3046vector_func.i = extractelement <16 x float> %call.i15.i, i32 3
  store float %exData3046vector_func.i, float addrspace(1)* %236, align 4
  br label %postload6471vector_func.i

postload6471vector_func.i:                        ; preds = %preload6470vector_func.i, %postload6468vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6428vector_func.i, label %postload6429vector_func.i

preload6428vector_func.i:                         ; preds = %postload6471vector_func.i
  %.sum9138vector_func.i = add i64 %dim_0_vector_tid.i, 179716
  %237 = getelementptr float addrspace(1)* %4, i64 %.sum9138vector_func.i
  %exData3049vector_func.i = extractelement <16 x float> %call.i15.i, i32 4
  store float %exData3049vector_func.i, float addrspace(1)* %237, align 4
  br label %postload6429vector_func.i

postload6429vector_func.i:                        ; preds = %preload6428vector_func.i, %postload6471vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6431vector_func.i, label %postload6432vector_func.i

preload6431vector_func.i:                         ; preds = %postload6429vector_func.i
  %.sum9137vector_func.i = add i64 %dim_0_vector_tid.i, 179717
  %238 = getelementptr float addrspace(1)* %4, i64 %.sum9137vector_func.i
  %exData3052vector_func.i = extractelement <16 x float> %call.i15.i, i32 5
  store float %exData3052vector_func.i, float addrspace(1)* %238, align 4
  br label %postload6432vector_func.i

postload6432vector_func.i:                        ; preds = %preload6431vector_func.i, %postload6429vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6434vector_func.i, label %postload6435vector_func.i

preload6434vector_func.i:                         ; preds = %postload6432vector_func.i
  %.sum9136vector_func.i = add i64 %dim_0_vector_tid.i, 179718
  %239 = getelementptr float addrspace(1)* %4, i64 %.sum9136vector_func.i
  %exData3055vector_func.i = extractelement <16 x float> %call.i15.i, i32 6
  store float %exData3055vector_func.i, float addrspace(1)* %239, align 4
  br label %postload6435vector_func.i

postload6435vector_func.i:                        ; preds = %preload6434vector_func.i, %postload6432vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6353vector_func.i, label %postload6354vector_func.i

preload6353vector_func.i:                         ; preds = %postload6435vector_func.i
  %.sum9135vector_func.i = add i64 %dim_0_vector_tid.i, 179719
  %240 = getelementptr float addrspace(1)* %4, i64 %.sum9135vector_func.i
  %exData3058vector_func.i = extractelement <16 x float> %call.i15.i, i32 7
  store float %exData3058vector_func.i, float addrspace(1)* %240, align 4
  br label %postload6354vector_func.i

postload6354vector_func.i:                        ; preds = %preload6353vector_func.i, %postload6435vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6356vector_func.i, label %postload6357vector_func.i

preload6356vector_func.i:                         ; preds = %postload6354vector_func.i
  %.sum9134vector_func.i = add i64 %dim_0_vector_tid.i, 179720
  %241 = getelementptr float addrspace(1)* %4, i64 %.sum9134vector_func.i
  %exData3061vector_func.i = extractelement <16 x float> %call.i15.i, i32 8
  store float %exData3061vector_func.i, float addrspace(1)* %241, align 4
  br label %postload6357vector_func.i

postload6357vector_func.i:                        ; preds = %preload6356vector_func.i, %postload6354vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6335vector_func.i, label %postload6336vector_func.i

preload6335vector_func.i:                         ; preds = %postload6357vector_func.i
  %.sum9133vector_func.i = add i64 %dim_0_vector_tid.i, 179721
  %242 = getelementptr float addrspace(1)* %4, i64 %.sum9133vector_func.i
  %exData3064vector_func.i = extractelement <16 x float> %call.i15.i, i32 9
  store float %exData3064vector_func.i, float addrspace(1)* %242, align 4
  br label %postload6336vector_func.i

postload6336vector_func.i:                        ; preds = %preload6335vector_func.i, %postload6357vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6338vector_func.i, label %postload6339vector_func.i

preload6338vector_func.i:                         ; preds = %postload6336vector_func.i
  %.sum9132vector_func.i = add i64 %dim_0_vector_tid.i, 179722
  %243 = getelementptr float addrspace(1)* %4, i64 %.sum9132vector_func.i
  %exData3067vector_func.i = extractelement <16 x float> %call.i15.i, i32 10
  store float %exData3067vector_func.i, float addrspace(1)* %243, align 4
  br label %postload6339vector_func.i

postload6339vector_func.i:                        ; preds = %preload6338vector_func.i, %postload6336vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6341vector_func.i, label %postload6342vector_func.i

preload6341vector_func.i:                         ; preds = %postload6339vector_func.i
  %.sum9131vector_func.i = add i64 %dim_0_vector_tid.i, 179723
  %244 = getelementptr float addrspace(1)* %4, i64 %.sum9131vector_func.i
  %exData3070vector_func.i = extractelement <16 x float> %call.i15.i, i32 11
  store float %exData3070vector_func.i, float addrspace(1)* %244, align 4
  br label %postload6342vector_func.i

postload6342vector_func.i:                        ; preds = %preload6341vector_func.i, %postload6339vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6224vector_func.i, label %postload6225vector_func.i

preload6224vector_func.i:                         ; preds = %postload6342vector_func.i
  %.sum9130vector_func.i = add i64 %dim_0_vector_tid.i, 179724
  %245 = getelementptr float addrspace(1)* %4, i64 %.sum9130vector_func.i
  %exData3073vector_func.i = extractelement <16 x float> %call.i15.i, i32 12
  store float %exData3073vector_func.i, float addrspace(1)* %245, align 4
  br label %postload6225vector_func.i

postload6225vector_func.i:                        ; preds = %preload6224vector_func.i, %postload6342vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6227vector_func.i, label %postload6228vector_func.i

preload6227vector_func.i:                         ; preds = %postload6225vector_func.i
  %.sum9129vector_func.i = add i64 %dim_0_vector_tid.i, 179725
  %246 = getelementptr float addrspace(1)* %4, i64 %.sum9129vector_func.i
  %exData3076vector_func.i = extractelement <16 x float> %call.i15.i, i32 13
  store float %exData3076vector_func.i, float addrspace(1)* %246, align 4
  br label %postload6228vector_func.i

postload6228vector_func.i:                        ; preds = %preload6227vector_func.i, %postload6225vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6164vector_func.i, label %postload6165vector_func.i

preload6164vector_func.i:                         ; preds = %postload6228vector_func.i
  %.sum9128vector_func.i = add i64 %dim_0_vector_tid.i, 179726
  %247 = getelementptr float addrspace(1)* %4, i64 %.sum9128vector_func.i
  %exData3079vector_func.i = extractelement <16 x float> %call.i15.i, i32 14
  store float %exData3079vector_func.i, float addrspace(1)* %247, align 4
  br label %postload6165vector_func.i

postload6165vector_func.i:                        ; preds = %preload6164vector_func.i, %postload6228vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6167vector_func.i, label %postload6168vector_func.i

preload6167vector_func.i:                         ; preds = %postload6165vector_func.i
  %.sum9127vector_func.i = add i64 %dim_0_vector_tid.i, 179727
  %248 = getelementptr float addrspace(1)* %4, i64 %.sum9127vector_func.i
  %exData3082vector_func.i = extractelement <16 x float> %call.i15.i, i32 15
  store float %exData3082vector_func.i, float addrspace(1)* %248, align 4
  br label %postload6168vector_func.i

postload6168vector_func.i:                        ; preds = %preload6167vector_func.i, %postload6165vector_func.i
  %mul141977vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000, float 0x40E7CEE540000000>
  %add142978vector_func.i = fadd <16 x float> %mul141977vector_func.i, <float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000, float 0x40022C50A0000000>
  %mul143979vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000, float 0x400EDC1420000000>
  %add144980vector_func.i = fadd <16 x float> %add142978vector_func.i, %mul143979vector_func.i
  %mul.i330981vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000, float 0xBCE542C280000000>
  %add.i331982vector_func.i = fadd <16 x float> %mul.i330981vector_func.i, <float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000, float 0x3DC7FB8EC0000000>
  %mul1.i332983vector_func.i = fmul <16 x float> %add.i331982vector_func.i, %mul567vector_func.i
  %add2.i333984vector_func.i = fadd <16 x float> %mul1.i332983vector_func.i, <float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000, float 0xBE98C5B3E0000000>
  %mul3.i334985vector_func.i = fmul <16 x float> %add2.i333984vector_func.i, %mul567vector_func.i
  %add4.i335986vector_func.i = fadd <16 x float> %mul3.i334985vector_func.i, <float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000, float 0x3F6214CD80000000>
  %mul5.i336987vector_func.i = fmul <16 x float> %add4.i335986vector_func.i, %mul567vector_func.i
  %add146988vector_func.i = fadd <16 x float> %add144980vector_func.i, %mul5.i336987vector_func.i
  %call.i16.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add146988vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6170vector_func.i, label %postload6171vector_func.i

preload6170vector_func.i:                         ; preds = %postload6168vector_func.i
  %extract990vector_func.i = add i64 %dim_0_vector_tid.i, 193536
  %249 = getelementptr inbounds float addrspace(1)* %4, i64 %extract990vector_func.i
  %exData3086vector_func.i = extractelement <16 x float> %call.i16.i, i32 0
  store float %exData3086vector_func.i, float addrspace(1)* %249, align 4
  br label %postload6171vector_func.i

postload6171vector_func.i:                        ; preds = %preload6170vector_func.i, %postload6168vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6158vector_func.i, label %postload6159vector_func.i

preload6158vector_func.i:                         ; preds = %postload6171vector_func.i
  %.sum9126vector_func.i = add i64 %dim_0_vector_tid.i, 193537
  %250 = getelementptr float addrspace(1)* %4, i64 %.sum9126vector_func.i
  %exData3089vector_func.i = extractelement <16 x float> %call.i16.i, i32 1
  store float %exData3089vector_func.i, float addrspace(1)* %250, align 4
  br label %postload6159vector_func.i

postload6159vector_func.i:                        ; preds = %preload6158vector_func.i, %postload6171vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6161vector_func.i, label %postload6162vector_func.i

preload6161vector_func.i:                         ; preds = %postload6159vector_func.i
  %.sum9125vector_func.i = add i64 %dim_0_vector_tid.i, 193538
  %251 = getelementptr float addrspace(1)* %4, i64 %.sum9125vector_func.i
  %exData3092vector_func.i = extractelement <16 x float> %call.i16.i, i32 2
  store float %exData3092vector_func.i, float addrspace(1)* %251, align 4
  br label %postload6162vector_func.i

postload6162vector_func.i:                        ; preds = %preload6161vector_func.i, %postload6159vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7451vector_func.i, label %postload7452vector_func.i

preload7451vector_func.i:                         ; preds = %postload6162vector_func.i
  %.sum9124vector_func.i = add i64 %dim_0_vector_tid.i, 193539
  %252 = getelementptr float addrspace(1)* %4, i64 %.sum9124vector_func.i
  %exData3095vector_func.i = extractelement <16 x float> %call.i16.i, i32 3
  store float %exData3095vector_func.i, float addrspace(1)* %252, align 4
  br label %postload7452vector_func.i

postload7452vector_func.i:                        ; preds = %preload7451vector_func.i, %postload6162vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7454vector_func.i, label %postload7455vector_func.i

preload7454vector_func.i:                         ; preds = %postload7452vector_func.i
  %.sum9123vector_func.i = add i64 %dim_0_vector_tid.i, 193540
  %253 = getelementptr float addrspace(1)* %4, i64 %.sum9123vector_func.i
  %exData3098vector_func.i = extractelement <16 x float> %call.i16.i, i32 4
  store float %exData3098vector_func.i, float addrspace(1)* %253, align 4
  br label %postload7455vector_func.i

postload7455vector_func.i:                        ; preds = %preload7454vector_func.i, %postload7452vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7457vector_func.i, label %postload7458vector_func.i

preload7457vector_func.i:                         ; preds = %postload7455vector_func.i
  %.sum9122vector_func.i = add i64 %dim_0_vector_tid.i, 193541
  %254 = getelementptr float addrspace(1)* %4, i64 %.sum9122vector_func.i
  %exData3101vector_func.i = extractelement <16 x float> %call.i16.i, i32 5
  store float %exData3101vector_func.i, float addrspace(1)* %254, align 4
  br label %postload7458vector_func.i

postload7458vector_func.i:                        ; preds = %preload7457vector_func.i, %postload7455vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7094vector_func.i, label %postload7095vector_func.i

preload7094vector_func.i:                         ; preds = %postload7458vector_func.i
  %.sum9121vector_func.i = add i64 %dim_0_vector_tid.i, 193542
  %255 = getelementptr float addrspace(1)* %4, i64 %.sum9121vector_func.i
  %exData3104vector_func.i = extractelement <16 x float> %call.i16.i, i32 6
  store float %exData3104vector_func.i, float addrspace(1)* %255, align 4
  br label %postload7095vector_func.i

postload7095vector_func.i:                        ; preds = %preload7094vector_func.i, %postload7458vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7097vector_func.i, label %postload7098vector_func.i

preload7097vector_func.i:                         ; preds = %postload7095vector_func.i
  %.sum9120vector_func.i = add i64 %dim_0_vector_tid.i, 193543
  %256 = getelementptr float addrspace(1)* %4, i64 %.sum9120vector_func.i
  %exData3107vector_func.i = extractelement <16 x float> %call.i16.i, i32 7
  store float %exData3107vector_func.i, float addrspace(1)* %256, align 4
  br label %postload7098vector_func.i

postload7098vector_func.i:                        ; preds = %preload7097vector_func.i, %postload7095vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6956vector_func.i, label %postload6957vector_func.i

preload6956vector_func.i:                         ; preds = %postload7098vector_func.i
  %.sum9119vector_func.i = add i64 %dim_0_vector_tid.i, 193544
  %257 = getelementptr float addrspace(1)* %4, i64 %.sum9119vector_func.i
  %exData3110vector_func.i = extractelement <16 x float> %call.i16.i, i32 8
  store float %exData3110vector_func.i, float addrspace(1)* %257, align 4
  br label %postload6957vector_func.i

postload6957vector_func.i:                        ; preds = %preload6956vector_func.i, %postload7098vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6959vector_func.i, label %postload6960vector_func.i

preload6959vector_func.i:                         ; preds = %postload6957vector_func.i
  %.sum9118vector_func.i = add i64 %dim_0_vector_tid.i, 193545
  %258 = getelementptr float addrspace(1)* %4, i64 %.sum9118vector_func.i
  %exData3113vector_func.i = extractelement <16 x float> %call.i16.i, i32 9
  store float %exData3113vector_func.i, float addrspace(1)* %258, align 4
  br label %postload6960vector_func.i

postload6960vector_func.i:                        ; preds = %preload6959vector_func.i, %postload6957vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6962vector_func.i, label %postload6963vector_func.i

preload6962vector_func.i:                         ; preds = %postload6960vector_func.i
  %.sum9117vector_func.i = add i64 %dim_0_vector_tid.i, 193546
  %259 = getelementptr float addrspace(1)* %4, i64 %.sum9117vector_func.i
  %exData3116vector_func.i = extractelement <16 x float> %call.i16.i, i32 10
  store float %exData3116vector_func.i, float addrspace(1)* %259, align 4
  br label %postload6963vector_func.i

postload6963vector_func.i:                        ; preds = %preload6962vector_func.i, %postload6960vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7412vector_func.i, label %postload7413vector_func.i

preload7412vector_func.i:                         ; preds = %postload6963vector_func.i
  %.sum9116vector_func.i = add i64 %dim_0_vector_tid.i, 193547
  %260 = getelementptr float addrspace(1)* %4, i64 %.sum9116vector_func.i
  %exData3119vector_func.i = extractelement <16 x float> %call.i16.i, i32 11
  store float %exData3119vector_func.i, float addrspace(1)* %260, align 4
  br label %postload7413vector_func.i

postload7413vector_func.i:                        ; preds = %preload7412vector_func.i, %postload6963vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7415vector_func.i, label %postload7416vector_func.i

preload7415vector_func.i:                         ; preds = %postload7413vector_func.i
  %.sum9115vector_func.i = add i64 %dim_0_vector_tid.i, 193548
  %261 = getelementptr float addrspace(1)* %4, i64 %.sum9115vector_func.i
  %exData3122vector_func.i = extractelement <16 x float> %call.i16.i, i32 12
  store float %exData3122vector_func.i, float addrspace(1)* %261, align 4
  br label %postload7416vector_func.i

postload7416vector_func.i:                        ; preds = %preload7415vector_func.i, %postload7413vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7418vector_func.i, label %postload7419vector_func.i

preload7418vector_func.i:                         ; preds = %postload7416vector_func.i
  %.sum9114vector_func.i = add i64 %dim_0_vector_tid.i, 193549
  %262 = getelementptr float addrspace(1)* %4, i64 %.sum9114vector_func.i
  %exData3125vector_func.i = extractelement <16 x float> %call.i16.i, i32 13
  store float %exData3125vector_func.i, float addrspace(1)* %262, align 4
  br label %postload7419vector_func.i

postload7419vector_func.i:                        ; preds = %preload7418vector_func.i, %postload7416vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7379vector_func.i, label %postload7380vector_func.i

preload7379vector_func.i:                         ; preds = %postload7419vector_func.i
  %.sum9113vector_func.i = add i64 %dim_0_vector_tid.i, 193550
  %263 = getelementptr float addrspace(1)* %4, i64 %.sum9113vector_func.i
  %exData3128vector_func.i = extractelement <16 x float> %call.i16.i, i32 14
  store float %exData3128vector_func.i, float addrspace(1)* %263, align 4
  br label %postload7380vector_func.i

postload7380vector_func.i:                        ; preds = %preload7379vector_func.i, %postload7419vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7382vector_func.i, label %postload7383vector_func.i

preload7382vector_func.i:                         ; preds = %postload7380vector_func.i
  %.sum9112vector_func.i = add i64 %dim_0_vector_tid.i, 193551
  %264 = getelementptr float addrspace(1)* %4, i64 %.sum9112vector_func.i
  %exData3131vector_func.i = extractelement <16 x float> %call.i16.i, i32 15
  store float %exData3131vector_func.i, float addrspace(1)* %264, align 4
  br label %postload7383vector_func.i

postload7383vector_func.i:                        ; preds = %preload7382vector_func.i, %postload7380vector_func.i
  %mul1511007vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000, float 0x40AF57D620000000>
  %sub1521008vector_func.i = fsub <16 x float> <float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000, float 0x402398C0A0000000>, %mul1511007vector_func.i
  %mul1531009vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000, float 0x40062D69C0000000>
  %add1541010vector_func.i = fadd <16 x float> %sub1521008vector_func.i, %mul1531009vector_func.i
  %mul.i3231011vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000, float 0xBCE806EFC0000000>
  %add.i3241012vector_func.i = fadd <16 x float> %mul.i3231011vector_func.i, <float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000, float 0x3DCAFDC320000000>
  %mul1.i3251013vector_func.i = fmul <16 x float> %add.i3241012vector_func.i, %mul567vector_func.i
  %add2.i3261014vector_func.i = fadd <16 x float> %mul1.i3251013vector_func.i, <float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000, float 0xBE9BC9C5A0000000>
  %mul3.i3271015vector_func.i = fmul <16 x float> %add2.i3261014vector_func.i, %mul567vector_func.i
  %add4.i3281016vector_func.i = fadd <16 x float> %mul3.i3271015vector_func.i, <float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000, float 0x3F644DBE80000000>
  %mul5.i3291017vector_func.i = fmul <16 x float> %add4.i3281016vector_func.i, %mul567vector_func.i
  %add1561018vector_func.i = fadd <16 x float> %add1541010vector_func.i, %mul5.i3291017vector_func.i
  %call.i17.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add1561018vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6449vector_func.i, label %postload6450vector_func.i

preload6449vector_func.i:                         ; preds = %postload7383vector_func.i
  %extract1020vector_func.i = add i64 %dim_0_vector_tid.i, 207360
  %265 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1020vector_func.i
  %exData3135vector_func.i = extractelement <16 x float> %call.i17.i, i32 0
  store float %exData3135vector_func.i, float addrspace(1)* %265, align 4
  br label %postload6450vector_func.i

postload6450vector_func.i:                        ; preds = %preload6449vector_func.i, %postload7383vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7364vector_func.i, label %postload7365vector_func.i

preload7364vector_func.i:                         ; preds = %postload6450vector_func.i
  %.sum9111vector_func.i = add i64 %dim_0_vector_tid.i, 207361
  %266 = getelementptr float addrspace(1)* %4, i64 %.sum9111vector_func.i
  %exData3138vector_func.i = extractelement <16 x float> %call.i17.i, i32 1
  store float %exData3138vector_func.i, float addrspace(1)* %266, align 4
  br label %postload7365vector_func.i

postload7365vector_func.i:                        ; preds = %preload7364vector_func.i, %postload6450vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7367vector_func.i, label %postload7368vector_func.i

preload7367vector_func.i:                         ; preds = %postload7365vector_func.i
  %.sum9110vector_func.i = add i64 %dim_0_vector_tid.i, 207362
  %267 = getelementptr float addrspace(1)* %4, i64 %.sum9110vector_func.i
  %exData3141vector_func.i = extractelement <16 x float> %call.i17.i, i32 2
  store float %exData3141vector_func.i, float addrspace(1)* %267, align 4
  br label %postload7368vector_func.i

postload7368vector_func.i:                        ; preds = %preload7367vector_func.i, %postload7365vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7343vector_func.i, label %postload7344vector_func.i

preload7343vector_func.i:                         ; preds = %postload7368vector_func.i
  %.sum9109vector_func.i = add i64 %dim_0_vector_tid.i, 207363
  %268 = getelementptr float addrspace(1)* %4, i64 %.sum9109vector_func.i
  %exData3144vector_func.i = extractelement <16 x float> %call.i17.i, i32 3
  store float %exData3144vector_func.i, float addrspace(1)* %268, align 4
  br label %postload7344vector_func.i

postload7344vector_func.i:                        ; preds = %preload7343vector_func.i, %postload7368vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7346vector_func.i, label %postload7347vector_func.i

preload7346vector_func.i:                         ; preds = %postload7344vector_func.i
  %.sum9108vector_func.i = add i64 %dim_0_vector_tid.i, 207364
  %269 = getelementptr float addrspace(1)* %4, i64 %.sum9108vector_func.i
  %exData3147vector_func.i = extractelement <16 x float> %call.i17.i, i32 4
  store float %exData3147vector_func.i, float addrspace(1)* %269, align 4
  br label %postload7347vector_func.i

postload7347vector_func.i:                        ; preds = %preload7346vector_func.i, %postload7344vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7349vector_func.i, label %postload7350vector_func.i

preload7349vector_func.i:                         ; preds = %postload7347vector_func.i
  %.sum9107vector_func.i = add i64 %dim_0_vector_tid.i, 207365
  %270 = getelementptr float addrspace(1)* %4, i64 %.sum9107vector_func.i
  %exData3150vector_func.i = extractelement <16 x float> %call.i17.i, i32 5
  store float %exData3150vector_func.i, float addrspace(1)* %270, align 4
  br label %postload7350vector_func.i

postload7350vector_func.i:                        ; preds = %preload7349vector_func.i, %postload7347vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7325vector_func.i, label %postload7326vector_func.i

preload7325vector_func.i:                         ; preds = %postload7350vector_func.i
  %.sum9106vector_func.i = add i64 %dim_0_vector_tid.i, 207366
  %271 = getelementptr float addrspace(1)* %4, i64 %.sum9106vector_func.i
  %exData3153vector_func.i = extractelement <16 x float> %call.i17.i, i32 6
  store float %exData3153vector_func.i, float addrspace(1)* %271, align 4
  br label %postload7326vector_func.i

postload7326vector_func.i:                        ; preds = %preload7325vector_func.i, %postload7350vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7328vector_func.i, label %postload7329vector_func.i

preload7328vector_func.i:                         ; preds = %postload7326vector_func.i
  %.sum9105vector_func.i = add i64 %dim_0_vector_tid.i, 207367
  %272 = getelementptr float addrspace(1)* %4, i64 %.sum9105vector_func.i
  %exData3156vector_func.i = extractelement <16 x float> %call.i17.i, i32 7
  store float %exData3156vector_func.i, float addrspace(1)* %272, align 4
  br label %postload7329vector_func.i

postload7329vector_func.i:                        ; preds = %preload7328vector_func.i, %postload7326vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7331vector_func.i, label %postload7332vector_func.i

preload7331vector_func.i:                         ; preds = %postload7329vector_func.i
  %.sum9104vector_func.i = add i64 %dim_0_vector_tid.i, 207368
  %273 = getelementptr float addrspace(1)* %4, i64 %.sum9104vector_func.i
  %exData3159vector_func.i = extractelement <16 x float> %call.i17.i, i32 8
  store float %exData3159vector_func.i, float addrspace(1)* %273, align 4
  br label %postload7332vector_func.i

postload7332vector_func.i:                        ; preds = %preload7331vector_func.i, %postload7329vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7277vector_func.i, label %postload7278vector_func.i

preload7277vector_func.i:                         ; preds = %postload7332vector_func.i
  %.sum9103vector_func.i = add i64 %dim_0_vector_tid.i, 207369
  %274 = getelementptr float addrspace(1)* %4, i64 %.sum9103vector_func.i
  %exData3162vector_func.i = extractelement <16 x float> %call.i17.i, i32 9
  store float %exData3162vector_func.i, float addrspace(1)* %274, align 4
  br label %postload7278vector_func.i

postload7278vector_func.i:                        ; preds = %preload7277vector_func.i, %postload7332vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7280vector_func.i, label %postload7281vector_func.i

preload7280vector_func.i:                         ; preds = %postload7278vector_func.i
  %.sum9102vector_func.i = add i64 %dim_0_vector_tid.i, 207370
  %275 = getelementptr float addrspace(1)* %4, i64 %.sum9102vector_func.i
  %exData3165vector_func.i = extractelement <16 x float> %call.i17.i, i32 10
  store float %exData3165vector_func.i, float addrspace(1)* %275, align 4
  br label %postload7281vector_func.i

postload7281vector_func.i:                        ; preds = %preload7280vector_func.i, %postload7278vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7283vector_func.i, label %postload7284vector_func.i

preload7283vector_func.i:                         ; preds = %postload7281vector_func.i
  %.sum9101vector_func.i = add i64 %dim_0_vector_tid.i, 207371
  %276 = getelementptr float addrspace(1)* %4, i64 %.sum9101vector_func.i
  %exData3168vector_func.i = extractelement <16 x float> %call.i17.i, i32 11
  store float %exData3168vector_func.i, float addrspace(1)* %276, align 4
  br label %postload7284vector_func.i

postload7284vector_func.i:                        ; preds = %preload7283vector_func.i, %postload7281vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7238vector_func.i, label %postload7239vector_func.i

preload7238vector_func.i:                         ; preds = %postload7284vector_func.i
  %.sum9100vector_func.i = add i64 %dim_0_vector_tid.i, 207372
  %277 = getelementptr float addrspace(1)* %4, i64 %.sum9100vector_func.i
  %exData3171vector_func.i = extractelement <16 x float> %call.i17.i, i32 12
  store float %exData3171vector_func.i, float addrspace(1)* %277, align 4
  br label %postload7239vector_func.i

postload7239vector_func.i:                        ; preds = %preload7238vector_func.i, %postload7284vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7241vector_func.i, label %postload7242vector_func.i

preload7241vector_func.i:                         ; preds = %postload7239vector_func.i
  %.sum9099vector_func.i = add i64 %dim_0_vector_tid.i, 207373
  %278 = getelementptr float addrspace(1)* %4, i64 %.sum9099vector_func.i
  %exData3174vector_func.i = extractelement <16 x float> %call.i17.i, i32 13
  store float %exData3174vector_func.i, float addrspace(1)* %278, align 4
  br label %postload7242vector_func.i

postload7242vector_func.i:                        ; preds = %preload7241vector_func.i, %postload7239vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6812vector_func.i, label %postload6813vector_func.i

preload6812vector_func.i:                         ; preds = %postload7242vector_func.i
  %.sum9098vector_func.i = add i64 %dim_0_vector_tid.i, 207374
  %279 = getelementptr float addrspace(1)* %4, i64 %.sum9098vector_func.i
  %exData3177vector_func.i = extractelement <16 x float> %call.i17.i, i32 14
  store float %exData3177vector_func.i, float addrspace(1)* %279, align 4
  br label %postload6813vector_func.i

postload6813vector_func.i:                        ; preds = %preload6812vector_func.i, %postload7242vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6815vector_func.i, label %postload6816vector_func.i

preload6815vector_func.i:                         ; preds = %postload6813vector_func.i
  %.sum9097vector_func.i = add i64 %dim_0_vector_tid.i, 207375
  %280 = getelementptr float addrspace(1)* %4, i64 %.sum9097vector_func.i
  %exData3180vector_func.i = extractelement <16 x float> %call.i17.i, i32 15
  store float %exData3180vector_func.i, float addrspace(1)* %280, align 4
  br label %postload6816vector_func.i

postload6816vector_func.i:                        ; preds = %preload6815vector_func.i, %postload6813vector_func.i
  %mul1611037vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000, float 0x40CB55EA80000000>
  %add1621038vector_func.i = fadd <16 x float> %mul1611037vector_func.i, <float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000, float 0x402B5009A0000000>
  %mul1631039vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000, float 0x3FFC2BC960000000>
  %add1641040vector_func.i = fadd <16 x float> %add1621038vector_func.i, %mul1631039vector_func.i
  %mul.i3161041vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000, float 0xBCF3E714C0000000>
  %add.i3171042vector_func.i = fadd <16 x float> %mul.i3161041vector_func.i, <float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000, float 0x3DD70DA9C0000000>
  %mul1.i3181043vector_func.i = fmul <16 x float> %add.i3171042vector_func.i, %mul567vector_func.i
  %add2.i3191044vector_func.i = fadd <16 x float> %mul1.i3181043vector_func.i, <float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000, float 0xBEA8BB9FC0000000>
  %mul3.i3201045vector_func.i = fmul <16 x float> %add2.i3191044vector_func.i, %mul567vector_func.i
  %add4.i3211046vector_func.i = fadd <16 x float> %mul3.i3201045vector_func.i, <float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000, float 0x3F72D77340000000>
  %mul5.i3221047vector_func.i = fmul <16 x float> %add4.i3211046vector_func.i, %mul567vector_func.i
  %add1661048vector_func.i = fadd <16 x float> %add1641040vector_func.i, %mul5.i3221047vector_func.i
  %call.i18.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add1661048vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6818vector_func.i, label %postload6819vector_func.i

preload6818vector_func.i:                         ; preds = %postload6816vector_func.i
  %extract1050vector_func.i = add i64 %dim_0_vector_tid.i, 221184
  %281 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1050vector_func.i
  %exData3184vector_func.i = extractelement <16 x float> %call.i18.i, i32 0
  store float %exData3184vector_func.i, float addrspace(1)* %281, align 4
  br label %postload6819vector_func.i

postload6819vector_func.i:                        ; preds = %preload6818vector_func.i, %postload6816vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5966vector_func.i, label %postload5967vector_func.i

preload5966vector_func.i:                         ; preds = %postload6819vector_func.i
  %.sum9096vector_func.i = add i64 %dim_0_vector_tid.i, 221185
  %282 = getelementptr float addrspace(1)* %4, i64 %.sum9096vector_func.i
  %exData3187vector_func.i = extractelement <16 x float> %call.i18.i, i32 1
  store float %exData3187vector_func.i, float addrspace(1)* %282, align 4
  br label %postload5967vector_func.i

postload5967vector_func.i:                        ; preds = %preload5966vector_func.i, %postload6819vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5969vector_func.i, label %postload5970vector_func.i

preload5969vector_func.i:                         ; preds = %postload5967vector_func.i
  %.sum9095vector_func.i = add i64 %dim_0_vector_tid.i, 221186
  %283 = getelementptr float addrspace(1)* %4, i64 %.sum9095vector_func.i
  %exData3190vector_func.i = extractelement <16 x float> %call.i18.i, i32 2
  store float %exData3190vector_func.i, float addrspace(1)* %283, align 4
  br label %postload5970vector_func.i

postload5970vector_func.i:                        ; preds = %preload5969vector_func.i, %postload5967vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5972vector_func.i, label %postload5973vector_func.i

preload5972vector_func.i:                         ; preds = %postload5970vector_func.i
  %.sum9094vector_func.i = add i64 %dim_0_vector_tid.i, 221187
  %284 = getelementptr float addrspace(1)* %4, i64 %.sum9094vector_func.i
  %exData3193vector_func.i = extractelement <16 x float> %call.i18.i, i32 3
  store float %exData3193vector_func.i, float addrspace(1)* %284, align 4
  br label %postload5973vector_func.i

postload5973vector_func.i:                        ; preds = %preload5972vector_func.i, %postload5970vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7220vector_func.i, label %postload7221vector_func.i

preload7220vector_func.i:                         ; preds = %postload5973vector_func.i
  %.sum9093vector_func.i = add i64 %dim_0_vector_tid.i, 221188
  %285 = getelementptr float addrspace(1)* %4, i64 %.sum9093vector_func.i
  %exData3196vector_func.i = extractelement <16 x float> %call.i18.i, i32 4
  store float %exData3196vector_func.i, float addrspace(1)* %285, align 4
  br label %postload7221vector_func.i

postload7221vector_func.i:                        ; preds = %preload7220vector_func.i, %postload5973vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7223vector_func.i, label %postload7224vector_func.i

preload7223vector_func.i:                         ; preds = %postload7221vector_func.i
  %.sum9092vector_func.i = add i64 %dim_0_vector_tid.i, 221189
  %286 = getelementptr float addrspace(1)* %4, i64 %.sum9092vector_func.i
  %exData3199vector_func.i = extractelement <16 x float> %call.i18.i, i32 5
  store float %exData3199vector_func.i, float addrspace(1)* %286, align 4
  br label %postload7224vector_func.i

postload7224vector_func.i:                        ; preds = %preload7223vector_func.i, %postload7221vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7226vector_func.i, label %postload7227vector_func.i

preload7226vector_func.i:                         ; preds = %postload7224vector_func.i
  %.sum9091vector_func.i = add i64 %dim_0_vector_tid.i, 221190
  %287 = getelementptr float addrspace(1)* %4, i64 %.sum9091vector_func.i
  %exData3202vector_func.i = extractelement <16 x float> %call.i18.i, i32 6
  store float %exData3202vector_func.i, float addrspace(1)* %287, align 4
  br label %postload7227vector_func.i

postload7227vector_func.i:                        ; preds = %preload7226vector_func.i, %postload7224vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7199vector_func.i, label %postload7200vector_func.i

preload7199vector_func.i:                         ; preds = %postload7227vector_func.i
  %.sum9090vector_func.i = add i64 %dim_0_vector_tid.i, 221191
  %288 = getelementptr float addrspace(1)* %4, i64 %.sum9090vector_func.i
  %exData3205vector_func.i = extractelement <16 x float> %call.i18.i, i32 7
  store float %exData3205vector_func.i, float addrspace(1)* %288, align 4
  br label %postload7200vector_func.i

postload7200vector_func.i:                        ; preds = %preload7199vector_func.i, %postload7227vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7202vector_func.i, label %postload7203vector_func.i

preload7202vector_func.i:                         ; preds = %postload7200vector_func.i
  %.sum9089vector_func.i = add i64 %dim_0_vector_tid.i, 221192
  %289 = getelementptr float addrspace(1)* %4, i64 %.sum9089vector_func.i
  %exData3208vector_func.i = extractelement <16 x float> %call.i18.i, i32 8
  store float %exData3208vector_func.i, float addrspace(1)* %289, align 4
  br label %postload7203vector_func.i

postload7203vector_func.i:                        ; preds = %preload7202vector_func.i, %postload7200vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7205vector_func.i, label %postload7206vector_func.i

preload7205vector_func.i:                         ; preds = %postload7203vector_func.i
  %.sum9088vector_func.i = add i64 %dim_0_vector_tid.i, 221193
  %290 = getelementptr float addrspace(1)* %4, i64 %.sum9088vector_func.i
  %exData3211vector_func.i = extractelement <16 x float> %call.i18.i, i32 9
  store float %exData3211vector_func.i, float addrspace(1)* %290, align 4
  br label %postload7206vector_func.i

postload7206vector_func.i:                        ; preds = %preload7205vector_func.i, %postload7203vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7166vector_func.i, label %postload7167vector_func.i

preload7166vector_func.i:                         ; preds = %postload7206vector_func.i
  %.sum9087vector_func.i = add i64 %dim_0_vector_tid.i, 221194
  %291 = getelementptr float addrspace(1)* %4, i64 %.sum9087vector_func.i
  %exData3214vector_func.i = extractelement <16 x float> %call.i18.i, i32 10
  store float %exData3214vector_func.i, float addrspace(1)* %291, align 4
  br label %postload7167vector_func.i

postload7167vector_func.i:                        ; preds = %preload7166vector_func.i, %postload7206vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7169vector_func.i, label %postload7170vector_func.i

preload7169vector_func.i:                         ; preds = %postload7167vector_func.i
  %.sum9086vector_func.i = add i64 %dim_0_vector_tid.i, 221195
  %292 = getelementptr float addrspace(1)* %4, i64 %.sum9086vector_func.i
  %exData3217vector_func.i = extractelement <16 x float> %call.i18.i, i32 11
  store float %exData3217vector_func.i, float addrspace(1)* %292, align 4
  br label %postload7170vector_func.i

postload7170vector_func.i:                        ; preds = %preload7169vector_func.i, %postload7167vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7148vector_func.i, label %postload7149vector_func.i

preload7148vector_func.i:                         ; preds = %postload7170vector_func.i
  %.sum9085vector_func.i = add i64 %dim_0_vector_tid.i, 221196
  %293 = getelementptr float addrspace(1)* %4, i64 %.sum9085vector_func.i
  %exData3220vector_func.i = extractelement <16 x float> %call.i18.i, i32 12
  store float %exData3220vector_func.i, float addrspace(1)* %293, align 4
  br label %postload7149vector_func.i

postload7149vector_func.i:                        ; preds = %preload7148vector_func.i, %postload7170vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7151vector_func.i, label %postload7152vector_func.i

preload7151vector_func.i:                         ; preds = %postload7149vector_func.i
  %.sum9084vector_func.i = add i64 %dim_0_vector_tid.i, 221197
  %294 = getelementptr float addrspace(1)* %4, i64 %.sum9084vector_func.i
  %exData3223vector_func.i = extractelement <16 x float> %call.i18.i, i32 13
  store float %exData3223vector_func.i, float addrspace(1)* %294, align 4
  br label %postload7152vector_func.i

postload7152vector_func.i:                        ; preds = %preload7151vector_func.i, %postload7149vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7154vector_func.i, label %postload7155vector_func.i

preload7154vector_func.i:                         ; preds = %postload7152vector_func.i
  %.sum9083vector_func.i = add i64 %dim_0_vector_tid.i, 221198
  %295 = getelementptr float addrspace(1)* %4, i64 %.sum9083vector_func.i
  %exData3226vector_func.i = extractelement <16 x float> %call.i18.i, i32 14
  store float %exData3226vector_func.i, float addrspace(1)* %295, align 4
  br label %postload7155vector_func.i

postload7155vector_func.i:                        ; preds = %preload7154vector_func.i, %postload7152vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7073vector_func.i, label %postload7074vector_func.i

preload7073vector_func.i:                         ; preds = %postload7155vector_func.i
  %.sum9082vector_func.i = add i64 %dim_0_vector_tid.i, 221199
  %296 = getelementptr float addrspace(1)* %4, i64 %.sum9082vector_func.i
  %exData3229vector_func.i = extractelement <16 x float> %call.i18.i, i32 15
  store float %exData3229vector_func.i, float addrspace(1)* %296, align 4
  br label %postload7074vector_func.i

postload7074vector_func.i:                        ; preds = %preload7073vector_func.i, %postload7155vector_func.i
  %mul1711067vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000, float 0x405FF54800000000>
  %sub1721068vector_func.i = fsub <16 x float> <float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000, float 0x40076FC500000000>, %mul1711067vector_func.i
  %mul1731069vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000, float 0x400E2A98A0000000>
  %add1741070vector_func.i = fadd <16 x float> %sub1721068vector_func.i, %mul1731069vector_func.i
  %mul.i3091071vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000, float 0xBCD3075C60000000>
  %add.i3101072vector_func.i = fadd <16 x float> %mul.i3091071vector_func.i, <float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000, float 0x3DC21213E0000000>
  %mul1.i3111073vector_func.i = fmul <16 x float> %add.i3101072vector_func.i, %mul567vector_func.i
  %add2.i3121074vector_func.i = fadd <16 x float> %mul1.i3111073vector_func.i, <float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000, float 0xBE9DB60E20000000>
  %mul3.i3131075vector_func.i = fmul <16 x float> %add2.i3121074vector_func.i, %mul567vector_func.i
  %add4.i3141076vector_func.i = fadd <16 x float> %mul3.i3131075vector_func.i, <float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000, float 0x3F701EEE80000000>
  %mul5.i3151077vector_func.i = fmul <16 x float> %add4.i3141076vector_func.i, %mul567vector_func.i
  %add1761078vector_func.i = fadd <16 x float> %add1741070vector_func.i, %mul5.i3151077vector_func.i
  %call.i19.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add1761078vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7076vector_func.i, label %postload7077vector_func.i

preload7076vector_func.i:                         ; preds = %postload7074vector_func.i
  %extract1080vector_func.i = add i64 %dim_0_vector_tid.i, 235008
  %297 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1080vector_func.i
  %exData3233vector_func.i = extractelement <16 x float> %call.i19.i, i32 0
  store float %exData3233vector_func.i, float addrspace(1)* %297, align 4
  br label %postload7077vector_func.i

postload7077vector_func.i:                        ; preds = %preload7076vector_func.i, %postload7074vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7079vector_func.i, label %postload7080vector_func.i

preload7079vector_func.i:                         ; preds = %postload7077vector_func.i
  %.sum9081vector_func.i = add i64 %dim_0_vector_tid.i, 235009
  %298 = getelementptr float addrspace(1)* %4, i64 %.sum9081vector_func.i
  %exData3236vector_func.i = extractelement <16 x float> %call.i19.i, i32 1
  store float %exData3236vector_func.i, float addrspace(1)* %298, align 4
  br label %postload7080vector_func.i

postload7080vector_func.i:                        ; preds = %preload7079vector_func.i, %postload7077vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7055vector_func.i, label %postload7056vector_func.i

preload7055vector_func.i:                         ; preds = %postload7080vector_func.i
  %.sum9080vector_func.i = add i64 %dim_0_vector_tid.i, 235010
  %299 = getelementptr float addrspace(1)* %4, i64 %.sum9080vector_func.i
  %exData3239vector_func.i = extractelement <16 x float> %call.i19.i, i32 2
  store float %exData3239vector_func.i, float addrspace(1)* %299, align 4
  br label %postload7056vector_func.i

postload7056vector_func.i:                        ; preds = %preload7055vector_func.i, %postload7080vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7058vector_func.i, label %postload7059vector_func.i

preload7058vector_func.i:                         ; preds = %postload7056vector_func.i
  %.sum9079vector_func.i = add i64 %dim_0_vector_tid.i, 235011
  %300 = getelementptr float addrspace(1)* %4, i64 %.sum9079vector_func.i
  %exData3242vector_func.i = extractelement <16 x float> %call.i19.i, i32 3
  store float %exData3242vector_func.i, float addrspace(1)* %300, align 4
  br label %postload7059vector_func.i

postload7059vector_func.i:                        ; preds = %preload7058vector_func.i, %postload7056vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7061vector_func.i, label %postload7062vector_func.i

preload7061vector_func.i:                         ; preds = %postload7059vector_func.i
  %.sum9078vector_func.i = add i64 %dim_0_vector_tid.i, 235012
  %301 = getelementptr float addrspace(1)* %4, i64 %.sum9078vector_func.i
  %exData3245vector_func.i = extractelement <16 x float> %call.i19.i, i32 4
  store float %exData3245vector_func.i, float addrspace(1)* %301, align 4
  br label %postload7062vector_func.i

postload7062vector_func.i:                        ; preds = %preload7061vector_func.i, %postload7059vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7037vector_func.i, label %postload7038vector_func.i

preload7037vector_func.i:                         ; preds = %postload7062vector_func.i
  %.sum9077vector_func.i = add i64 %dim_0_vector_tid.i, 235013
  %302 = getelementptr float addrspace(1)* %4, i64 %.sum9077vector_func.i
  %exData3248vector_func.i = extractelement <16 x float> %call.i19.i, i32 5
  store float %exData3248vector_func.i, float addrspace(1)* %302, align 4
  br label %postload7038vector_func.i

postload7038vector_func.i:                        ; preds = %preload7037vector_func.i, %postload7062vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7040vector_func.i, label %postload7041vector_func.i

preload7040vector_func.i:                         ; preds = %postload7038vector_func.i
  %.sum9076vector_func.i = add i64 %dim_0_vector_tid.i, 235014
  %303 = getelementptr float addrspace(1)* %4, i64 %.sum9076vector_func.i
  %exData3251vector_func.i = extractelement <16 x float> %call.i19.i, i32 6
  store float %exData3251vector_func.i, float addrspace(1)* %303, align 4
  br label %postload7041vector_func.i

postload7041vector_func.i:                        ; preds = %preload7040vector_func.i, %postload7038vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7043vector_func.i, label %postload7044vector_func.i

preload7043vector_func.i:                         ; preds = %postload7041vector_func.i
  %.sum9075vector_func.i = add i64 %dim_0_vector_tid.i, 235015
  %304 = getelementptr float addrspace(1)* %4, i64 %.sum9075vector_func.i
  %exData3254vector_func.i = extractelement <16 x float> %call.i19.i, i32 7
  store float %exData3254vector_func.i, float addrspace(1)* %304, align 4
  br label %postload7044vector_func.i

postload7044vector_func.i:                        ; preds = %preload7043vector_func.i, %postload7041vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7001vector_func.i, label %postload7002vector_func.i

preload7001vector_func.i:                         ; preds = %postload7044vector_func.i
  %.sum9074vector_func.i = add i64 %dim_0_vector_tid.i, 235016
  %305 = getelementptr float addrspace(1)* %4, i64 %.sum9074vector_func.i
  %exData3257vector_func.i = extractelement <16 x float> %call.i19.i, i32 8
  store float %exData3257vector_func.i, float addrspace(1)* %305, align 4
  br label %postload7002vector_func.i

postload7002vector_func.i:                        ; preds = %preload7001vector_func.i, %postload7044vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7004vector_func.i, label %postload7005vector_func.i

preload7004vector_func.i:                         ; preds = %postload7002vector_func.i
  %.sum9073vector_func.i = add i64 %dim_0_vector_tid.i, 235017
  %306 = getelementptr float addrspace(1)* %4, i64 %.sum9073vector_func.i
  %exData3260vector_func.i = extractelement <16 x float> %call.i19.i, i32 9
  store float %exData3260vector_func.i, float addrspace(1)* %306, align 4
  br label %postload7005vector_func.i

postload7005vector_func.i:                        ; preds = %preload7004vector_func.i, %postload7002vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6563vector_func.i, label %postload6564vector_func.i

preload6563vector_func.i:                         ; preds = %postload7005vector_func.i
  %.sum9072vector_func.i = add i64 %dim_0_vector_tid.i, 235018
  %307 = getelementptr float addrspace(1)* %4, i64 %.sum9072vector_func.i
  %exData3263vector_func.i = extractelement <16 x float> %call.i19.i, i32 10
  store float %exData3263vector_func.i, float addrspace(1)* %307, align 4
  br label %postload6564vector_func.i

postload6564vector_func.i:                        ; preds = %preload6563vector_func.i, %postload7005vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6566vector_func.i, label %postload6567vector_func.i

preload6566vector_func.i:                         ; preds = %postload6564vector_func.i
  %.sum9071vector_func.i = add i64 %dim_0_vector_tid.i, 235019
  %308 = getelementptr float addrspace(1)* %4, i64 %.sum9071vector_func.i
  %exData3266vector_func.i = extractelement <16 x float> %call.i19.i, i32 11
  store float %exData3266vector_func.i, float addrspace(1)* %308, align 4
  br label %postload6567vector_func.i

postload6567vector_func.i:                        ; preds = %preload6566vector_func.i, %postload6564vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6569vector_func.i, label %postload6570vector_func.i

preload6569vector_func.i:                         ; preds = %postload6567vector_func.i
  %.sum9070vector_func.i = add i64 %dim_0_vector_tid.i, 235020
  %309 = getelementptr float addrspace(1)* %4, i64 %.sum9070vector_func.i
  %exData3269vector_func.i = extractelement <16 x float> %call.i19.i, i32 12
  store float %exData3269vector_func.i, float addrspace(1)* %309, align 4
  br label %postload6570vector_func.i

postload6570vector_func.i:                        ; preds = %preload6569vector_func.i, %postload6567vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6545vector_func.i, label %postload6546vector_func.i

preload6545vector_func.i:                         ; preds = %postload6570vector_func.i
  %.sum9069vector_func.i = add i64 %dim_0_vector_tid.i, 235021
  %310 = getelementptr float addrspace(1)* %4, i64 %.sum9069vector_func.i
  %exData3272vector_func.i = extractelement <16 x float> %call.i19.i, i32 13
  store float %exData3272vector_func.i, float addrspace(1)* %310, align 4
  br label %postload6546vector_func.i

postload6546vector_func.i:                        ; preds = %preload6545vector_func.i, %postload6570vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6548vector_func.i, label %postload6549vector_func.i

preload6548vector_func.i:                         ; preds = %postload6546vector_func.i
  %.sum9068vector_func.i = add i64 %dim_0_vector_tid.i, 235022
  %311 = getelementptr float addrspace(1)* %4, i64 %.sum9068vector_func.i
  %exData3275vector_func.i = extractelement <16 x float> %call.i19.i, i32 14
  store float %exData3275vector_func.i, float addrspace(1)* %311, align 4
  br label %postload6549vector_func.i

postload6549vector_func.i:                        ; preds = %preload6548vector_func.i, %postload6546vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6551vector_func.i, label %postload6552vector_func.i

preload6551vector_func.i:                         ; preds = %postload6549vector_func.i
  %.sum9067vector_func.i = add i64 %dim_0_vector_tid.i, 235023
  %312 = getelementptr float addrspace(1)* %4, i64 %.sum9067vector_func.i
  %exData3278vector_func.i = extractelement <16 x float> %call.i19.i, i32 15
  store float %exData3278vector_func.i, float addrspace(1)* %312, align 4
  br label %postload6552vector_func.i

postload6552vector_func.i:                        ; preds = %preload6551vector_func.i, %postload6549vector_func.i
  %mul1811097vector_func.i = fmul <16 x float> %div568vector_func.i, <float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04, float 2.593600e+04>
  %sub1821098vector_func.i = fsub <16 x float> <float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000, float 0xBFF3AF3B60000000>, %mul1811097vector_func.i
  %mul1831099vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000, float 0x4010971C80000000>
  %add1841100vector_func.i = fadd <16 x float> %sub1821098vector_func.i, %mul1831099vector_func.i
  %mul.i3021101vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000, float 0xBCE044C220000000>
  %add.i3031102vector_func.i = fadd <16 x float> %mul.i3021101vector_func.i, <float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000, float 0x3DC569DE40000000>
  %mul1.i3041103vector_func.i = fmul <16 x float> %add.i3031102vector_func.i, %mul567vector_func.i
  %add2.i3051104vector_func.i = fadd <16 x float> %mul1.i3041103vector_func.i, <float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000, float 0xBE9A8A7DA0000000>
  %mul3.i3061105vector_func.i = fmul <16 x float> %add2.i3051104vector_func.i, %mul567vector_func.i
  %add4.i3071106vector_func.i = fadd <16 x float> %mul3.i3061105vector_func.i, <float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000, float 0x3F686B42C0000000>
  %mul5.i3081107vector_func.i = fmul <16 x float> %add4.i3071106vector_func.i, %mul567vector_func.i
  %add1861108vector_func.i = fadd <16 x float> %add1841100vector_func.i, %mul5.i3081107vector_func.i
  %call.i20.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add1861108vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6503vector_func.i, label %postload6504vector_func.i

preload6503vector_func.i:                         ; preds = %postload6552vector_func.i
  %extract1110vector_func.i = add i64 %dim_0_vector_tid.i, 248832
  %313 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1110vector_func.i
  %exData3282vector_func.i = extractelement <16 x float> %call.i20.i, i32 0
  store float %exData3282vector_func.i, float addrspace(1)* %313, align 4
  br label %postload6504vector_func.i

postload6504vector_func.i:                        ; preds = %preload6503vector_func.i, %postload6552vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6506vector_func.i, label %postload6507vector_func.i

preload6506vector_func.i:                         ; preds = %postload6504vector_func.i
  %.sum9066vector_func.i = add i64 %dim_0_vector_tid.i, 248833
  %314 = getelementptr float addrspace(1)* %4, i64 %.sum9066vector_func.i
  %exData3285vector_func.i = extractelement <16 x float> %call.i20.i, i32 1
  store float %exData3285vector_func.i, float addrspace(1)* %314, align 4
  br label %postload6507vector_func.i

postload6507vector_func.i:                        ; preds = %preload6506vector_func.i, %postload6504vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6509vector_func.i, label %postload6510vector_func.i

preload6509vector_func.i:                         ; preds = %postload6507vector_func.i
  %.sum9065vector_func.i = add i64 %dim_0_vector_tid.i, 248834
  %315 = getelementptr float addrspace(1)* %4, i64 %.sum9065vector_func.i
  %exData3288vector_func.i = extractelement <16 x float> %call.i20.i, i32 2
  store float %exData3288vector_func.i, float addrspace(1)* %315, align 4
  br label %postload6510vector_func.i

postload6510vector_func.i:                        ; preds = %preload6509vector_func.i, %postload6507vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6482vector_func.i, label %postload6483vector_func.i

preload6482vector_func.i:                         ; preds = %postload6510vector_func.i
  %.sum9064vector_func.i = add i64 %dim_0_vector_tid.i, 248835
  %316 = getelementptr float addrspace(1)* %4, i64 %.sum9064vector_func.i
  %exData3291vector_func.i = extractelement <16 x float> %call.i20.i, i32 3
  store float %exData3291vector_func.i, float addrspace(1)* %316, align 4
  br label %postload6483vector_func.i

postload6483vector_func.i:                        ; preds = %preload6482vector_func.i, %postload6510vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6485vector_func.i, label %postload6486vector_func.i

preload6485vector_func.i:                         ; preds = %postload6483vector_func.i
  %.sum9063vector_func.i = add i64 %dim_0_vector_tid.i, 248836
  %317 = getelementptr float addrspace(1)* %4, i64 %.sum9063vector_func.i
  %exData3294vector_func.i = extractelement <16 x float> %call.i20.i, i32 4
  store float %exData3294vector_func.i, float addrspace(1)* %317, align 4
  br label %postload6486vector_func.i

postload6486vector_func.i:                        ; preds = %preload6485vector_func.i, %postload6483vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6488vector_func.i, label %postload6489vector_func.i

preload6488vector_func.i:                         ; preds = %postload6486vector_func.i
  %.sum9062vector_func.i = add i64 %dim_0_vector_tid.i, 248837
  %318 = getelementptr float addrspace(1)* %4, i64 %.sum9062vector_func.i
  %exData3297vector_func.i = extractelement <16 x float> %call.i20.i, i32 5
  store float %exData3297vector_func.i, float addrspace(1)* %318, align 4
  br label %postload6489vector_func.i

postload6489vector_func.i:                        ; preds = %preload6488vector_func.i, %postload6486vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6452vector_func.i, label %postload6453vector_func.i

preload6452vector_func.i:                         ; preds = %postload6489vector_func.i
  %.sum9061vector_func.i = add i64 %dim_0_vector_tid.i, 248838
  %319 = getelementptr float addrspace(1)* %4, i64 %.sum9061vector_func.i
  %exData3300vector_func.i = extractelement <16 x float> %call.i20.i, i32 6
  store float %exData3300vector_func.i, float addrspace(1)* %319, align 4
  br label %postload6453vector_func.i

postload6453vector_func.i:                        ; preds = %preload6452vector_func.i, %postload6489vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6455vector_func.i, label %postload6456vector_func.i

preload6455vector_func.i:                         ; preds = %postload6453vector_func.i
  %.sum9060vector_func.i = add i64 %dim_0_vector_tid.i, 248839
  %320 = getelementptr float addrspace(1)* %4, i64 %.sum9060vector_func.i
  %exData3303vector_func.i = extractelement <16 x float> %call.i20.i, i32 7
  store float %exData3303vector_func.i, float addrspace(1)* %320, align 4
  br label %postload6456vector_func.i

postload6456vector_func.i:                        ; preds = %preload6455vector_func.i, %postload6453vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6410vector_func.i, label %postload6411vector_func.i

preload6410vector_func.i:                         ; preds = %postload6456vector_func.i
  %.sum9059vector_func.i = add i64 %dim_0_vector_tid.i, 248840
  %321 = getelementptr float addrspace(1)* %4, i64 %.sum9059vector_func.i
  %exData3306vector_func.i = extractelement <16 x float> %call.i20.i, i32 8
  store float %exData3306vector_func.i, float addrspace(1)* %321, align 4
  br label %postload6411vector_func.i

postload6411vector_func.i:                        ; preds = %preload6410vector_func.i, %postload6456vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6413vector_func.i, label %postload6414vector_func.i

preload6413vector_func.i:                         ; preds = %postload6411vector_func.i
  %.sum9058vector_func.i = add i64 %dim_0_vector_tid.i, 248841
  %322 = getelementptr float addrspace(1)* %4, i64 %.sum9058vector_func.i
  %exData3309vector_func.i = extractelement <16 x float> %call.i20.i, i32 9
  store float %exData3309vector_func.i, float addrspace(1)* %322, align 4
  br label %postload6414vector_func.i

postload6414vector_func.i:                        ; preds = %preload6413vector_func.i, %postload6411vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6416vector_func.i, label %postload6417vector_func.i

preload6416vector_func.i:                         ; preds = %postload6414vector_func.i
  %.sum9057vector_func.i = add i64 %dim_0_vector_tid.i, 248842
  %323 = getelementptr float addrspace(1)* %4, i64 %.sum9057vector_func.i
  %exData3312vector_func.i = extractelement <16 x float> %call.i20.i, i32 10
  store float %exData3312vector_func.i, float addrspace(1)* %323, align 4
  br label %postload6417vector_func.i

postload6417vector_func.i:                        ; preds = %preload6416vector_func.i, %postload6414vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6392vector_func.i, label %postload6393vector_func.i

preload6392vector_func.i:                         ; preds = %postload6417vector_func.i
  %.sum9056vector_func.i = add i64 %dim_0_vector_tid.i, 248843
  %324 = getelementptr float addrspace(1)* %4, i64 %.sum9056vector_func.i
  %exData3315vector_func.i = extractelement <16 x float> %call.i20.i, i32 11
  store float %exData3315vector_func.i, float addrspace(1)* %324, align 4
  br label %postload6393vector_func.i

postload6393vector_func.i:                        ; preds = %preload6392vector_func.i, %postload6417vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6395vector_func.i, label %postload6396vector_func.i

preload6395vector_func.i:                         ; preds = %postload6393vector_func.i
  %.sum9055vector_func.i = add i64 %dim_0_vector_tid.i, 248844
  %325 = getelementptr float addrspace(1)* %4, i64 %.sum9055vector_func.i
  %exData3318vector_func.i = extractelement <16 x float> %call.i20.i, i32 12
  store float %exData3318vector_func.i, float addrspace(1)* %325, align 4
  br label %postload6396vector_func.i

postload6396vector_func.i:                        ; preds = %preload6395vector_func.i, %postload6393vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6398vector_func.i, label %postload6399vector_func.i

preload6398vector_func.i:                         ; preds = %postload6396vector_func.i
  %.sum9054vector_func.i = add i64 %dim_0_vector_tid.i, 248845
  %326 = getelementptr float addrspace(1)* %4, i64 %.sum9054vector_func.i
  %exData3321vector_func.i = extractelement <16 x float> %call.i20.i, i32 13
  store float %exData3321vector_func.i, float addrspace(1)* %326, align 4
  br label %postload6399vector_func.i

postload6399vector_func.i:                        ; preds = %preload6398vector_func.i, %postload6396vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6317vector_func.i, label %postload6318vector_func.i

preload6317vector_func.i:                         ; preds = %postload6399vector_func.i
  %.sum9053vector_func.i = add i64 %dim_0_vector_tid.i, 248846
  %327 = getelementptr float addrspace(1)* %4, i64 %.sum9053vector_func.i
  %exData3324vector_func.i = extractelement <16 x float> %call.i20.i, i32 14
  store float %exData3324vector_func.i, float addrspace(1)* %327, align 4
  br label %postload6318vector_func.i

postload6318vector_func.i:                        ; preds = %preload6317vector_func.i, %postload6399vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6320vector_func.i, label %postload6321vector_func.i

preload6320vector_func.i:                         ; preds = %postload6318vector_func.i
  %.sum9052vector_func.i = add i64 %dim_0_vector_tid.i, 248847
  %328 = getelementptr float addrspace(1)* %4, i64 %.sum9052vector_func.i
  %exData3327vector_func.i = extractelement <16 x float> %call.i20.i, i32 15
  store float %exData3327vector_func.i, float addrspace(1)* %328, align 4
  br label %postload6321vector_func.i

postload6321vector_func.i:                        ; preds = %preload6320vector_func.i, %postload6318vector_func.i
  %mul1911127vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000, float 0x40E7979600000000>
  %sub1921128vector_func.i = fsub <16 x float> <float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000, float 0x3FE47CD260000000>, %mul1911127vector_func.i
  %mul1931129vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000, float 0x40111CB500000000>
  %add1941130vector_func.i = fadd <16 x float> %sub1921128vector_func.i, %mul1931129vector_func.i
  %mul.i2951131vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000, float 0xBCCAD12160000000>
  %add.i2961132vector_func.i = fadd <16 x float> %mul.i2951131vector_func.i, <float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000, float 0x3DB7549E80000000>
  %mul1.i2971133vector_func.i = fmul <16 x float> %add.i2961132vector_func.i, %mul567vector_func.i
  %add2.i2981134vector_func.i = fadd <16 x float> %mul1.i2971133vector_func.i, <float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000, float 0xBE923B7CA0000000>
  %mul3.i2991135vector_func.i = fmul <16 x float> %add2.i2981134vector_func.i, %mul567vector_func.i
  %add4.i3001136vector_func.i = fadd <16 x float> %mul3.i2991135vector_func.i, <float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000, float 0x3F637B5240000000>
  %mul5.i3011137vector_func.i = fmul <16 x float> %add4.i3001136vector_func.i, %mul567vector_func.i
  %add1961138vector_func.i = fadd <16 x float> %add1941130vector_func.i, %mul5.i3011137vector_func.i
  %call.i21.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add1961138vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6323vector_func.i, label %postload6324vector_func.i

preload6323vector_func.i:                         ; preds = %postload6321vector_func.i
  %extract1140vector_func.i = add i64 %dim_0_vector_tid.i, 262656
  %329 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1140vector_func.i
  %exData3331vector_func.i = extractelement <16 x float> %call.i21.i, i32 0
  store float %exData3331vector_func.i, float addrspace(1)* %329, align 4
  br label %postload6324vector_func.i

postload6324vector_func.i:                        ; preds = %preload6323vector_func.i, %postload6321vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6296vector_func.i, label %postload6297vector_func.i

preload6296vector_func.i:                         ; preds = %postload6324vector_func.i
  %.sum9051vector_func.i = add i64 %dim_0_vector_tid.i, 262657
  %330 = getelementptr float addrspace(1)* %4, i64 %.sum9051vector_func.i
  %exData3334vector_func.i = extractelement <16 x float> %call.i21.i, i32 1
  store float %exData3334vector_func.i, float addrspace(1)* %330, align 4
  br label %postload6297vector_func.i

postload6297vector_func.i:                        ; preds = %preload6296vector_func.i, %postload6324vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6299vector_func.i, label %postload6300vector_func.i

preload6299vector_func.i:                         ; preds = %postload6297vector_func.i
  %.sum9050vector_func.i = add i64 %dim_0_vector_tid.i, 262658
  %331 = getelementptr float addrspace(1)* %4, i64 %.sum9050vector_func.i
  %exData3337vector_func.i = extractelement <16 x float> %call.i21.i, i32 2
  store float %exData3337vector_func.i, float addrspace(1)* %331, align 4
  br label %postload6300vector_func.i

postload6300vector_func.i:                        ; preds = %preload6299vector_func.i, %postload6297vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6302vector_func.i, label %postload6303vector_func.i

preload6302vector_func.i:                         ; preds = %postload6300vector_func.i
  %.sum9049vector_func.i = add i64 %dim_0_vector_tid.i, 262659
  %332 = getelementptr float addrspace(1)* %4, i64 %.sum9049vector_func.i
  %exData3340vector_func.i = extractelement <16 x float> %call.i21.i, i32 3
  store float %exData3340vector_func.i, float addrspace(1)* %332, align 4
  br label %postload6303vector_func.i

postload6303vector_func.i:                        ; preds = %preload6302vector_func.i, %postload6300vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6206vector_func.i, label %postload6207vector_func.i

preload6206vector_func.i:                         ; preds = %postload6303vector_func.i
  %.sum9048vector_func.i = add i64 %dim_0_vector_tid.i, 262660
  %333 = getelementptr float addrspace(1)* %4, i64 %.sum9048vector_func.i
  %exData3343vector_func.i = extractelement <16 x float> %call.i21.i, i32 4
  store float %exData3343vector_func.i, float addrspace(1)* %333, align 4
  br label %postload6207vector_func.i

postload6207vector_func.i:                        ; preds = %preload6206vector_func.i, %postload6303vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6209vector_func.i, label %postload6210vector_func.i

preload6209vector_func.i:                         ; preds = %postload6207vector_func.i
  %.sum9047vector_func.i = add i64 %dim_0_vector_tid.i, 262661
  %334 = getelementptr float addrspace(1)* %4, i64 %.sum9047vector_func.i
  %exData3346vector_func.i = extractelement <16 x float> %call.i21.i, i32 5
  store float %exData3346vector_func.i, float addrspace(1)* %334, align 4
  br label %postload6210vector_func.i

postload6210vector_func.i:                        ; preds = %preload6209vector_func.i, %postload6207vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6182vector_func.i, label %postload6183vector_func.i

preload6182vector_func.i:                         ; preds = %postload6210vector_func.i
  %.sum9046vector_func.i = add i64 %dim_0_vector_tid.i, 262662
  %335 = getelementptr float addrspace(1)* %4, i64 %.sum9046vector_func.i
  %exData3349vector_func.i = extractelement <16 x float> %call.i21.i, i32 6
  store float %exData3349vector_func.i, float addrspace(1)* %335, align 4
  br label %postload6183vector_func.i

postload6183vector_func.i:                        ; preds = %preload6182vector_func.i, %postload6210vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6185vector_func.i, label %postload6186vector_func.i

preload6185vector_func.i:                         ; preds = %postload6183vector_func.i
  %.sum9045vector_func.i = add i64 %dim_0_vector_tid.i, 262663
  %336 = getelementptr float addrspace(1)* %4, i64 %.sum9045vector_func.i
  %exData3352vector_func.i = extractelement <16 x float> %call.i21.i, i32 7
  store float %exData3352vector_func.i, float addrspace(1)* %336, align 4
  br label %postload6186vector_func.i

postload6186vector_func.i:                        ; preds = %preload6185vector_func.i, %postload6183vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6188vector_func.i, label %postload6189vector_func.i

preload6188vector_func.i:                         ; preds = %postload6186vector_func.i
  %.sum9044vector_func.i = add i64 %dim_0_vector_tid.i, 262664
  %337 = getelementptr float addrspace(1)* %4, i64 %.sum9044vector_func.i
  %exData3355vector_func.i = extractelement <16 x float> %call.i21.i, i32 8
  store float %exData3355vector_func.i, float addrspace(1)* %337, align 4
  br label %postload6189vector_func.i

postload6189vector_func.i:                        ; preds = %preload6188vector_func.i, %postload6186vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6137vector_func.i, label %postload6138vector_func.i

preload6137vector_func.i:                         ; preds = %postload6189vector_func.i
  %.sum9043vector_func.i = add i64 %dim_0_vector_tid.i, 262665
  %338 = getelementptr float addrspace(1)* %4, i64 %.sum9043vector_func.i
  %exData3358vector_func.i = extractelement <16 x float> %call.i21.i, i32 9
  store float %exData3358vector_func.i, float addrspace(1)* %338, align 4
  br label %postload6138vector_func.i

postload6138vector_func.i:                        ; preds = %preload6137vector_func.i, %postload6189vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6140vector_func.i, label %postload6141vector_func.i

preload6140vector_func.i:                         ; preds = %postload6138vector_func.i
  %.sum9042vector_func.i = add i64 %dim_0_vector_tid.i, 262666
  %339 = getelementptr float addrspace(1)* %4, i64 %.sum9042vector_func.i
  %exData3361vector_func.i = extractelement <16 x float> %call.i21.i, i32 10
  store float %exData3361vector_func.i, float addrspace(1)* %339, align 4
  br label %postload6141vector_func.i

postload6141vector_func.i:                        ; preds = %preload6140vector_func.i, %postload6138vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6143vector_func.i, label %postload6144vector_func.i

preload6143vector_func.i:                         ; preds = %postload6141vector_func.i
  %.sum9041vector_func.i = add i64 %dim_0_vector_tid.i, 262667
  %340 = getelementptr float addrspace(1)* %4, i64 %.sum9041vector_func.i
  %exData3364vector_func.i = extractelement <16 x float> %call.i21.i, i32 11
  store float %exData3364vector_func.i, float addrspace(1)* %340, align 4
  br label %postload6144vector_func.i

postload6144vector_func.i:                        ; preds = %preload6143vector_func.i, %postload6141vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7430vector_func.i, label %postload7431vector_func.i

preload7430vector_func.i:                         ; preds = %postload6144vector_func.i
  %.sum9040vector_func.i = add i64 %dim_0_vector_tid.i, 262668
  %341 = getelementptr float addrspace(1)* %4, i64 %.sum9040vector_func.i
  %exData3367vector_func.i = extractelement <16 x float> %call.i21.i, i32 12
  store float %exData3367vector_func.i, float addrspace(1)* %341, align 4
  br label %postload7431vector_func.i

postload7431vector_func.i:                        ; preds = %preload7430vector_func.i, %postload6144vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7433vector_func.i, label %postload7434vector_func.i

preload7433vector_func.i:                         ; preds = %postload7431vector_func.i
  %.sum9039vector_func.i = add i64 %dim_0_vector_tid.i, 262669
  %342 = getelementptr float addrspace(1)* %4, i64 %.sum9039vector_func.i
  %exData3370vector_func.i = extractelement <16 x float> %call.i21.i, i32 13
  store float %exData3370vector_func.i, float addrspace(1)* %342, align 4
  br label %postload7434vector_func.i

postload7434vector_func.i:                        ; preds = %preload7433vector_func.i, %postload7431vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7436vector_func.i, label %postload7437vector_func.i

preload7436vector_func.i:                         ; preds = %postload7434vector_func.i
  %.sum9038vector_func.i = add i64 %dim_0_vector_tid.i, 262670
  %343 = getelementptr float addrspace(1)* %4, i64 %.sum9038vector_func.i
  %exData3373vector_func.i = extractelement <16 x float> %call.i21.i, i32 14
  store float %exData3373vector_func.i, float addrspace(1)* %343, align 4
  br label %postload7437vector_func.i

postload7437vector_func.i:                        ; preds = %preload7436vector_func.i, %postload7434vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6383vector_func.i, label %postload6384vector_func.i

preload6383vector_func.i:                         ; preds = %postload7437vector_func.i
  %.sum9037vector_func.i = add i64 %dim_0_vector_tid.i, 262671
  %344 = getelementptr float addrspace(1)* %4, i64 %.sum9037vector_func.i
  %exData3376vector_func.i = extractelement <16 x float> %call.i21.i, i32 15
  store float %exData3376vector_func.i, float addrspace(1)* %344, align 4
  br label %postload6384vector_func.i

postload6384vector_func.i:                        ; preds = %preload6383vector_func.i, %postload7437vector_func.i
  %mul2011157vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000, float 0x40E0E69C00000000>
  %sub2021158vector_func.i = fsub <16 x float> <float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000, float 0x401F263840000000>, %mul2011157vector_func.i
  %mul2031159vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000, float 0x4008224040000000>
  %add2041160vector_func.i = fadd <16 x float> %sub2021158vector_func.i, %mul2031159vector_func.i
  %mul.i2881161vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000, float 0xBCF36C9740000000>
  %add.i2891162vector_func.i = fadd <16 x float> %mul.i2881161vector_func.i, <float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000, float 0x3DD74F7660000000>
  %mul1.i2901163vector_func.i = fmul <16 x float> %add.i2891162vector_func.i, %mul567vector_func.i
  %add2.i2911164vector_func.i = fadd <16 x float> %mul1.i2901163vector_func.i, <float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000, float 0xBEAA2D5400000000>
  %mul3.i2921165vector_func.i = fmul <16 x float> %add2.i2911164vector_func.i, %mul567vector_func.i
  %add4.i2931166vector_func.i = fadd <16 x float> %mul3.i2921165vector_func.i, <float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000, float 0x3F752803E0000000>
  %mul5.i2941167vector_func.i = fmul <16 x float> %add4.i2931166vector_func.i, %mul567vector_func.i
  %add2061168vector_func.i = fadd <16 x float> %add2041160vector_func.i, %mul5.i2941167vector_func.i
  %call.i22.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2061168vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6386vector_func.i, label %postload6387vector_func.i

preload6386vector_func.i:                         ; preds = %postload6384vector_func.i
  %extract1170vector_func.i = add i64 %dim_0_vector_tid.i, 276480
  %345 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1170vector_func.i
  %exData3380vector_func.i = extractelement <16 x float> %call.i22.i, i32 0
  store float %exData3380vector_func.i, float addrspace(1)* %345, align 4
  br label %postload6387vector_func.i

postload6387vector_func.i:                        ; preds = %preload6386vector_func.i, %postload6384vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6389vector_func.i, label %postload6390vector_func.i

preload6389vector_func.i:                         ; preds = %postload6387vector_func.i
  %.sum9036vector_func.i = add i64 %dim_0_vector_tid.i, 276481
  %346 = getelementptr float addrspace(1)* %4, i64 %.sum9036vector_func.i
  %exData3383vector_func.i = extractelement <16 x float> %call.i22.i, i32 1
  store float %exData3383vector_func.i, float addrspace(1)* %346, align 4
  br label %postload6390vector_func.i

postload6390vector_func.i:                        ; preds = %preload6389vector_func.i, %postload6387vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6512vector_func.i, label %postload6513vector_func.i

preload6512vector_func.i:                         ; preds = %postload6390vector_func.i
  %.sum9035vector_func.i = add i64 %dim_0_vector_tid.i, 276482
  %347 = getelementptr float addrspace(1)* %4, i64 %.sum9035vector_func.i
  %exData3386vector_func.i = extractelement <16 x float> %call.i22.i, i32 2
  store float %exData3386vector_func.i, float addrspace(1)* %347, align 4
  br label %postload6513vector_func.i

postload6513vector_func.i:                        ; preds = %preload6512vector_func.i, %postload6390vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6515vector_func.i, label %postload6516vector_func.i

preload6515vector_func.i:                         ; preds = %postload6513vector_func.i
  %.sum9034vector_func.i = add i64 %dim_0_vector_tid.i, 276483
  %348 = getelementptr float addrspace(1)* %4, i64 %.sum9034vector_func.i
  %exData3389vector_func.i = extractelement <16 x float> %call.i22.i, i32 3
  store float %exData3389vector_func.i, float addrspace(1)* %348, align 4
  br label %postload6516vector_func.i

postload6516vector_func.i:                        ; preds = %preload6515vector_func.i, %postload6513vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6518vector_func.i, label %postload6519vector_func.i

preload6518vector_func.i:                         ; preds = %postload6516vector_func.i
  %.sum9033vector_func.i = add i64 %dim_0_vector_tid.i, 276484
  %349 = getelementptr float addrspace(1)* %4, i64 %.sum9033vector_func.i
  %exData3392vector_func.i = extractelement <16 x float> %call.i22.i, i32 4
  store float %exData3392vector_func.i, float addrspace(1)* %349, align 4
  br label %postload6519vector_func.i

postload6519vector_func.i:                        ; preds = %preload6518vector_func.i, %postload6516vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7007vector_func.i, label %postload7008vector_func.i

preload7007vector_func.i:                         ; preds = %postload6519vector_func.i
  %.sum9032vector_func.i = add i64 %dim_0_vector_tid.i, 276485
  %350 = getelementptr float addrspace(1)* %4, i64 %.sum9032vector_func.i
  %exData3395vector_func.i = extractelement <16 x float> %call.i22.i, i32 5
  store float %exData3395vector_func.i, float addrspace(1)* %350, align 4
  br label %postload7008vector_func.i

postload7008vector_func.i:                        ; preds = %preload7007vector_func.i, %postload6519vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7010vector_func.i, label %postload7011vector_func.i

preload7010vector_func.i:                         ; preds = %postload7008vector_func.i
  %.sum9031vector_func.i = add i64 %dim_0_vector_tid.i, 276486
  %351 = getelementptr float addrspace(1)* %4, i64 %.sum9031vector_func.i
  %exData3398vector_func.i = extractelement <16 x float> %call.i22.i, i32 6
  store float %exData3398vector_func.i, float addrspace(1)* %351, align 4
  br label %postload7011vector_func.i

postload7011vector_func.i:                        ; preds = %preload7010vector_func.i, %postload7008vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7013vector_func.i, label %postload7014vector_func.i

preload7013vector_func.i:                         ; preds = %postload7011vector_func.i
  %.sum9030vector_func.i = add i64 %dim_0_vector_tid.i, 276487
  %352 = getelementptr float addrspace(1)* %4, i64 %.sum9030vector_func.i
  %exData3401vector_func.i = extractelement <16 x float> %call.i22.i, i32 7
  store float %exData3401vector_func.i, float addrspace(1)* %352, align 4
  br label %postload7014vector_func.i

postload7014vector_func.i:                        ; preds = %preload7013vector_func.i, %postload7011vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7064vector_func.i, label %postload7065vector_func.i

preload7064vector_func.i:                         ; preds = %postload7014vector_func.i
  %.sum9029vector_func.i = add i64 %dim_0_vector_tid.i, 276488
  %353 = getelementptr float addrspace(1)* %4, i64 %.sum9029vector_func.i
  %exData3404vector_func.i = extractelement <16 x float> %call.i22.i, i32 8
  store float %exData3404vector_func.i, float addrspace(1)* %353, align 4
  br label %postload7065vector_func.i

postload7065vector_func.i:                        ; preds = %preload7064vector_func.i, %postload7014vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7067vector_func.i, label %postload7068vector_func.i

preload7067vector_func.i:                         ; preds = %postload7065vector_func.i
  %.sum9028vector_func.i = add i64 %dim_0_vector_tid.i, 276489
  %354 = getelementptr float addrspace(1)* %4, i64 %.sum9028vector_func.i
  %exData3407vector_func.i = extractelement <16 x float> %call.i22.i, i32 9
  store float %exData3407vector_func.i, float addrspace(1)* %354, align 4
  br label %postload7068vector_func.i

postload7068vector_func.i:                        ; preds = %preload7067vector_func.i, %postload7065vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7070vector_func.i, label %postload7071vector_func.i

preload7070vector_func.i:                         ; preds = %postload7068vector_func.i
  %.sum9027vector_func.i = add i64 %dim_0_vector_tid.i, 276490
  %355 = getelementptr float addrspace(1)* %4, i64 %.sum9027vector_func.i
  %exData3410vector_func.i = extractelement <16 x float> %call.i22.i, i32 10
  store float %exData3410vector_func.i, float addrspace(1)* %355, align 4
  br label %postload7071vector_func.i

postload7071vector_func.i:                        ; preds = %preload7070vector_func.i, %postload7068vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7229vector_func.i, label %postload7230vector_func.i

preload7229vector_func.i:                         ; preds = %postload7071vector_func.i
  %.sum9026vector_func.i = add i64 %dim_0_vector_tid.i, 276491
  %356 = getelementptr float addrspace(1)* %4, i64 %.sum9026vector_func.i
  %exData3413vector_func.i = extractelement <16 x float> %call.i22.i, i32 11
  store float %exData3413vector_func.i, float addrspace(1)* %356, align 4
  br label %postload7230vector_func.i

postload7230vector_func.i:                        ; preds = %preload7229vector_func.i, %postload7071vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7232vector_func.i, label %postload7233vector_func.i

preload7232vector_func.i:                         ; preds = %postload7230vector_func.i
  %.sum9025vector_func.i = add i64 %dim_0_vector_tid.i, 276492
  %357 = getelementptr float addrspace(1)* %4, i64 %.sum9025vector_func.i
  %exData3416vector_func.i = extractelement <16 x float> %call.i22.i, i32 12
  store float %exData3416vector_func.i, float addrspace(1)* %357, align 4
  br label %postload7233vector_func.i

postload7233vector_func.i:                        ; preds = %preload7232vector_func.i, %postload7230vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7235vector_func.i, label %postload7236vector_func.i

preload7235vector_func.i:                         ; preds = %postload7233vector_func.i
  %.sum9024vector_func.i = add i64 %dim_0_vector_tid.i, 276493
  %358 = getelementptr float addrspace(1)* %4, i64 %.sum9024vector_func.i
  %exData3419vector_func.i = extractelement <16 x float> %call.i22.i, i32 13
  store float %exData3419vector_func.i, float addrspace(1)* %358, align 4
  br label %postload7236vector_func.i

postload7236vector_func.i:                        ; preds = %preload7235vector_func.i, %postload7233vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7244vector_func.i, label %postload7245vector_func.i

preload7244vector_func.i:                         ; preds = %postload7236vector_func.i
  %.sum9023vector_func.i = add i64 %dim_0_vector_tid.i, 276494
  %359 = getelementptr float addrspace(1)* %4, i64 %.sum9023vector_func.i
  %exData3422vector_func.i = extractelement <16 x float> %call.i22.i, i32 14
  store float %exData3422vector_func.i, float addrspace(1)* %359, align 4
  br label %postload7245vector_func.i

postload7245vector_func.i:                        ; preds = %preload7244vector_func.i, %postload7236vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7247vector_func.i, label %postload7248vector_func.i

preload7247vector_func.i:                         ; preds = %postload7245vector_func.i
  %.sum9022vector_func.i = add i64 %dim_0_vector_tid.i, 276495
  %360 = getelementptr float addrspace(1)* %4, i64 %.sum9022vector_func.i
  %exData3425vector_func.i = extractelement <16 x float> %call.i22.i, i32 15
  store float %exData3425vector_func.i, float addrspace(1)* %360, align 4
  br label %postload7248vector_func.i

postload7248vector_func.i:                        ; preds = %preload7247vector_func.i, %postload7245vector_func.i
  %mul2111187vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000, float 0x40B34BE2E0000000>
  %sub2121188vector_func.i = fsub <16 x float> <float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000, float 0x40249C5960000000>, %mul2111187vector_func.i
  %mul2131189vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000, float 0x400049F4A0000000>
  %add2141190vector_func.i = fadd <16 x float> %sub2121188vector_func.i, %mul2131189vector_func.i
  %mul.i2811191vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000, float 0xBCFC4E7600000000>
  %add.i2821192vector_func.i = fadd <16 x float> %mul.i2811191vector_func.i, <float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000, float 0x3DE0DC9F20000000>
  %mul1.i2831193vector_func.i = fmul <16 x float> %add.i2821192vector_func.i, %mul567vector_func.i
  %add2.i2841194vector_func.i = fadd <16 x float> %mul1.i2831193vector_func.i, <float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000, float 0xBEB2C3C340000000>
  %mul3.i2851195vector_func.i = fmul <16 x float> %add2.i2841194vector_func.i, %mul567vector_func.i
  %add4.i2861196vector_func.i = fadd <16 x float> %mul3.i2851195vector_func.i, <float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000, float 0x3F7DFE6A60000000>
  %mul5.i2871197vector_func.i = fmul <16 x float> %add4.i2861196vector_func.i, %mul567vector_func.i
  %add2161198vector_func.i = fadd <16 x float> %add2141190vector_func.i, %mul5.i2871197vector_func.i
  %call.i23.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2161198vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7250vector_func.i, label %postload7251vector_func.i

preload7250vector_func.i:                         ; preds = %postload7248vector_func.i
  %extract1200vector_func.i = add i64 %dim_0_vector_tid.i, 290304
  %361 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1200vector_func.i
  %exData3429vector_func.i = extractelement <16 x float> %call.i23.i, i32 0
  store float %exData3429vector_func.i, float addrspace(1)* %361, align 4
  br label %postload7251vector_func.i

postload7251vector_func.i:                        ; preds = %preload7250vector_func.i, %postload7248vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7253vector_func.i, label %postload7254vector_func.i

preload7253vector_func.i:                         ; preds = %postload7251vector_func.i
  %.sum9021vector_func.i = add i64 %dim_0_vector_tid.i, 290305
  %362 = getelementptr float addrspace(1)* %4, i64 %.sum9021vector_func.i
  %exData3432vector_func.i = extractelement <16 x float> %call.i23.i, i32 1
  store float %exData3432vector_func.i, float addrspace(1)* %362, align 4
  br label %postload7254vector_func.i

postload7254vector_func.i:                        ; preds = %preload7253vector_func.i, %postload7251vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7286vector_func.i, label %postload7287vector_func.i

preload7286vector_func.i:                         ; preds = %postload7254vector_func.i
  %.sum9020vector_func.i = add i64 %dim_0_vector_tid.i, 290306
  %363 = getelementptr float addrspace(1)* %4, i64 %.sum9020vector_func.i
  %exData3435vector_func.i = extractelement <16 x float> %call.i23.i, i32 2
  store float %exData3435vector_func.i, float addrspace(1)* %363, align 4
  br label %postload7287vector_func.i

postload7287vector_func.i:                        ; preds = %preload7286vector_func.i, %postload7254vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7289vector_func.i, label %postload7290vector_func.i

preload7289vector_func.i:                         ; preds = %postload7287vector_func.i
  %.sum9019vector_func.i = add i64 %dim_0_vector_tid.i, 290307
  %364 = getelementptr float addrspace(1)* %4, i64 %.sum9019vector_func.i
  %exData3438vector_func.i = extractelement <16 x float> %call.i23.i, i32 3
  store float %exData3438vector_func.i, float addrspace(1)* %364, align 4
  br label %postload7290vector_func.i

postload7290vector_func.i:                        ; preds = %preload7289vector_func.i, %postload7287vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7292vector_func.i, label %postload7293vector_func.i

preload7292vector_func.i:                         ; preds = %postload7290vector_func.i
  %.sum9018vector_func.i = add i64 %dim_0_vector_tid.i, 290308
  %365 = getelementptr float addrspace(1)* %4, i64 %.sum9018vector_func.i
  %exData3441vector_func.i = extractelement <16 x float> %call.i23.i, i32 4
  store float %exData3441vector_func.i, float addrspace(1)* %365, align 4
  br label %postload7293vector_func.i

postload7293vector_func.i:                        ; preds = %preload7292vector_func.i, %postload7290vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7370vector_func.i, label %postload7371vector_func.i

preload7370vector_func.i:                         ; preds = %postload7293vector_func.i
  %.sum9017vector_func.i = add i64 %dim_0_vector_tid.i, 290309
  %366 = getelementptr float addrspace(1)* %4, i64 %.sum9017vector_func.i
  %exData3444vector_func.i = extractelement <16 x float> %call.i23.i, i32 5
  store float %exData3444vector_func.i, float addrspace(1)* %366, align 4
  br label %postload7371vector_func.i

postload7371vector_func.i:                        ; preds = %preload7370vector_func.i, %postload7293vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7373vector_func.i, label %postload7374vector_func.i

preload7373vector_func.i:                         ; preds = %postload7371vector_func.i
  %.sum9016vector_func.i = add i64 %dim_0_vector_tid.i, 290310
  %367 = getelementptr float addrspace(1)* %4, i64 %.sum9016vector_func.i
  %exData3447vector_func.i = extractelement <16 x float> %call.i23.i, i32 6
  store float %exData3447vector_func.i, float addrspace(1)* %367, align 4
  br label %postload7374vector_func.i

postload7374vector_func.i:                        ; preds = %preload7373vector_func.i, %postload7371vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7376vector_func.i, label %postload7377vector_func.i

preload7376vector_func.i:                         ; preds = %postload7374vector_func.i
  %.sum9015vector_func.i = add i64 %dim_0_vector_tid.i, 290311
  %368 = getelementptr float addrspace(1)* %4, i64 %.sum9015vector_func.i
  %exData3450vector_func.i = extractelement <16 x float> %call.i23.i, i32 7
  store float %exData3450vector_func.i, float addrspace(1)* %368, align 4
  br label %postload7377vector_func.i

postload7377vector_func.i:                        ; preds = %preload7376vector_func.i, %postload7374vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6419vector_func.i, label %postload6420vector_func.i

preload6419vector_func.i:                         ; preds = %postload7377vector_func.i
  %.sum9014vector_func.i = add i64 %dim_0_vector_tid.i, 290312
  %369 = getelementptr float addrspace(1)* %4, i64 %.sum9014vector_func.i
  %exData3453vector_func.i = extractelement <16 x float> %call.i23.i, i32 8
  store float %exData3453vector_func.i, float addrspace(1)* %369, align 4
  br label %postload6420vector_func.i

postload6420vector_func.i:                        ; preds = %preload6419vector_func.i, %postload7377vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6422vector_func.i, label %postload6423vector_func.i

preload6422vector_func.i:                         ; preds = %postload6420vector_func.i
  %.sum9013vector_func.i = add i64 %dim_0_vector_tid.i, 290313
  %370 = getelementptr float addrspace(1)* %4, i64 %.sum9013vector_func.i
  %exData3456vector_func.i = extractelement <16 x float> %call.i23.i, i32 9
  store float %exData3456vector_func.i, float addrspace(1)* %370, align 4
  br label %postload6423vector_func.i

postload6423vector_func.i:                        ; preds = %preload6422vector_func.i, %postload6420vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6425vector_func.i, label %postload6426vector_func.i

preload6425vector_func.i:                         ; preds = %postload6423vector_func.i
  %.sum9012vector_func.i = add i64 %dim_0_vector_tid.i, 290314
  %371 = getelementptr float addrspace(1)* %4, i64 %.sum9012vector_func.i
  %exData3459vector_func.i = extractelement <16 x float> %call.i23.i, i32 10
  store float %exData3459vector_func.i, float addrspace(1)* %371, align 4
  br label %postload6426vector_func.i

postload6426vector_func.i:                        ; preds = %preload6425vector_func.i, %postload6423vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6326vector_func.i, label %postload6327vector_func.i

preload6326vector_func.i:                         ; preds = %postload6426vector_func.i
  %.sum9011vector_func.i = add i64 %dim_0_vector_tid.i, 290315
  %372 = getelementptr float addrspace(1)* %4, i64 %.sum9011vector_func.i
  %exData3462vector_func.i = extractelement <16 x float> %call.i23.i, i32 11
  store float %exData3462vector_func.i, float addrspace(1)* %372, align 4
  br label %postload6327vector_func.i

postload6327vector_func.i:                        ; preds = %preload6326vector_func.i, %postload6426vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6329vector_func.i, label %postload6330vector_func.i

preload6329vector_func.i:                         ; preds = %postload6327vector_func.i
  %.sum9010vector_func.i = add i64 %dim_0_vector_tid.i, 290316
  %373 = getelementptr float addrspace(1)* %4, i64 %.sum9010vector_func.i
  %exData3465vector_func.i = extractelement <16 x float> %call.i23.i, i32 12
  store float %exData3465vector_func.i, float addrspace(1)* %373, align 4
  br label %postload6330vector_func.i

postload6330vector_func.i:                        ; preds = %preload6329vector_func.i, %postload6327vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6332vector_func.i, label %postload6333vector_func.i

preload6332vector_func.i:                         ; preds = %postload6330vector_func.i
  %.sum9009vector_func.i = add i64 %dim_0_vector_tid.i, 290317
  %374 = getelementptr float addrspace(1)* %4, i64 %.sum9009vector_func.i
  %exData3468vector_func.i = extractelement <16 x float> %call.i23.i, i32 13
  store float %exData3468vector_func.i, float addrspace(1)* %374, align 4
  br label %postload6333vector_func.i

postload6333vector_func.i:                        ; preds = %preload6332vector_func.i, %postload6330vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6458vector_func.i, label %postload6459vector_func.i

preload6458vector_func.i:                         ; preds = %postload6333vector_func.i
  %.sum9008vector_func.i = add i64 %dim_0_vector_tid.i, 290318
  %375 = getelementptr float addrspace(1)* %4, i64 %.sum9008vector_func.i
  %exData3471vector_func.i = extractelement <16 x float> %call.i23.i, i32 14
  store float %exData3471vector_func.i, float addrspace(1)* %375, align 4
  br label %postload6459vector_func.i

postload6459vector_func.i:                        ; preds = %preload6458vector_func.i, %postload6333vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6461vector_func.i, label %postload6462vector_func.i

preload6461vector_func.i:                         ; preds = %postload6459vector_func.i
  %.sum9007vector_func.i = add i64 %dim_0_vector_tid.i, 290319
  %376 = getelementptr float addrspace(1)* %4, i64 %.sum9007vector_func.i
  %exData3474vector_func.i = extractelement <16 x float> %call.i23.i, i32 15
  store float %exData3474vector_func.i, float addrspace(1)* %376, align 4
  br label %postload6462vector_func.i

postload6462vector_func.i:                        ; preds = %preload6461vector_func.i, %postload6459vector_func.i
  %mul2211217vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000, float 0x40C91CC280000000>
  %sub2221218vector_func.i = fsub <16 x float> <float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000, float 0x402AECC440000000>, %mul2211217vector_func.i
  %mul2231219vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000, float 0x3FFF4645C0000000>
  %add2241220vector_func.i = fadd <16 x float> %sub2221218vector_func.i, %mul2231219vector_func.i
  %mul.i2741221vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000, float 0xBD00D92000000000>
  %add.i2751222vector_func.i = fadd <16 x float> %mul.i2741221vector_func.i, <float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000, float 0x3DE4116FE0000000>
  %mul1.i2761223vector_func.i = fmul <16 x float> %add.i2751222vector_func.i, %mul567vector_func.i
  %add2.i2771224vector_func.i = fadd <16 x float> %mul1.i2761223vector_func.i, <float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000, float 0xBEB651C940000000>
  %mul3.i2781225vector_func.i = fmul <16 x float> %add2.i2771224vector_func.i, %mul567vector_func.i
  %add4.i2791226vector_func.i = fadd <16 x float> %mul3.i2781225vector_func.i, <float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000, float 0x3F81D09720000000>
  %mul5.i2801227vector_func.i = fmul <16 x float> %add4.i2791226vector_func.i, %mul567vector_func.i
  %add2261228vector_func.i = fadd <16 x float> %add2241220vector_func.i, %mul5.i2801227vector_func.i
  %call.i24.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2261228vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6464vector_func.i, label %postload6465vector_func.i

preload6464vector_func.i:                         ; preds = %postload6462vector_func.i
  %extract1230vector_func.i = add i64 %dim_0_vector_tid.i, 304128
  %377 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1230vector_func.i
  %exData3478vector_func.i = extractelement <16 x float> %call.i24.i, i32 0
  store float %exData3478vector_func.i, float addrspace(1)* %377, align 4
  br label %postload6465vector_func.i

postload6465vector_func.i:                        ; preds = %preload6464vector_func.i, %postload6462vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6554vector_func.i, label %postload6555vector_func.i

preload6554vector_func.i:                         ; preds = %postload6465vector_func.i
  %.sum9006vector_func.i = add i64 %dim_0_vector_tid.i, 304129
  %378 = getelementptr float addrspace(1)* %4, i64 %.sum9006vector_func.i
  %exData3481vector_func.i = extractelement <16 x float> %call.i24.i, i32 1
  store float %exData3481vector_func.i, float addrspace(1)* %378, align 4
  br label %postload6555vector_func.i

postload6555vector_func.i:                        ; preds = %preload6554vector_func.i, %postload6465vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6557vector_func.i, label %postload6558vector_func.i

preload6557vector_func.i:                         ; preds = %postload6555vector_func.i
  %.sum9005vector_func.i = add i64 %dim_0_vector_tid.i, 304130
  %379 = getelementptr float addrspace(1)* %4, i64 %.sum9005vector_func.i
  %exData3484vector_func.i = extractelement <16 x float> %call.i24.i, i32 2
  store float %exData3484vector_func.i, float addrspace(1)* %379, align 4
  br label %postload6558vector_func.i

postload6558vector_func.i:                        ; preds = %preload6557vector_func.i, %postload6555vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6560vector_func.i, label %postload6561vector_func.i

preload6560vector_func.i:                         ; preds = %postload6558vector_func.i
  %.sum9004vector_func.i = add i64 %dim_0_vector_tid.i, 304131
  %380 = getelementptr float addrspace(1)* %4, i64 %.sum9004vector_func.i
  %exData3487vector_func.i = extractelement <16 x float> %call.i24.i, i32 3
  store float %exData3487vector_func.i, float addrspace(1)* %380, align 4
  br label %postload6561vector_func.i

postload6561vector_func.i:                        ; preds = %preload6560vector_func.i, %postload6558vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6890vector_func.i, label %postload6891vector_func.i

preload6890vector_func.i:                         ; preds = %postload6561vector_func.i
  %.sum9003vector_func.i = add i64 %dim_0_vector_tid.i, 304132
  %381 = getelementptr float addrspace(1)* %4, i64 %.sum9003vector_func.i
  %exData3490vector_func.i = extractelement <16 x float> %call.i24.i, i32 4
  store float %exData3490vector_func.i, float addrspace(1)* %381, align 4
  br label %postload6891vector_func.i

postload6891vector_func.i:                        ; preds = %preload6890vector_func.i, %postload6561vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6893vector_func.i, label %postload6894vector_func.i

preload6893vector_func.i:                         ; preds = %postload6891vector_func.i
  %.sum9002vector_func.i = add i64 %dim_0_vector_tid.i, 304133
  %382 = getelementptr float addrspace(1)* %4, i64 %.sum9002vector_func.i
  %exData3493vector_func.i = extractelement <16 x float> %call.i24.i, i32 5
  store float %exData3493vector_func.i, float addrspace(1)* %382, align 4
  br label %postload6894vector_func.i

postload6894vector_func.i:                        ; preds = %preload6893vector_func.i, %postload6891vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6896vector_func.i, label %postload6897vector_func.i

preload6896vector_func.i:                         ; preds = %postload6894vector_func.i
  %.sum9001vector_func.i = add i64 %dim_0_vector_tid.i, 304134
  %383 = getelementptr float addrspace(1)* %4, i64 %.sum9001vector_func.i
  %exData3496vector_func.i = extractelement <16 x float> %call.i24.i, i32 6
  store float %exData3496vector_func.i, float addrspace(1)* %383, align 4
  br label %postload6897vector_func.i

postload6897vector_func.i:                        ; preds = %preload6896vector_func.i, %postload6894vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6899vector_func.i, label %postload6900vector_func.i

preload6899vector_func.i:                         ; preds = %postload6897vector_func.i
  %.sum9000vector_func.i = add i64 %dim_0_vector_tid.i, 304135
  %384 = getelementptr float addrspace(1)* %4, i64 %.sum9000vector_func.i
  %exData3499vector_func.i = extractelement <16 x float> %call.i24.i, i32 7
  store float %exData3499vector_func.i, float addrspace(1)* %384, align 4
  br label %postload6900vector_func.i

postload6900vector_func.i:                        ; preds = %preload6899vector_func.i, %postload6897vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6974vector_func.i, label %postload6975vector_func.i

preload6974vector_func.i:                         ; preds = %postload6900vector_func.i
  %.sum8999vector_func.i = add i64 %dim_0_vector_tid.i, 304136
  %385 = getelementptr float addrspace(1)* %4, i64 %.sum8999vector_func.i
  %exData3502vector_func.i = extractelement <16 x float> %call.i24.i, i32 8
  store float %exData3502vector_func.i, float addrspace(1)* %385, align 4
  br label %postload6975vector_func.i

postload6975vector_func.i:                        ; preds = %preload6974vector_func.i, %postload6900vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6977vector_func.i, label %postload6978vector_func.i

preload6977vector_func.i:                         ; preds = %postload6975vector_func.i
  %.sum8998vector_func.i = add i64 %dim_0_vector_tid.i, 304137
  %386 = getelementptr float addrspace(1)* %4, i64 %.sum8998vector_func.i
  %exData3505vector_func.i = extractelement <16 x float> %call.i24.i, i32 9
  store float %exData3505vector_func.i, float addrspace(1)* %386, align 4
  br label %postload6978vector_func.i

postload6978vector_func.i:                        ; preds = %preload6977vector_func.i, %postload6975vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6980vector_func.i, label %postload6981vector_func.i

preload6980vector_func.i:                         ; preds = %postload6978vector_func.i
  %.sum8997vector_func.i = add i64 %dim_0_vector_tid.i, 304138
  %387 = getelementptr float addrspace(1)* %4, i64 %.sum8997vector_func.i
  %exData3508vector_func.i = extractelement <16 x float> %call.i24.i, i32 10
  store float %exData3508vector_func.i, float addrspace(1)* %387, align 4
  br label %postload6981vector_func.i

postload6981vector_func.i:                        ; preds = %preload6980vector_func.i, %postload6978vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7046vector_func.i, label %postload7047vector_func.i

preload7046vector_func.i:                         ; preds = %postload6981vector_func.i
  %.sum8996vector_func.i = add i64 %dim_0_vector_tid.i, 304139
  %388 = getelementptr float addrspace(1)* %4, i64 %.sum8996vector_func.i
  %exData3511vector_func.i = extractelement <16 x float> %call.i24.i, i32 11
  store float %exData3511vector_func.i, float addrspace(1)* %388, align 4
  br label %postload7047vector_func.i

postload7047vector_func.i:                        ; preds = %preload7046vector_func.i, %postload6981vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7049vector_func.i, label %postload7050vector_func.i

preload7049vector_func.i:                         ; preds = %postload7047vector_func.i
  %.sum8995vector_func.i = add i64 %dim_0_vector_tid.i, 304140
  %389 = getelementptr float addrspace(1)* %4, i64 %.sum8995vector_func.i
  %exData3514vector_func.i = extractelement <16 x float> %call.i24.i, i32 12
  store float %exData3514vector_func.i, float addrspace(1)* %389, align 4
  br label %postload7050vector_func.i

postload7050vector_func.i:                        ; preds = %preload7049vector_func.i, %postload7047vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7052vector_func.i, label %postload7053vector_func.i

preload7052vector_func.i:                         ; preds = %postload7050vector_func.i
  %.sum8994vector_func.i = add i64 %dim_0_vector_tid.i, 304141
  %390 = getelementptr float addrspace(1)* %4, i64 %.sum8994vector_func.i
  %exData3517vector_func.i = extractelement <16 x float> %call.i24.i, i32 13
  store float %exData3517vector_func.i, float addrspace(1)* %390, align 4
  br label %postload7053vector_func.i

postload7053vector_func.i:                        ; preds = %preload7052vector_func.i, %postload7050vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7082vector_func.i, label %postload7083vector_func.i

preload7082vector_func.i:                         ; preds = %postload7053vector_func.i
  %.sum8993vector_func.i = add i64 %dim_0_vector_tid.i, 304142
  %391 = getelementptr float addrspace(1)* %4, i64 %.sum8993vector_func.i
  %exData3520vector_func.i = extractelement <16 x float> %call.i24.i, i32 14
  store float %exData3520vector_func.i, float addrspace(1)* %391, align 4
  br label %postload7083vector_func.i

postload7083vector_func.i:                        ; preds = %preload7082vector_func.i, %postload7053vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7085vector_func.i, label %postload7086vector_func.i

preload7085vector_func.i:                         ; preds = %postload7083vector_func.i
  %.sum8992vector_func.i = add i64 %dim_0_vector_tid.i, 304143
  %392 = getelementptr float addrspace(1)* %4, i64 %.sum8992vector_func.i
  %exData3523vector_func.i = extractelement <16 x float> %call.i24.i, i32 15
  store float %exData3523vector_func.i, float addrspace(1)* %392, align 4
  br label %postload7086vector_func.i

postload7086vector_func.i:                        ; preds = %preload7085vector_func.i, %postload7083vector_func.i
  %mul2311247vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000, float 0x40C6513260000000>
  %add2321248vector_func.i = fadd <16 x float> %mul2311247vector_func.i, <float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000, float 0x402E3B3160000000>
  %mul2331249vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000, float 0x3FF1266D40000000>
  %add2341250vector_func.i = fadd <16 x float> %add2321248vector_func.i, %mul2331249vector_func.i
  %mul.i2671251vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000, float 0xBD056475E0000000>
  %add.i2681252vector_func.i = fadd <16 x float> %mul.i2671251vector_func.i, <float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000, float 0x3DE95BDE60000000>
  %mul1.i2691253vector_func.i = fmul <16 x float> %add.i2681252vector_func.i, %mul567vector_func.i
  %add2.i2701254vector_func.i = fadd <16 x float> %mul1.i2691253vector_func.i, <float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000, float 0xBEBC089BE0000000>
  %mul3.i2711255vector_func.i = fmul <16 x float> %add2.i2701254vector_func.i, %mul567vector_func.i
  %add4.i2721256vector_func.i = fadd <16 x float> %mul3.i2711255vector_func.i, <float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000, float 0x3F8634A9C0000000>
  %mul5.i2731257vector_func.i = fmul <16 x float> %add4.i2721256vector_func.i, %mul567vector_func.i
  %add2361258vector_func.i = fadd <16 x float> %add2341250vector_func.i, %mul5.i2731257vector_func.i
  %call.i25.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2361258vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7088vector_func.i, label %postload7089vector_func.i

preload7088vector_func.i:                         ; preds = %postload7086vector_func.i
  %extract1260vector_func.i = add i64 %dim_0_vector_tid.i, 317952
  %393 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1260vector_func.i
  %exData3527vector_func.i = extractelement <16 x float> %call.i25.i, i32 0
  store float %exData3527vector_func.i, float addrspace(1)* %393, align 4
  br label %postload7089vector_func.i

postload7089vector_func.i:                        ; preds = %preload7088vector_func.i, %postload7086vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7091vector_func.i, label %postload7092vector_func.i

preload7091vector_func.i:                         ; preds = %postload7089vector_func.i
  %.sum8991vector_func.i = add i64 %dim_0_vector_tid.i, 317953
  %394 = getelementptr float addrspace(1)* %4, i64 %.sum8991vector_func.i
  %exData3530vector_func.i = extractelement <16 x float> %call.i25.i, i32 1
  store float %exData3530vector_func.i, float addrspace(1)* %394, align 4
  br label %postload7092vector_func.i

postload7092vector_func.i:                        ; preds = %preload7091vector_func.i, %postload7089vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7172vector_func.i, label %postload7173vector_func.i

preload7172vector_func.i:                         ; preds = %postload7092vector_func.i
  %.sum8990vector_func.i = add i64 %dim_0_vector_tid.i, 317954
  %395 = getelementptr float addrspace(1)* %4, i64 %.sum8990vector_func.i
  %exData3533vector_func.i = extractelement <16 x float> %call.i25.i, i32 2
  store float %exData3533vector_func.i, float addrspace(1)* %395, align 4
  br label %postload7173vector_func.i

postload7173vector_func.i:                        ; preds = %preload7172vector_func.i, %postload7092vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7175vector_func.i, label %postload7176vector_func.i

preload7175vector_func.i:                         ; preds = %postload7173vector_func.i
  %.sum8989vector_func.i = add i64 %dim_0_vector_tid.i, 317955
  %396 = getelementptr float addrspace(1)* %4, i64 %.sum8989vector_func.i
  %exData3536vector_func.i = extractelement <16 x float> %call.i25.i, i32 3
  store float %exData3536vector_func.i, float addrspace(1)* %396, align 4
  br label %postload7176vector_func.i

postload7176vector_func.i:                        ; preds = %preload7175vector_func.i, %postload7173vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7178vector_func.i, label %postload7179vector_func.i

preload7178vector_func.i:                         ; preds = %postload7176vector_func.i
  %.sum8988vector_func.i = add i64 %dim_0_vector_tid.i, 317956
  %397 = getelementptr float addrspace(1)* %4, i64 %.sum8988vector_func.i
  %exData3539vector_func.i = extractelement <16 x float> %call.i25.i, i32 4
  store float %exData3539vector_func.i, float addrspace(1)* %397, align 4
  br label %postload7179vector_func.i

postload7179vector_func.i:                        ; preds = %preload7178vector_func.i, %postload7176vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7352vector_func.i, label %postload7353vector_func.i

preload7352vector_func.i:                         ; preds = %postload7179vector_func.i
  %.sum8987vector_func.i = add i64 %dim_0_vector_tid.i, 317957
  %398 = getelementptr float addrspace(1)* %4, i64 %.sum8987vector_func.i
  %exData3542vector_func.i = extractelement <16 x float> %call.i25.i, i32 5
  store float %exData3542vector_func.i, float addrspace(1)* %398, align 4
  br label %postload7353vector_func.i

postload7353vector_func.i:                        ; preds = %preload7352vector_func.i, %postload7179vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7355vector_func.i, label %postload7356vector_func.i

preload7355vector_func.i:                         ; preds = %postload7353vector_func.i
  %.sum8986vector_func.i = add i64 %dim_0_vector_tid.i, 317958
  %399 = getelementptr float addrspace(1)* %4, i64 %.sum8986vector_func.i
  %exData3545vector_func.i = extractelement <16 x float> %call.i25.i, i32 6
  store float %exData3545vector_func.i, float addrspace(1)* %399, align 4
  br label %postload7356vector_func.i

postload7356vector_func.i:                        ; preds = %preload7355vector_func.i, %postload7353vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7358vector_func.i, label %postload7359vector_func.i

preload7358vector_func.i:                         ; preds = %postload7356vector_func.i
  %.sum8985vector_func.i = add i64 %dim_0_vector_tid.i, 317959
  %400 = getelementptr float addrspace(1)* %4, i64 %.sum8985vector_func.i
  %exData3548vector_func.i = extractelement <16 x float> %call.i25.i, i32 7
  store float %exData3548vector_func.i, float addrspace(1)* %400, align 4
  br label %postload7359vector_func.i

postload7359vector_func.i:                        ; preds = %preload7358vector_func.i, %postload7356vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7439vector_func.i, label %postload7440vector_func.i

preload7439vector_func.i:                         ; preds = %postload7359vector_func.i
  %.sum8984vector_func.i = add i64 %dim_0_vector_tid.i, 317960
  %401 = getelementptr float addrspace(1)* %4, i64 %.sum8984vector_func.i
  %exData3551vector_func.i = extractelement <16 x float> %call.i25.i, i32 8
  store float %exData3551vector_func.i, float addrspace(1)* %401, align 4
  br label %postload7440vector_func.i

postload7440vector_func.i:                        ; preds = %preload7439vector_func.i, %postload7359vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7442vector_func.i, label %postload7443vector_func.i

preload7442vector_func.i:                         ; preds = %postload7440vector_func.i
  %.sum8983vector_func.i = add i64 %dim_0_vector_tid.i, 317961
  %402 = getelementptr float addrspace(1)* %4, i64 %.sum8983vector_func.i
  %exData3554vector_func.i = extractelement <16 x float> %call.i25.i, i32 9
  store float %exData3554vector_func.i, float addrspace(1)* %402, align 4
  br label %postload7443vector_func.i

postload7443vector_func.i:                        ; preds = %preload7442vector_func.i, %postload7440vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7445vector_func.i, label %postload7446vector_func.i

preload7445vector_func.i:                         ; preds = %postload7443vector_func.i
  %.sum8982vector_func.i = add i64 %dim_0_vector_tid.i, 317962
  %403 = getelementptr float addrspace(1)* %4, i64 %.sum8982vector_func.i
  %exData3557vector_func.i = extractelement <16 x float> %call.i25.i, i32 10
  store float %exData3557vector_func.i, float addrspace(1)* %403, align 4
  br label %postload7446vector_func.i

postload7446vector_func.i:                        ; preds = %preload7445vector_func.i, %postload7443vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6212vector_func.i, label %postload6213vector_func.i

preload6212vector_func.i:                         ; preds = %postload7446vector_func.i
  %.sum8981vector_func.i = add i64 %dim_0_vector_tid.i, 317963
  %404 = getelementptr float addrspace(1)* %4, i64 %.sum8981vector_func.i
  %exData3560vector_func.i = extractelement <16 x float> %call.i25.i, i32 11
  store float %exData3560vector_func.i, float addrspace(1)* %404, align 4
  br label %postload6213vector_func.i

postload6213vector_func.i:                        ; preds = %preload6212vector_func.i, %postload7446vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6215vector_func.i, label %postload6216vector_func.i

preload6215vector_func.i:                         ; preds = %postload6213vector_func.i
  %.sum8980vector_func.i = add i64 %dim_0_vector_tid.i, 317964
  %405 = getelementptr float addrspace(1)* %4, i64 %.sum8980vector_func.i
  %exData3563vector_func.i = extractelement <16 x float> %call.i25.i, i32 12
  store float %exData3563vector_func.i, float addrspace(1)* %405, align 4
  br label %postload6216vector_func.i

postload6216vector_func.i:                        ; preds = %preload6215vector_func.i, %postload6213vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6218vector_func.i, label %postload6219vector_func.i

preload6218vector_func.i:                         ; preds = %postload6216vector_func.i
  %.sum8979vector_func.i = add i64 %dim_0_vector_tid.i, 317965
  %406 = getelementptr float addrspace(1)* %4, i64 %.sum8979vector_func.i
  %exData3566vector_func.i = extractelement <16 x float> %call.i25.i, i32 13
  store float %exData3566vector_func.i, float addrspace(1)* %406, align 4
  br label %postload6219vector_func.i

postload6219vector_func.i:                        ; preds = %preload6218vector_func.i, %postload6216vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6221vector_func.i, label %postload6222vector_func.i

preload6221vector_func.i:                         ; preds = %postload6219vector_func.i
  %.sum8978vector_func.i = add i64 %dim_0_vector_tid.i, 317966
  %407 = getelementptr float addrspace(1)* %4, i64 %.sum8978vector_func.i
  %exData3569vector_func.i = extractelement <16 x float> %call.i25.i, i32 14
  store float %exData3569vector_func.i, float addrspace(1)* %407, align 4
  br label %postload6222vector_func.i

postload6222vector_func.i:                        ; preds = %preload6221vector_func.i, %postload6219vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6359vector_func.i, label %postload6360vector_func.i

preload6359vector_func.i:                         ; preds = %postload6222vector_func.i
  %.sum8977vector_func.i = add i64 %dim_0_vector_tid.i, 317967
  %408 = getelementptr float addrspace(1)* %4, i64 %.sum8977vector_func.i
  %exData3572vector_func.i = extractelement <16 x float> %call.i25.i, i32 15
  store float %exData3572vector_func.i, float addrspace(1)* %408, align 4
  br label %postload6360vector_func.i

postload6360vector_func.i:                        ; preds = %preload6359vector_func.i, %postload6222vector_func.i
  %mul2411277vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000, float 0x40D2DFCDC0000000>
  %sub2421278vector_func.i = fsub <16 x float> <float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000, float 0xC00F712BE0000000>, %mul2411277vector_func.i
  %mul2431279vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000, float 0x4016834860000000>
  %add2441280vector_func.i = fadd <16 x float> %sub2421278vector_func.i, %mul2431279vector_func.i
  %mul.i2601281vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000, float 0xBCD17B2440000000>
  %add.i2611282vector_func.i = fadd <16 x float> %mul.i2601281vector_func.i, <float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000, float 0x3DBA3A9900000000>
  %mul1.i2621283vector_func.i = fmul <16 x float> %add.i2611282vector_func.i, %mul567vector_func.i
  %add2.i2631284vector_func.i = fadd <16 x float> %mul1.i2621283vector_func.i, <float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000, float 0xBE91D28EA0000000>
  %mul3.i2641285vector_func.i = fmul <16 x float> %add2.i2631284vector_func.i, %mul567vector_func.i
  %add4.i2651286vector_func.i = fadd <16 x float> %mul3.i2641285vector_func.i, <float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000, float 0x3F60BBCA20000000>
  %mul5.i2661287vector_func.i = fmul <16 x float> %add4.i2651286vector_func.i, %mul567vector_func.i
  %add2461288vector_func.i = fadd <16 x float> %add2441280vector_func.i, %mul5.i2661287vector_func.i
  %call.i26.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2461288vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6362vector_func.i, label %postload6363vector_func.i

preload6362vector_func.i:                         ; preds = %postload6360vector_func.i
  %extract1290vector_func.i = add i64 %dim_0_vector_tid.i, 331776
  %409 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1290vector_func.i
  %exData3576vector_func.i = extractelement <16 x float> %call.i26.i, i32 0
  store float %exData3576vector_func.i, float addrspace(1)* %409, align 4
  br label %postload6363vector_func.i

postload6363vector_func.i:                        ; preds = %preload6362vector_func.i, %postload6360vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6365vector_func.i, label %postload6366vector_func.i

preload6365vector_func.i:                         ; preds = %postload6363vector_func.i
  %.sum8976vector_func.i = add i64 %dim_0_vector_tid.i, 331777
  %410 = getelementptr float addrspace(1)* %4, i64 %.sum8976vector_func.i
  %exData3579vector_func.i = extractelement <16 x float> %call.i26.i, i32 1
  store float %exData3579vector_func.i, float addrspace(1)* %410, align 4
  br label %postload6366vector_func.i

postload6366vector_func.i:                        ; preds = %preload6365vector_func.i, %postload6363vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6401vector_func.i, label %postload6402vector_func.i

preload6401vector_func.i:                         ; preds = %postload6366vector_func.i
  %.sum8975vector_func.i = add i64 %dim_0_vector_tid.i, 331778
  %411 = getelementptr float addrspace(1)* %4, i64 %.sum8975vector_func.i
  %exData3582vector_func.i = extractelement <16 x float> %call.i26.i, i32 2
  store float %exData3582vector_func.i, float addrspace(1)* %411, align 4
  br label %postload6402vector_func.i

postload6402vector_func.i:                        ; preds = %preload6401vector_func.i, %postload6366vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6404vector_func.i, label %postload6405vector_func.i

preload6404vector_func.i:                         ; preds = %postload6402vector_func.i
  %.sum8974vector_func.i = add i64 %dim_0_vector_tid.i, 331779
  %412 = getelementptr float addrspace(1)* %4, i64 %.sum8974vector_func.i
  %exData3585vector_func.i = extractelement <16 x float> %call.i26.i, i32 3
  store float %exData3585vector_func.i, float addrspace(1)* %412, align 4
  br label %postload6405vector_func.i

postload6405vector_func.i:                        ; preds = %preload6404vector_func.i, %postload6402vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6407vector_func.i, label %postload6408vector_func.i

preload6407vector_func.i:                         ; preds = %postload6405vector_func.i
  %.sum8973vector_func.i = add i64 %dim_0_vector_tid.i, 331780
  %413 = getelementptr float addrspace(1)* %4, i64 %.sum8973vector_func.i
  %exData3588vector_func.i = extractelement <16 x float> %call.i26.i, i32 4
  store float %exData3588vector_func.i, float addrspace(1)* %413, align 4
  br label %postload6408vector_func.i

postload6408vector_func.i:                        ; preds = %preload6407vector_func.i, %postload6405vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6491vector_func.i, label %postload6492vector_func.i

preload6491vector_func.i:                         ; preds = %postload6408vector_func.i
  %.sum8972vector_func.i = add i64 %dim_0_vector_tid.i, 331781
  %414 = getelementptr float addrspace(1)* %4, i64 %.sum8972vector_func.i
  %exData3591vector_func.i = extractelement <16 x float> %call.i26.i, i32 5
  store float %exData3591vector_func.i, float addrspace(1)* %414, align 4
  br label %postload6492vector_func.i

postload6492vector_func.i:                        ; preds = %preload6491vector_func.i, %postload6408vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6494vector_func.i, label %postload6495vector_func.i

preload6494vector_func.i:                         ; preds = %postload6492vector_func.i
  %.sum8971vector_func.i = add i64 %dim_0_vector_tid.i, 331782
  %415 = getelementptr float addrspace(1)* %4, i64 %.sum8971vector_func.i
  %exData3594vector_func.i = extractelement <16 x float> %call.i26.i, i32 6
  store float %exData3594vector_func.i, float addrspace(1)* %415, align 4
  br label %postload6495vector_func.i

postload6495vector_func.i:                        ; preds = %preload6494vector_func.i, %postload6492vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6497vector_func.i, label %postload6498vector_func.i

preload6497vector_func.i:                         ; preds = %postload6495vector_func.i
  %.sum8970vector_func.i = add i64 %dim_0_vector_tid.i, 331783
  %416 = getelementptr float addrspace(1)* %4, i64 %.sum8970vector_func.i
  %exData3597vector_func.i = extractelement <16 x float> %call.i26.i, i32 7
  store float %exData3597vector_func.i, float addrspace(1)* %416, align 4
  br label %postload6498vector_func.i

postload6498vector_func.i:                        ; preds = %preload6497vector_func.i, %postload6495vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6500vector_func.i, label %postload6501vector_func.i

preload6500vector_func.i:                         ; preds = %postload6498vector_func.i
  %.sum8969vector_func.i = add i64 %dim_0_vector_tid.i, 331784
  %417 = getelementptr float addrspace(1)* %4, i64 %.sum8969vector_func.i
  %exData3600vector_func.i = extractelement <16 x float> %call.i26.i, i32 8
  store float %exData3600vector_func.i, float addrspace(1)* %417, align 4
  br label %postload6501vector_func.i

postload6501vector_func.i:                        ; preds = %preload6500vector_func.i, %postload6498vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6989vector_func.i, label %postload6990vector_func.i

preload6989vector_func.i:                         ; preds = %postload6501vector_func.i
  %.sum8968vector_func.i = add i64 %dim_0_vector_tid.i, 331785
  %418 = getelementptr float addrspace(1)* %4, i64 %.sum8968vector_func.i
  %exData3603vector_func.i = extractelement <16 x float> %call.i26.i, i32 9
  store float %exData3603vector_func.i, float addrspace(1)* %418, align 4
  br label %postload6990vector_func.i

postload6990vector_func.i:                        ; preds = %preload6989vector_func.i, %postload6501vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6992vector_func.i, label %postload6993vector_func.i

preload6992vector_func.i:                         ; preds = %postload6990vector_func.i
  %.sum8967vector_func.i = add i64 %dim_0_vector_tid.i, 331786
  %419 = getelementptr float addrspace(1)* %4, i64 %.sum8967vector_func.i
  %exData3606vector_func.i = extractelement <16 x float> %call.i26.i, i32 10
  store float %exData3606vector_func.i, float addrspace(1)* %419, align 4
  br label %postload6993vector_func.i

postload6993vector_func.i:                        ; preds = %preload6992vector_func.i, %postload6990vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6995vector_func.i, label %postload6996vector_func.i

preload6995vector_func.i:                         ; preds = %postload6993vector_func.i
  %.sum8966vector_func.i = add i64 %dim_0_vector_tid.i, 331787
  %420 = getelementptr float addrspace(1)* %4, i64 %.sum8966vector_func.i
  %exData3609vector_func.i = extractelement <16 x float> %call.i26.i, i32 11
  store float %exData3609vector_func.i, float addrspace(1)* %420, align 4
  br label %postload6996vector_func.i

postload6996vector_func.i:                        ; preds = %preload6995vector_func.i, %postload6993vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7157vector_func.i, label %postload7158vector_func.i

preload7157vector_func.i:                         ; preds = %postload6996vector_func.i
  %.sum8965vector_func.i = add i64 %dim_0_vector_tid.i, 331788
  %421 = getelementptr float addrspace(1)* %4, i64 %.sum8965vector_func.i
  %exData3612vector_func.i = extractelement <16 x float> %call.i26.i, i32 12
  store float %exData3612vector_func.i, float addrspace(1)* %421, align 4
  br label %postload7158vector_func.i

postload7158vector_func.i:                        ; preds = %preload7157vector_func.i, %postload6996vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7160vector_func.i, label %postload7161vector_func.i

preload7160vector_func.i:                         ; preds = %postload7158vector_func.i
  %.sum8964vector_func.i = add i64 %dim_0_vector_tid.i, 331789
  %422 = getelementptr float addrspace(1)* %4, i64 %.sum8964vector_func.i
  %exData3615vector_func.i = extractelement <16 x float> %call.i26.i, i32 13
  store float %exData3615vector_func.i, float addrspace(1)* %422, align 4
  br label %postload7161vector_func.i

postload7161vector_func.i:                        ; preds = %preload7160vector_func.i, %postload7158vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7163vector_func.i, label %postload7164vector_func.i

preload7163vector_func.i:                         ; preds = %postload7161vector_func.i
  %.sum8963vector_func.i = add i64 %dim_0_vector_tid.i, 331790
  %423 = getelementptr float addrspace(1)* %4, i64 %.sum8963vector_func.i
  %exData3618vector_func.i = extractelement <16 x float> %call.i26.i, i32 14
  store float %exData3618vector_func.i, float addrspace(1)* %423, align 4
  br label %postload7164vector_func.i

postload7164vector_func.i:                        ; preds = %preload7163vector_func.i, %postload7161vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7208vector_func.i, label %postload7209vector_func.i

preload7208vector_func.i:                         ; preds = %postload7164vector_func.i
  %.sum8962vector_func.i = add i64 %dim_0_vector_tid.i, 331791
  %424 = getelementptr float addrspace(1)* %4, i64 %.sum8962vector_func.i
  %exData3621vector_func.i = extractelement <16 x float> %call.i26.i, i32 15
  store float %exData3621vector_func.i, float addrspace(1)* %424, align 4
  br label %postload7209vector_func.i

postload7209vector_func.i:                        ; preds = %preload7208vector_func.i, %postload7164vector_func.i
  %mul2511307vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000, float 0x40BD7F0DA0000000>
  %add2521308vector_func.i = fadd <16 x float> %mul2511307vector_func.i, <float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000, float 0x3FE43B5E80000000>
  %mul2531309vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000, float 0x40120B9180000000>
  %add2541310vector_func.i = fadd <16 x float> %add2521308vector_func.i, %mul2531309vector_func.i
  %mul.i2531311vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000, float 0xBCF1E5EE20000000>
  %add.i2541312vector_func.i = fadd <16 x float> %mul.i2531311vector_func.i, <float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000, float 0x3DD5268EC0000000>
  %mul1.i2551313vector_func.i = fmul <16 x float> %add.i2541312vector_func.i, %mul567vector_func.i
  %add2.i2561314vector_func.i = fadd <16 x float> %mul1.i2551313vector_func.i, <float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000, float 0xBEA75123E0000000>
  %mul3.i2571315vector_func.i = fmul <16 x float> %add2.i2561314vector_func.i, %mul567vector_func.i
  %add4.i2581316vector_func.i = fadd <16 x float> %mul3.i2571315vector_func.i, <float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000, float 0x3F72707A60000000>
  %mul5.i2591317vector_func.i = fmul <16 x float> %add4.i2581316vector_func.i, %mul567vector_func.i
  %add2561318vector_func.i = fadd <16 x float> %add2541310vector_func.i, %mul5.i2591317vector_func.i
  %call.i27.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2561318vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7211vector_func.i, label %postload7212vector_func.i

preload7211vector_func.i:                         ; preds = %postload7209vector_func.i
  %extract1320vector_func.i = add i64 %dim_0_vector_tid.i, 345600
  %425 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1320vector_func.i
  %exData3625vector_func.i = extractelement <16 x float> %call.i27.i, i32 0
  store float %exData3625vector_func.i, float addrspace(1)* %425, align 4
  br label %postload7212vector_func.i

postload7212vector_func.i:                        ; preds = %preload7211vector_func.i, %postload7209vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7214vector_func.i, label %postload7215vector_func.i

preload7214vector_func.i:                         ; preds = %postload7212vector_func.i
  %.sum8961vector_func.i = add i64 %dim_0_vector_tid.i, 345601
  %426 = getelementptr float addrspace(1)* %4, i64 %.sum8961vector_func.i
  %exData3628vector_func.i = extractelement <16 x float> %call.i27.i, i32 1
  store float %exData3628vector_func.i, float addrspace(1)* %426, align 4
  br label %postload7215vector_func.i

postload7215vector_func.i:                        ; preds = %preload7214vector_func.i, %postload7212vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7217vector_func.i, label %postload7218vector_func.i

preload7217vector_func.i:                         ; preds = %postload7215vector_func.i
  %.sum8960vector_func.i = add i64 %dim_0_vector_tid.i, 345602
  %427 = getelementptr float addrspace(1)* %4, i64 %.sum8960vector_func.i
  %exData3631vector_func.i = extractelement <16 x float> %call.i27.i, i32 2
  store float %exData3631vector_func.i, float addrspace(1)* %427, align 4
  br label %postload7218vector_func.i

postload7218vector_func.i:                        ; preds = %preload7217vector_func.i, %postload7215vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7268vector_func.i, label %postload7269vector_func.i

preload7268vector_func.i:                         ; preds = %postload7218vector_func.i
  %.sum8959vector_func.i = add i64 %dim_0_vector_tid.i, 345603
  %428 = getelementptr float addrspace(1)* %4, i64 %.sum8959vector_func.i
  %exData3634vector_func.i = extractelement <16 x float> %call.i27.i, i32 3
  store float %exData3634vector_func.i, float addrspace(1)* %428, align 4
  br label %postload7269vector_func.i

postload7269vector_func.i:                        ; preds = %preload7268vector_func.i, %postload7218vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7271vector_func.i, label %postload7272vector_func.i

preload7271vector_func.i:                         ; preds = %postload7269vector_func.i
  %.sum8958vector_func.i = add i64 %dim_0_vector_tid.i, 345604
  %429 = getelementptr float addrspace(1)* %4, i64 %.sum8958vector_func.i
  %exData3637vector_func.i = extractelement <16 x float> %call.i27.i, i32 4
  store float %exData3637vector_func.i, float addrspace(1)* %429, align 4
  br label %postload7272vector_func.i

postload7272vector_func.i:                        ; preds = %preload7271vector_func.i, %postload7269vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7274vector_func.i, label %postload7275vector_func.i

preload7274vector_func.i:                         ; preds = %postload7272vector_func.i
  %.sum8957vector_func.i = add i64 %dim_0_vector_tid.i, 345605
  %430 = getelementptr float addrspace(1)* %4, i64 %.sum8957vector_func.i
  %exData3640vector_func.i = extractelement <16 x float> %call.i27.i, i32 5
  store float %exData3640vector_func.i, float addrspace(1)* %430, align 4
  br label %postload7275vector_func.i

postload7275vector_func.i:                        ; preds = %preload7274vector_func.i, %postload7272vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7334vector_func.i, label %postload7335vector_func.i

preload7334vector_func.i:                         ; preds = %postload7275vector_func.i
  %.sum8956vector_func.i = add i64 %dim_0_vector_tid.i, 345606
  %431 = getelementptr float addrspace(1)* %4, i64 %.sum8956vector_func.i
  %exData3643vector_func.i = extractelement <16 x float> %call.i27.i, i32 6
  store float %exData3643vector_func.i, float addrspace(1)* %431, align 4
  br label %postload7335vector_func.i

postload7335vector_func.i:                        ; preds = %preload7334vector_func.i, %postload7275vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7337vector_func.i, label %postload7338vector_func.i

preload7337vector_func.i:                         ; preds = %postload7335vector_func.i
  %.sum8955vector_func.i = add i64 %dim_0_vector_tid.i, 345607
  %432 = getelementptr float addrspace(1)* %4, i64 %.sum8955vector_func.i
  %exData3646vector_func.i = extractelement <16 x float> %call.i27.i, i32 7
  store float %exData3646vector_func.i, float addrspace(1)* %432, align 4
  br label %postload7338vector_func.i

postload7338vector_func.i:                        ; preds = %preload7337vector_func.i, %postload7335vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7340vector_func.i, label %postload7341vector_func.i

preload7340vector_func.i:                         ; preds = %postload7338vector_func.i
  %.sum8954vector_func.i = add i64 %dim_0_vector_tid.i, 345608
  %433 = getelementptr float addrspace(1)* %4, i64 %.sum8954vector_func.i
  %exData3649vector_func.i = extractelement <16 x float> %call.i27.i, i32 8
  store float %exData3649vector_func.i, float addrspace(1)* %433, align 4
  br label %postload7341vector_func.i

postload7341vector_func.i:                        ; preds = %preload7340vector_func.i, %postload7338vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7421vector_func.i, label %postload7422vector_func.i

preload7421vector_func.i:                         ; preds = %postload7341vector_func.i
  %.sum8953vector_func.i = add i64 %dim_0_vector_tid.i, 345609
  %434 = getelementptr float addrspace(1)* %4, i64 %.sum8953vector_func.i
  %exData3652vector_func.i = extractelement <16 x float> %call.i27.i, i32 9
  store float %exData3652vector_func.i, float addrspace(1)* %434, align 4
  br label %postload7422vector_func.i

postload7422vector_func.i:                        ; preds = %preload7421vector_func.i, %postload7341vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7424vector_func.i, label %postload7425vector_func.i

preload7424vector_func.i:                         ; preds = %postload7422vector_func.i
  %.sum8952vector_func.i = add i64 %dim_0_vector_tid.i, 345610
  %435 = getelementptr float addrspace(1)* %4, i64 %.sum8952vector_func.i
  %exData3655vector_func.i = extractelement <16 x float> %call.i27.i, i32 10
  store float %exData3655vector_func.i, float addrspace(1)* %435, align 4
  br label %postload7425vector_func.i

postload7425vector_func.i:                        ; preds = %preload7424vector_func.i, %postload7422vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7427vector_func.i, label %postload7428vector_func.i

preload7427vector_func.i:                         ; preds = %postload7425vector_func.i
  %.sum8951vector_func.i = add i64 %dim_0_vector_tid.i, 345611
  %436 = getelementptr float addrspace(1)* %4, i64 %.sum8951vector_func.i
  %exData3658vector_func.i = extractelement <16 x float> %call.i27.i, i32 11
  store float %exData3658vector_func.i, float addrspace(1)* %436, align 4
  br label %postload7428vector_func.i

postload7428vector_func.i:                        ; preds = %preload7427vector_func.i, %postload7425vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6305vector_func.i, label %postload6306vector_func.i

preload6305vector_func.i:                         ; preds = %postload7428vector_func.i
  %.sum8950vector_func.i = add i64 %dim_0_vector_tid.i, 345612
  %437 = getelementptr float addrspace(1)* %4, i64 %.sum8950vector_func.i
  %exData3661vector_func.i = extractelement <16 x float> %call.i27.i, i32 12
  store float %exData3661vector_func.i, float addrspace(1)* %437, align 4
  br label %postload6306vector_func.i

postload6306vector_func.i:                        ; preds = %preload6305vector_func.i, %postload7428vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6308vector_func.i, label %postload6309vector_func.i

preload6308vector_func.i:                         ; preds = %postload6306vector_func.i
  %.sum8949vector_func.i = add i64 %dim_0_vector_tid.i, 345613
  %438 = getelementptr float addrspace(1)* %4, i64 %.sum8949vector_func.i
  %exData3664vector_func.i = extractelement <16 x float> %call.i27.i, i32 13
  store float %exData3664vector_func.i, float addrspace(1)* %438, align 4
  br label %postload6309vector_func.i

postload6309vector_func.i:                        ; preds = %preload6308vector_func.i, %postload6306vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6311vector_func.i, label %postload6312vector_func.i

preload6311vector_func.i:                         ; preds = %postload6309vector_func.i
  %.sum8948vector_func.i = add i64 %dim_0_vector_tid.i, 345614
  %439 = getelementptr float addrspace(1)* %4, i64 %.sum8948vector_func.i
  %exData3667vector_func.i = extractelement <16 x float> %call.i27.i, i32 14
  store float %exData3667vector_func.i, float addrspace(1)* %439, align 4
  br label %postload6312vector_func.i

postload6312vector_func.i:                        ; preds = %preload6311vector_func.i, %postload6309vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6314vector_func.i, label %postload6315vector_func.i

preload6314vector_func.i:                         ; preds = %postload6312vector_func.i
  %.sum8947vector_func.i = add i64 %dim_0_vector_tid.i, 345615
  %440 = getelementptr float addrspace(1)* %4, i64 %.sum8947vector_func.i
  %exData3670vector_func.i = extractelement <16 x float> %call.i27.i, i32 15
  store float %exData3670vector_func.i, float addrspace(1)* %440, align 4
  br label %postload6315vector_func.i

postload6315vector_func.i:                        ; preds = %preload6314vector_func.i, %postload6312vector_func.i
  %mul2611337vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000, float 0x407EA52600000000>
  %sub2621338vector_func.i = fsub <16 x float> <float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000, float 0xC01420DBA0000000>, %mul2611337vector_func.i
  %mul2631339vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000, float 0x4017E71600000000>
  %add2641340vector_func.i = fadd <16 x float> %sub2621338vector_func.i, %mul2631339vector_func.i
  %mul.i2461341vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000, float 0xBCD3998DC0000000>
  %add.i2471342vector_func.i = fadd <16 x float> %mul.i2461341vector_func.i, <float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000, float 0x3DC2A5B400000000>
  %mul1.i2481343vector_func.i = fmul <16 x float> %add.i2471342vector_func.i, %mul567vector_func.i
  %add2.i2491344vector_func.i = fadd <16 x float> %mul1.i2481343vector_func.i, <float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000, float 0xBE9EAFDA00000000>
  %mul3.i2501345vector_func.i = fmul <16 x float> %add2.i2491344vector_func.i, %mul567vector_func.i
  %add4.i2511346vector_func.i = fadd <16 x float> %mul3.i2501345vector_func.i, <float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000, float 0x3F70A6C580000000>
  %mul5.i2521347vector_func.i = fmul <16 x float> %add4.i2511346vector_func.i, %mul567vector_func.i
  %add2661348vector_func.i = fadd <16 x float> %add2641340vector_func.i, %mul5.i2521347vector_func.i
  %call.i28.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2661348vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6872vector_func.i, label %postload6873vector_func.i

preload6872vector_func.i:                         ; preds = %postload6315vector_func.i
  %extract1350vector_func.i = add i64 %dim_0_vector_tid.i, 359424
  %441 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1350vector_func.i
  %exData3674vector_func.i = extractelement <16 x float> %call.i28.i, i32 0
  store float %exData3674vector_func.i, float addrspace(1)* %441, align 4
  br label %postload6873vector_func.i

postload6873vector_func.i:                        ; preds = %preload6872vector_func.i, %postload6315vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6875vector_func.i, label %postload6876vector_func.i

preload6875vector_func.i:                         ; preds = %postload6873vector_func.i
  %.sum8946vector_func.i = add i64 %dim_0_vector_tid.i, 359425
  %442 = getelementptr float addrspace(1)* %4, i64 %.sum8946vector_func.i
  %exData3677vector_func.i = extractelement <16 x float> %call.i28.i, i32 1
  store float %exData3677vector_func.i, float addrspace(1)* %442, align 4
  br label %postload6876vector_func.i

postload6876vector_func.i:                        ; preds = %preload6875vector_func.i, %postload6873vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6878vector_func.i, label %postload6879vector_func.i

preload6878vector_func.i:                         ; preds = %postload6876vector_func.i
  %.sum8945vector_func.i = add i64 %dim_0_vector_tid.i, 359426
  %443 = getelementptr float addrspace(1)* %4, i64 %.sum8945vector_func.i
  %exData3680vector_func.i = extractelement <16 x float> %call.i28.i, i32 2
  store float %exData3680vector_func.i, float addrspace(1)* %443, align 4
  br label %postload6879vector_func.i

postload6879vector_func.i:                        ; preds = %preload6878vector_func.i, %postload6876vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6908vector_func.i, label %postload6909vector_func.i

preload6908vector_func.i:                         ; preds = %postload6879vector_func.i
  %.sum8944vector_func.i = add i64 %dim_0_vector_tid.i, 359427
  %444 = getelementptr float addrspace(1)* %4, i64 %.sum8944vector_func.i
  %exData3683vector_func.i = extractelement <16 x float> %call.i28.i, i32 3
  store float %exData3683vector_func.i, float addrspace(1)* %444, align 4
  br label %postload6909vector_func.i

postload6909vector_func.i:                        ; preds = %preload6908vector_func.i, %postload6879vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6911vector_func.i, label %postload6912vector_func.i

preload6911vector_func.i:                         ; preds = %postload6909vector_func.i
  %.sum8943vector_func.i = add i64 %dim_0_vector_tid.i, 359428
  %445 = getelementptr float addrspace(1)* %4, i64 %.sum8943vector_func.i
  %exData3686vector_func.i = extractelement <16 x float> %call.i28.i, i32 4
  store float %exData3686vector_func.i, float addrspace(1)* %445, align 4
  br label %postload6912vector_func.i

postload6912vector_func.i:                        ; preds = %preload6911vector_func.i, %postload6909vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6914vector_func.i, label %postload6915vector_func.i

preload6914vector_func.i:                         ; preds = %postload6912vector_func.i
  %.sum8942vector_func.i = add i64 %dim_0_vector_tid.i, 359429
  %446 = getelementptr float addrspace(1)* %4, i64 %.sum8942vector_func.i
  %exData3689vector_func.i = extractelement <16 x float> %call.i28.i, i32 5
  store float %exData3689vector_func.i, float addrspace(1)* %446, align 4
  br label %postload6915vector_func.i

postload6915vector_func.i:                        ; preds = %preload6914vector_func.i, %postload6912vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6944vector_func.i, label %postload6945vector_func.i

preload6944vector_func.i:                         ; preds = %postload6915vector_func.i
  %.sum8941vector_func.i = add i64 %dim_0_vector_tid.i, 359430
  %447 = getelementptr float addrspace(1)* %4, i64 %.sum8941vector_func.i
  %exData3692vector_func.i = extractelement <16 x float> %call.i28.i, i32 6
  store float %exData3692vector_func.i, float addrspace(1)* %447, align 4
  br label %postload6945vector_func.i

postload6945vector_func.i:                        ; preds = %preload6944vector_func.i, %postload6915vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6947vector_func.i, label %postload6948vector_func.i

preload6947vector_func.i:                         ; preds = %postload6945vector_func.i
  %.sum8940vector_func.i = add i64 %dim_0_vector_tid.i, 359431
  %448 = getelementptr float addrspace(1)* %4, i64 %.sum8940vector_func.i
  %exData3695vector_func.i = extractelement <16 x float> %call.i28.i, i32 7
  store float %exData3695vector_func.i, float addrspace(1)* %448, align 4
  br label %postload6948vector_func.i

postload6948vector_func.i:                        ; preds = %preload6947vector_func.i, %postload6945vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6950vector_func.i, label %postload6951vector_func.i

preload6950vector_func.i:                         ; preds = %postload6948vector_func.i
  %.sum8939vector_func.i = add i64 %dim_0_vector_tid.i, 359432
  %449 = getelementptr float addrspace(1)* %4, i64 %.sum8939vector_func.i
  %exData3698vector_func.i = extractelement <16 x float> %call.i28.i, i32 8
  store float %exData3698vector_func.i, float addrspace(1)* %449, align 4
  br label %postload6951vector_func.i

postload6951vector_func.i:                        ; preds = %preload6950vector_func.i, %postload6948vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6953vector_func.i, label %postload6954vector_func.i

preload6953vector_func.i:                         ; preds = %postload6951vector_func.i
  %.sum8938vector_func.i = add i64 %dim_0_vector_tid.i, 359433
  %450 = getelementptr float addrspace(1)* %4, i64 %.sum8938vector_func.i
  %exData3701vector_func.i = extractelement <16 x float> %call.i28.i, i32 9
  store float %exData3701vector_func.i, float addrspace(1)* %450, align 4
  br label %postload6954vector_func.i

postload6954vector_func.i:                        ; preds = %preload6953vector_func.i, %postload6951vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7385vector_func.i, label %postload7386vector_func.i

preload7385vector_func.i:                         ; preds = %postload6954vector_func.i
  %.sum8937vector_func.i = add i64 %dim_0_vector_tid.i, 359434
  %451 = getelementptr float addrspace(1)* %4, i64 %.sum8937vector_func.i
  %exData3704vector_func.i = extractelement <16 x float> %call.i28.i, i32 10
  store float %exData3704vector_func.i, float addrspace(1)* %451, align 4
  br label %postload7386vector_func.i

postload7386vector_func.i:                        ; preds = %preload7385vector_func.i, %postload6954vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7388vector_func.i, label %postload7389vector_func.i

preload7388vector_func.i:                         ; preds = %postload7386vector_func.i
  %.sum8936vector_func.i = add i64 %dim_0_vector_tid.i, 359435
  %452 = getelementptr float addrspace(1)* %4, i64 %.sum8936vector_func.i
  %exData3707vector_func.i = extractelement <16 x float> %call.i28.i, i32 11
  store float %exData3707vector_func.i, float addrspace(1)* %452, align 4
  br label %postload7389vector_func.i

postload7389vector_func.i:                        ; preds = %preload7388vector_func.i, %postload7386vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7391vector_func.i, label %postload7392vector_func.i

preload7391vector_func.i:                         ; preds = %postload7389vector_func.i
  %.sum8935vector_func.i = add i64 %dim_0_vector_tid.i, 359436
  %453 = getelementptr float addrspace(1)* %4, i64 %.sum8935vector_func.i
  %exData3710vector_func.i = extractelement <16 x float> %call.i28.i, i32 12
  store float %exData3710vector_func.i, float addrspace(1)* %453, align 4
  br label %postload7392vector_func.i

postload7392vector_func.i:                        ; preds = %preload7391vector_func.i, %postload7389vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6836vector_func.i, label %postload6837vector_func.i

preload6836vector_func.i:                         ; preds = %postload7392vector_func.i
  %.sum8934vector_func.i = add i64 %dim_0_vector_tid.i, 359437
  %454 = getelementptr float addrspace(1)* %4, i64 %.sum8934vector_func.i
  %exData3713vector_func.i = extractelement <16 x float> %call.i28.i, i32 13
  store float %exData3713vector_func.i, float addrspace(1)* %454, align 4
  br label %postload6837vector_func.i

postload6837vector_func.i:                        ; preds = %preload6836vector_func.i, %postload7392vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6839vector_func.i, label %postload6840vector_func.i

preload6839vector_func.i:                         ; preds = %postload6837vector_func.i
  %.sum8933vector_func.i = add i64 %dim_0_vector_tid.i, 359438
  %455 = getelementptr float addrspace(1)* %4, i64 %.sum8933vector_func.i
  %exData3716vector_func.i = extractelement <16 x float> %call.i28.i, i32 14
  store float %exData3716vector_func.i, float addrspace(1)* %455, align 4
  br label %postload6840vector_func.i

postload6840vector_func.i:                        ; preds = %preload6839vector_func.i, %postload6837vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6842vector_func.i, label %postload6843vector_func.i

preload6842vector_func.i:                         ; preds = %postload6840vector_func.i
  %.sum8932vector_func.i = add i64 %dim_0_vector_tid.i, 359439
  %456 = getelementptr float addrspace(1)* %4, i64 %.sum8932vector_func.i
  %exData3719vector_func.i = extractelement <16 x float> %call.i28.i, i32 15
  store float %exData3719vector_func.i, float addrspace(1)* %456, align 4
  br label %postload6843vector_func.i

postload6843vector_func.i:                        ; preds = %preload6842vector_func.i, %postload6840vector_func.i
  %mul2711367vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000, float 0x40D61047C0000000>
  %add2721368vector_func.i = fadd <16 x float> %mul2711367vector_func.i, <float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000, float 0xC00BD8A960000000>
  %mul2731369vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000, float 0x40159DCF40000000>
  %add2741370vector_func.i = fadd <16 x float> %add2721368vector_func.i, %mul2731369vector_func.i
  %mul.i2391371vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000, float 0xBCE2753BA0000000>
  %add.i2401372vector_func.i = fadd <16 x float> %mul.i2391371vector_func.i, <float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000, float 0x3DCF52CE40000000>
  %mul1.i2411373vector_func.i = fmul <16 x float> %add.i2401372vector_func.i, %mul567vector_func.i
  %add2.i2421374vector_func.i = fadd <16 x float> %mul1.i2411373vector_func.i, <float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000, float 0xBEA7A2A060000000>
  %mul3.i2431375vector_func.i = fmul <16 x float> %add2.i2421374vector_func.i, %mul567vector_func.i
  %add4.i2441376vector_func.i = fadd <16 x float> %mul3.i2431375vector_func.i, <float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000, float 0x3F78024260000000>
  %mul5.i2451377vector_func.i = fmul <16 x float> %add4.i2441376vector_func.i, %mul567vector_func.i
  %add2761378vector_func.i = fadd <16 x float> %add2741370vector_func.i, %mul5.i2451377vector_func.i
  %call.i29.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2761378vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload5954vector_func.i, label %postload5955vector_func.i

preload5954vector_func.i:                         ; preds = %postload6843vector_func.i
  %extract1380vector_func.i = add i64 %dim_0_vector_tid.i, 373248
  %457 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1380vector_func.i
  %exData3723vector_func.i = extractelement <16 x float> %call.i29.i, i32 0
  store float %exData3723vector_func.i, float addrspace(1)* %457, align 4
  br label %postload5955vector_func.i

postload5955vector_func.i:                        ; preds = %preload5954vector_func.i, %postload6843vector_func.i
  br i1 %exmask2402vector_func.i, label %preload5957vector_func.i, label %postload5958vector_func.i

preload5957vector_func.i:                         ; preds = %postload5955vector_func.i
  %.sum8931vector_func.i = add i64 %dim_0_vector_tid.i, 373249
  %458 = getelementptr float addrspace(1)* %4, i64 %.sum8931vector_func.i
  %exData3726vector_func.i = extractelement <16 x float> %call.i29.i, i32 1
  store float %exData3726vector_func.i, float addrspace(1)* %458, align 4
  br label %postload5958vector_func.i

postload5958vector_func.i:                        ; preds = %preload5957vector_func.i, %postload5955vector_func.i
  br i1 %exmask2405vector_func.i, label %preload5960vector_func.i, label %postload5961vector_func.i

preload5960vector_func.i:                         ; preds = %postload5958vector_func.i
  %.sum8930vector_func.i = add i64 %dim_0_vector_tid.i, 373250
  %459 = getelementptr float addrspace(1)* %4, i64 %.sum8930vector_func.i
  %exData3729vector_func.i = extractelement <16 x float> %call.i29.i, i32 2
  store float %exData3729vector_func.i, float addrspace(1)* %459, align 4
  br label %postload5961vector_func.i

postload5961vector_func.i:                        ; preds = %preload5960vector_func.i, %postload5958vector_func.i
  br i1 %exmask2408vector_func.i, label %preload5963vector_func.i, label %postload5964vector_func.i

preload5963vector_func.i:                         ; preds = %postload5961vector_func.i
  %.sum8929vector_func.i = add i64 %dim_0_vector_tid.i, 373251
  %460 = getelementptr float addrspace(1)* %4, i64 %.sum8929vector_func.i
  %exData3732vector_func.i = extractelement <16 x float> %call.i29.i, i32 3
  store float %exData3732vector_func.i, float addrspace(1)* %460, align 4
  br label %postload5964vector_func.i

postload5964vector_func.i:                        ; preds = %preload5963vector_func.i, %postload5961vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6926vector_func.i, label %postload6927vector_func.i

preload6926vector_func.i:                         ; preds = %postload5964vector_func.i
  %.sum8928vector_func.i = add i64 %dim_0_vector_tid.i, 373252
  %461 = getelementptr float addrspace(1)* %4, i64 %.sum8928vector_func.i
  %exData3735vector_func.i = extractelement <16 x float> %call.i29.i, i32 4
  store float %exData3735vector_func.i, float addrspace(1)* %461, align 4
  br label %postload6927vector_func.i

postload6927vector_func.i:                        ; preds = %preload6926vector_func.i, %postload5964vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6929vector_func.i, label %postload6930vector_func.i

preload6929vector_func.i:                         ; preds = %postload6927vector_func.i
  %.sum8927vector_func.i = add i64 %dim_0_vector_tid.i, 373253
  %462 = getelementptr float addrspace(1)* %4, i64 %.sum8927vector_func.i
  %exData3738vector_func.i = extractelement <16 x float> %call.i29.i, i32 5
  store float %exData3738vector_func.i, float addrspace(1)* %462, align 4
  br label %postload6930vector_func.i

postload6930vector_func.i:                        ; preds = %preload6929vector_func.i, %postload6927vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6932vector_func.i, label %postload6933vector_func.i

preload6932vector_func.i:                         ; preds = %postload6930vector_func.i
  %.sum8926vector_func.i = add i64 %dim_0_vector_tid.i, 373254
  %463 = getelementptr float addrspace(1)* %4, i64 %.sum8926vector_func.i
  %exData3741vector_func.i = extractelement <16 x float> %call.i29.i, i32 6
  store float %exData3741vector_func.i, float addrspace(1)* %463, align 4
  br label %postload6933vector_func.i

postload6933vector_func.i:                        ; preds = %preload6932vector_func.i, %postload6930vector_func.i
  br i1 %exmask2420vector_func.i, label %preload5975vector_func.i, label %postload5976vector_func.i

preload5975vector_func.i:                         ; preds = %postload6933vector_func.i
  %.sum8925vector_func.i = add i64 %dim_0_vector_tid.i, 373255
  %464 = getelementptr float addrspace(1)* %4, i64 %.sum8925vector_func.i
  %exData3744vector_func.i = extractelement <16 x float> %call.i29.i, i32 7
  store float %exData3744vector_func.i, float addrspace(1)* %464, align 4
  br label %postload5976vector_func.i

postload5976vector_func.i:                        ; preds = %preload5975vector_func.i, %postload6933vector_func.i
  br i1 %exmask2423vector_func.i, label %preload5978vector_func.i, label %postload5979vector_func.i

preload5978vector_func.i:                         ; preds = %postload5976vector_func.i
  %.sum8924vector_func.i = add i64 %dim_0_vector_tid.i, 373256
  %465 = getelementptr float addrspace(1)* %4, i64 %.sum8924vector_func.i
  %exData3747vector_func.i = extractelement <16 x float> %call.i29.i, i32 8
  store float %exData3747vector_func.i, float addrspace(1)* %465, align 4
  br label %postload5979vector_func.i

postload5979vector_func.i:                        ; preds = %preload5978vector_func.i, %postload5976vector_func.i
  br i1 %exmask2426vector_func.i, label %preload5981vector_func.i, label %postload5982vector_func.i

preload5981vector_func.i:                         ; preds = %postload5979vector_func.i
  %.sum8923vector_func.i = add i64 %dim_0_vector_tid.i, 373257
  %466 = getelementptr float addrspace(1)* %4, i64 %.sum8923vector_func.i
  %exData3750vector_func.i = extractelement <16 x float> %call.i29.i, i32 9
  store float %exData3750vector_func.i, float addrspace(1)* %466, align 4
  br label %postload5982vector_func.i

postload5982vector_func.i:                        ; preds = %preload5981vector_func.i, %postload5979vector_func.i
  br i1 %exmask2429vector_func.i, label %preload5984vector_func.i, label %postload5985vector_func.i

preload5984vector_func.i:                         ; preds = %postload5982vector_func.i
  %.sum8922vector_func.i = add i64 %dim_0_vector_tid.i, 373258
  %467 = getelementptr float addrspace(1)* %4, i64 %.sum8922vector_func.i
  %exData3753vector_func.i = extractelement <16 x float> %call.i29.i, i32 10
  store float %exData3753vector_func.i, float addrspace(1)* %467, align 4
  br label %postload5985vector_func.i

postload5985vector_func.i:                        ; preds = %preload5984vector_func.i, %postload5982vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6821vector_func.i, label %postload6822vector_func.i

preload6821vector_func.i:                         ; preds = %postload5985vector_func.i
  %.sum8921vector_func.i = add i64 %dim_0_vector_tid.i, 373259
  %468 = getelementptr float addrspace(1)* %4, i64 %.sum8921vector_func.i
  %exData3756vector_func.i = extractelement <16 x float> %call.i29.i, i32 11
  store float %exData3756vector_func.i, float addrspace(1)* %468, align 4
  br label %postload6822vector_func.i

postload6822vector_func.i:                        ; preds = %preload6821vector_func.i, %postload5985vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6824vector_func.i, label %postload6825vector_func.i

preload6824vector_func.i:                         ; preds = %postload6822vector_func.i
  %.sum8920vector_func.i = add i64 %dim_0_vector_tid.i, 373260
  %469 = getelementptr float addrspace(1)* %4, i64 %.sum8920vector_func.i
  %exData3759vector_func.i = extractelement <16 x float> %call.i29.i, i32 12
  store float %exData3759vector_func.i, float addrspace(1)* %469, align 4
  br label %postload6825vector_func.i

postload6825vector_func.i:                        ; preds = %preload6824vector_func.i, %postload6822vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6827vector_func.i, label %postload6828vector_func.i

preload6827vector_func.i:                         ; preds = %postload6825vector_func.i
  %.sum8919vector_func.i = add i64 %dim_0_vector_tid.i, 373261
  %470 = getelementptr float addrspace(1)* %4, i64 %.sum8919vector_func.i
  %exData3762vector_func.i = extractelement <16 x float> %call.i29.i, i32 13
  store float %exData3762vector_func.i, float addrspace(1)* %470, align 4
  br label %postload6828vector_func.i

postload6828vector_func.i:                        ; preds = %preload6827vector_func.i, %postload6825vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6191vector_func.i, label %postload6192vector_func.i

preload6191vector_func.i:                         ; preds = %postload6828vector_func.i
  %.sum8918vector_func.i = add i64 %dim_0_vector_tid.i, 373262
  %471 = getelementptr float addrspace(1)* %4, i64 %.sum8918vector_func.i
  %exData3765vector_func.i = extractelement <16 x float> %call.i29.i, i32 14
  store float %exData3765vector_func.i, float addrspace(1)* %471, align 4
  br label %postload6192vector_func.i

postload6192vector_func.i:                        ; preds = %preload6191vector_func.i, %postload6828vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6194vector_func.i, label %postload6195vector_func.i

preload6194vector_func.i:                         ; preds = %postload6192vector_func.i
  %.sum8917vector_func.i = add i64 %dim_0_vector_tid.i, 373263
  %472 = getelementptr float addrspace(1)* %4, i64 %.sum8917vector_func.i
  %exData3768vector_func.i = extractelement <16 x float> %call.i29.i, i32 15
  store float %exData3768vector_func.i, float addrspace(1)* %472, align 4
  br label %postload6195vector_func.i

postload6195vector_func.i:                        ; preds = %preload6194vector_func.i, %postload6192vector_func.i
  %mul2811397vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000, float 0x40D1129CC0000000>
  %sub2821398vector_func.i = fsub <16 x float> <float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000, float 0xC0267C7100000000>, %mul2811397vector_func.i
  %mul2831399vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000, float 0x401A00CE80000000>
  %add2841400vector_func.i = fadd <16 x float> %sub2821398vector_func.i, %mul2831399vector_func.i
  %mul.i2321401vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000, float 0xBCF4591FA0000000>
  %add.i2331402vector_func.i = fadd <16 x float> %mul.i2321401vector_func.i, <float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000, float 0x3DD961D9C0000000>
  %mul1.i2341403vector_func.i = fmul <16 x float> %add.i2331402vector_func.i, %mul567vector_func.i
  %add2.i2351404vector_func.i = fadd <16 x float> %mul1.i2341403vector_func.i, <float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000, float 0xBEAFC12CE0000000>
  %mul3.i2361405vector_func.i = fmul <16 x float> %add2.i2351404vector_func.i, %mul567vector_func.i
  %add4.i2371406vector_func.i = fadd <16 x float> %mul3.i2361405vector_func.i, <float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000, float 0x3F7D5648E0000000>
  %mul5.i2381407vector_func.i = fmul <16 x float> %add4.i2371406vector_func.i, %mul567vector_func.i
  %add2861408vector_func.i = fadd <16 x float> %add2841400vector_func.i, %mul5.i2381407vector_func.i
  %call.i30.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2861408vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6197vector_func.i, label %postload6198vector_func.i

preload6197vector_func.i:                         ; preds = %postload6195vector_func.i
  %extract1410vector_func.i = add i64 %dim_0_vector_tid.i, 387072
  %473 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1410vector_func.i
  %exData3772vector_func.i = extractelement <16 x float> %call.i30.i, i32 0
  store float %exData3772vector_func.i, float addrspace(1)* %473, align 4
  br label %postload6198vector_func.i

postload6198vector_func.i:                        ; preds = %preload6197vector_func.i, %postload6195vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6200vector_func.i, label %postload6201vector_func.i

preload6200vector_func.i:                         ; preds = %postload6198vector_func.i
  %.sum8916vector_func.i = add i64 %dim_0_vector_tid.i, 387073
  %474 = getelementptr float addrspace(1)* %4, i64 %.sum8916vector_func.i
  %exData3775vector_func.i = extractelement <16 x float> %call.i30.i, i32 1
  store float %exData3775vector_func.i, float addrspace(1)* %474, align 4
  br label %postload6201vector_func.i

postload6201vector_func.i:                        ; preds = %preload6200vector_func.i, %postload6198vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6173vector_func.i, label %postload6174vector_func.i

preload6173vector_func.i:                         ; preds = %postload6201vector_func.i
  %.sum8915vector_func.i = add i64 %dim_0_vector_tid.i, 387074
  %475 = getelementptr float addrspace(1)* %4, i64 %.sum8915vector_func.i
  %exData3778vector_func.i = extractelement <16 x float> %call.i30.i, i32 2
  store float %exData3778vector_func.i, float addrspace(1)* %475, align 4
  br label %postload6174vector_func.i

postload6174vector_func.i:                        ; preds = %preload6173vector_func.i, %postload6201vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6176vector_func.i, label %postload6177vector_func.i

preload6176vector_func.i:                         ; preds = %postload6174vector_func.i
  %.sum8914vector_func.i = add i64 %dim_0_vector_tid.i, 387075
  %476 = getelementptr float addrspace(1)* %4, i64 %.sum8914vector_func.i
  %exData3781vector_func.i = extractelement <16 x float> %call.i30.i, i32 3
  store float %exData3781vector_func.i, float addrspace(1)* %476, align 4
  br label %postload6177vector_func.i

postload6177vector_func.i:                        ; preds = %preload6176vector_func.i, %postload6174vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6179vector_func.i, label %postload6180vector_func.i

preload6179vector_func.i:                         ; preds = %postload6177vector_func.i
  %.sum8913vector_func.i = add i64 %dim_0_vector_tid.i, 387076
  %477 = getelementptr float addrspace(1)* %4, i64 %.sum8913vector_func.i
  %exData3784vector_func.i = extractelement <16 x float> %call.i30.i, i32 4
  store float %exData3784vector_func.i, float addrspace(1)* %477, align 4
  br label %postload6180vector_func.i

postload6180vector_func.i:                        ; preds = %preload6179vector_func.i, %postload6177vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6230vector_func.i, label %postload6231vector_func.i

preload6230vector_func.i:                         ; preds = %postload6180vector_func.i
  %.sum8912vector_func.i = add i64 %dim_0_vector_tid.i, 387077
  %478 = getelementptr float addrspace(1)* %4, i64 %.sum8912vector_func.i
  %exData3787vector_func.i = extractelement <16 x float> %call.i30.i, i32 5
  store float %exData3787vector_func.i, float addrspace(1)* %478, align 4
  br label %postload6231vector_func.i

postload6231vector_func.i:                        ; preds = %preload6230vector_func.i, %postload6180vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6233vector_func.i, label %postload6234vector_func.i

preload6233vector_func.i:                         ; preds = %postload6231vector_func.i
  %.sum8911vector_func.i = add i64 %dim_0_vector_tid.i, 387078
  %479 = getelementptr float addrspace(1)* %4, i64 %.sum8911vector_func.i
  %exData3790vector_func.i = extractelement <16 x float> %call.i30.i, i32 6
  store float %exData3790vector_func.i, float addrspace(1)* %479, align 4
  br label %postload6234vector_func.i

postload6234vector_func.i:                        ; preds = %preload6233vector_func.i, %postload6231vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6236vector_func.i, label %postload6237vector_func.i

preload6236vector_func.i:                         ; preds = %postload6234vector_func.i
  %.sum8910vector_func.i = add i64 %dim_0_vector_tid.i, 387079
  %480 = getelementptr float addrspace(1)* %4, i64 %.sum8910vector_func.i
  %exData3793vector_func.i = extractelement <16 x float> %call.i30.i, i32 7
  store float %exData3793vector_func.i, float addrspace(1)* %480, align 4
  br label %postload6237vector_func.i

postload6237vector_func.i:                        ; preds = %preload6236vector_func.i, %postload6234vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6239vector_func.i, label %postload6240vector_func.i

preload6239vector_func.i:                         ; preds = %postload6237vector_func.i
  %.sum8909vector_func.i = add i64 %dim_0_vector_tid.i, 387080
  %481 = getelementptr float addrspace(1)* %4, i64 %.sum8909vector_func.i
  %exData3796vector_func.i = extractelement <16 x float> %call.i30.i, i32 8
  store float %exData3796vector_func.i, float addrspace(1)* %481, align 4
  br label %postload6240vector_func.i

postload6240vector_func.i:                        ; preds = %preload6239vector_func.i, %postload6237vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6344vector_func.i, label %postload6345vector_func.i

preload6344vector_func.i:                         ; preds = %postload6240vector_func.i
  %.sum8908vector_func.i = add i64 %dim_0_vector_tid.i, 387081
  %482 = getelementptr float addrspace(1)* %4, i64 %.sum8908vector_func.i
  %exData3799vector_func.i = extractelement <16 x float> %call.i30.i, i32 9
  store float %exData3799vector_func.i, float addrspace(1)* %482, align 4
  br label %postload6345vector_func.i

postload6345vector_func.i:                        ; preds = %preload6344vector_func.i, %postload6240vector_func.i
  br i1 %exmask2429vector_func.i, label %preload6347vector_func.i, label %postload6348vector_func.i

preload6347vector_func.i:                         ; preds = %postload6345vector_func.i
  %.sum8907vector_func.i = add i64 %dim_0_vector_tid.i, 387082
  %483 = getelementptr float addrspace(1)* %4, i64 %.sum8907vector_func.i
  %exData3802vector_func.i = extractelement <16 x float> %call.i30.i, i32 10
  store float %exData3802vector_func.i, float addrspace(1)* %483, align 4
  br label %postload6348vector_func.i

postload6348vector_func.i:                        ; preds = %preload6347vector_func.i, %postload6345vector_func.i
  br i1 %exmask2432vector_func.i, label %preload6350vector_func.i, label %postload6351vector_func.i

preload6350vector_func.i:                         ; preds = %postload6348vector_func.i
  %.sum8906vector_func.i = add i64 %dim_0_vector_tid.i, 387083
  %484 = getelementptr float addrspace(1)* %4, i64 %.sum8906vector_func.i
  %exData3805vector_func.i = extractelement <16 x float> %call.i30.i, i32 11
  store float %exData3805vector_func.i, float addrspace(1)* %484, align 4
  br label %postload6351vector_func.i

postload6351vector_func.i:                        ; preds = %preload6350vector_func.i, %postload6348vector_func.i
  br i1 %exmask2435vector_func.i, label %preload6437vector_func.i, label %postload6438vector_func.i

preload6437vector_func.i:                         ; preds = %postload6351vector_func.i
  %.sum8905vector_func.i = add i64 %dim_0_vector_tid.i, 387084
  %485 = getelementptr float addrspace(1)* %4, i64 %.sum8905vector_func.i
  %exData3808vector_func.i = extractelement <16 x float> %call.i30.i, i32 12
  store float %exData3808vector_func.i, float addrspace(1)* %485, align 4
  br label %postload6438vector_func.i

postload6438vector_func.i:                        ; preds = %preload6437vector_func.i, %postload6351vector_func.i
  br i1 %exmask2438vector_func.i, label %preload6440vector_func.i, label %postload6441vector_func.i

preload6440vector_func.i:                         ; preds = %postload6438vector_func.i
  %.sum8904vector_func.i = add i64 %dim_0_vector_tid.i, 387085
  %486 = getelementptr float addrspace(1)* %4, i64 %.sum8904vector_func.i
  %exData3811vector_func.i = extractelement <16 x float> %call.i30.i, i32 13
  store float %exData3811vector_func.i, float addrspace(1)* %486, align 4
  br label %postload6441vector_func.i

postload6441vector_func.i:                        ; preds = %preload6440vector_func.i, %postload6438vector_func.i
  br i1 %exmask2441vector_func.i, label %preload6443vector_func.i, label %postload6444vector_func.i

preload6443vector_func.i:                         ; preds = %postload6441vector_func.i
  %.sum8903vector_func.i = add i64 %dim_0_vector_tid.i, 387086
  %487 = getelementptr float addrspace(1)* %4, i64 %.sum8903vector_func.i
  %exData3814vector_func.i = extractelement <16 x float> %call.i30.i, i32 14
  store float %exData3814vector_func.i, float addrspace(1)* %487, align 4
  br label %postload6444vector_func.i

postload6444vector_func.i:                        ; preds = %preload6443vector_func.i, %postload6441vector_func.i
  br i1 %exmask2444vector_func.i, label %preload6446vector_func.i, label %postload6447vector_func.i

preload6446vector_func.i:                         ; preds = %postload6444vector_func.i
  %.sum8902vector_func.i = add i64 %dim_0_vector_tid.i, 387087
  %488 = getelementptr float addrspace(1)* %4, i64 %.sum8902vector_func.i
  %exData3817vector_func.i = extractelement <16 x float> %call.i30.i, i32 15
  store float %exData3817vector_func.i, float addrspace(1)* %488, align 4
  br label %postload6447vector_func.i

postload6447vector_func.i:                        ; preds = %preload6446vector_func.i, %postload6444vector_func.i
  %mul2911427vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000, float 0x408CDC9000000000>
  %add2921428vector_func.i = fadd <16 x float> %mul2911427vector_func.i, <float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000, float 0xC02AA06F60000000>
  %mul2931429vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000, float 0x401AEDD4C0000000>
  %add2941430vector_func.i = fadd <16 x float> %add2921428vector_func.i, %mul2931429vector_func.i
  %mul.i2251431vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000, float 0xBCE0F62340000000>
  %add.i2261432vector_func.i = fadd <16 x float> %mul.i2251431vector_func.i, <float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000, float 0x3DD0852CA0000000>
  %mul1.i2271433vector_func.i = fmul <16 x float> %add.i2261432vector_func.i, %mul567vector_func.i
  %add2.i2281434vector_func.i = fadd <16 x float> %mul1.i2271433vector_func.i, <float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000, float 0xBEABAE8D20000000>
  %mul3.i2291435vector_func.i = fmul <16 x float> %add2.i2281434vector_func.i, %mul567vector_func.i
  %add4.i2301436vector_func.i = fadd <16 x float> %mul3.i2291435vector_func.i, <float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000, float 0x3F7E884380000000>
  %mul5.i2311437vector_func.i = fmul <16 x float> %add4.i2301436vector_func.i, %mul567vector_func.i
  %add2961438vector_func.i = fadd <16 x float> %add2941430vector_func.i, %mul5.i2311437vector_func.i
  %call.i31.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add2961438vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload6473vector_func.i, label %postload6474vector_func.i

preload6473vector_func.i:                         ; preds = %postload6447vector_func.i
  %extract1440vector_func.i = add i64 %dim_0_vector_tid.i, 400896
  %489 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1440vector_func.i
  %exData3821vector_func.i = extractelement <16 x float> %call.i31.i, i32 0
  store float %exData3821vector_func.i, float addrspace(1)* %489, align 4
  br label %postload6474vector_func.i

postload6474vector_func.i:                        ; preds = %preload6473vector_func.i, %postload6447vector_func.i
  br i1 %exmask2402vector_func.i, label %preload6476vector_func.i, label %postload6477vector_func.i

preload6476vector_func.i:                         ; preds = %postload6474vector_func.i
  %.sum8901vector_func.i = add i64 %dim_0_vector_tid.i, 400897
  %490 = getelementptr float addrspace(1)* %4, i64 %.sum8901vector_func.i
  %exData3824vector_func.i = extractelement <16 x float> %call.i31.i, i32 1
  store float %exData3824vector_func.i, float addrspace(1)* %490, align 4
  br label %postload6477vector_func.i

postload6477vector_func.i:                        ; preds = %preload6476vector_func.i, %postload6474vector_func.i
  br i1 %exmask2405vector_func.i, label %preload6479vector_func.i, label %postload6480vector_func.i

preload6479vector_func.i:                         ; preds = %postload6477vector_func.i
  %.sum8900vector_func.i = add i64 %dim_0_vector_tid.i, 400898
  %491 = getelementptr float addrspace(1)* %4, i64 %.sum8900vector_func.i
  %exData3827vector_func.i = extractelement <16 x float> %call.i31.i, i32 2
  store float %exData3827vector_func.i, float addrspace(1)* %491, align 4
  br label %postload6480vector_func.i

postload6480vector_func.i:                        ; preds = %preload6479vector_func.i, %postload6477vector_func.i
  br i1 %exmask2408vector_func.i, label %preload6533vector_func.i, label %postload6534vector_func.i

preload6533vector_func.i:                         ; preds = %postload6480vector_func.i
  %.sum8899vector_func.i = add i64 %dim_0_vector_tid.i, 400899
  %492 = getelementptr float addrspace(1)* %4, i64 %.sum8899vector_func.i
  %exData3830vector_func.i = extractelement <16 x float> %call.i31.i, i32 3
  store float %exData3830vector_func.i, float addrspace(1)* %492, align 4
  br label %postload6534vector_func.i

postload6534vector_func.i:                        ; preds = %preload6533vector_func.i, %postload6480vector_func.i
  br i1 %exmask2411vector_func.i, label %preload6536vector_func.i, label %postload6537vector_func.i

preload6536vector_func.i:                         ; preds = %postload6534vector_func.i
  %.sum8898vector_func.i = add i64 %dim_0_vector_tid.i, 400900
  %493 = getelementptr float addrspace(1)* %4, i64 %.sum8898vector_func.i
  %exData3833vector_func.i = extractelement <16 x float> %call.i31.i, i32 4
  store float %exData3833vector_func.i, float addrspace(1)* %493, align 4
  br label %postload6537vector_func.i

postload6537vector_func.i:                        ; preds = %preload6536vector_func.i, %postload6534vector_func.i
  br i1 %exmask2414vector_func.i, label %preload6539vector_func.i, label %postload6540vector_func.i

preload6539vector_func.i:                         ; preds = %postload6537vector_func.i
  %.sum8897vector_func.i = add i64 %dim_0_vector_tid.i, 400901
  %494 = getelementptr float addrspace(1)* %4, i64 %.sum8897vector_func.i
  %exData3836vector_func.i = extractelement <16 x float> %call.i31.i, i32 5
  store float %exData3836vector_func.i, float addrspace(1)* %494, align 4
  br label %postload6540vector_func.i

postload6540vector_func.i:                        ; preds = %preload6539vector_func.i, %postload6537vector_func.i
  br i1 %exmask2417vector_func.i, label %preload6542vector_func.i, label %postload6543vector_func.i

preload6542vector_func.i:                         ; preds = %postload6540vector_func.i
  %.sum8896vector_func.i = add i64 %dim_0_vector_tid.i, 400902
  %495 = getelementptr float addrspace(1)* %4, i64 %.sum8896vector_func.i
  %exData3839vector_func.i = extractelement <16 x float> %call.i31.i, i32 6
  store float %exData3839vector_func.i, float addrspace(1)* %495, align 4
  br label %postload6543vector_func.i

postload6543vector_func.i:                        ; preds = %preload6542vector_func.i, %postload6540vector_func.i
  br i1 %exmask2420vector_func.i, label %preload6374vector_func.i, label %postload6375vector_func.i

preload6374vector_func.i:                         ; preds = %postload6543vector_func.i
  %.sum8895vector_func.i = add i64 %dim_0_vector_tid.i, 400903
  %496 = getelementptr float addrspace(1)* %4, i64 %.sum8895vector_func.i
  %exData3842vector_func.i = extractelement <16 x float> %call.i31.i, i32 7
  store float %exData3842vector_func.i, float addrspace(1)* %496, align 4
  br label %postload6375vector_func.i

postload6375vector_func.i:                        ; preds = %preload6374vector_func.i, %postload6543vector_func.i
  br i1 %exmask2423vector_func.i, label %preload6377vector_func.i, label %postload6378vector_func.i

preload6377vector_func.i:                         ; preds = %postload6375vector_func.i
  %.sum8894vector_func.i = add i64 %dim_0_vector_tid.i, 400904
  %497 = getelementptr float addrspace(1)* %4, i64 %.sum8894vector_func.i
  %exData3845vector_func.i = extractelement <16 x float> %call.i31.i, i32 8
  store float %exData3845vector_func.i, float addrspace(1)* %497, align 4
  br label %postload6378vector_func.i

postload6378vector_func.i:                        ; preds = %preload6377vector_func.i, %postload6375vector_func.i
  br i1 %exmask2426vector_func.i, label %preload6380vector_func.i, label %postload6381vector_func.i

preload6380vector_func.i:                         ; preds = %postload6378vector_func.i
  %.sum8893vector_func.i = add i64 %dim_0_vector_tid.i, 400905
  %498 = getelementptr float addrspace(1)* %4, i64 %.sum8893vector_func.i
  %exData3848vector_func.i = extractelement <16 x float> %call.i31.i, i32 9
  store float %exData3848vector_func.i, float addrspace(1)* %498, align 4
  br label %postload6381vector_func.i

postload6381vector_func.i:                        ; preds = %preload6380vector_func.i, %postload6378vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7025vector_func.i, label %postload7026vector_func.i

preload7025vector_func.i:                         ; preds = %postload6381vector_func.i
  %.sum8892vector_func.i = add i64 %dim_0_vector_tid.i, 400906
  %499 = getelementptr float addrspace(1)* %4, i64 %.sum8892vector_func.i
  %exData3851vector_func.i = extractelement <16 x float> %call.i31.i, i32 10
  store float %exData3851vector_func.i, float addrspace(1)* %499, align 4
  br label %postload7026vector_func.i

postload7026vector_func.i:                        ; preds = %preload7025vector_func.i, %postload6381vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7028vector_func.i, label %postload7029vector_func.i

preload7028vector_func.i:                         ; preds = %postload7026vector_func.i
  %.sum8891vector_func.i = add i64 %dim_0_vector_tid.i, 400907
  %500 = getelementptr float addrspace(1)* %4, i64 %.sum8891vector_func.i
  %exData3854vector_func.i = extractelement <16 x float> %call.i31.i, i32 11
  store float %exData3854vector_func.i, float addrspace(1)* %500, align 4
  br label %postload7029vector_func.i

postload7029vector_func.i:                        ; preds = %preload7028vector_func.i, %postload7026vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7031vector_func.i, label %postload7032vector_func.i

preload7031vector_func.i:                         ; preds = %postload7029vector_func.i
  %.sum8890vector_func.i = add i64 %dim_0_vector_tid.i, 400908
  %501 = getelementptr float addrspace(1)* %4, i64 %.sum8890vector_func.i
  %exData3857vector_func.i = extractelement <16 x float> %call.i31.i, i32 12
  store float %exData3857vector_func.i, float addrspace(1)* %501, align 4
  br label %postload7032vector_func.i

postload7032vector_func.i:                        ; preds = %preload7031vector_func.i, %postload7029vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7034vector_func.i, label %postload7035vector_func.i

preload7034vector_func.i:                         ; preds = %postload7032vector_func.i
  %.sum8889vector_func.i = add i64 %dim_0_vector_tid.i, 400909
  %502 = getelementptr float addrspace(1)* %4, i64 %.sum8889vector_func.i
  %exData3860vector_func.i = extractelement <16 x float> %call.i31.i, i32 13
  store float %exData3860vector_func.i, float addrspace(1)* %502, align 4
  br label %postload7035vector_func.i

postload7035vector_func.i:                        ; preds = %preload7034vector_func.i, %postload7032vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7100vector_func.i, label %postload7101vector_func.i

preload7100vector_func.i:                         ; preds = %postload7035vector_func.i
  %.sum8888vector_func.i = add i64 %dim_0_vector_tid.i, 400910
  %503 = getelementptr float addrspace(1)* %4, i64 %.sum8888vector_func.i
  %exData3863vector_func.i = extractelement <16 x float> %call.i31.i, i32 14
  store float %exData3863vector_func.i, float addrspace(1)* %503, align 4
  br label %postload7101vector_func.i

postload7101vector_func.i:                        ; preds = %preload7100vector_func.i, %postload7035vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7103vector_func.i, label %postload7104vector_func.i

preload7103vector_func.i:                         ; preds = %postload7101vector_func.i
  %.sum8887vector_func.i = add i64 %dim_0_vector_tid.i, 400911
  %504 = getelementptr float addrspace(1)* %4, i64 %.sum8887vector_func.i
  %exData3866vector_func.i = extractelement <16 x float> %call.i31.i, i32 15
  store float %exData3866vector_func.i, float addrspace(1)* %504, align 4
  br label %postload7104vector_func.i

postload7104vector_func.i:                        ; preds = %preload7103vector_func.i, %postload7101vector_func.i
  %mul3011457vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000, float 0x40BF283940000000>
  %sub3021458vector_func.i = fsub <16 x float> <float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000, float 0xC02F07D500000000>, %mul3011457vector_func.i
  %mul3031459vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000, float 0x401ED6C820000000>
  %add3041460vector_func.i = fadd <16 x float> %sub3021458vector_func.i, %mul3031459vector_func.i
  %mul.i2181461vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000, float 0xBCE1809100000000>
  %add.i2191462vector_func.i = fadd <16 x float> %mul.i2181461vector_func.i, <float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000, float 0x3DD16223E0000000>
  %mul1.i2201463vector_func.i = fmul <16 x float> %add.i2191462vector_func.i, %mul567vector_func.i
  %add2.i2211464vector_func.i = fadd <16 x float> %mul1.i2201463vector_func.i, <float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000, float 0xBEAD7BB920000000>
  %mul3.i2221465vector_func.i = fmul <16 x float> %add2.i2211464vector_func.i, %mul567vector_func.i
  %add4.i2231466vector_func.i = fadd <16 x float> %mul3.i2221465vector_func.i, <float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000, float 0x3F806A8EC0000000>
  %mul5.i2241467vector_func.i = fmul <16 x float> %add4.i2231466vector_func.i, %mul567vector_func.i
  %add3061468vector_func.i = fadd <16 x float> %add3041460vector_func.i, %mul5.i2241467vector_func.i
  %call.i32.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3061468vector_func.i) nounwind readnone
  br i1 %exmaskvector_func.i, label %preload7106vector_func.i, label %postload7107vector_func.i

preload7106vector_func.i:                         ; preds = %postload7104vector_func.i
  %extract1470vector_func.i = add i64 %dim_0_vector_tid.i, 414720
  %505 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1470vector_func.i
  %exData3870vector_func.i = extractelement <16 x float> %call.i32.i, i32 0
  store float %exData3870vector_func.i, float addrspace(1)* %505, align 4
  br label %postload7107vector_func.i

postload7107vector_func.i:                        ; preds = %preload7106vector_func.i, %postload7104vector_func.i
  br i1 %exmask2402vector_func.i, label %preload7115vector_func.i, label %postload7116vector_func.i

preload7115vector_func.i:                         ; preds = %postload7107vector_func.i
  %.sum8886vector_func.i = add i64 %dim_0_vector_tid.i, 414721
  %506 = getelementptr float addrspace(1)* %4, i64 %.sum8886vector_func.i
  %exData3873vector_func.i = extractelement <16 x float> %call.i32.i, i32 1
  store float %exData3873vector_func.i, float addrspace(1)* %506, align 4
  br label %postload7116vector_func.i

postload7116vector_func.i:                        ; preds = %preload7115vector_func.i, %postload7107vector_func.i
  br i1 %exmask2405vector_func.i, label %preload7118vector_func.i, label %postload7119vector_func.i

preload7118vector_func.i:                         ; preds = %postload7116vector_func.i
  %.sum8885vector_func.i = add i64 %dim_0_vector_tid.i, 414722
  %507 = getelementptr float addrspace(1)* %4, i64 %.sum8885vector_func.i
  %exData3876vector_func.i = extractelement <16 x float> %call.i32.i, i32 2
  store float %exData3876vector_func.i, float addrspace(1)* %507, align 4
  br label %postload7119vector_func.i

postload7119vector_func.i:                        ; preds = %preload7118vector_func.i, %postload7116vector_func.i
  br i1 %exmask2408vector_func.i, label %preload7121vector_func.i, label %postload7122vector_func.i

preload7121vector_func.i:                         ; preds = %postload7119vector_func.i
  %.sum8884vector_func.i = add i64 %dim_0_vector_tid.i, 414723
  %508 = getelementptr float addrspace(1)* %4, i64 %.sum8884vector_func.i
  %exData3879vector_func.i = extractelement <16 x float> %call.i32.i, i32 3
  store float %exData3879vector_func.i, float addrspace(1)* %508, align 4
  br label %postload7122vector_func.i

postload7122vector_func.i:                        ; preds = %preload7121vector_func.i, %postload7119vector_func.i
  br i1 %exmask2411vector_func.i, label %preload7124vector_func.i, label %postload7125vector_func.i

preload7124vector_func.i:                         ; preds = %postload7122vector_func.i
  %.sum8883vector_func.i = add i64 %dim_0_vector_tid.i, 414724
  %509 = getelementptr float addrspace(1)* %4, i64 %.sum8883vector_func.i
  %exData3882vector_func.i = extractelement <16 x float> %call.i32.i, i32 4
  store float %exData3882vector_func.i, float addrspace(1)* %509, align 4
  br label %postload7125vector_func.i

postload7125vector_func.i:                        ; preds = %preload7124vector_func.i, %postload7122vector_func.i
  br i1 %exmask2414vector_func.i, label %preload7139vector_func.i, label %postload7140vector_func.i

preload7139vector_func.i:                         ; preds = %postload7125vector_func.i
  %.sum8882vector_func.i = add i64 %dim_0_vector_tid.i, 414725
  %510 = getelementptr float addrspace(1)* %4, i64 %.sum8882vector_func.i
  %exData3885vector_func.i = extractelement <16 x float> %call.i32.i, i32 5
  store float %exData3885vector_func.i, float addrspace(1)* %510, align 4
  br label %postload7140vector_func.i

postload7140vector_func.i:                        ; preds = %preload7139vector_func.i, %postload7125vector_func.i
  br i1 %exmask2417vector_func.i, label %preload7142vector_func.i, label %postload7143vector_func.i

preload7142vector_func.i:                         ; preds = %postload7140vector_func.i
  %.sum8881vector_func.i = add i64 %dim_0_vector_tid.i, 414726
  %511 = getelementptr float addrspace(1)* %4, i64 %.sum8881vector_func.i
  %exData3888vector_func.i = extractelement <16 x float> %call.i32.i, i32 6
  store float %exData3888vector_func.i, float addrspace(1)* %511, align 4
  br label %postload7143vector_func.i

postload7143vector_func.i:                        ; preds = %preload7142vector_func.i, %postload7140vector_func.i
  br i1 %exmask2420vector_func.i, label %preload7145vector_func.i, label %postload7146vector_func.i

preload7145vector_func.i:                         ; preds = %postload7143vector_func.i
  %.sum8880vector_func.i = add i64 %dim_0_vector_tid.i, 414727
  %512 = getelementptr float addrspace(1)* %4, i64 %.sum8880vector_func.i
  %exData3891vector_func.i = extractelement <16 x float> %call.i32.i, i32 7
  store float %exData3891vector_func.i, float addrspace(1)* %512, align 4
  br label %postload7146vector_func.i

postload7146vector_func.i:                        ; preds = %preload7145vector_func.i, %postload7143vector_func.i
  br i1 %exmask2423vector_func.i, label %preload7187vector_func.i, label %postload7188vector_func.i

preload7187vector_func.i:                         ; preds = %postload7146vector_func.i
  %.sum8879vector_func.i = add i64 %dim_0_vector_tid.i, 414728
  %513 = getelementptr float addrspace(1)* %4, i64 %.sum8879vector_func.i
  %exData3894vector_func.i = extractelement <16 x float> %call.i32.i, i32 8
  store float %exData3894vector_func.i, float addrspace(1)* %513, align 4
  br label %postload7188vector_func.i

postload7188vector_func.i:                        ; preds = %preload7187vector_func.i, %postload7146vector_func.i
  br i1 %exmask2426vector_func.i, label %preload7190vector_func.i, label %postload7191vector_func.i

preload7190vector_func.i:                         ; preds = %postload7188vector_func.i
  %.sum8878vector_func.i = add i64 %dim_0_vector_tid.i, 414729
  %514 = getelementptr float addrspace(1)* %4, i64 %.sum8878vector_func.i
  %exData3897vector_func.i = extractelement <16 x float> %call.i32.i, i32 9
  store float %exData3897vector_func.i, float addrspace(1)* %514, align 4
  br label %postload7191vector_func.i

postload7191vector_func.i:                        ; preds = %preload7190vector_func.i, %postload7188vector_func.i
  br i1 %exmask2429vector_func.i, label %preload7193vector_func.i, label %postload7194vector_func.i

preload7193vector_func.i:                         ; preds = %postload7191vector_func.i
  %.sum8877vector_func.i = add i64 %dim_0_vector_tid.i, 414730
  %515 = getelementptr float addrspace(1)* %4, i64 %.sum8877vector_func.i
  %exData3900vector_func.i = extractelement <16 x float> %call.i32.i, i32 10
  store float %exData3900vector_func.i, float addrspace(1)* %515, align 4
  br label %postload7194vector_func.i

postload7194vector_func.i:                        ; preds = %preload7193vector_func.i, %postload7191vector_func.i
  br i1 %exmask2432vector_func.i, label %preload7196vector_func.i, label %postload7197vector_func.i

preload7196vector_func.i:                         ; preds = %postload7194vector_func.i
  %.sum8876vector_func.i = add i64 %dim_0_vector_tid.i, 414731
  %516 = getelementptr float addrspace(1)* %4, i64 %.sum8876vector_func.i
  %exData3903vector_func.i = extractelement <16 x float> %call.i32.i, i32 11
  store float %exData3903vector_func.i, float addrspace(1)* %516, align 4
  br label %postload7197vector_func.i

postload7197vector_func.i:                        ; preds = %preload7196vector_func.i, %postload7194vector_func.i
  br i1 %exmask2435vector_func.i, label %preload7316vector_func.i, label %postload7317vector_func.i

preload7316vector_func.i:                         ; preds = %postload7197vector_func.i
  %.sum8875vector_func.i = add i64 %dim_0_vector_tid.i, 414732
  %517 = getelementptr float addrspace(1)* %4, i64 %.sum8875vector_func.i
  %exData3906vector_func.i = extractelement <16 x float> %call.i32.i, i32 12
  store float %exData3906vector_func.i, float addrspace(1)* %517, align 4
  br label %postload7317vector_func.i

postload7317vector_func.i:                        ; preds = %preload7316vector_func.i, %postload7197vector_func.i
  br i1 %exmask2438vector_func.i, label %preload7319vector_func.i, label %postload7320vector_func.i

preload7319vector_func.i:                         ; preds = %postload7317vector_func.i
  %.sum8874vector_func.i = add i64 %dim_0_vector_tid.i, 414733
  %518 = getelementptr float addrspace(1)* %4, i64 %.sum8874vector_func.i
  %exData3909vector_func.i = extractelement <16 x float> %call.i32.i, i32 13
  store float %exData3909vector_func.i, float addrspace(1)* %518, align 4
  br label %postload7320vector_func.i

postload7320vector_func.i:                        ; preds = %preload7319vector_func.i, %postload7317vector_func.i
  br i1 %exmask2441vector_func.i, label %preload7322vector_func.i, label %postload7323vector_func.i

preload7322vector_func.i:                         ; preds = %postload7320vector_func.i
  %.sum8873vector_func.i = add i64 %dim_0_vector_tid.i, 414734
  %519 = getelementptr float addrspace(1)* %4, i64 %.sum8873vector_func.i
  %exData3912vector_func.i = extractelement <16 x float> %call.i32.i, i32 14
  store float %exData3912vector_func.i, float addrspace(1)* %519, align 4
  br label %postload7323vector_func.i

postload7323vector_func.i:                        ; preds = %preload7322vector_func.i, %postload7320vector_func.i
  br i1 %exmask2444vector_func.i, label %preload7400vector_func.i, label %if.elsevector_func.i

preload7400vector_func.i:                         ; preds = %postload7323vector_func.i
  %.sum8872vector_func.i = add i64 %dim_0_vector_tid.i, 414735
  %520 = getelementptr float addrspace(1)* %4, i64 %.sum8872vector_func.i
  %exData3915vector_func.i = extractelement <16 x float> %call.i32.i, i32 15
  store float %exData3915vector_func.i, float addrspace(1)* %520, align 4
  br label %if.elsevector_func.i

if.elsevector_func.i:                             ; preds = %preload7400vector_func.i, %postload7323vector_func.i
  %mul3111487vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000, float 0x408CAF7B40000000>
  %add3121488vector_func.i = fadd <16 x float> %mul3111487vector_func.i, <float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000, float 0x3FE5DB3840000000>
  %mul3131489vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000, float 0x4002C130A0000000>
  %add3141490vector_func.i = fadd <16 x float> %add3121488vector_func.i, %mul3131489vector_func.i
  %mul.i2111491vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000, float 0xBD59F3D0E0000000>
  %add.i2121492vector_func.i = fadd <16 x float> %mul.i2111491vector_func.i, <float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000, float 0x3E1CDBB200000000>
  %mul1.i2131493vector_func.i = fmul <16 x float> %add.i2121492vector_func.i, %mul567vector_func.i
  %add2.i2141494vector_func.i = fadd <16 x float> %mul1.i2131493vector_func.i, <float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000, float 0xBECB3B8080000000>
  %mul3.i2151495vector_func.i = fmul <16 x float> %add2.i2141494vector_func.i, %mul567vector_func.i
  %add4.i2161496vector_func.i = fadd <16 x float> %mul3.i2151495vector_func.i, <float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000, float 0x3F70581760000000>
  %mul5.i2171497vector_func.i = fmul <16 x float> %add4.i2161496vector_func.i, %mul567vector_func.i
  %add3161498vector_func.i = fadd <16 x float> %add3141490vector_func.i, %mul5.i2171497vector_func.i
  %call.i33.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3161498vector_func.i) nounwind readnone
  %exmask3918vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 0
  br i1 %exmask3918vector_func.i, label %preload7403vector_func.i, label %postload7404vector_func.i

preload7403vector_func.i:                         ; preds = %if.elsevector_func.i
  %521 = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_vector_tid.i
  %exData3919vector_func.i = extractelement <16 x float> %call.i33.i, i32 0
  store float %exData3919vector_func.i, float addrspace(1)* %521, align 4
  br label %postload7404vector_func.i

postload7404vector_func.i:                        ; preds = %preload7403vector_func.i, %if.elsevector_func.i
  %exmask3921vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 1
  br i1 %exmask3921vector_func.i, label %preload7406vector_func.i, label %postload7407vector_func.i

preload7406vector_func.i:                         ; preds = %postload7404vector_func.i
  %.sum8871vector_func.i = add i64 %dim_0_vector_tid.i, 1
  %522 = getelementptr float addrspace(1)* %4, i64 %.sum8871vector_func.i
  %exData3922vector_func.i = extractelement <16 x float> %call.i33.i, i32 1
  store float %exData3922vector_func.i, float addrspace(1)* %522, align 4
  br label %postload7407vector_func.i

postload7407vector_func.i:                        ; preds = %preload7406vector_func.i, %postload7404vector_func.i
  %exmask3924vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 2
  br i1 %exmask3924vector_func.i, label %preload7409vector_func.i, label %postload7410vector_func.i

preload7409vector_func.i:                         ; preds = %postload7407vector_func.i
  %.sum8870vector_func.i = add i64 %dim_0_vector_tid.i, 2
  %523 = getelementptr float addrspace(1)* %4, i64 %.sum8870vector_func.i
  %exData3925vector_func.i = extractelement <16 x float> %call.i33.i, i32 2
  store float %exData3925vector_func.i, float addrspace(1)* %523, align 4
  br label %postload7410vector_func.i

postload7410vector_func.i:                        ; preds = %preload7409vector_func.i, %postload7407vector_func.i
  %exmask3927vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 3
  br i1 %exmask3927vector_func.i, label %preload7460vector_func.i, label %postload7461vector_func.i

preload7460vector_func.i:                         ; preds = %postload7410vector_func.i
  %.sum8869vector_func.i = add i64 %dim_0_vector_tid.i, 3
  %524 = getelementptr float addrspace(1)* %4, i64 %.sum8869vector_func.i
  %exData3928vector_func.i = extractelement <16 x float> %call.i33.i, i32 3
  store float %exData3928vector_func.i, float addrspace(1)* %524, align 4
  br label %postload7461vector_func.i

postload7461vector_func.i:                        ; preds = %preload7460vector_func.i, %postload7410vector_func.i
  %exmask3930vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 4
  br i1 %exmask3930vector_func.i, label %preload7463vector_func.i, label %postload7464vector_func.i

preload7463vector_func.i:                         ; preds = %postload7461vector_func.i
  %.sum8868vector_func.i = add i64 %dim_0_vector_tid.i, 4
  %525 = getelementptr float addrspace(1)* %4, i64 %.sum8868vector_func.i
  %exData3931vector_func.i = extractelement <16 x float> %call.i33.i, i32 4
  store float %exData3931vector_func.i, float addrspace(1)* %525, align 4
  br label %postload7464vector_func.i

postload7464vector_func.i:                        ; preds = %preload7463vector_func.i, %postload7461vector_func.i
  %exmask3933vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 5
  br i1 %exmask3933vector_func.i, label %preload7466vector_func.i, label %postload7467vector_func.i

preload7466vector_func.i:                         ; preds = %postload7464vector_func.i
  %.sum8867vector_func.i = add i64 %dim_0_vector_tid.i, 5
  %526 = getelementptr float addrspace(1)* %4, i64 %.sum8867vector_func.i
  %exData3934vector_func.i = extractelement <16 x float> %call.i33.i, i32 5
  store float %exData3934vector_func.i, float addrspace(1)* %526, align 4
  br label %postload7467vector_func.i

postload7467vector_func.i:                        ; preds = %preload7466vector_func.i, %postload7464vector_func.i
  %exmask3936vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 6
  br i1 %exmask3936vector_func.i, label %preload6146vector_func.i, label %postload6147vector_func.i

preload6146vector_func.i:                         ; preds = %postload7467vector_func.i
  %.sum8866vector_func.i = add i64 %dim_0_vector_tid.i, 6
  %527 = getelementptr float addrspace(1)* %4, i64 %.sum8866vector_func.i
  %exData3937vector_func.i = extractelement <16 x float> %call.i33.i, i32 6
  store float %exData3937vector_func.i, float addrspace(1)* %527, align 4
  br label %postload6147vector_func.i

postload6147vector_func.i:                        ; preds = %preload6146vector_func.i, %postload7467vector_func.i
  %exmask3939vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 7
  br i1 %exmask3939vector_func.i, label %preload6149vector_func.i, label %postload6150vector_func.i

preload6149vector_func.i:                         ; preds = %postload6147vector_func.i
  %.sum8865vector_func.i = add i64 %dim_0_vector_tid.i, 7
  %528 = getelementptr float addrspace(1)* %4, i64 %.sum8865vector_func.i
  %exData3940vector_func.i = extractelement <16 x float> %call.i33.i, i32 7
  store float %exData3940vector_func.i, float addrspace(1)* %528, align 4
  br label %postload6150vector_func.i

postload6150vector_func.i:                        ; preds = %preload6149vector_func.i, %postload6147vector_func.i
  %exmask3942vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 8
  br i1 %exmask3942vector_func.i, label %preload6152vector_func.i, label %postload6153vector_func.i

preload6152vector_func.i:                         ; preds = %postload6150vector_func.i
  %.sum8864vector_func.i = add i64 %dim_0_vector_tid.i, 8
  %529 = getelementptr float addrspace(1)* %4, i64 %.sum8864vector_func.i
  %exData3943vector_func.i = extractelement <16 x float> %call.i33.i, i32 8
  store float %exData3943vector_func.i, float addrspace(1)* %529, align 4
  br label %postload6153vector_func.i

postload6153vector_func.i:                        ; preds = %preload6152vector_func.i, %postload6150vector_func.i
  %exmask3945vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 9
  br i1 %exmask3945vector_func.i, label %preload6155vector_func.i, label %postload6156vector_func.i

preload6155vector_func.i:                         ; preds = %postload6153vector_func.i
  %.sum8863vector_func.i = add i64 %dim_0_vector_tid.i, 9
  %530 = getelementptr float addrspace(1)* %4, i64 %.sum8863vector_func.i
  %exData3946vector_func.i = extractelement <16 x float> %call.i33.i, i32 9
  store float %exData3946vector_func.i, float addrspace(1)* %530, align 4
  br label %postload6156vector_func.i

postload6156vector_func.i:                        ; preds = %preload6155vector_func.i, %postload6153vector_func.i
  %exmask3948vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 10
  br i1 %exmask3948vector_func.i, label %preload5789vector_func.i, label %postload5790vector_func.i

preload5789vector_func.i:                         ; preds = %postload6156vector_func.i
  %.sum8862vector_func.i = add i64 %dim_0_vector_tid.i, 10
  %531 = getelementptr float addrspace(1)* %4, i64 %.sum8862vector_func.i
  %exData3949vector_func.i = extractelement <16 x float> %call.i33.i, i32 10
  store float %exData3949vector_func.i, float addrspace(1)* %531, align 4
  br label %postload5790vector_func.i

postload5790vector_func.i:                        ; preds = %preload5789vector_func.i, %postload6156vector_func.i
  %exmask3951vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 11
  br i1 %exmask3951vector_func.i, label %preload5792vector_func.i, label %postload5793vector_func.i

preload5792vector_func.i:                         ; preds = %postload5790vector_func.i
  %.sum8861vector_func.i = add i64 %dim_0_vector_tid.i, 11
  %532 = getelementptr float addrspace(1)* %4, i64 %.sum8861vector_func.i
  %exData3952vector_func.i = extractelement <16 x float> %call.i33.i, i32 11
  store float %exData3952vector_func.i, float addrspace(1)* %532, align 4
  br label %postload5793vector_func.i

postload5793vector_func.i:                        ; preds = %preload5792vector_func.i, %postload5790vector_func.i
  %exmask3954vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 12
  br i1 %exmask3954vector_func.i, label %preload5795vector_func.i, label %postload5796vector_func.i

preload5795vector_func.i:                         ; preds = %postload5793vector_func.i
  %.sum8860vector_func.i = add i64 %dim_0_vector_tid.i, 12
  %533 = getelementptr float addrspace(1)* %4, i64 %.sum8860vector_func.i
  %exData3955vector_func.i = extractelement <16 x float> %call.i33.i, i32 12
  store float %exData3955vector_func.i, float addrspace(1)* %533, align 4
  br label %postload5796vector_func.i

postload5796vector_func.i:                        ; preds = %preload5795vector_func.i, %postload5793vector_func.i
  %exmask3957vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 13
  br i1 %exmask3957vector_func.i, label %preload6281vector_func.i, label %postload6282vector_func.i

preload6281vector_func.i:                         ; preds = %postload5796vector_func.i
  %.sum8859vector_func.i = add i64 %dim_0_vector_tid.i, 13
  %534 = getelementptr float addrspace(1)* %4, i64 %.sum8859vector_func.i
  %exData3958vector_func.i = extractelement <16 x float> %call.i33.i, i32 13
  store float %exData3958vector_func.i, float addrspace(1)* %534, align 4
  br label %postload6282vector_func.i

postload6282vector_func.i:                        ; preds = %preload6281vector_func.i, %postload5796vector_func.i
  %exmask3960vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 14
  br i1 %exmask3960vector_func.i, label %preload6284vector_func.i, label %postload6285vector_func.i

preload6284vector_func.i:                         ; preds = %postload6282vector_func.i
  %.sum8858vector_func.i = add i64 %dim_0_vector_tid.i, 14
  %535 = getelementptr float addrspace(1)* %4, i64 %.sum8858vector_func.i
  %exData3961vector_func.i = extractelement <16 x float> %call.i33.i, i32 14
  store float %exData3961vector_func.i, float addrspace(1)* %535, align 4
  br label %postload6285vector_func.i

postload6285vector_func.i:                        ; preds = %preload6284vector_func.i, %postload6282vector_func.i
  %exmask3963vector_func.i = extractelement <16 x i1> %Mneg570vector_func.i, i32 15
  br i1 %exmask3963vector_func.i, label %preload6287vector_func.i, label %postload6288vector_func.i

preload6287vector_func.i:                         ; preds = %postload6285vector_func.i
  %.sum8857vector_func.i = add i64 %dim_0_vector_tid.i, 15
  %536 = getelementptr float addrspace(1)* %4, i64 %.sum8857vector_func.i
  %exData3964vector_func.i = extractelement <16 x float> %call.i33.i, i32 15
  store float %exData3964vector_func.i, float addrspace(1)* %536, align 4
  br label %postload6288vector_func.i

postload6288vector_func.i:                        ; preds = %preload6287vector_func.i, %postload6285vector_func.i
  %sub3221501vector_func.i = fsub <16 x float> <float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000, float 0xBFDC9673A0000000>, %mul11587vector_func.i
  %add3241503vector_func.i = fadd <16 x float> %sub3221501vector_func.i, %mul13589vector_func.i
  %mul.i2041504vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000, float 0xBB4C09FB40000000>
  %add.i2051505vector_func.i = fadd <16 x float> %mul.i2041504vector_func.i, <float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000, float 0x3C0C4B8820000000>
  %mul1.i2061506vector_func.i = fmul <16 x float> %add.i2051505vector_func.i, %mul567vector_func.i
  %add2.i2071507vector_func.i = fadd <16 x float> %mul1.i2061506vector_func.i, <float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000, float 0xBCB7F85EA0000000>
  %mul3.i2081508vector_func.i = fmul <16 x float> %add2.i2071507vector_func.i, %mul567vector_func.i
  %add4.i2091509vector_func.i = fadd <16 x float> %mul3.i2081508vector_func.i, <float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000, float 0x3D58D112C0000000>
  %mul5.i2101510vector_func.i = fmul <16 x float> %add4.i2091509vector_func.i, %mul567vector_func.i
  %add3261511vector_func.i = fadd <16 x float> %add3241503vector_func.i, %mul5.i2101510vector_func.i
  %call.i34.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3261511vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6290vector_func.i, label %postload6291vector_func.i

preload6290vector_func.i:                         ; preds = %postload6288vector_func.i
  %extract1513vector_func.i = add i64 %dim_0_vector_tid.i, 13824
  %537 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1513vector_func.i
  %exData3968vector_func.i = extractelement <16 x float> %call.i34.i, i32 0
  store float %exData3968vector_func.i, float addrspace(1)* %537, align 4
  br label %postload6291vector_func.i

postload6291vector_func.i:                        ; preds = %preload6290vector_func.i, %postload6288vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6293vector_func.i, label %postload6294vector_func.i

preload6293vector_func.i:                         ; preds = %postload6291vector_func.i
  %.sum8856vector_func.i = add i64 %dim_0_vector_tid.i, 13825
  %538 = getelementptr float addrspace(1)* %4, i64 %.sum8856vector_func.i
  %exData3971vector_func.i = extractelement <16 x float> %call.i34.i, i32 1
  store float %exData3971vector_func.i, float addrspace(1)* %538, align 4
  br label %postload6294vector_func.i

postload6294vector_func.i:                        ; preds = %preload6293vector_func.i, %postload6291vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6848vector_func.i, label %postload6849vector_func.i

preload6848vector_func.i:                         ; preds = %postload6294vector_func.i
  %.sum8855vector_func.i = add i64 %dim_0_vector_tid.i, 13826
  %539 = getelementptr float addrspace(1)* %4, i64 %.sum8855vector_func.i
  %exData3974vector_func.i = extractelement <16 x float> %call.i34.i, i32 2
  store float %exData3974vector_func.i, float addrspace(1)* %539, align 4
  br label %postload6849vector_func.i

postload6849vector_func.i:                        ; preds = %preload6848vector_func.i, %postload6294vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6851vector_func.i, label %postload6852vector_func.i

preload6851vector_func.i:                         ; preds = %postload6849vector_func.i
  %.sum8854vector_func.i = add i64 %dim_0_vector_tid.i, 13827
  %540 = getelementptr float addrspace(1)* %4, i64 %.sum8854vector_func.i
  %exData3977vector_func.i = extractelement <16 x float> %call.i34.i, i32 3
  store float %exData3977vector_func.i, float addrspace(1)* %540, align 4
  br label %postload6852vector_func.i

postload6852vector_func.i:                        ; preds = %preload6851vector_func.i, %postload6849vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6854vector_func.i, label %postload6855vector_func.i

preload6854vector_func.i:                         ; preds = %postload6852vector_func.i
  %.sum8853vector_func.i = add i64 %dim_0_vector_tid.i, 13828
  %541 = getelementptr float addrspace(1)* %4, i64 %.sum8853vector_func.i
  %exData3980vector_func.i = extractelement <16 x float> %call.i34.i, i32 4
  store float %exData3980vector_func.i, float addrspace(1)* %541, align 4
  br label %postload6855vector_func.i

postload6855vector_func.i:                        ; preds = %preload6854vector_func.i, %postload6852vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6857vector_func.i, label %postload6858vector_func.i

preload6857vector_func.i:                         ; preds = %postload6855vector_func.i
  %.sum8852vector_func.i = add i64 %dim_0_vector_tid.i, 13829
  %542 = getelementptr float addrspace(1)* %4, i64 %.sum8852vector_func.i
  %exData3983vector_func.i = extractelement <16 x float> %call.i34.i, i32 5
  store float %exData3983vector_func.i, float addrspace(1)* %542, align 4
  br label %postload6858vector_func.i

postload6858vector_func.i:                        ; preds = %preload6857vector_func.i, %postload6855vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6860vector_func.i, label %postload6861vector_func.i

preload6860vector_func.i:                         ; preds = %postload6858vector_func.i
  %.sum8851vector_func.i = add i64 %dim_0_vector_tid.i, 13830
  %543 = getelementptr float addrspace(1)* %4, i64 %.sum8851vector_func.i
  %exData3986vector_func.i = extractelement <16 x float> %call.i34.i, i32 6
  store float %exData3986vector_func.i, float addrspace(1)* %543, align 4
  br label %postload6861vector_func.i

postload6861vector_func.i:                        ; preds = %preload6860vector_func.i, %postload6858vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7295vector_func.i, label %postload7296vector_func.i

preload7295vector_func.i:                         ; preds = %postload6861vector_func.i
  %.sum8850vector_func.i = add i64 %dim_0_vector_tid.i, 13831
  %544 = getelementptr float addrspace(1)* %4, i64 %.sum8850vector_func.i
  %exData3989vector_func.i = extractelement <16 x float> %call.i34.i, i32 7
  store float %exData3989vector_func.i, float addrspace(1)* %544, align 4
  br label %postload7296vector_func.i

postload7296vector_func.i:                        ; preds = %preload7295vector_func.i, %postload6861vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7298vector_func.i, label %postload7299vector_func.i

preload7298vector_func.i:                         ; preds = %postload7296vector_func.i
  %.sum8849vector_func.i = add i64 %dim_0_vector_tid.i, 13832
  %545 = getelementptr float addrspace(1)* %4, i64 %.sum8849vector_func.i
  %exData3992vector_func.i = extractelement <16 x float> %call.i34.i, i32 8
  store float %exData3992vector_func.i, float addrspace(1)* %545, align 4
  br label %postload7299vector_func.i

postload7299vector_func.i:                        ; preds = %preload7298vector_func.i, %postload7296vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7301vector_func.i, label %postload7302vector_func.i

preload7301vector_func.i:                         ; preds = %postload7299vector_func.i
  %.sum8848vector_func.i = add i64 %dim_0_vector_tid.i, 13833
  %546 = getelementptr float addrspace(1)* %4, i64 %.sum8848vector_func.i
  %exData3995vector_func.i = extractelement <16 x float> %call.i34.i, i32 9
  store float %exData3995vector_func.i, float addrspace(1)* %546, align 4
  br label %postload7302vector_func.i

postload7302vector_func.i:                        ; preds = %preload7301vector_func.i, %postload7299vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7304vector_func.i, label %postload7305vector_func.i

preload7304vector_func.i:                         ; preds = %postload7302vector_func.i
  %.sum8847vector_func.i = add i64 %dim_0_vector_tid.i, 13834
  %547 = getelementptr float addrspace(1)* %4, i64 %.sum8847vector_func.i
  %exData3998vector_func.i = extractelement <16 x float> %call.i34.i, i32 10
  store float %exData3998vector_func.i, float addrspace(1)* %547, align 4
  br label %postload7305vector_func.i

postload7305vector_func.i:                        ; preds = %preload7304vector_func.i, %postload7302vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7307vector_func.i, label %postload7308vector_func.i

preload7307vector_func.i:                         ; preds = %postload7305vector_func.i
  %.sum8846vector_func.i = add i64 %dim_0_vector_tid.i, 13835
  %548 = getelementptr float addrspace(1)* %4, i64 %.sum8846vector_func.i
  %exData4001vector_func.i = extractelement <16 x float> %call.i34.i, i32 11
  store float %exData4001vector_func.i, float addrspace(1)* %548, align 4
  br label %postload7308vector_func.i

postload7308vector_func.i:                        ; preds = %preload7307vector_func.i, %postload7305vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7310vector_func.i, label %postload7311vector_func.i

preload7310vector_func.i:                         ; preds = %postload7308vector_func.i
  %.sum8845vector_func.i = add i64 %dim_0_vector_tid.i, 13836
  %549 = getelementptr float addrspace(1)* %4, i64 %.sum8845vector_func.i
  %exData4004vector_func.i = extractelement <16 x float> %call.i34.i, i32 12
  store float %exData4004vector_func.i, float addrspace(1)* %549, align 4
  br label %postload7311vector_func.i

postload7311vector_func.i:                        ; preds = %preload7310vector_func.i, %postload7308vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6251vector_func.i, label %postload6252vector_func.i

preload6251vector_func.i:                         ; preds = %postload7311vector_func.i
  %.sum8844vector_func.i = add i64 %dim_0_vector_tid.i, 13837
  %550 = getelementptr float addrspace(1)* %4, i64 %.sum8844vector_func.i
  %exData4007vector_func.i = extractelement <16 x float> %call.i34.i, i32 13
  store float %exData4007vector_func.i, float addrspace(1)* %550, align 4
  br label %postload6252vector_func.i

postload6252vector_func.i:                        ; preds = %preload6251vector_func.i, %postload7311vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6254vector_func.i, label %postload6255vector_func.i

preload6254vector_func.i:                         ; preds = %postload6252vector_func.i
  %.sum8843vector_func.i = add i64 %dim_0_vector_tid.i, 13838
  %551 = getelementptr float addrspace(1)* %4, i64 %.sum8843vector_func.i
  %exData4010vector_func.i = extractelement <16 x float> %call.i34.i, i32 14
  store float %exData4010vector_func.i, float addrspace(1)* %551, align 4
  br label %postload6255vector_func.i

postload6255vector_func.i:                        ; preds = %preload6254vector_func.i, %postload6252vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6257vector_func.i, label %postload6258vector_func.i

preload6257vector_func.i:                         ; preds = %postload6255vector_func.i
  %.sum8842vector_func.i = add i64 %dim_0_vector_tid.i, 13839
  %552 = getelementptr float addrspace(1)* %4, i64 %.sum8842vector_func.i
  %exData4013vector_func.i = extractelement <16 x float> %call.i34.i, i32 15
  store float %exData4013vector_func.i, float addrspace(1)* %552, align 4
  br label %postload6258vector_func.i

postload6258vector_func.i:                        ; preds = %preload6257vector_func.i, %postload6255vector_func.i
  %mul3311530vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000, float 0x40DC7090A0000000>
  %sub3321531vector_func.i = fsub <16 x float> <float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000, float 0x40006A5C20000000>, %mul3311530vector_func.i
  %mul3331532vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000, float 0x4009589C60000000>
  %add3341533vector_func.i = fadd <16 x float> %sub3321531vector_func.i, %mul3331532vector_func.i
  %mul.i1971534vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000, float 0x3D3DBBA8A0000000>
  %add.i1981535vector_func.i = fadd <16 x float> %mul.i1971534vector_func.i, <float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000, float 0xBE018BEB80000000>
  %mul1.i1991536vector_func.i = fmul <16 x float> %add.i1981535vector_func.i, %mul567vector_func.i
  %add2.i2001537vector_func.i = fadd <16 x float> %mul1.i1991536vector_func.i, <float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000, float 0x3EB2934A60000000>
  %mul3.i2011538vector_func.i = fmul <16 x float> %add2.i2001537vector_func.i, %mul567vector_func.i
  %add4.i2021539vector_func.i = fadd <16 x float> %mul3.i2011538vector_func.i, <float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000, float 0xBF5ADD3AE0000000>
  %mul5.i2031540vector_func.i = fmul <16 x float> %add4.i2021539vector_func.i, %mul567vector_func.i
  %add3361541vector_func.i = fadd <16 x float> %add3341533vector_func.i, %mul5.i2031540vector_func.i
  %call.i35.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3361541vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6260vector_func.i, label %postload6261vector_func.i

preload6260vector_func.i:                         ; preds = %postload6258vector_func.i
  %extract1543vector_func.i = add i64 %dim_0_vector_tid.i, 27648
  %553 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1543vector_func.i
  %exData4017vector_func.i = extractelement <16 x float> %call.i35.i, i32 0
  store float %exData4017vector_func.i, float addrspace(1)* %553, align 4
  br label %postload6261vector_func.i

postload6261vector_func.i:                        ; preds = %preload6260vector_func.i, %postload6258vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6263vector_func.i, label %postload6264vector_func.i

preload6263vector_func.i:                         ; preds = %postload6261vector_func.i
  %.sum8841vector_func.i = add i64 %dim_0_vector_tid.i, 27649
  %554 = getelementptr float addrspace(1)* %4, i64 %.sum8841vector_func.i
  %exData4020vector_func.i = extractelement <16 x float> %call.i35.i, i32 1
  store float %exData4020vector_func.i, float addrspace(1)* %554, align 4
  br label %postload6264vector_func.i

postload6264vector_func.i:                        ; preds = %preload6263vector_func.i, %postload6261vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6266vector_func.i, label %postload6267vector_func.i

preload6266vector_func.i:                         ; preds = %postload6264vector_func.i
  %.sum8840vector_func.i = add i64 %dim_0_vector_tid.i, 27650
  %555 = getelementptr float addrspace(1)* %4, i64 %.sum8840vector_func.i
  %exData4023vector_func.i = extractelement <16 x float> %call.i35.i, i32 2
  store float %exData4023vector_func.i, float addrspace(1)* %555, align 4
  br label %postload6267vector_func.i

postload6267vector_func.i:                        ; preds = %preload6266vector_func.i, %postload6264vector_func.i
  br i1 %exmask3927vector_func.i, label %preload5990vector_func.i, label %postload5991vector_func.i

preload5990vector_func.i:                         ; preds = %postload6267vector_func.i
  %.sum8839vector_func.i = add i64 %dim_0_vector_tid.i, 27651
  %556 = getelementptr float addrspace(1)* %4, i64 %.sum8839vector_func.i
  %exData4026vector_func.i = extractelement <16 x float> %call.i35.i, i32 3
  store float %exData4026vector_func.i, float addrspace(1)* %556, align 4
  br label %postload5991vector_func.i

postload5991vector_func.i:                        ; preds = %preload5990vector_func.i, %postload6267vector_func.i
  br i1 %exmask3930vector_func.i, label %preload5993vector_func.i, label %postload5994vector_func.i

preload5993vector_func.i:                         ; preds = %postload5991vector_func.i
  %.sum8838vector_func.i = add i64 %dim_0_vector_tid.i, 27652
  %557 = getelementptr float addrspace(1)* %4, i64 %.sum8838vector_func.i
  %exData4029vector_func.i = extractelement <16 x float> %call.i35.i, i32 4
  store float %exData4029vector_func.i, float addrspace(1)* %557, align 4
  br label %postload5994vector_func.i

postload5994vector_func.i:                        ; preds = %preload5993vector_func.i, %postload5991vector_func.i
  br i1 %exmask3933vector_func.i, label %preload5996vector_func.i, label %postload5997vector_func.i

preload5996vector_func.i:                         ; preds = %postload5994vector_func.i
  %.sum8837vector_func.i = add i64 %dim_0_vector_tid.i, 27653
  %558 = getelementptr float addrspace(1)* %4, i64 %.sum8837vector_func.i
  %exData4032vector_func.i = extractelement <16 x float> %call.i35.i, i32 5
  store float %exData4032vector_func.i, float addrspace(1)* %558, align 4
  br label %postload5997vector_func.i

postload5997vector_func.i:                        ; preds = %preload5996vector_func.i, %postload5994vector_func.i
  br i1 %exmask3936vector_func.i, label %preload5999vector_func.i, label %postload6000vector_func.i

preload5999vector_func.i:                         ; preds = %postload5997vector_func.i
  %.sum8836vector_func.i = add i64 %dim_0_vector_tid.i, 27654
  %559 = getelementptr float addrspace(1)* %4, i64 %.sum8836vector_func.i
  %exData4035vector_func.i = extractelement <16 x float> %call.i35.i, i32 6
  store float %exData4035vector_func.i, float addrspace(1)* %559, align 4
  br label %postload6000vector_func.i

postload6000vector_func.i:                        ; preds = %preload5999vector_func.i, %postload5997vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6002vector_func.i, label %postload6003vector_func.i

preload6002vector_func.i:                         ; preds = %postload6000vector_func.i
  %.sum8835vector_func.i = add i64 %dim_0_vector_tid.i, 27655
  %560 = getelementptr float addrspace(1)* %4, i64 %.sum8835vector_func.i
  %exData4038vector_func.i = extractelement <16 x float> %call.i35.i, i32 7
  store float %exData4038vector_func.i, float addrspace(1)* %560, align 4
  br label %postload6003vector_func.i

postload6003vector_func.i:                        ; preds = %preload6002vector_func.i, %postload6000vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6005vector_func.i, label %postload6006vector_func.i

preload6005vector_func.i:                         ; preds = %postload6003vector_func.i
  %.sum8834vector_func.i = add i64 %dim_0_vector_tid.i, 27656
  %561 = getelementptr float addrspace(1)* %4, i64 %.sum8834vector_func.i
  %exData4041vector_func.i = extractelement <16 x float> %call.i35.i, i32 8
  store float %exData4041vector_func.i, float addrspace(1)* %561, align 4
  br label %postload6006vector_func.i

postload6006vector_func.i:                        ; preds = %preload6005vector_func.i, %postload6003vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6008vector_func.i, label %postload6009vector_func.i

preload6008vector_func.i:                         ; preds = %postload6006vector_func.i
  %.sum8833vector_func.i = add i64 %dim_0_vector_tid.i, 27657
  %562 = getelementptr float addrspace(1)* %4, i64 %.sum8833vector_func.i
  %exData4044vector_func.i = extractelement <16 x float> %call.i35.i, i32 9
  store float %exData4044vector_func.i, float addrspace(1)* %562, align 4
  br label %postload6009vector_func.i

postload6009vector_func.i:                        ; preds = %preload6008vector_func.i, %postload6006vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6011vector_func.i, label %postload6012vector_func.i

preload6011vector_func.i:                         ; preds = %postload6009vector_func.i
  %.sum8832vector_func.i = add i64 %dim_0_vector_tid.i, 27658
  %563 = getelementptr float addrspace(1)* %4, i64 %.sum8832vector_func.i
  %exData4047vector_func.i = extractelement <16 x float> %call.i35.i, i32 10
  store float %exData4047vector_func.i, float addrspace(1)* %563, align 4
  br label %postload6012vector_func.i

postload6012vector_func.i:                        ; preds = %preload6011vector_func.i, %postload6009vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6014vector_func.i, label %postload6015vector_func.i

preload6014vector_func.i:                         ; preds = %postload6012vector_func.i
  %.sum8831vector_func.i = add i64 %dim_0_vector_tid.i, 27659
  %564 = getelementptr float addrspace(1)* %4, i64 %.sum8831vector_func.i
  %exData4050vector_func.i = extractelement <16 x float> %call.i35.i, i32 11
  store float %exData4050vector_func.i, float addrspace(1)* %564, align 4
  br label %postload6015vector_func.i

postload6015vector_func.i:                        ; preds = %preload6014vector_func.i, %postload6012vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6017vector_func.i, label %postload6018vector_func.i

preload6017vector_func.i:                         ; preds = %postload6015vector_func.i
  %.sum8830vector_func.i = add i64 %dim_0_vector_tid.i, 27660
  %565 = getelementptr float addrspace(1)* %4, i64 %.sum8830vector_func.i
  %exData4053vector_func.i = extractelement <16 x float> %call.i35.i, i32 12
  store float %exData4053vector_func.i, float addrspace(1)* %565, align 4
  br label %postload6018vector_func.i

postload6018vector_func.i:                        ; preds = %preload6017vector_func.i, %postload6015vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6020vector_func.i, label %postload6021vector_func.i

preload6020vector_func.i:                         ; preds = %postload6018vector_func.i
  %.sum8829vector_func.i = add i64 %dim_0_vector_tid.i, 27661
  %566 = getelementptr float addrspace(1)* %4, i64 %.sum8829vector_func.i
  %exData4056vector_func.i = extractelement <16 x float> %call.i35.i, i32 13
  store float %exData4056vector_func.i, float addrspace(1)* %566, align 4
  br label %postload6021vector_func.i

postload6021vector_func.i:                        ; preds = %preload6020vector_func.i, %postload6018vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6023vector_func.i, label %postload6024vector_func.i

preload6023vector_func.i:                         ; preds = %postload6021vector_func.i
  %.sum8828vector_func.i = add i64 %dim_0_vector_tid.i, 27662
  %567 = getelementptr float addrspace(1)* %4, i64 %.sum8828vector_func.i
  %exData4059vector_func.i = extractelement <16 x float> %call.i35.i, i32 14
  store float %exData4059vector_func.i, float addrspace(1)* %567, align 4
  br label %postload6024vector_func.i

postload6024vector_func.i:                        ; preds = %preload6023vector_func.i, %postload6021vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6026vector_func.i, label %postload6027vector_func.i

preload6026vector_func.i:                         ; preds = %postload6024vector_func.i
  %.sum8827vector_func.i = add i64 %dim_0_vector_tid.i, 27663
  %568 = getelementptr float addrspace(1)* %4, i64 %.sum8827vector_func.i
  %exData4062vector_func.i = extractelement <16 x float> %call.i35.i, i32 15
  store float %exData4062vector_func.i, float addrspace(1)* %568, align 4
  br label %postload6027vector_func.i

postload6027vector_func.i:                        ; preds = %preload6026vector_func.i, %postload6024vector_func.i
  %mul3411560vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000, float 0x40909FC640000000>
  %add3421561vector_func.i = fadd <16 x float> %mul3411560vector_func.i, <float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000, float 0x400D42EB80000000>
  %mul3431562vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000, float 0x400E427880000000>
  %add3441563vector_func.i = fadd <16 x float> %add3421561vector_func.i, %mul3431562vector_func.i
  %mul.i1901564vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000, float 0x3D46D361A0000000>
  %add.i1911565vector_func.i = fadd <16 x float> %mul.i1901564vector_func.i, <float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000, float 0xBE0BB876E0000000>
  %mul1.i1921566vector_func.i = fmul <16 x float> %add.i1911565vector_func.i, %mul567vector_func.i
  %add2.i1931567vector_func.i = fadd <16 x float> %mul1.i1921566vector_func.i, <float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000, float 0x3EBB88F920000000>
  %mul3.i1941568vector_func.i = fmul <16 x float> %add2.i1931567vector_func.i, %mul567vector_func.i
  %add4.i1951569vector_func.i = fadd <16 x float> %mul3.i1941568vector_func.i, <float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000, float 0xBF588C9B60000000>
  %mul5.i1961570vector_func.i = fmul <16 x float> %add4.i1951569vector_func.i, %mul567vector_func.i
  %add3461571vector_func.i = fadd <16 x float> %add3441563vector_func.i, %mul5.i1961570vector_func.i
  %call.i36.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3461571vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6029vector_func.i, label %postload6030vector_func.i

preload6029vector_func.i:                         ; preds = %postload6027vector_func.i
  %extract1573vector_func.i = add i64 %dim_0_vector_tid.i, 41472
  %569 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1573vector_func.i
  %exData4066vector_func.i = extractelement <16 x float> %call.i36.i, i32 0
  store float %exData4066vector_func.i, float addrspace(1)* %569, align 4
  br label %postload6030vector_func.i

postload6030vector_func.i:                        ; preds = %preload6029vector_func.i, %postload6027vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6032vector_func.i, label %postload6033vector_func.i

preload6032vector_func.i:                         ; preds = %postload6030vector_func.i
  %.sum8826vector_func.i = add i64 %dim_0_vector_tid.i, 41473
  %570 = getelementptr float addrspace(1)* %4, i64 %.sum8826vector_func.i
  %exData4069vector_func.i = extractelement <16 x float> %call.i36.i, i32 1
  store float %exData4069vector_func.i, float addrspace(1)* %570, align 4
  br label %postload6033vector_func.i

postload6033vector_func.i:                        ; preds = %preload6032vector_func.i, %postload6030vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6035vector_func.i, label %postload6036vector_func.i

preload6035vector_func.i:                         ; preds = %postload6033vector_func.i
  %.sum8825vector_func.i = add i64 %dim_0_vector_tid.i, 41474
  %571 = getelementptr float addrspace(1)* %4, i64 %.sum8825vector_func.i
  %exData4072vector_func.i = extractelement <16 x float> %call.i36.i, i32 2
  store float %exData4072vector_func.i, float addrspace(1)* %571, align 4
  br label %postload6036vector_func.i

postload6036vector_func.i:                        ; preds = %preload6035vector_func.i, %postload6033vector_func.i
  br i1 %exmask3927vector_func.i, label %preload5639vector_func.i, label %postload5640vector_func.i

preload5639vector_func.i:                         ; preds = %postload6036vector_func.i
  %.sum8824vector_func.i = add i64 %dim_0_vector_tid.i, 41475
  %572 = getelementptr float addrspace(1)* %4, i64 %.sum8824vector_func.i
  %exData4075vector_func.i = extractelement <16 x float> %call.i36.i, i32 3
  store float %exData4075vector_func.i, float addrspace(1)* %572, align 4
  br label %postload5640vector_func.i

postload5640vector_func.i:                        ; preds = %preload5639vector_func.i, %postload6036vector_func.i
  br i1 %exmask3930vector_func.i, label %preload5642vector_func.i, label %postload5643vector_func.i

preload5642vector_func.i:                         ; preds = %postload5640vector_func.i
  %.sum8823vector_func.i = add i64 %dim_0_vector_tid.i, 41476
  %573 = getelementptr float addrspace(1)* %4, i64 %.sum8823vector_func.i
  %exData4078vector_func.i = extractelement <16 x float> %call.i36.i, i32 4
  store float %exData4078vector_func.i, float addrspace(1)* %573, align 4
  br label %postload5643vector_func.i

postload5643vector_func.i:                        ; preds = %preload5642vector_func.i, %postload5640vector_func.i
  br i1 %exmask3933vector_func.i, label %preload5645vector_func.i, label %postload5646vector_func.i

preload5645vector_func.i:                         ; preds = %postload5643vector_func.i
  %.sum8822vector_func.i = add i64 %dim_0_vector_tid.i, 41477
  %574 = getelementptr float addrspace(1)* %4, i64 %.sum8822vector_func.i
  %exData4081vector_func.i = extractelement <16 x float> %call.i36.i, i32 5
  store float %exData4081vector_func.i, float addrspace(1)* %574, align 4
  br label %postload5646vector_func.i

postload5646vector_func.i:                        ; preds = %preload5645vector_func.i, %postload5643vector_func.i
  br i1 %exmask3936vector_func.i, label %preload5648vector_func.i, label %postload5649vector_func.i

preload5648vector_func.i:                         ; preds = %postload5646vector_func.i
  %.sum8821vector_func.i = add i64 %dim_0_vector_tid.i, 41478
  %575 = getelementptr float addrspace(1)* %4, i64 %.sum8821vector_func.i
  %exData4084vector_func.i = extractelement <16 x float> %call.i36.i, i32 6
  store float %exData4084vector_func.i, float addrspace(1)* %575, align 4
  br label %postload5649vector_func.i

postload5649vector_func.i:                        ; preds = %preload5648vector_func.i, %postload5646vector_func.i
  br i1 %exmask3939vector_func.i, label %preload5651vector_func.i, label %postload5652vector_func.i

preload5651vector_func.i:                         ; preds = %postload5649vector_func.i
  %.sum8820vector_func.i = add i64 %dim_0_vector_tid.i, 41479
  %576 = getelementptr float addrspace(1)* %4, i64 %.sum8820vector_func.i
  %exData4087vector_func.i = extractelement <16 x float> %call.i36.i, i32 7
  store float %exData4087vector_func.i, float addrspace(1)* %576, align 4
  br label %postload5652vector_func.i

postload5652vector_func.i:                        ; preds = %preload5651vector_func.i, %postload5649vector_func.i
  br i1 %exmask3942vector_func.i, label %preload5654vector_func.i, label %postload5655vector_func.i

preload5654vector_func.i:                         ; preds = %postload5652vector_func.i
  %.sum8819vector_func.i = add i64 %dim_0_vector_tid.i, 41480
  %577 = getelementptr float addrspace(1)* %4, i64 %.sum8819vector_func.i
  %exData4090vector_func.i = extractelement <16 x float> %call.i36.i, i32 8
  store float %exData4090vector_func.i, float addrspace(1)* %577, align 4
  br label %postload5655vector_func.i

postload5655vector_func.i:                        ; preds = %preload5654vector_func.i, %postload5652vector_func.i
  br i1 %exmask3945vector_func.i, label %preload5657vector_func.i, label %postload5658vector_func.i

preload5657vector_func.i:                         ; preds = %postload5655vector_func.i
  %.sum8818vector_func.i = add i64 %dim_0_vector_tid.i, 41481
  %578 = getelementptr float addrspace(1)* %4, i64 %.sum8818vector_func.i
  %exData4093vector_func.i = extractelement <16 x float> %call.i36.i, i32 9
  store float %exData4093vector_func.i, float addrspace(1)* %578, align 4
  br label %postload5658vector_func.i

postload5658vector_func.i:                        ; preds = %preload5657vector_func.i, %postload5655vector_func.i
  br i1 %exmask3948vector_func.i, label %preload5660vector_func.i, label %postload5661vector_func.i

preload5660vector_func.i:                         ; preds = %postload5658vector_func.i
  %.sum8817vector_func.i = add i64 %dim_0_vector_tid.i, 41482
  %579 = getelementptr float addrspace(1)* %4, i64 %.sum8817vector_func.i
  %exData4096vector_func.i = extractelement <16 x float> %call.i36.i, i32 10
  store float %exData4096vector_func.i, float addrspace(1)* %579, align 4
  br label %postload5661vector_func.i

postload5661vector_func.i:                        ; preds = %preload5660vector_func.i, %postload5658vector_func.i
  br i1 %exmask3951vector_func.i, label %preload5663vector_func.i, label %postload5664vector_func.i

preload5663vector_func.i:                         ; preds = %postload5661vector_func.i
  %.sum8816vector_func.i = add i64 %dim_0_vector_tid.i, 41483
  %580 = getelementptr float addrspace(1)* %4, i64 %.sum8816vector_func.i
  %exData4099vector_func.i = extractelement <16 x float> %call.i36.i, i32 11
  store float %exData4099vector_func.i, float addrspace(1)* %580, align 4
  br label %postload5664vector_func.i

postload5664vector_func.i:                        ; preds = %preload5663vector_func.i, %postload5661vector_func.i
  br i1 %exmask3954vector_func.i, label %preload5666vector_func.i, label %postload5667vector_func.i

preload5666vector_func.i:                         ; preds = %postload5664vector_func.i
  %.sum8815vector_func.i = add i64 %dim_0_vector_tid.i, 41484
  %581 = getelementptr float addrspace(1)* %4, i64 %.sum8815vector_func.i
  %exData4102vector_func.i = extractelement <16 x float> %call.i36.i, i32 12
  store float %exData4102vector_func.i, float addrspace(1)* %581, align 4
  br label %postload5667vector_func.i

postload5667vector_func.i:                        ; preds = %preload5666vector_func.i, %postload5664vector_func.i
  br i1 %exmask3957vector_func.i, label %preload5669vector_func.i, label %postload5670vector_func.i

preload5669vector_func.i:                         ; preds = %postload5667vector_func.i
  %.sum8814vector_func.i = add i64 %dim_0_vector_tid.i, 41485
  %582 = getelementptr float addrspace(1)* %4, i64 %.sum8814vector_func.i
  %exData4105vector_func.i = extractelement <16 x float> %call.i36.i, i32 13
  store float %exData4105vector_func.i, float addrspace(1)* %582, align 4
  br label %postload5670vector_func.i

postload5670vector_func.i:                        ; preds = %preload5669vector_func.i, %postload5667vector_func.i
  br i1 %exmask3960vector_func.i, label %preload5672vector_func.i, label %postload5673vector_func.i

preload5672vector_func.i:                         ; preds = %postload5670vector_func.i
  %.sum8813vector_func.i = add i64 %dim_0_vector_tid.i, 41486
  %583 = getelementptr float addrspace(1)* %4, i64 %.sum8813vector_func.i
  %exData4108vector_func.i = extractelement <16 x float> %call.i36.i, i32 14
  store float %exData4108vector_func.i, float addrspace(1)* %583, align 4
  br label %postload5673vector_func.i

postload5673vector_func.i:                        ; preds = %preload5672vector_func.i, %postload5670vector_func.i
  br i1 %exmask3963vector_func.i, label %preload5675vector_func.i, label %postload5676vector_func.i

preload5675vector_func.i:                         ; preds = %postload5673vector_func.i
  %.sum8812vector_func.i = add i64 %dim_0_vector_tid.i, 41487
  %584 = getelementptr float addrspace(1)* %4, i64 %.sum8812vector_func.i
  %exData4111vector_func.i = extractelement <16 x float> %call.i36.i, i32 15
  store float %exData4111vector_func.i, float addrspace(1)* %584, align 4
  br label %postload5676vector_func.i

postload5676vector_func.i:                        ; preds = %preload5675vector_func.i, %postload5673vector_func.i
  %mul3511590vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000, float 0x40AC3E2940000000>
  %sub3521591vector_func.i = fsub <16 x float> <float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000, float 0xBFBA9ADBE0000000>, %mul3511590vector_func.i
  %mul3531592vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000, float 0x400FEFA5C0000000>
  %add3541593vector_func.i = fadd <16 x float> %sub3521591vector_func.i, %mul3531592vector_func.i
  %mul.i1831594vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000, float 0x3D3332BDC0000000>
  %add.i1841595vector_func.i = fadd <16 x float> %mul.i1831594vector_func.i, <float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000, float 0xBDF639CD40000000>
  %mul1.i1851596vector_func.i = fmul <16 x float> %add.i1841595vector_func.i, %mul567vector_func.i
  %add2.i1861597vector_func.i = fadd <16 x float> %mul1.i1851596vector_func.i, <float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000, float 0x3EA9D34C60000000>
  %mul3.i1871598vector_func.i = fmul <16 x float> %add2.i1861597vector_func.i, %mul567vector_func.i
  %add4.i1881599vector_func.i = fadd <16 x float> %mul3.i1871598vector_func.i, <float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000, float 0xBF53ABED80000000>
  %mul5.i1891600vector_func.i = fmul <16 x float> %add4.i1881599vector_func.i, %mul567vector_func.i
  %add3561601vector_func.i = fadd <16 x float> %add3541593vector_func.i, %mul5.i1891600vector_func.i
  %call.i37.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3561601vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload5678vector_func.i, label %postload5679vector_func.i

preload5678vector_func.i:                         ; preds = %postload5676vector_func.i
  %extract1603vector_func.i = add i64 %dim_0_vector_tid.i, 55296
  %585 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1603vector_func.i
  %exData4115vector_func.i = extractelement <16 x float> %call.i37.i, i32 0
  store float %exData4115vector_func.i, float addrspace(1)* %585, align 4
  br label %postload5679vector_func.i

postload5679vector_func.i:                        ; preds = %preload5678vector_func.i, %postload5676vector_func.i
  br i1 %exmask3921vector_func.i, label %preload5681vector_func.i, label %postload5682vector_func.i

preload5681vector_func.i:                         ; preds = %postload5679vector_func.i
  %.sum8811vector_func.i = add i64 %dim_0_vector_tid.i, 55297
  %586 = getelementptr float addrspace(1)* %4, i64 %.sum8811vector_func.i
  %exData4118vector_func.i = extractelement <16 x float> %call.i37.i, i32 1
  store float %exData4118vector_func.i, float addrspace(1)* %586, align 4
  br label %postload5682vector_func.i

postload5682vector_func.i:                        ; preds = %preload5681vector_func.i, %postload5679vector_func.i
  br i1 %exmask3924vector_func.i, label %preload5684vector_func.i, label %postload5685vector_func.i

preload5684vector_func.i:                         ; preds = %postload5682vector_func.i
  %.sum8810vector_func.i = add i64 %dim_0_vector_tid.i, 55298
  %587 = getelementptr float addrspace(1)* %4, i64 %.sum8810vector_func.i
  %exData4121vector_func.i = extractelement <16 x float> %call.i37.i, i32 2
  store float %exData4121vector_func.i, float addrspace(1)* %587, align 4
  br label %postload5685vector_func.i

postload5685vector_func.i:                        ; preds = %preload5684vector_func.i, %postload5682vector_func.i
  br i1 %exmask3927vector_func.i, label %preload5687vector_func.i, label %postload5688vector_func.i

preload5687vector_func.i:                         ; preds = %postload5685vector_func.i
  %.sum8809vector_func.i = add i64 %dim_0_vector_tid.i, 55299
  %588 = getelementptr float addrspace(1)* %4, i64 %.sum8809vector_func.i
  %exData4124vector_func.i = extractelement <16 x float> %call.i37.i, i32 3
  store float %exData4124vector_func.i, float addrspace(1)* %588, align 4
  br label %postload5688vector_func.i

postload5688vector_func.i:                        ; preds = %preload5687vector_func.i, %postload5685vector_func.i
  br i1 %exmask3930vector_func.i, label %preload5690vector_func.i, label %postload5691vector_func.i

preload5690vector_func.i:                         ; preds = %postload5688vector_func.i
  %.sum8808vector_func.i = add i64 %dim_0_vector_tid.i, 55300
  %589 = getelementptr float addrspace(1)* %4, i64 %.sum8808vector_func.i
  %exData4127vector_func.i = extractelement <16 x float> %call.i37.i, i32 4
  store float %exData4127vector_func.i, float addrspace(1)* %589, align 4
  br label %postload5691vector_func.i

postload5691vector_func.i:                        ; preds = %preload5690vector_func.i, %postload5688vector_func.i
  br i1 %exmask3933vector_func.i, label %preload5693vector_func.i, label %postload5694vector_func.i

preload5693vector_func.i:                         ; preds = %postload5691vector_func.i
  %.sum8807vector_func.i = add i64 %dim_0_vector_tid.i, 55301
  %590 = getelementptr float addrspace(1)* %4, i64 %.sum8807vector_func.i
  %exData4130vector_func.i = extractelement <16 x float> %call.i37.i, i32 5
  store float %exData4130vector_func.i, float addrspace(1)* %590, align 4
  br label %postload5694vector_func.i

postload5694vector_func.i:                        ; preds = %preload5693vector_func.i, %postload5691vector_func.i
  br i1 %exmask3936vector_func.i, label %preload5696vector_func.i, label %postload5697vector_func.i

preload5696vector_func.i:                         ; preds = %postload5694vector_func.i
  %.sum8806vector_func.i = add i64 %dim_0_vector_tid.i, 55302
  %591 = getelementptr float addrspace(1)* %4, i64 %.sum8806vector_func.i
  %exData4133vector_func.i = extractelement <16 x float> %call.i37.i, i32 6
  store float %exData4133vector_func.i, float addrspace(1)* %591, align 4
  br label %postload5697vector_func.i

postload5697vector_func.i:                        ; preds = %preload5696vector_func.i, %postload5694vector_func.i
  br i1 %exmask3939vector_func.i, label %preload5699vector_func.i, label %postload5700vector_func.i

preload5699vector_func.i:                         ; preds = %postload5697vector_func.i
  %.sum8805vector_func.i = add i64 %dim_0_vector_tid.i, 55303
  %592 = getelementptr float addrspace(1)* %4, i64 %.sum8805vector_func.i
  %exData4136vector_func.i = extractelement <16 x float> %call.i37.i, i32 7
  store float %exData4136vector_func.i, float addrspace(1)* %592, align 4
  br label %postload5700vector_func.i

postload5700vector_func.i:                        ; preds = %preload5699vector_func.i, %postload5697vector_func.i
  br i1 %exmask3942vector_func.i, label %preload5852vector_func.i, label %postload5853vector_func.i

preload5852vector_func.i:                         ; preds = %postload5700vector_func.i
  %.sum8804vector_func.i = add i64 %dim_0_vector_tid.i, 55304
  %593 = getelementptr float addrspace(1)* %4, i64 %.sum8804vector_func.i
  %exData4139vector_func.i = extractelement <16 x float> %call.i37.i, i32 8
  store float %exData4139vector_func.i, float addrspace(1)* %593, align 4
  br label %postload5853vector_func.i

postload5853vector_func.i:                        ; preds = %preload5852vector_func.i, %postload5700vector_func.i
  br i1 %exmask3945vector_func.i, label %preload5855vector_func.i, label %postload5856vector_func.i

preload5855vector_func.i:                         ; preds = %postload5853vector_func.i
  %.sum8803vector_func.i = add i64 %dim_0_vector_tid.i, 55305
  %594 = getelementptr float addrspace(1)* %4, i64 %.sum8803vector_func.i
  %exData4142vector_func.i = extractelement <16 x float> %call.i37.i, i32 9
  store float %exData4142vector_func.i, float addrspace(1)* %594, align 4
  br label %postload5856vector_func.i

postload5856vector_func.i:                        ; preds = %preload5855vector_func.i, %postload5853vector_func.i
  br i1 %exmask3948vector_func.i, label %preload5858vector_func.i, label %postload5859vector_func.i

preload5858vector_func.i:                         ; preds = %postload5856vector_func.i
  %.sum8802vector_func.i = add i64 %dim_0_vector_tid.i, 55306
  %595 = getelementptr float addrspace(1)* %4, i64 %.sum8802vector_func.i
  %exData4145vector_func.i = extractelement <16 x float> %call.i37.i, i32 10
  store float %exData4145vector_func.i, float addrspace(1)* %595, align 4
  br label %postload5859vector_func.i

postload5859vector_func.i:                        ; preds = %preload5858vector_func.i, %postload5856vector_func.i
  br i1 %exmask3951vector_func.i, label %preload5861vector_func.i, label %postload5862vector_func.i

preload5861vector_func.i:                         ; preds = %postload5859vector_func.i
  %.sum8801vector_func.i = add i64 %dim_0_vector_tid.i, 55307
  %596 = getelementptr float addrspace(1)* %4, i64 %.sum8801vector_func.i
  %exData4148vector_func.i = extractelement <16 x float> %call.i37.i, i32 11
  store float %exData4148vector_func.i, float addrspace(1)* %596, align 4
  br label %postload5862vector_func.i

postload5862vector_func.i:                        ; preds = %preload5861vector_func.i, %postload5859vector_func.i
  br i1 %exmask3954vector_func.i, label %preload5864vector_func.i, label %postload5865vector_func.i

preload5864vector_func.i:                         ; preds = %postload5862vector_func.i
  %.sum8800vector_func.i = add i64 %dim_0_vector_tid.i, 55308
  %597 = getelementptr float addrspace(1)* %4, i64 %.sum8800vector_func.i
  %exData4151vector_func.i = extractelement <16 x float> %call.i37.i, i32 12
  store float %exData4151vector_func.i, float addrspace(1)* %597, align 4
  br label %postload5865vector_func.i

postload5865vector_func.i:                        ; preds = %preload5864vector_func.i, %postload5862vector_func.i
  br i1 %exmask3957vector_func.i, label %preload5867vector_func.i, label %postload5868vector_func.i

preload5867vector_func.i:                         ; preds = %postload5865vector_func.i
  %.sum8799vector_func.i = add i64 %dim_0_vector_tid.i, 55309
  %598 = getelementptr float addrspace(1)* %4, i64 %.sum8799vector_func.i
  %exData4154vector_func.i = extractelement <16 x float> %call.i37.i, i32 13
  store float %exData4154vector_func.i, float addrspace(1)* %598, align 4
  br label %postload5868vector_func.i

postload5868vector_func.i:                        ; preds = %preload5867vector_func.i, %postload5865vector_func.i
  br i1 %exmask3960vector_func.i, label %preload5870vector_func.i, label %postload5871vector_func.i

preload5870vector_func.i:                         ; preds = %postload5868vector_func.i
  %.sum8798vector_func.i = add i64 %dim_0_vector_tid.i, 55310
  %599 = getelementptr float addrspace(1)* %4, i64 %.sum8798vector_func.i
  %exData4157vector_func.i = extractelement <16 x float> %call.i37.i, i32 14
  store float %exData4157vector_func.i, float addrspace(1)* %599, align 4
  br label %postload5871vector_func.i

postload5871vector_func.i:                        ; preds = %preload5870vector_func.i, %postload5868vector_func.i
  br i1 %exmask3963vector_func.i, label %preload5873vector_func.i, label %postload5874vector_func.i

preload5873vector_func.i:                         ; preds = %postload5871vector_func.i
  %.sum8797vector_func.i = add i64 %dim_0_vector_tid.i, 55311
  %600 = getelementptr float addrspace(1)* %4, i64 %.sum8797vector_func.i
  %exData4160vector_func.i = extractelement <16 x float> %call.i37.i, i32 15
  store float %exData4160vector_func.i, float addrspace(1)* %600, align 4
  br label %postload5874vector_func.i

postload5874vector_func.i:                        ; preds = %preload5873vector_func.i, %postload5871vector_func.i
  %mul3611620vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000, float 0x40DD956E80000000>
  %add3621621vector_func.i = fadd <16 x float> %mul3611620vector_func.i, <float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000, float 0xBFEB2B45A0000000>
  %mul3631622vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000, float 0x4010CB6860000000>
  %add3641623vector_func.i = fadd <16 x float> %add3621621vector_func.i, %mul3631622vector_func.i
  %mul.i1761624vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000, float 0x3D38F03960000000>
  %add.i1771625vector_func.i = fadd <16 x float> %mul.i1761624vector_func.i, <float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000, float 0xBDFF6D7340000000>
  %mul1.i1781626vector_func.i = fmul <16 x float> %add.i1771625vector_func.i, %mul567vector_func.i
  %add2.i1791627vector_func.i = fadd <16 x float> %mul1.i1781626vector_func.i, <float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000, float 0x3EB23B7C60000000>
  %mul3.i1801628vector_func.i = fmul <16 x float> %add2.i1791627vector_func.i, %mul567vector_func.i
  %add4.i1811629vector_func.i = fadd <16 x float> %mul3.i1801628vector_func.i, <float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000, float 0xBF50AEB640000000>
  %mul5.i1821630vector_func.i = fmul <16 x float> %add4.i1811629vector_func.i, %mul567vector_func.i
  %add3661631vector_func.i = fadd <16 x float> %add3641623vector_func.i, %mul5.i1821630vector_func.i
  %call.i38.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3661631vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload5876vector_func.i, label %postload5877vector_func.i

preload5876vector_func.i:                         ; preds = %postload5874vector_func.i
  %extract1633vector_func.i = add i64 %dim_0_vector_tid.i, 69120
  %601 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1633vector_func.i
  %exData4164vector_func.i = extractelement <16 x float> %call.i38.i, i32 0
  store float %exData4164vector_func.i, float addrspace(1)* %601, align 4
  br label %postload5877vector_func.i

postload5877vector_func.i:                        ; preds = %preload5876vector_func.i, %postload5874vector_func.i
  br i1 %exmask3921vector_func.i, label %preload5879vector_func.i, label %postload5880vector_func.i

preload5879vector_func.i:                         ; preds = %postload5877vector_func.i
  %.sum8796vector_func.i = add i64 %dim_0_vector_tid.i, 69121
  %602 = getelementptr float addrspace(1)* %4, i64 %.sum8796vector_func.i
  %exData4167vector_func.i = extractelement <16 x float> %call.i38.i, i32 1
  store float %exData4167vector_func.i, float addrspace(1)* %602, align 4
  br label %postload5880vector_func.i

postload5880vector_func.i:                        ; preds = %preload5879vector_func.i, %postload5877vector_func.i
  br i1 %exmask3924vector_func.i, label %preload5882vector_func.i, label %postload5883vector_func.i

preload5882vector_func.i:                         ; preds = %postload5880vector_func.i
  %.sum8795vector_func.i = add i64 %dim_0_vector_tid.i, 69122
  %603 = getelementptr float addrspace(1)* %4, i64 %.sum8795vector_func.i
  %exData4170vector_func.i = extractelement <16 x float> %call.i38.i, i32 2
  store float %exData4170vector_func.i, float addrspace(1)* %603, align 4
  br label %postload5883vector_func.i

postload5883vector_func.i:                        ; preds = %preload5882vector_func.i, %postload5880vector_func.i
  br i1 %exmask3927vector_func.i, label %preload5885vector_func.i, label %postload5886vector_func.i

preload5885vector_func.i:                         ; preds = %postload5883vector_func.i
  %.sum8794vector_func.i = add i64 %dim_0_vector_tid.i, 69123
  %604 = getelementptr float addrspace(1)* %4, i64 %.sum8794vector_func.i
  %exData4173vector_func.i = extractelement <16 x float> %call.i38.i, i32 3
  store float %exData4173vector_func.i, float addrspace(1)* %604, align 4
  br label %postload5886vector_func.i

postload5886vector_func.i:                        ; preds = %preload5885vector_func.i, %postload5883vector_func.i
  br i1 %exmask3930vector_func.i, label %preload5888vector_func.i, label %postload5889vector_func.i

preload5888vector_func.i:                         ; preds = %postload5886vector_func.i
  %.sum8793vector_func.i = add i64 %dim_0_vector_tid.i, 69124
  %605 = getelementptr float addrspace(1)* %4, i64 %.sum8793vector_func.i
  %exData4176vector_func.i = extractelement <16 x float> %call.i38.i, i32 4
  store float %exData4176vector_func.i, float addrspace(1)* %605, align 4
  br label %postload5889vector_func.i

postload5889vector_func.i:                        ; preds = %preload5888vector_func.i, %postload5886vector_func.i
  br i1 %exmask3933vector_func.i, label %preload5891vector_func.i, label %postload5892vector_func.i

preload5891vector_func.i:                         ; preds = %postload5889vector_func.i
  %.sum8792vector_func.i = add i64 %dim_0_vector_tid.i, 69125
  %606 = getelementptr float addrspace(1)* %4, i64 %.sum8792vector_func.i
  %exData4179vector_func.i = extractelement <16 x float> %call.i38.i, i32 5
  store float %exData4179vector_func.i, float addrspace(1)* %606, align 4
  br label %postload5892vector_func.i

postload5892vector_func.i:                        ; preds = %preload5891vector_func.i, %postload5889vector_func.i
  br i1 %exmask3936vector_func.i, label %preload5894vector_func.i, label %postload5895vector_func.i

preload5894vector_func.i:                         ; preds = %postload5892vector_func.i
  %.sum8791vector_func.i = add i64 %dim_0_vector_tid.i, 69126
  %607 = getelementptr float addrspace(1)* %4, i64 %.sum8791vector_func.i
  %exData4182vector_func.i = extractelement <16 x float> %call.i38.i, i32 6
  store float %exData4182vector_func.i, float addrspace(1)* %607, align 4
  br label %postload5895vector_func.i

postload5895vector_func.i:                        ; preds = %preload5894vector_func.i, %postload5892vector_func.i
  br i1 %exmask3939vector_func.i, label %preload5897vector_func.i, label %postload5898vector_func.i

preload5897vector_func.i:                         ; preds = %postload5895vector_func.i
  %.sum8790vector_func.i = add i64 %dim_0_vector_tid.i, 69127
  %608 = getelementptr float addrspace(1)* %4, i64 %.sum8790vector_func.i
  %exData4185vector_func.i = extractelement <16 x float> %call.i38.i, i32 7
  store float %exData4185vector_func.i, float addrspace(1)* %608, align 4
  br label %postload5898vector_func.i

postload5898vector_func.i:                        ; preds = %preload5897vector_func.i, %postload5895vector_func.i
  br i1 %exmask3942vector_func.i, label %preload5900vector_func.i, label %postload5901vector_func.i

preload5900vector_func.i:                         ; preds = %postload5898vector_func.i
  %.sum8789vector_func.i = add i64 %dim_0_vector_tid.i, 69128
  %609 = getelementptr float addrspace(1)* %4, i64 %.sum8789vector_func.i
  %exData4188vector_func.i = extractelement <16 x float> %call.i38.i, i32 8
  store float %exData4188vector_func.i, float addrspace(1)* %609, align 4
  br label %postload5901vector_func.i

postload5901vector_func.i:                        ; preds = %preload5900vector_func.i, %postload5898vector_func.i
  br i1 %exmask3945vector_func.i, label %preload5903vector_func.i, label %postload5904vector_func.i

preload5903vector_func.i:                         ; preds = %postload5901vector_func.i
  %.sum8788vector_func.i = add i64 %dim_0_vector_tid.i, 69129
  %610 = getelementptr float addrspace(1)* %4, i64 %.sum8788vector_func.i
  %exData4191vector_func.i = extractelement <16 x float> %call.i38.i, i32 9
  store float %exData4191vector_func.i, float addrspace(1)* %610, align 4
  br label %postload5904vector_func.i

postload5904vector_func.i:                        ; preds = %preload5903vector_func.i, %postload5901vector_func.i
  br i1 %exmask3948vector_func.i, label %preload5906vector_func.i, label %postload5907vector_func.i

preload5906vector_func.i:                         ; preds = %postload5904vector_func.i
  %.sum8787vector_func.i = add i64 %dim_0_vector_tid.i, 69130
  %611 = getelementptr float addrspace(1)* %4, i64 %.sum8787vector_func.i
  %exData4194vector_func.i = extractelement <16 x float> %call.i38.i, i32 10
  store float %exData4194vector_func.i, float addrspace(1)* %611, align 4
  br label %postload5907vector_func.i

postload5907vector_func.i:                        ; preds = %preload5906vector_func.i, %postload5904vector_func.i
  br i1 %exmask3951vector_func.i, label %preload5909vector_func.i, label %postload5910vector_func.i

preload5909vector_func.i:                         ; preds = %postload5907vector_func.i
  %.sum8786vector_func.i = add i64 %dim_0_vector_tid.i, 69131
  %612 = getelementptr float addrspace(1)* %4, i64 %.sum8786vector_func.i
  %exData4197vector_func.i = extractelement <16 x float> %call.i38.i, i32 11
  store float %exData4197vector_func.i, float addrspace(1)* %612, align 4
  br label %postload5910vector_func.i

postload5910vector_func.i:                        ; preds = %preload5909vector_func.i, %postload5907vector_func.i
  br i1 %exmask3954vector_func.i, label %preload5912vector_func.i, label %postload5913vector_func.i

preload5912vector_func.i:                         ; preds = %postload5910vector_func.i
  %.sum8785vector_func.i = add i64 %dim_0_vector_tid.i, 69132
  %613 = getelementptr float addrspace(1)* %4, i64 %.sum8785vector_func.i
  %exData4200vector_func.i = extractelement <16 x float> %call.i38.i, i32 12
  store float %exData4200vector_func.i, float addrspace(1)* %613, align 4
  br label %postload5913vector_func.i

postload5913vector_func.i:                        ; preds = %preload5912vector_func.i, %postload5910vector_func.i
  br i1 %exmask3957vector_func.i, label %preload5915vector_func.i, label %postload5916vector_func.i

preload5915vector_func.i:                         ; preds = %postload5913vector_func.i
  %.sum8784vector_func.i = add i64 %dim_0_vector_tid.i, 69133
  %614 = getelementptr float addrspace(1)* %4, i64 %.sum8784vector_func.i
  %exData4203vector_func.i = extractelement <16 x float> %call.i38.i, i32 13
  store float %exData4203vector_func.i, float addrspace(1)* %614, align 4
  br label %postload5916vector_func.i

postload5916vector_func.i:                        ; preds = %preload5915vector_func.i, %postload5913vector_func.i
  br i1 %exmask3960vector_func.i, label %preload5918vector_func.i, label %postload5919vector_func.i

preload5918vector_func.i:                         ; preds = %postload5916vector_func.i
  %.sum8783vector_func.i = add i64 %dim_0_vector_tid.i, 69134
  %615 = getelementptr float addrspace(1)* %4, i64 %.sum8783vector_func.i
  %exData4206vector_func.i = extractelement <16 x float> %call.i38.i, i32 14
  store float %exData4206vector_func.i, float addrspace(1)* %615, align 4
  br label %postload5919vector_func.i

postload5919vector_func.i:                        ; preds = %preload5918vector_func.i, %postload5916vector_func.i
  br i1 %exmask3963vector_func.i, label %preload5921vector_func.i, label %postload5922vector_func.i

preload5921vector_func.i:                         ; preds = %postload5919vector_func.i
  %.sum8782vector_func.i = add i64 %dim_0_vector_tid.i, 69135
  %616 = getelementptr float addrspace(1)* %4, i64 %.sum8782vector_func.i
  %exData4209vector_func.i = extractelement <16 x float> %call.i38.i, i32 15
  store float %exData4209vector_func.i, float addrspace(1)* %616, align 4
  br label %postload5922vector_func.i

postload5922vector_func.i:                        ; preds = %preload5921vector_func.i, %postload5919vector_func.i
  %mul3711650vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000, float 0x40726CEDC0000000>
  %sub3721651vector_func.i = fsub <16 x float> <float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000, float 0x400DBBB980000000>, %mul3711650vector_func.i
  %mul3731652vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000, float 0x4011350A80000000>
  %add3741653vector_func.i = fadd <16 x float> %sub3721651vector_func.i, %mul3731652vector_func.i
  %mul.i1691654vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000, float 0x3D6058DBA0000000>
  %add.i1701655vector_func.i = fadd <16 x float> %mul.i1691654vector_func.i, <float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000, float 0xBE2160B200000000>
  %mul1.i1711656vector_func.i = fmul <16 x float> %add.i1701655vector_func.i, %mul567vector_func.i
  %add2.i1721657vector_func.i = fadd <16 x float> %mul1.i1711656vector_func.i, <float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000, float 0x3ECD94D8C0000000>
  %mul3.i1731658vector_func.i = fmul <16 x float> %add2.i1721657vector_func.i, %mul567vector_func.i
  %add4.i1741659vector_func.i = fadd <16 x float> %mul3.i1731658vector_func.i, <float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000, float 0xBF6373D060000000>
  %mul5.i1751660vector_func.i = fmul <16 x float> %add4.i1741659vector_func.i, %mul567vector_func.i
  %add3761661vector_func.i = fadd <16 x float> %add3741653vector_func.i, %mul5.i1751660vector_func.i
  %call.i39.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3761661vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload5924vector_func.i, label %postload5925vector_func.i

preload5924vector_func.i:                         ; preds = %postload5922vector_func.i
  %extract1663vector_func.i = add i64 %dim_0_vector_tid.i, 82944
  %617 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1663vector_func.i
  %exData4213vector_func.i = extractelement <16 x float> %call.i39.i, i32 0
  store float %exData4213vector_func.i, float addrspace(1)* %617, align 4
  br label %postload5925vector_func.i

postload5925vector_func.i:                        ; preds = %preload5924vector_func.i, %postload5922vector_func.i
  br i1 %exmask3921vector_func.i, label %preload5927vector_func.i, label %postload5928vector_func.i

preload5927vector_func.i:                         ; preds = %postload5925vector_func.i
  %.sum8781vector_func.i = add i64 %dim_0_vector_tid.i, 82945
  %618 = getelementptr float addrspace(1)* %4, i64 %.sum8781vector_func.i
  %exData4216vector_func.i = extractelement <16 x float> %call.i39.i, i32 1
  store float %exData4216vector_func.i, float addrspace(1)* %618, align 4
  br label %postload5928vector_func.i

postload5928vector_func.i:                        ; preds = %preload5927vector_func.i, %postload5925vector_func.i
  br i1 %exmask3924vector_func.i, label %preload5930vector_func.i, label %postload5931vector_func.i

preload5930vector_func.i:                         ; preds = %postload5928vector_func.i
  %.sum8780vector_func.i = add i64 %dim_0_vector_tid.i, 82946
  %619 = getelementptr float addrspace(1)* %4, i64 %.sum8780vector_func.i
  %exData4219vector_func.i = extractelement <16 x float> %call.i39.i, i32 2
  store float %exData4219vector_func.i, float addrspace(1)* %619, align 4
  br label %postload5931vector_func.i

postload5931vector_func.i:                        ; preds = %preload5930vector_func.i, %postload5928vector_func.i
  br i1 %exmask3927vector_func.i, label %preload5933vector_func.i, label %postload5934vector_func.i

preload5933vector_func.i:                         ; preds = %postload5931vector_func.i
  %.sum8779vector_func.i = add i64 %dim_0_vector_tid.i, 82947
  %620 = getelementptr float addrspace(1)* %4, i64 %.sum8779vector_func.i
  %exData4222vector_func.i = extractelement <16 x float> %call.i39.i, i32 3
  store float %exData4222vector_func.i, float addrspace(1)* %620, align 4
  br label %postload5934vector_func.i

postload5934vector_func.i:                        ; preds = %preload5933vector_func.i, %postload5931vector_func.i
  br i1 %exmask3930vector_func.i, label %preload5936vector_func.i, label %postload5937vector_func.i

preload5936vector_func.i:                         ; preds = %postload5934vector_func.i
  %.sum8778vector_func.i = add i64 %dim_0_vector_tid.i, 82948
  %621 = getelementptr float addrspace(1)* %4, i64 %.sum8778vector_func.i
  %exData4225vector_func.i = extractelement <16 x float> %call.i39.i, i32 4
  store float %exData4225vector_func.i, float addrspace(1)* %621, align 4
  br label %postload5937vector_func.i

postload5937vector_func.i:                        ; preds = %preload5936vector_func.i, %postload5934vector_func.i
  br i1 %exmask3933vector_func.i, label %preload5939vector_func.i, label %postload5940vector_func.i

preload5939vector_func.i:                         ; preds = %postload5937vector_func.i
  %.sum8777vector_func.i = add i64 %dim_0_vector_tid.i, 82949
  %622 = getelementptr float addrspace(1)* %4, i64 %.sum8777vector_func.i
  %exData4228vector_func.i = extractelement <16 x float> %call.i39.i, i32 5
  store float %exData4228vector_func.i, float addrspace(1)* %622, align 4
  br label %postload5940vector_func.i

postload5940vector_func.i:                        ; preds = %preload5939vector_func.i, %postload5937vector_func.i
  br i1 %exmask3936vector_func.i, label %preload5942vector_func.i, label %postload5943vector_func.i

preload5942vector_func.i:                         ; preds = %postload5940vector_func.i
  %.sum8776vector_func.i = add i64 %dim_0_vector_tid.i, 82950
  %623 = getelementptr float addrspace(1)* %4, i64 %.sum8776vector_func.i
  %exData4231vector_func.i = extractelement <16 x float> %call.i39.i, i32 6
  store float %exData4231vector_func.i, float addrspace(1)* %623, align 4
  br label %postload5943vector_func.i

postload5943vector_func.i:                        ; preds = %preload5942vector_func.i, %postload5940vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6572vector_func.i, label %postload6573vector_func.i

preload6572vector_func.i:                         ; preds = %postload5943vector_func.i
  %.sum8775vector_func.i = add i64 %dim_0_vector_tid.i, 82951
  %624 = getelementptr float addrspace(1)* %4, i64 %.sum8775vector_func.i
  %exData4234vector_func.i = extractelement <16 x float> %call.i39.i, i32 7
  store float %exData4234vector_func.i, float addrspace(1)* %624, align 4
  br label %postload6573vector_func.i

postload6573vector_func.i:                        ; preds = %preload6572vector_func.i, %postload5943vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6575vector_func.i, label %postload6576vector_func.i

preload6575vector_func.i:                         ; preds = %postload6573vector_func.i
  %.sum8774vector_func.i = add i64 %dim_0_vector_tid.i, 82952
  %625 = getelementptr float addrspace(1)* %4, i64 %.sum8774vector_func.i
  %exData4237vector_func.i = extractelement <16 x float> %call.i39.i, i32 8
  store float %exData4237vector_func.i, float addrspace(1)* %625, align 4
  br label %postload6576vector_func.i

postload6576vector_func.i:                        ; preds = %preload6575vector_func.i, %postload6573vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6578vector_func.i, label %postload6579vector_func.i

preload6578vector_func.i:                         ; preds = %postload6576vector_func.i
  %.sum8773vector_func.i = add i64 %dim_0_vector_tid.i, 82953
  %626 = getelementptr float addrspace(1)* %4, i64 %.sum8773vector_func.i
  %exData4240vector_func.i = extractelement <16 x float> %call.i39.i, i32 9
  store float %exData4240vector_func.i, float addrspace(1)* %626, align 4
  br label %postload6579vector_func.i

postload6579vector_func.i:                        ; preds = %preload6578vector_func.i, %postload6576vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6581vector_func.i, label %postload6582vector_func.i

preload6581vector_func.i:                         ; preds = %postload6579vector_func.i
  %.sum8772vector_func.i = add i64 %dim_0_vector_tid.i, 82954
  %627 = getelementptr float addrspace(1)* %4, i64 %.sum8772vector_func.i
  %exData4243vector_func.i = extractelement <16 x float> %call.i39.i, i32 10
  store float %exData4243vector_func.i, float addrspace(1)* %627, align 4
  br label %postload6582vector_func.i

postload6582vector_func.i:                        ; preds = %preload6581vector_func.i, %postload6579vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6584vector_func.i, label %postload6585vector_func.i

preload6584vector_func.i:                         ; preds = %postload6582vector_func.i
  %.sum8771vector_func.i = add i64 %dim_0_vector_tid.i, 82955
  %628 = getelementptr float addrspace(1)* %4, i64 %.sum8771vector_func.i
  %exData4246vector_func.i = extractelement <16 x float> %call.i39.i, i32 11
  store float %exData4246vector_func.i, float addrspace(1)* %628, align 4
  br label %postload6585vector_func.i

postload6585vector_func.i:                        ; preds = %preload6584vector_func.i, %postload6582vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6587vector_func.i, label %postload6588vector_func.i

preload6587vector_func.i:                         ; preds = %postload6585vector_func.i
  %.sum8770vector_func.i = add i64 %dim_0_vector_tid.i, 82956
  %629 = getelementptr float addrspace(1)* %4, i64 %.sum8770vector_func.i
  %exData4249vector_func.i = extractelement <16 x float> %call.i39.i, i32 12
  store float %exData4249vector_func.i, float addrspace(1)* %629, align 4
  br label %postload6588vector_func.i

postload6588vector_func.i:                        ; preds = %preload6587vector_func.i, %postload6585vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6590vector_func.i, label %postload6591vector_func.i

preload6590vector_func.i:                         ; preds = %postload6588vector_func.i
  %.sum8769vector_func.i = add i64 %dim_0_vector_tid.i, 82957
  %630 = getelementptr float addrspace(1)* %4, i64 %.sum8769vector_func.i
  %exData4252vector_func.i = extractelement <16 x float> %call.i39.i, i32 13
  store float %exData4252vector_func.i, float addrspace(1)* %630, align 4
  br label %postload6591vector_func.i

postload6591vector_func.i:                        ; preds = %preload6590vector_func.i, %postload6588vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6593vector_func.i, label %postload6594vector_func.i

preload6593vector_func.i:                         ; preds = %postload6591vector_func.i
  %.sum8768vector_func.i = add i64 %dim_0_vector_tid.i, 82958
  %631 = getelementptr float addrspace(1)* %4, i64 %.sum8768vector_func.i
  %exData4255vector_func.i = extractelement <16 x float> %call.i39.i, i32 14
  store float %exData4255vector_func.i, float addrspace(1)* %631, align 4
  br label %postload6594vector_func.i

postload6594vector_func.i:                        ; preds = %preload6593vector_func.i, %postload6591vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6596vector_func.i, label %postload6597vector_func.i

preload6596vector_func.i:                         ; preds = %postload6594vector_func.i
  %.sum8767vector_func.i = add i64 %dim_0_vector_tid.i, 82959
  %632 = getelementptr float addrspace(1)* %4, i64 %.sum8767vector_func.i
  %exData4258vector_func.i = extractelement <16 x float> %call.i39.i, i32 15
  store float %exData4258vector_func.i, float addrspace(1)* %632, align 4
  br label %postload6597vector_func.i

postload6597vector_func.i:                        ; preds = %preload6596vector_func.i, %postload6594vector_func.i
  %mul3811680vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000, float 0x40D149A540000000>
  %add3821681vector_func.i = fadd <16 x float> %mul3811680vector_func.i, <float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000, float 0x400B7AFBE0000000>
  %mul3831682vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000, float 0x40111ABD40000000>
  %add3841683vector_func.i = fadd <16 x float> %add3821681vector_func.i, %mul3831682vector_func.i
  %mul.i1621684vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000, float 0x3D5E584C60000000>
  %add.i1631685vector_func.i = fadd <16 x float> %mul.i1621684vector_func.i, <float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000, float 0xBE1EE41580000000>
  %mul1.i1641686vector_func.i = fmul <16 x float> %add.i1631685vector_func.i, %mul567vector_func.i
  %add2.i1651687vector_func.i = fadd <16 x float> %mul1.i1641686vector_func.i, <float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000, float 0x3EC7652DA0000000>
  %mul3.i1661688vector_func.i = fmul <16 x float> %add2.i1651687vector_func.i, %mul567vector_func.i
  %add4.i1671689vector_func.i = fadd <16 x float> %mul3.i1661688vector_func.i, <float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000, float 0xBF31C98640000000>
  %mul5.i1681690vector_func.i = fmul <16 x float> %add4.i1671689vector_func.i, %mul567vector_func.i
  %add3861691vector_func.i = fadd <16 x float> %add3841683vector_func.i, %mul5.i1681690vector_func.i
  %call.i40.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3861691vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6599vector_func.i, label %postload6600vector_func.i

preload6599vector_func.i:                         ; preds = %postload6597vector_func.i
  %extract1693vector_func.i = add i64 %dim_0_vector_tid.i, 96768
  %633 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1693vector_func.i
  %exData4262vector_func.i = extractelement <16 x float> %call.i40.i, i32 0
  store float %exData4262vector_func.i, float addrspace(1)* %633, align 4
  br label %postload6600vector_func.i

postload6600vector_func.i:                        ; preds = %preload6599vector_func.i, %postload6597vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6602vector_func.i, label %postload6603vector_func.i

preload6602vector_func.i:                         ; preds = %postload6600vector_func.i
  %.sum8766vector_func.i = add i64 %dim_0_vector_tid.i, 96769
  %634 = getelementptr float addrspace(1)* %4, i64 %.sum8766vector_func.i
  %exData4265vector_func.i = extractelement <16 x float> %call.i40.i, i32 1
  store float %exData4265vector_func.i, float addrspace(1)* %634, align 4
  br label %postload6603vector_func.i

postload6603vector_func.i:                        ; preds = %preload6602vector_func.i, %postload6600vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6605vector_func.i, label %postload6606vector_func.i

preload6605vector_func.i:                         ; preds = %postload6603vector_func.i
  %.sum8765vector_func.i = add i64 %dim_0_vector_tid.i, 96770
  %635 = getelementptr float addrspace(1)* %4, i64 %.sum8765vector_func.i
  %exData4268vector_func.i = extractelement <16 x float> %call.i40.i, i32 2
  store float %exData4268vector_func.i, float addrspace(1)* %635, align 4
  br label %postload6606vector_func.i

postload6606vector_func.i:                        ; preds = %preload6605vector_func.i, %postload6603vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6608vector_func.i, label %postload6609vector_func.i

preload6608vector_func.i:                         ; preds = %postload6606vector_func.i
  %.sum8764vector_func.i = add i64 %dim_0_vector_tid.i, 96771
  %636 = getelementptr float addrspace(1)* %4, i64 %.sum8764vector_func.i
  %exData4271vector_func.i = extractelement <16 x float> %call.i40.i, i32 3
  store float %exData4271vector_func.i, float addrspace(1)* %636, align 4
  br label %postload6609vector_func.i

postload6609vector_func.i:                        ; preds = %preload6608vector_func.i, %postload6606vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6611vector_func.i, label %postload6612vector_func.i

preload6611vector_func.i:                         ; preds = %postload6609vector_func.i
  %.sum8763vector_func.i = add i64 %dim_0_vector_tid.i, 96772
  %637 = getelementptr float addrspace(1)* %4, i64 %.sum8763vector_func.i
  %exData4274vector_func.i = extractelement <16 x float> %call.i40.i, i32 4
  store float %exData4274vector_func.i, float addrspace(1)* %637, align 4
  br label %postload6612vector_func.i

postload6612vector_func.i:                        ; preds = %preload6611vector_func.i, %postload6609vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6614vector_func.i, label %postload6615vector_func.i

preload6614vector_func.i:                         ; preds = %postload6612vector_func.i
  %.sum8762vector_func.i = add i64 %dim_0_vector_tid.i, 96773
  %638 = getelementptr float addrspace(1)* %4, i64 %.sum8762vector_func.i
  %exData4277vector_func.i = extractelement <16 x float> %call.i40.i, i32 5
  store float %exData4277vector_func.i, float addrspace(1)* %638, align 4
  br label %postload6615vector_func.i

postload6615vector_func.i:                        ; preds = %preload6614vector_func.i, %postload6612vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6617vector_func.i, label %postload6618vector_func.i

preload6617vector_func.i:                         ; preds = %postload6615vector_func.i
  %.sum8761vector_func.i = add i64 %dim_0_vector_tid.i, 96774
  %639 = getelementptr float addrspace(1)* %4, i64 %.sum8761vector_func.i
  %exData4280vector_func.i = extractelement <16 x float> %call.i40.i, i32 6
  store float %exData4280vector_func.i, float addrspace(1)* %639, align 4
  br label %postload6618vector_func.i

postload6618vector_func.i:                        ; preds = %preload6617vector_func.i, %postload6615vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6620vector_func.i, label %postload6621vector_func.i

preload6620vector_func.i:                         ; preds = %postload6618vector_func.i
  %.sum8760vector_func.i = add i64 %dim_0_vector_tid.i, 96775
  %640 = getelementptr float addrspace(1)* %4, i64 %.sum8760vector_func.i
  %exData4283vector_func.i = extractelement <16 x float> %call.i40.i, i32 7
  store float %exData4283vector_func.i, float addrspace(1)* %640, align 4
  br label %postload6621vector_func.i

postload6621vector_func.i:                        ; preds = %preload6620vector_func.i, %postload6618vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6623vector_func.i, label %postload6624vector_func.i

preload6623vector_func.i:                         ; preds = %postload6621vector_func.i
  %.sum8759vector_func.i = add i64 %dim_0_vector_tid.i, 96776
  %641 = getelementptr float addrspace(1)* %4, i64 %.sum8759vector_func.i
  %exData4286vector_func.i = extractelement <16 x float> %call.i40.i, i32 8
  store float %exData4286vector_func.i, float addrspace(1)* %641, align 4
  br label %postload6624vector_func.i

postload6624vector_func.i:                        ; preds = %preload6623vector_func.i, %postload6621vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6626vector_func.i, label %postload6627vector_func.i

preload6626vector_func.i:                         ; preds = %postload6624vector_func.i
  %.sum8758vector_func.i = add i64 %dim_0_vector_tid.i, 96777
  %642 = getelementptr float addrspace(1)* %4, i64 %.sum8758vector_func.i
  %exData4289vector_func.i = extractelement <16 x float> %call.i40.i, i32 9
  store float %exData4289vector_func.i, float addrspace(1)* %642, align 4
  br label %postload6627vector_func.i

postload6627vector_func.i:                        ; preds = %preload6626vector_func.i, %postload6624vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6629vector_func.i, label %postload6630vector_func.i

preload6629vector_func.i:                         ; preds = %postload6627vector_func.i
  %.sum8757vector_func.i = add i64 %dim_0_vector_tid.i, 96778
  %643 = getelementptr float addrspace(1)* %4, i64 %.sum8757vector_func.i
  %exData4292vector_func.i = extractelement <16 x float> %call.i40.i, i32 10
  store float %exData4292vector_func.i, float addrspace(1)* %643, align 4
  br label %postload6630vector_func.i

postload6630vector_func.i:                        ; preds = %preload6629vector_func.i, %postload6627vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6632vector_func.i, label %postload6633vector_func.i

preload6632vector_func.i:                         ; preds = %postload6630vector_func.i
  %.sum8756vector_func.i = add i64 %dim_0_vector_tid.i, 96779
  %644 = getelementptr float addrspace(1)* %4, i64 %.sum8756vector_func.i
  %exData4295vector_func.i = extractelement <16 x float> %call.i40.i, i32 11
  store float %exData4295vector_func.i, float addrspace(1)* %644, align 4
  br label %postload6633vector_func.i

postload6633vector_func.i:                        ; preds = %preload6632vector_func.i, %postload6630vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6635vector_func.i, label %postload6636vector_func.i

preload6635vector_func.i:                         ; preds = %postload6633vector_func.i
  %.sum8755vector_func.i = add i64 %dim_0_vector_tid.i, 96780
  %645 = getelementptr float addrspace(1)* %4, i64 %.sum8755vector_func.i
  %exData4298vector_func.i = extractelement <16 x float> %call.i40.i, i32 12
  store float %exData4298vector_func.i, float addrspace(1)* %645, align 4
  br label %postload6636vector_func.i

postload6636vector_func.i:                        ; preds = %preload6635vector_func.i, %postload6633vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6638vector_func.i, label %postload6639vector_func.i

preload6638vector_func.i:                         ; preds = %postload6636vector_func.i
  %.sum8754vector_func.i = add i64 %dim_0_vector_tid.i, 96781
  %646 = getelementptr float addrspace(1)* %4, i64 %.sum8754vector_func.i
  %exData4301vector_func.i = extractelement <16 x float> %call.i40.i, i32 13
  store float %exData4301vector_func.i, float addrspace(1)* %646, align 4
  br label %postload6639vector_func.i

postload6639vector_func.i:                        ; preds = %preload6638vector_func.i, %postload6636vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6641vector_func.i, label %postload6642vector_func.i

preload6641vector_func.i:                         ; preds = %postload6639vector_func.i
  %.sum8753vector_func.i = add i64 %dim_0_vector_tid.i, 96782
  %647 = getelementptr float addrspace(1)* %4, i64 %.sum8753vector_func.i
  %exData4304vector_func.i = extractelement <16 x float> %call.i40.i, i32 14
  store float %exData4304vector_func.i, float addrspace(1)* %647, align 4
  br label %postload6642vector_func.i

postload6642vector_func.i:                        ; preds = %preload6641vector_func.i, %postload6639vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6644vector_func.i, label %postload6645vector_func.i

preload6644vector_func.i:                         ; preds = %postload6642vector_func.i
  %.sum8752vector_func.i = add i64 %dim_0_vector_tid.i, 96783
  %648 = getelementptr float addrspace(1)* %4, i64 %.sum8752vector_func.i
  %exData4307vector_func.i = extractelement <16 x float> %call.i40.i, i32 15
  store float %exData4307vector_func.i, float addrspace(1)* %648, align 4
  br label %postload6645vector_func.i

postload6645vector_func.i:                        ; preds = %preload6644vector_func.i, %postload6642vector_func.i
  %mul3911710vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000, float 0x40F148D4C0000000>
  %sub3921711vector_func.i = fsub <16 x float> <float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000, float 0x4000AC0E00000000>, %mul3911710vector_func.i
  %mul3931712vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000, float 0x400BEB2500000000>
  %add3941713vector_func.i = fadd <16 x float> %sub3921711vector_func.i, %mul3931712vector_func.i
  %mul.i1551714vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000, float 0xBD33C9F9C0000000>
  %add.i1561715vector_func.i = fadd <16 x float> %mul.i1551714vector_func.i, <float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000, float 0x3DF21BCB80000000>
  %mul1.i1571716vector_func.i = fmul <16 x float> %add.i1561715vector_func.i, %mul567vector_func.i
  %add2.i1581717vector_func.i = fadd <16 x float> %mul1.i1571716vector_func.i, <float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000, float 0xBE92E41B40000000>
  %mul3.i1591718vector_func.i = fmul <16 x float> %add2.i1581717vector_func.i, %mul567vector_func.i
  %add4.i1601719vector_func.i = fadd <16 x float> %mul3.i1591718vector_func.i, <float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000, float 0x3F25390F00000000>
  %mul5.i1611720vector_func.i = fmul <16 x float> %add4.i1601719vector_func.i, %mul567vector_func.i
  %add3961721vector_func.i = fadd <16 x float> %add3941713vector_func.i, %mul5.i1611720vector_func.i
  %call.i41.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add3961721vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6647vector_func.i, label %postload6648vector_func.i

preload6647vector_func.i:                         ; preds = %postload6645vector_func.i
  %extract1723vector_func.i = add i64 %dim_0_vector_tid.i, 110592
  %649 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1723vector_func.i
  %exData4311vector_func.i = extractelement <16 x float> %call.i41.i, i32 0
  store float %exData4311vector_func.i, float addrspace(1)* %649, align 4
  br label %postload6648vector_func.i

postload6648vector_func.i:                        ; preds = %preload6647vector_func.i, %postload6645vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6650vector_func.i, label %postload6651vector_func.i

preload6650vector_func.i:                         ; preds = %postload6648vector_func.i
  %.sum8751vector_func.i = add i64 %dim_0_vector_tid.i, 110593
  %650 = getelementptr float addrspace(1)* %4, i64 %.sum8751vector_func.i
  %exData4314vector_func.i = extractelement <16 x float> %call.i41.i, i32 1
  store float %exData4314vector_func.i, float addrspace(1)* %650, align 4
  br label %postload6651vector_func.i

postload6651vector_func.i:                        ; preds = %preload6650vector_func.i, %postload6648vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6653vector_func.i, label %postload6654vector_func.i

preload6653vector_func.i:                         ; preds = %postload6651vector_func.i
  %.sum8750vector_func.i = add i64 %dim_0_vector_tid.i, 110594
  %651 = getelementptr float addrspace(1)* %4, i64 %.sum8750vector_func.i
  %exData4317vector_func.i = extractelement <16 x float> %call.i41.i, i32 2
  store float %exData4317vector_func.i, float addrspace(1)* %651, align 4
  br label %postload6654vector_func.i

postload6654vector_func.i:                        ; preds = %preload6653vector_func.i, %postload6651vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6656vector_func.i, label %postload6657vector_func.i

preload6656vector_func.i:                         ; preds = %postload6654vector_func.i
  %.sum8749vector_func.i = add i64 %dim_0_vector_tid.i, 110595
  %652 = getelementptr float addrspace(1)* %4, i64 %.sum8749vector_func.i
  %exData4320vector_func.i = extractelement <16 x float> %call.i41.i, i32 3
  store float %exData4320vector_func.i, float addrspace(1)* %652, align 4
  br label %postload6657vector_func.i

postload6657vector_func.i:                        ; preds = %preload6656vector_func.i, %postload6654vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6659vector_func.i, label %postload6660vector_func.i

preload6659vector_func.i:                         ; preds = %postload6657vector_func.i
  %.sum8748vector_func.i = add i64 %dim_0_vector_tid.i, 110596
  %653 = getelementptr float addrspace(1)* %4, i64 %.sum8748vector_func.i
  %exData4323vector_func.i = extractelement <16 x float> %call.i41.i, i32 4
  store float %exData4323vector_func.i, float addrspace(1)* %653, align 4
  br label %postload6660vector_func.i

postload6660vector_func.i:                        ; preds = %preload6659vector_func.i, %postload6657vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6662vector_func.i, label %postload6663vector_func.i

preload6662vector_func.i:                         ; preds = %postload6660vector_func.i
  %.sum8747vector_func.i = add i64 %dim_0_vector_tid.i, 110597
  %654 = getelementptr float addrspace(1)* %4, i64 %.sum8747vector_func.i
  %exData4326vector_func.i = extractelement <16 x float> %call.i41.i, i32 5
  store float %exData4326vector_func.i, float addrspace(1)* %654, align 4
  br label %postload6663vector_func.i

postload6663vector_func.i:                        ; preds = %preload6662vector_func.i, %postload6660vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6665vector_func.i, label %postload6666vector_func.i

preload6665vector_func.i:                         ; preds = %postload6663vector_func.i
  %.sum8746vector_func.i = add i64 %dim_0_vector_tid.i, 110598
  %655 = getelementptr float addrspace(1)* %4, i64 %.sum8746vector_func.i
  %exData4329vector_func.i = extractelement <16 x float> %call.i41.i, i32 6
  store float %exData4329vector_func.i, float addrspace(1)* %655, align 4
  br label %postload6666vector_func.i

postload6666vector_func.i:                        ; preds = %preload6665vector_func.i, %postload6663vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6668vector_func.i, label %postload6669vector_func.i

preload6668vector_func.i:                         ; preds = %postload6666vector_func.i
  %.sum8745vector_func.i = add i64 %dim_0_vector_tid.i, 110599
  %656 = getelementptr float addrspace(1)* %4, i64 %.sum8745vector_func.i
  %exData4332vector_func.i = extractelement <16 x float> %call.i41.i, i32 7
  store float %exData4332vector_func.i, float addrspace(1)* %656, align 4
  br label %postload6669vector_func.i

postload6669vector_func.i:                        ; preds = %preload6668vector_func.i, %postload6666vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6671vector_func.i, label %postload6672vector_func.i

preload6671vector_func.i:                         ; preds = %postload6669vector_func.i
  %.sum8744vector_func.i = add i64 %dim_0_vector_tid.i, 110600
  %657 = getelementptr float addrspace(1)* %4, i64 %.sum8744vector_func.i
  %exData4335vector_func.i = extractelement <16 x float> %call.i41.i, i32 8
  store float %exData4335vector_func.i, float addrspace(1)* %657, align 4
  br label %postload6672vector_func.i

postload6672vector_func.i:                        ; preds = %preload6671vector_func.i, %postload6669vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6674vector_func.i, label %postload6675vector_func.i

preload6674vector_func.i:                         ; preds = %postload6672vector_func.i
  %.sum8743vector_func.i = add i64 %dim_0_vector_tid.i, 110601
  %658 = getelementptr float addrspace(1)* %4, i64 %.sum8743vector_func.i
  %exData4338vector_func.i = extractelement <16 x float> %call.i41.i, i32 9
  store float %exData4338vector_func.i, float addrspace(1)* %658, align 4
  br label %postload6675vector_func.i

postload6675vector_func.i:                        ; preds = %preload6674vector_func.i, %postload6672vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6677vector_func.i, label %postload6678vector_func.i

preload6677vector_func.i:                         ; preds = %postload6675vector_func.i
  %.sum8742vector_func.i = add i64 %dim_0_vector_tid.i, 110602
  %659 = getelementptr float addrspace(1)* %4, i64 %.sum8742vector_func.i
  %exData4341vector_func.i = extractelement <16 x float> %call.i41.i, i32 10
  store float %exData4341vector_func.i, float addrspace(1)* %659, align 4
  br label %postload6678vector_func.i

postload6678vector_func.i:                        ; preds = %preload6677vector_func.i, %postload6675vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6680vector_func.i, label %postload6681vector_func.i

preload6680vector_func.i:                         ; preds = %postload6678vector_func.i
  %.sum8741vector_func.i = add i64 %dim_0_vector_tid.i, 110603
  %660 = getelementptr float addrspace(1)* %4, i64 %.sum8741vector_func.i
  %exData4344vector_func.i = extractelement <16 x float> %call.i41.i, i32 11
  store float %exData4344vector_func.i, float addrspace(1)* %660, align 4
  br label %postload6681vector_func.i

postload6681vector_func.i:                        ; preds = %preload6680vector_func.i, %postload6678vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6683vector_func.i, label %postload6684vector_func.i

preload6683vector_func.i:                         ; preds = %postload6681vector_func.i
  %.sum8740vector_func.i = add i64 %dim_0_vector_tid.i, 110604
  %661 = getelementptr float addrspace(1)* %4, i64 %.sum8740vector_func.i
  %exData4347vector_func.i = extractelement <16 x float> %call.i41.i, i32 12
  store float %exData4347vector_func.i, float addrspace(1)* %661, align 4
  br label %postload6684vector_func.i

postload6684vector_func.i:                        ; preds = %preload6683vector_func.i, %postload6681vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6686vector_func.i, label %postload6687vector_func.i

preload6686vector_func.i:                         ; preds = %postload6684vector_func.i
  %.sum8739vector_func.i = add i64 %dim_0_vector_tid.i, 110605
  %662 = getelementptr float addrspace(1)* %4, i64 %.sum8739vector_func.i
  %exData4350vector_func.i = extractelement <16 x float> %call.i41.i, i32 13
  store float %exData4350vector_func.i, float addrspace(1)* %662, align 4
  br label %postload6687vector_func.i

postload6687vector_func.i:                        ; preds = %preload6686vector_func.i, %postload6684vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6689vector_func.i, label %postload6690vector_func.i

preload6689vector_func.i:                         ; preds = %postload6687vector_func.i
  %.sum8738vector_func.i = add i64 %dim_0_vector_tid.i, 110606
  %663 = getelementptr float addrspace(1)* %4, i64 %.sum8738vector_func.i
  %exData4353vector_func.i = extractelement <16 x float> %call.i41.i, i32 14
  store float %exData4353vector_func.i, float addrspace(1)* %663, align 4
  br label %postload6690vector_func.i

postload6690vector_func.i:                        ; preds = %preload6689vector_func.i, %postload6687vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6692vector_func.i, label %postload6693vector_func.i

preload6692vector_func.i:                         ; preds = %postload6690vector_func.i
  %.sum8737vector_func.i = add i64 %dim_0_vector_tid.i, 110607
  %664 = getelementptr float addrspace(1)* %4, i64 %.sum8737vector_func.i
  %exData4356vector_func.i = extractelement <16 x float> %call.i41.i, i32 15
  store float %exData4356vector_func.i, float addrspace(1)* %664, align 4
  br label %postload6693vector_func.i

postload6693vector_func.i:                        ; preds = %preload6692vector_func.i, %postload6690vector_func.i
  %mul4011740vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000, float 0x40E6768140000000>
  %sub4021741vector_func.i = fsub <16 x float> <float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000, float 0x3FF9002160000000>, %mul4011740vector_func.i
  %mul4031742vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000, float 0x400E19F740000000>
  %add4041743vector_func.i = fadd <16 x float> %sub4021741vector_func.i, %mul4031742vector_func.i
  %mul.i1481744vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000, float 0x3D37BF8FA0000000>
  %add.i1491745vector_func.i = fadd <16 x float> %mul.i1481744vector_func.i, <float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000, float 0xBDF60D7F00000000>
  %mul1.i1501746vector_func.i = fmul <16 x float> %add.i1491745vector_func.i, %mul567vector_func.i
  %add2.i1511747vector_func.i = fadd <16 x float> %mul1.i1501746vector_func.i, <float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000, float 0x3E9F42AA40000000>
  %mul3.i1521748vector_func.i = fmul <16 x float> %add2.i1511747vector_func.i, %mul567vector_func.i
  %add4.i1531749vector_func.i = fadd <16 x float> %mul3.i1521748vector_func.i, <float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000, float 0x3F3FBF7D20000000>
  %mul5.i1541750vector_func.i = fmul <16 x float> %add4.i1531749vector_func.i, %mul567vector_func.i
  %add4061751vector_func.i = fadd <16 x float> %add4041743vector_func.i, %mul5.i1541750vector_func.i
  %call.i42.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4061751vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6695vector_func.i, label %postload6696vector_func.i

preload6695vector_func.i:                         ; preds = %postload6693vector_func.i
  %extract1753vector_func.i = add i64 %dim_0_vector_tid.i, 124416
  %665 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1753vector_func.i
  %exData4360vector_func.i = extractelement <16 x float> %call.i42.i, i32 0
  store float %exData4360vector_func.i, float addrspace(1)* %665, align 4
  br label %postload6696vector_func.i

postload6696vector_func.i:                        ; preds = %preload6695vector_func.i, %postload6693vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6698vector_func.i, label %postload6699vector_func.i

preload6698vector_func.i:                         ; preds = %postload6696vector_func.i
  %.sum8736vector_func.i = add i64 %dim_0_vector_tid.i, 124417
  %666 = getelementptr float addrspace(1)* %4, i64 %.sum8736vector_func.i
  %exData4363vector_func.i = extractelement <16 x float> %call.i42.i, i32 1
  store float %exData4363vector_func.i, float addrspace(1)* %666, align 4
  br label %postload6699vector_func.i

postload6699vector_func.i:                        ; preds = %preload6698vector_func.i, %postload6696vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6701vector_func.i, label %postload6702vector_func.i

preload6701vector_func.i:                         ; preds = %postload6699vector_func.i
  %.sum8735vector_func.i = add i64 %dim_0_vector_tid.i, 124418
  %667 = getelementptr float addrspace(1)* %4, i64 %.sum8735vector_func.i
  %exData4366vector_func.i = extractelement <16 x float> %call.i42.i, i32 2
  store float %exData4366vector_func.i, float addrspace(1)* %667, align 4
  br label %postload6702vector_func.i

postload6702vector_func.i:                        ; preds = %preload6701vector_func.i, %postload6699vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6704vector_func.i, label %postload6705vector_func.i

preload6704vector_func.i:                         ; preds = %postload6702vector_func.i
  %.sum8734vector_func.i = add i64 %dim_0_vector_tid.i, 124419
  %668 = getelementptr float addrspace(1)* %4, i64 %.sum8734vector_func.i
  %exData4369vector_func.i = extractelement <16 x float> %call.i42.i, i32 3
  store float %exData4369vector_func.i, float addrspace(1)* %668, align 4
  br label %postload6705vector_func.i

postload6705vector_func.i:                        ; preds = %preload6704vector_func.i, %postload6702vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6707vector_func.i, label %postload6708vector_func.i

preload6707vector_func.i:                         ; preds = %postload6705vector_func.i
  %.sum8733vector_func.i = add i64 %dim_0_vector_tid.i, 124420
  %669 = getelementptr float addrspace(1)* %4, i64 %.sum8733vector_func.i
  %exData4372vector_func.i = extractelement <16 x float> %call.i42.i, i32 4
  store float %exData4372vector_func.i, float addrspace(1)* %669, align 4
  br label %postload6708vector_func.i

postload6708vector_func.i:                        ; preds = %preload6707vector_func.i, %postload6705vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6710vector_func.i, label %postload6711vector_func.i

preload6710vector_func.i:                         ; preds = %postload6708vector_func.i
  %.sum8732vector_func.i = add i64 %dim_0_vector_tid.i, 124421
  %670 = getelementptr float addrspace(1)* %4, i64 %.sum8732vector_func.i
  %exData4375vector_func.i = extractelement <16 x float> %call.i42.i, i32 5
  store float %exData4375vector_func.i, float addrspace(1)* %670, align 4
  br label %postload6711vector_func.i

postload6711vector_func.i:                        ; preds = %preload6710vector_func.i, %postload6708vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6713vector_func.i, label %postload6714vector_func.i

preload6713vector_func.i:                         ; preds = %postload6711vector_func.i
  %.sum8731vector_func.i = add i64 %dim_0_vector_tid.i, 124422
  %671 = getelementptr float addrspace(1)* %4, i64 %.sum8731vector_func.i
  %exData4378vector_func.i = extractelement <16 x float> %call.i42.i, i32 6
  store float %exData4378vector_func.i, float addrspace(1)* %671, align 4
  br label %postload6714vector_func.i

postload6714vector_func.i:                        ; preds = %preload6713vector_func.i, %postload6711vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6716vector_func.i, label %postload6717vector_func.i

preload6716vector_func.i:                         ; preds = %postload6714vector_func.i
  %.sum8730vector_func.i = add i64 %dim_0_vector_tid.i, 124423
  %672 = getelementptr float addrspace(1)* %4, i64 %.sum8730vector_func.i
  %exData4381vector_func.i = extractelement <16 x float> %call.i42.i, i32 7
  store float %exData4381vector_func.i, float addrspace(1)* %672, align 4
  br label %postload6717vector_func.i

postload6717vector_func.i:                        ; preds = %preload6716vector_func.i, %postload6714vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6719vector_func.i, label %postload6720vector_func.i

preload6719vector_func.i:                         ; preds = %postload6717vector_func.i
  %.sum8729vector_func.i = add i64 %dim_0_vector_tid.i, 124424
  %673 = getelementptr float addrspace(1)* %4, i64 %.sum8729vector_func.i
  %exData4384vector_func.i = extractelement <16 x float> %call.i42.i, i32 8
  store float %exData4384vector_func.i, float addrspace(1)* %673, align 4
  br label %postload6720vector_func.i

postload6720vector_func.i:                        ; preds = %preload6719vector_func.i, %postload6717vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6722vector_func.i, label %postload6723vector_func.i

preload6722vector_func.i:                         ; preds = %postload6720vector_func.i
  %.sum8728vector_func.i = add i64 %dim_0_vector_tid.i, 124425
  %674 = getelementptr float addrspace(1)* %4, i64 %.sum8728vector_func.i
  %exData4387vector_func.i = extractelement <16 x float> %call.i42.i, i32 9
  store float %exData4387vector_func.i, float addrspace(1)* %674, align 4
  br label %postload6723vector_func.i

postload6723vector_func.i:                        ; preds = %preload6722vector_func.i, %postload6720vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6725vector_func.i, label %postload6726vector_func.i

preload6725vector_func.i:                         ; preds = %postload6723vector_func.i
  %.sum8727vector_func.i = add i64 %dim_0_vector_tid.i, 124426
  %675 = getelementptr float addrspace(1)* %4, i64 %.sum8727vector_func.i
  %exData4390vector_func.i = extractelement <16 x float> %call.i42.i, i32 10
  store float %exData4390vector_func.i, float addrspace(1)* %675, align 4
  br label %postload6726vector_func.i

postload6726vector_func.i:                        ; preds = %preload6725vector_func.i, %postload6723vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6728vector_func.i, label %postload6729vector_func.i

preload6728vector_func.i:                         ; preds = %postload6726vector_func.i
  %.sum8726vector_func.i = add i64 %dim_0_vector_tid.i, 124427
  %676 = getelementptr float addrspace(1)* %4, i64 %.sum8726vector_func.i
  %exData4393vector_func.i = extractelement <16 x float> %call.i42.i, i32 11
  store float %exData4393vector_func.i, float addrspace(1)* %676, align 4
  br label %postload6729vector_func.i

postload6729vector_func.i:                        ; preds = %preload6728vector_func.i, %postload6726vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6731vector_func.i, label %postload6732vector_func.i

preload6731vector_func.i:                         ; preds = %postload6729vector_func.i
  %.sum8725vector_func.i = add i64 %dim_0_vector_tid.i, 124428
  %677 = getelementptr float addrspace(1)* %4, i64 %.sum8725vector_func.i
  %exData4396vector_func.i = extractelement <16 x float> %call.i42.i, i32 12
  store float %exData4396vector_func.i, float addrspace(1)* %677, align 4
  br label %postload6732vector_func.i

postload6732vector_func.i:                        ; preds = %preload6731vector_func.i, %postload6729vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6734vector_func.i, label %postload6735vector_func.i

preload6734vector_func.i:                         ; preds = %postload6732vector_func.i
  %.sum8724vector_func.i = add i64 %dim_0_vector_tid.i, 124429
  %678 = getelementptr float addrspace(1)* %4, i64 %.sum8724vector_func.i
  %exData4399vector_func.i = extractelement <16 x float> %call.i42.i, i32 13
  store float %exData4399vector_func.i, float addrspace(1)* %678, align 4
  br label %postload6735vector_func.i

postload6735vector_func.i:                        ; preds = %preload6734vector_func.i, %postload6732vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6737vector_func.i, label %postload6738vector_func.i

preload6737vector_func.i:                         ; preds = %postload6735vector_func.i
  %.sum8723vector_func.i = add i64 %dim_0_vector_tid.i, 124430
  %679 = getelementptr float addrspace(1)* %4, i64 %.sum8723vector_func.i
  %exData4402vector_func.i = extractelement <16 x float> %call.i42.i, i32 14
  store float %exData4402vector_func.i, float addrspace(1)* %679, align 4
  br label %postload6738vector_func.i

postload6738vector_func.i:                        ; preds = %preload6737vector_func.i, %postload6735vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6740vector_func.i, label %postload6741vector_func.i

preload6740vector_func.i:                         ; preds = %postload6738vector_func.i
  %.sum8722vector_func.i = add i64 %dim_0_vector_tid.i, 124431
  %680 = getelementptr float addrspace(1)* %4, i64 %.sum8722vector_func.i
  %exData4405vector_func.i = extractelement <16 x float> %call.i42.i, i32 15
  store float %exData4405vector_func.i, float addrspace(1)* %680, align 4
  br label %postload6741vector_func.i

postload6741vector_func.i:                        ; preds = %preload6740vector_func.i, %postload6738vector_func.i
  %mul4111770vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000, float 0x40E8A81A20000000>
  %sub4121771vector_func.i = fsub <16 x float> <float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000, float 0xBFE89C9F60000000>, %mul4111770vector_func.i
  %mul4131772vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000, float 0x4010CB5EE0000000>
  %add4141773vector_func.i = fadd <16 x float> %sub4121771vector_func.i, %mul4131772vector_func.i
  %mul.i1411774vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000, float 0x3D3B58ED20000000>
  %add.i1421775vector_func.i = fadd <16 x float> %mul.i1411774vector_func.i, <float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000, float 0xBE03267920000000>
  %mul1.i1431776vector_func.i = fmul <16 x float> %add.i1421775vector_func.i, %mul567vector_func.i
  %add2.i1441777vector_func.i = fadd <16 x float> %mul1.i1431776vector_func.i, <float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000, float 0x3EB7056240000000>
  %mul3.i1451778vector_func.i = fmul <16 x float> %add2.i1441777vector_func.i, %mul567vector_func.i
  %add4.i1461779vector_func.i = fadd <16 x float> %mul3.i1451778vector_func.i, <float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000, float 0xBF53632660000000>
  %mul5.i1471780vector_func.i = fmul <16 x float> %add4.i1461779vector_func.i, %mul567vector_func.i
  %add4161781vector_func.i = fadd <16 x float> %add4141773vector_func.i, %mul5.i1471780vector_func.i
  %call.i43.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4161781vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6743vector_func.i, label %postload6744vector_func.i

preload6743vector_func.i:                         ; preds = %postload6741vector_func.i
  %extract1783vector_func.i = add i64 %dim_0_vector_tid.i, 138240
  %681 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1783vector_func.i
  %exData4409vector_func.i = extractelement <16 x float> %call.i43.i, i32 0
  store float %exData4409vector_func.i, float addrspace(1)* %681, align 4
  br label %postload6744vector_func.i

postload6744vector_func.i:                        ; preds = %preload6743vector_func.i, %postload6741vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6746vector_func.i, label %postload6747vector_func.i

preload6746vector_func.i:                         ; preds = %postload6744vector_func.i
  %.sum8721vector_func.i = add i64 %dim_0_vector_tid.i, 138241
  %682 = getelementptr float addrspace(1)* %4, i64 %.sum8721vector_func.i
  %exData4412vector_func.i = extractelement <16 x float> %call.i43.i, i32 1
  store float %exData4412vector_func.i, float addrspace(1)* %682, align 4
  br label %postload6747vector_func.i

postload6747vector_func.i:                        ; preds = %preload6746vector_func.i, %postload6744vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6749vector_func.i, label %postload6750vector_func.i

preload6749vector_func.i:                         ; preds = %postload6747vector_func.i
  %.sum8720vector_func.i = add i64 %dim_0_vector_tid.i, 138242
  %683 = getelementptr float addrspace(1)* %4, i64 %.sum8720vector_func.i
  %exData4415vector_func.i = extractelement <16 x float> %call.i43.i, i32 2
  store float %exData4415vector_func.i, float addrspace(1)* %683, align 4
  br label %postload6750vector_func.i

postload6750vector_func.i:                        ; preds = %preload6749vector_func.i, %postload6747vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6752vector_func.i, label %postload6753vector_func.i

preload6752vector_func.i:                         ; preds = %postload6750vector_func.i
  %.sum8719vector_func.i = add i64 %dim_0_vector_tid.i, 138243
  %684 = getelementptr float addrspace(1)* %4, i64 %.sum8719vector_func.i
  %exData4418vector_func.i = extractelement <16 x float> %call.i43.i, i32 3
  store float %exData4418vector_func.i, float addrspace(1)* %684, align 4
  br label %postload6753vector_func.i

postload6753vector_func.i:                        ; preds = %preload6752vector_func.i, %postload6750vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6755vector_func.i, label %postload6756vector_func.i

preload6755vector_func.i:                         ; preds = %postload6753vector_func.i
  %.sum8718vector_func.i = add i64 %dim_0_vector_tid.i, 138244
  %685 = getelementptr float addrspace(1)* %4, i64 %.sum8718vector_func.i
  %exData4421vector_func.i = extractelement <16 x float> %call.i43.i, i32 4
  store float %exData4421vector_func.i, float addrspace(1)* %685, align 4
  br label %postload6756vector_func.i

postload6756vector_func.i:                        ; preds = %preload6755vector_func.i, %postload6753vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6758vector_func.i, label %postload6759vector_func.i

preload6758vector_func.i:                         ; preds = %postload6756vector_func.i
  %.sum8717vector_func.i = add i64 %dim_0_vector_tid.i, 138245
  %686 = getelementptr float addrspace(1)* %4, i64 %.sum8717vector_func.i
  %exData4424vector_func.i = extractelement <16 x float> %call.i43.i, i32 5
  store float %exData4424vector_func.i, float addrspace(1)* %686, align 4
  br label %postload6759vector_func.i

postload6759vector_func.i:                        ; preds = %preload6758vector_func.i, %postload6756vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6761vector_func.i, label %postload6762vector_func.i

preload6761vector_func.i:                         ; preds = %postload6759vector_func.i
  %.sum8716vector_func.i = add i64 %dim_0_vector_tid.i, 138246
  %687 = getelementptr float addrspace(1)* %4, i64 %.sum8716vector_func.i
  %exData4427vector_func.i = extractelement <16 x float> %call.i43.i, i32 6
  store float %exData4427vector_func.i, float addrspace(1)* %687, align 4
  br label %postload6762vector_func.i

postload6762vector_func.i:                        ; preds = %preload6761vector_func.i, %postload6759vector_func.i
  br i1 %exmask3939vector_func.i, label %preload6764vector_func.i, label %postload6765vector_func.i

preload6764vector_func.i:                         ; preds = %postload6762vector_func.i
  %.sum8715vector_func.i = add i64 %dim_0_vector_tid.i, 138247
  %688 = getelementptr float addrspace(1)* %4, i64 %.sum8715vector_func.i
  %exData4430vector_func.i = extractelement <16 x float> %call.i43.i, i32 7
  store float %exData4430vector_func.i, float addrspace(1)* %688, align 4
  br label %postload6765vector_func.i

postload6765vector_func.i:                        ; preds = %preload6764vector_func.i, %postload6762vector_func.i
  br i1 %exmask3942vector_func.i, label %preload6767vector_func.i, label %postload6768vector_func.i

preload6767vector_func.i:                         ; preds = %postload6765vector_func.i
  %.sum8714vector_func.i = add i64 %dim_0_vector_tid.i, 138248
  %689 = getelementptr float addrspace(1)* %4, i64 %.sum8714vector_func.i
  %exData4433vector_func.i = extractelement <16 x float> %call.i43.i, i32 8
  store float %exData4433vector_func.i, float addrspace(1)* %689, align 4
  br label %postload6768vector_func.i

postload6768vector_func.i:                        ; preds = %preload6767vector_func.i, %postload6765vector_func.i
  br i1 %exmask3945vector_func.i, label %preload6770vector_func.i, label %postload6771vector_func.i

preload6770vector_func.i:                         ; preds = %postload6768vector_func.i
  %.sum8713vector_func.i = add i64 %dim_0_vector_tid.i, 138249
  %690 = getelementptr float addrspace(1)* %4, i64 %.sum8713vector_func.i
  %exData4436vector_func.i = extractelement <16 x float> %call.i43.i, i32 9
  store float %exData4436vector_func.i, float addrspace(1)* %690, align 4
  br label %postload6771vector_func.i

postload6771vector_func.i:                        ; preds = %preload6770vector_func.i, %postload6768vector_func.i
  br i1 %exmask3948vector_func.i, label %preload6773vector_func.i, label %postload6774vector_func.i

preload6773vector_func.i:                         ; preds = %postload6771vector_func.i
  %.sum8712vector_func.i = add i64 %dim_0_vector_tid.i, 138250
  %691 = getelementptr float addrspace(1)* %4, i64 %.sum8712vector_func.i
  %exData4439vector_func.i = extractelement <16 x float> %call.i43.i, i32 10
  store float %exData4439vector_func.i, float addrspace(1)* %691, align 4
  br label %postload6774vector_func.i

postload6774vector_func.i:                        ; preds = %preload6773vector_func.i, %postload6771vector_func.i
  br i1 %exmask3951vector_func.i, label %preload6776vector_func.i, label %postload6777vector_func.i

preload6776vector_func.i:                         ; preds = %postload6774vector_func.i
  %.sum8711vector_func.i = add i64 %dim_0_vector_tid.i, 138251
  %692 = getelementptr float addrspace(1)* %4, i64 %.sum8711vector_func.i
  %exData4442vector_func.i = extractelement <16 x float> %call.i43.i, i32 11
  store float %exData4442vector_func.i, float addrspace(1)* %692, align 4
  br label %postload6777vector_func.i

postload6777vector_func.i:                        ; preds = %preload6776vector_func.i, %postload6774vector_func.i
  br i1 %exmask3954vector_func.i, label %preload6779vector_func.i, label %postload6780vector_func.i

preload6779vector_func.i:                         ; preds = %postload6777vector_func.i
  %.sum8710vector_func.i = add i64 %dim_0_vector_tid.i, 138252
  %693 = getelementptr float addrspace(1)* %4, i64 %.sum8710vector_func.i
  %exData4445vector_func.i = extractelement <16 x float> %call.i43.i, i32 12
  store float %exData4445vector_func.i, float addrspace(1)* %693, align 4
  br label %postload6780vector_func.i

postload6780vector_func.i:                        ; preds = %preload6779vector_func.i, %postload6777vector_func.i
  br i1 %exmask3957vector_func.i, label %preload6782vector_func.i, label %postload6783vector_func.i

preload6782vector_func.i:                         ; preds = %postload6780vector_func.i
  %.sum8709vector_func.i = add i64 %dim_0_vector_tid.i, 138253
  %694 = getelementptr float addrspace(1)* %4, i64 %.sum8709vector_func.i
  %exData4448vector_func.i = extractelement <16 x float> %call.i43.i, i32 13
  store float %exData4448vector_func.i, float addrspace(1)* %694, align 4
  br label %postload6783vector_func.i

postload6783vector_func.i:                        ; preds = %preload6782vector_func.i, %postload6780vector_func.i
  br i1 %exmask3960vector_func.i, label %preload6785vector_func.i, label %postload6786vector_func.i

preload6785vector_func.i:                         ; preds = %postload6783vector_func.i
  %.sum8708vector_func.i = add i64 %dim_0_vector_tid.i, 138254
  %695 = getelementptr float addrspace(1)* %4, i64 %.sum8708vector_func.i
  %exData4451vector_func.i = extractelement <16 x float> %call.i43.i, i32 14
  store float %exData4451vector_func.i, float addrspace(1)* %695, align 4
  br label %postload6786vector_func.i

postload6786vector_func.i:                        ; preds = %preload6785vector_func.i, %postload6783vector_func.i
  br i1 %exmask3963vector_func.i, label %preload6788vector_func.i, label %postload6789vector_func.i

preload6788vector_func.i:                         ; preds = %postload6786vector_func.i
  %.sum8707vector_func.i = add i64 %dim_0_vector_tid.i, 138255
  %696 = getelementptr float addrspace(1)* %4, i64 %.sum8707vector_func.i
  %exData4454vector_func.i = extractelement <16 x float> %call.i43.i, i32 15
  store float %exData4454vector_func.i, float addrspace(1)* %696, align 4
  br label %postload6789vector_func.i

postload6789vector_func.i:                        ; preds = %preload6788vector_func.i, %postload6786vector_func.i
  %mul4211800vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000, float 0x40D00F3FE0000000>
  %sub4221801vector_func.i = fsub <16 x float> <float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000, float 0x3FF9AC4BA0000000>, %mul4211800vector_func.i
  %mul4231802vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000, float 0x400D638360000000>
  %add4241803vector_func.i = fadd <16 x float> %sub4221801vector_func.i, %mul4231802vector_func.i
  %mul.i1341804vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000, float 0x3D41E69B20000000>
  %add.i1351805vector_func.i = fadd <16 x float> %mul.i1341804vector_func.i, <float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000, float 0xBE03AC9FC0000000>
  %mul1.i1361806vector_func.i = fmul <16 x float> %add.i1351805vector_func.i, %mul567vector_func.i
  %add2.i1371807vector_func.i = fadd <16 x float> %mul1.i1361806vector_func.i, <float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000, float 0x3EB005D9A0000000>
  %mul3.i1381808vector_func.i = fmul <16 x float> %add2.i1371807vector_func.i, %mul567vector_func.i
  %add4.i1391809vector_func.i = fadd <16 x float> %mul3.i1381808vector_func.i, <float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000, float 0x3F50794580000000>
  %mul5.i1401810vector_func.i = fmul <16 x float> %add4.i1391809vector_func.i, %mul567vector_func.i
  %add4261811vector_func.i = fadd <16 x float> %add4241803vector_func.i, %mul5.i1401810vector_func.i
  %call.i44.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4261811vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload6791vector_func.i, label %postload6792vector_func.i

preload6791vector_func.i:                         ; preds = %postload6789vector_func.i
  %extract1813vector_func.i = add i64 %dim_0_vector_tid.i, 152064
  %697 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1813vector_func.i
  %exData4458vector_func.i = extractelement <16 x float> %call.i44.i, i32 0
  store float %exData4458vector_func.i, float addrspace(1)* %697, align 4
  br label %postload6792vector_func.i

postload6792vector_func.i:                        ; preds = %preload6791vector_func.i, %postload6789vector_func.i
  br i1 %exmask3921vector_func.i, label %preload6794vector_func.i, label %postload6795vector_func.i

preload6794vector_func.i:                         ; preds = %postload6792vector_func.i
  %.sum8706vector_func.i = add i64 %dim_0_vector_tid.i, 152065
  %698 = getelementptr float addrspace(1)* %4, i64 %.sum8706vector_func.i
  %exData4461vector_func.i = extractelement <16 x float> %call.i44.i, i32 1
  store float %exData4461vector_func.i, float addrspace(1)* %698, align 4
  br label %postload6795vector_func.i

postload6795vector_func.i:                        ; preds = %preload6794vector_func.i, %postload6792vector_func.i
  br i1 %exmask3924vector_func.i, label %preload6797vector_func.i, label %postload6798vector_func.i

preload6797vector_func.i:                         ; preds = %postload6795vector_func.i
  %.sum8705vector_func.i = add i64 %dim_0_vector_tid.i, 152066
  %699 = getelementptr float addrspace(1)* %4, i64 %.sum8705vector_func.i
  %exData4464vector_func.i = extractelement <16 x float> %call.i44.i, i32 2
  store float %exData4464vector_func.i, float addrspace(1)* %699, align 4
  br label %postload6798vector_func.i

postload6798vector_func.i:                        ; preds = %preload6797vector_func.i, %postload6795vector_func.i
  br i1 %exmask3927vector_func.i, label %preload6800vector_func.i, label %postload6801vector_func.i

preload6800vector_func.i:                         ; preds = %postload6798vector_func.i
  %.sum8704vector_func.i = add i64 %dim_0_vector_tid.i, 152067
  %700 = getelementptr float addrspace(1)* %4, i64 %.sum8704vector_func.i
  %exData4467vector_func.i = extractelement <16 x float> %call.i44.i, i32 3
  store float %exData4467vector_func.i, float addrspace(1)* %700, align 4
  br label %postload6801vector_func.i

postload6801vector_func.i:                        ; preds = %preload6800vector_func.i, %postload6798vector_func.i
  br i1 %exmask3930vector_func.i, label %preload6803vector_func.i, label %postload6804vector_func.i

preload6803vector_func.i:                         ; preds = %postload6801vector_func.i
  %.sum8703vector_func.i = add i64 %dim_0_vector_tid.i, 152068
  %701 = getelementptr float addrspace(1)* %4, i64 %.sum8703vector_func.i
  %exData4470vector_func.i = extractelement <16 x float> %call.i44.i, i32 4
  store float %exData4470vector_func.i, float addrspace(1)* %701, align 4
  br label %postload6804vector_func.i

postload6804vector_func.i:                        ; preds = %preload6803vector_func.i, %postload6801vector_func.i
  br i1 %exmask3933vector_func.i, label %preload6806vector_func.i, label %postload6807vector_func.i

preload6806vector_func.i:                         ; preds = %postload6804vector_func.i
  %.sum8702vector_func.i = add i64 %dim_0_vector_tid.i, 152069
  %702 = getelementptr float addrspace(1)* %4, i64 %.sum8702vector_func.i
  %exData4473vector_func.i = extractelement <16 x float> %call.i44.i, i32 5
  store float %exData4473vector_func.i, float addrspace(1)* %702, align 4
  br label %postload6807vector_func.i

postload6807vector_func.i:                        ; preds = %preload6806vector_func.i, %postload6804vector_func.i
  br i1 %exmask3936vector_func.i, label %preload6809vector_func.i, label %postload6810vector_func.i

preload6809vector_func.i:                         ; preds = %postload6807vector_func.i
  %.sum8701vector_func.i = add i64 %dim_0_vector_tid.i, 152070
  %703 = getelementptr float addrspace(1)* %4, i64 %.sum8701vector_func.i
  %exData4476vector_func.i = extractelement <16 x float> %call.i44.i, i32 6
  store float %exData4476vector_func.i, float addrspace(1)* %703, align 4
  br label %postload6810vector_func.i

postload6810vector_func.i:                        ; preds = %preload6809vector_func.i, %postload6807vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7706vector_func.i, label %postload7707vector_func.i

preload7706vector_func.i:                         ; preds = %postload6810vector_func.i
  %.sum8700vector_func.i = add i64 %dim_0_vector_tid.i, 152071
  %704 = getelementptr float addrspace(1)* %4, i64 %.sum8700vector_func.i
  %exData4479vector_func.i = extractelement <16 x float> %call.i44.i, i32 7
  store float %exData4479vector_func.i, float addrspace(1)* %704, align 4
  br label %postload7707vector_func.i

postload7707vector_func.i:                        ; preds = %preload7706vector_func.i, %postload6810vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7709vector_func.i, label %postload7710vector_func.i

preload7709vector_func.i:                         ; preds = %postload7707vector_func.i
  %.sum8699vector_func.i = add i64 %dim_0_vector_tid.i, 152072
  %705 = getelementptr float addrspace(1)* %4, i64 %.sum8699vector_func.i
  %exData4482vector_func.i = extractelement <16 x float> %call.i44.i, i32 8
  store float %exData4482vector_func.i, float addrspace(1)* %705, align 4
  br label %postload7710vector_func.i

postload7710vector_func.i:                        ; preds = %preload7709vector_func.i, %postload7707vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7712vector_func.i, label %postload7713vector_func.i

preload7712vector_func.i:                         ; preds = %postload7710vector_func.i
  %.sum8698vector_func.i = add i64 %dim_0_vector_tid.i, 152073
  %706 = getelementptr float addrspace(1)* %4, i64 %.sum8698vector_func.i
  %exData4485vector_func.i = extractelement <16 x float> %call.i44.i, i32 9
  store float %exData4485vector_func.i, float addrspace(1)* %706, align 4
  br label %postload7713vector_func.i

postload7713vector_func.i:                        ; preds = %preload7712vector_func.i, %postload7710vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7715vector_func.i, label %postload7716vector_func.i

preload7715vector_func.i:                         ; preds = %postload7713vector_func.i
  %.sum8697vector_func.i = add i64 %dim_0_vector_tid.i, 152074
  %707 = getelementptr float addrspace(1)* %4, i64 %.sum8697vector_func.i
  %exData4488vector_func.i = extractelement <16 x float> %call.i44.i, i32 10
  store float %exData4488vector_func.i, float addrspace(1)* %707, align 4
  br label %postload7716vector_func.i

postload7716vector_func.i:                        ; preds = %preload7715vector_func.i, %postload7713vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7718vector_func.i, label %postload7719vector_func.i

preload7718vector_func.i:                         ; preds = %postload7716vector_func.i
  %.sum8696vector_func.i = add i64 %dim_0_vector_tid.i, 152075
  %708 = getelementptr float addrspace(1)* %4, i64 %.sum8696vector_func.i
  %exData4491vector_func.i = extractelement <16 x float> %call.i44.i, i32 11
  store float %exData4491vector_func.i, float addrspace(1)* %708, align 4
  br label %postload7719vector_func.i

postload7719vector_func.i:                        ; preds = %preload7718vector_func.i, %postload7716vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7721vector_func.i, label %postload7722vector_func.i

preload7721vector_func.i:                         ; preds = %postload7719vector_func.i
  %.sum8695vector_func.i = add i64 %dim_0_vector_tid.i, 152076
  %709 = getelementptr float addrspace(1)* %4, i64 %.sum8695vector_func.i
  %exData4494vector_func.i = extractelement <16 x float> %call.i44.i, i32 12
  store float %exData4494vector_func.i, float addrspace(1)* %709, align 4
  br label %postload7722vector_func.i

postload7722vector_func.i:                        ; preds = %preload7721vector_func.i, %postload7719vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7724vector_func.i, label %postload7725vector_func.i

preload7724vector_func.i:                         ; preds = %postload7722vector_func.i
  %.sum8694vector_func.i = add i64 %dim_0_vector_tid.i, 152077
  %710 = getelementptr float addrspace(1)* %4, i64 %.sum8694vector_func.i
  %exData4497vector_func.i = extractelement <16 x float> %call.i44.i, i32 13
  store float %exData4497vector_func.i, float addrspace(1)* %710, align 4
  br label %postload7725vector_func.i

postload7725vector_func.i:                        ; preds = %preload7724vector_func.i, %postload7722vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7727vector_func.i, label %postload7728vector_func.i

preload7727vector_func.i:                         ; preds = %postload7725vector_func.i
  %.sum8693vector_func.i = add i64 %dim_0_vector_tid.i, 152078
  %711 = getelementptr float addrspace(1)* %4, i64 %.sum8693vector_func.i
  %exData4500vector_func.i = extractelement <16 x float> %call.i44.i, i32 14
  store float %exData4500vector_func.i, float addrspace(1)* %711, align 4
  br label %postload7728vector_func.i

postload7728vector_func.i:                        ; preds = %preload7727vector_func.i, %postload7725vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7730vector_func.i, label %postload7731vector_func.i

preload7730vector_func.i:                         ; preds = %postload7728vector_func.i
  %.sum8692vector_func.i = add i64 %dim_0_vector_tid.i, 152079
  %712 = getelementptr float addrspace(1)* %4, i64 %.sum8692vector_func.i
  %exData4503vector_func.i = extractelement <16 x float> %call.i44.i, i32 15
  store float %exData4503vector_func.i, float addrspace(1)* %712, align 4
  br label %postload7731vector_func.i

postload7731vector_func.i:                        ; preds = %preload7730vector_func.i, %postload7728vector_func.i
  %mul4311830vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000, float 0x40C40352E0000000>
  %add4321831vector_func.i = fadd <16 x float> %mul4311830vector_func.i, <float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000, float 0xC01290B1E0000000>
  %mul4331832vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000, float 0x4014997920000000>
  %add4341833vector_func.i = fadd <16 x float> %add4321831vector_func.i, %mul4331832vector_func.i
  %mul.i1271834vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000, float 0x3D6D533A80000000>
  %add.i1281835vector_func.i = fadd <16 x float> %mul.i1271834vector_func.i, <float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000, float 0xBE31598140000000>
  %mul1.i1291836vector_func.i = fmul <16 x float> %add.i1281835vector_func.i, %mul567vector_func.i
  %add2.i1301837vector_func.i = fadd <16 x float> %mul1.i1291836vector_func.i, <float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000, float 0x3EE1308EA0000000>
  %mul3.i1311838vector_func.i = fmul <16 x float> %add2.i1301837vector_func.i, %mul567vector_func.i
  %add4.i1321839vector_func.i = fadd <16 x float> %mul3.i1311838vector_func.i, <float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000, float 0xBF7BFF87C0000000>
  %mul5.i1331840vector_func.i = fmul <16 x float> %add4.i1321839vector_func.i, %mul567vector_func.i
  %add4361841vector_func.i = fadd <16 x float> %add4341833vector_func.i, %mul5.i1331840vector_func.i
  %call.i45.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4361841vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7733vector_func.i, label %postload7734vector_func.i

preload7733vector_func.i:                         ; preds = %postload7731vector_func.i
  %extract1843vector_func.i = add i64 %dim_0_vector_tid.i, 165888
  %713 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1843vector_func.i
  %exData4507vector_func.i = extractelement <16 x float> %call.i45.i, i32 0
  store float %exData4507vector_func.i, float addrspace(1)* %713, align 4
  br label %postload7734vector_func.i

postload7734vector_func.i:                        ; preds = %preload7733vector_func.i, %postload7731vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7736vector_func.i, label %postload7737vector_func.i

preload7736vector_func.i:                         ; preds = %postload7734vector_func.i
  %.sum8691vector_func.i = add i64 %dim_0_vector_tid.i, 165889
  %714 = getelementptr float addrspace(1)* %4, i64 %.sum8691vector_func.i
  %exData4510vector_func.i = extractelement <16 x float> %call.i45.i, i32 1
  store float %exData4510vector_func.i, float addrspace(1)* %714, align 4
  br label %postload7737vector_func.i

postload7737vector_func.i:                        ; preds = %preload7736vector_func.i, %postload7734vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7739vector_func.i, label %postload7740vector_func.i

preload7739vector_func.i:                         ; preds = %postload7737vector_func.i
  %.sum8690vector_func.i = add i64 %dim_0_vector_tid.i, 165890
  %715 = getelementptr float addrspace(1)* %4, i64 %.sum8690vector_func.i
  %exData4513vector_func.i = extractelement <16 x float> %call.i45.i, i32 2
  store float %exData4513vector_func.i, float addrspace(1)* %715, align 4
  br label %postload7740vector_func.i

postload7740vector_func.i:                        ; preds = %preload7739vector_func.i, %postload7737vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7742vector_func.i, label %postload7743vector_func.i

preload7742vector_func.i:                         ; preds = %postload7740vector_func.i
  %.sum8689vector_func.i = add i64 %dim_0_vector_tid.i, 165891
  %716 = getelementptr float addrspace(1)* %4, i64 %.sum8689vector_func.i
  %exData4516vector_func.i = extractelement <16 x float> %call.i45.i, i32 3
  store float %exData4516vector_func.i, float addrspace(1)* %716, align 4
  br label %postload7743vector_func.i

postload7743vector_func.i:                        ; preds = %preload7742vector_func.i, %postload7740vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7745vector_func.i, label %postload7746vector_func.i

preload7745vector_func.i:                         ; preds = %postload7743vector_func.i
  %.sum8688vector_func.i = add i64 %dim_0_vector_tid.i, 165892
  %717 = getelementptr float addrspace(1)* %4, i64 %.sum8688vector_func.i
  %exData4519vector_func.i = extractelement <16 x float> %call.i45.i, i32 4
  store float %exData4519vector_func.i, float addrspace(1)* %717, align 4
  br label %postload7746vector_func.i

postload7746vector_func.i:                        ; preds = %preload7745vector_func.i, %postload7743vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7748vector_func.i, label %postload7749vector_func.i

preload7748vector_func.i:                         ; preds = %postload7746vector_func.i
  %.sum8687vector_func.i = add i64 %dim_0_vector_tid.i, 165893
  %718 = getelementptr float addrspace(1)* %4, i64 %.sum8687vector_func.i
  %exData4522vector_func.i = extractelement <16 x float> %call.i45.i, i32 5
  store float %exData4522vector_func.i, float addrspace(1)* %718, align 4
  br label %postload7749vector_func.i

postload7749vector_func.i:                        ; preds = %preload7748vector_func.i, %postload7746vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7469vector_func.i, label %postload7470vector_func.i

preload7469vector_func.i:                         ; preds = %postload7749vector_func.i
  %.sum8686vector_func.i = add i64 %dim_0_vector_tid.i, 165894
  %719 = getelementptr float addrspace(1)* %4, i64 %.sum8686vector_func.i
  %exData4525vector_func.i = extractelement <16 x float> %call.i45.i, i32 6
  store float %exData4525vector_func.i, float addrspace(1)* %719, align 4
  br label %postload7470vector_func.i

postload7470vector_func.i:                        ; preds = %preload7469vector_func.i, %postload7749vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7472vector_func.i, label %postload7473vector_func.i

preload7472vector_func.i:                         ; preds = %postload7470vector_func.i
  %.sum8685vector_func.i = add i64 %dim_0_vector_tid.i, 165895
  %720 = getelementptr float addrspace(1)* %4, i64 %.sum8685vector_func.i
  %exData4528vector_func.i = extractelement <16 x float> %call.i45.i, i32 7
  store float %exData4528vector_func.i, float addrspace(1)* %720, align 4
  br label %postload7473vector_func.i

postload7473vector_func.i:                        ; preds = %preload7472vector_func.i, %postload7470vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7475vector_func.i, label %postload7476vector_func.i

preload7475vector_func.i:                         ; preds = %postload7473vector_func.i
  %.sum8684vector_func.i = add i64 %dim_0_vector_tid.i, 165896
  %721 = getelementptr float addrspace(1)* %4, i64 %.sum8684vector_func.i
  %exData4531vector_func.i = extractelement <16 x float> %call.i45.i, i32 8
  store float %exData4531vector_func.i, float addrspace(1)* %721, align 4
  br label %postload7476vector_func.i

postload7476vector_func.i:                        ; preds = %preload7475vector_func.i, %postload7473vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7478vector_func.i, label %postload7479vector_func.i

preload7478vector_func.i:                         ; preds = %postload7476vector_func.i
  %.sum8683vector_func.i = add i64 %dim_0_vector_tid.i, 165897
  %722 = getelementptr float addrspace(1)* %4, i64 %.sum8683vector_func.i
  %exData4534vector_func.i = extractelement <16 x float> %call.i45.i, i32 9
  store float %exData4534vector_func.i, float addrspace(1)* %722, align 4
  br label %postload7479vector_func.i

postload7479vector_func.i:                        ; preds = %preload7478vector_func.i, %postload7476vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7481vector_func.i, label %postload7482vector_func.i

preload7481vector_func.i:                         ; preds = %postload7479vector_func.i
  %.sum8682vector_func.i = add i64 %dim_0_vector_tid.i, 165898
  %723 = getelementptr float addrspace(1)* %4, i64 %.sum8682vector_func.i
  %exData4537vector_func.i = extractelement <16 x float> %call.i45.i, i32 10
  store float %exData4537vector_func.i, float addrspace(1)* %723, align 4
  br label %postload7482vector_func.i

postload7482vector_func.i:                        ; preds = %preload7481vector_func.i, %postload7479vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7484vector_func.i, label %postload7485vector_func.i

preload7484vector_func.i:                         ; preds = %postload7482vector_func.i
  %.sum8681vector_func.i = add i64 %dim_0_vector_tid.i, 165899
  %724 = getelementptr float addrspace(1)* %4, i64 %.sum8681vector_func.i
  %exData4540vector_func.i = extractelement <16 x float> %call.i45.i, i32 11
  store float %exData4540vector_func.i, float addrspace(1)* %724, align 4
  br label %postload7485vector_func.i

postload7485vector_func.i:                        ; preds = %preload7484vector_func.i, %postload7482vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7487vector_func.i, label %postload7488vector_func.i

preload7487vector_func.i:                         ; preds = %postload7485vector_func.i
  %.sum8680vector_func.i = add i64 %dim_0_vector_tid.i, 165900
  %725 = getelementptr float addrspace(1)* %4, i64 %.sum8680vector_func.i
  %exData4543vector_func.i = extractelement <16 x float> %call.i45.i, i32 12
  store float %exData4543vector_func.i, float addrspace(1)* %725, align 4
  br label %postload7488vector_func.i

postload7488vector_func.i:                        ; preds = %preload7487vector_func.i, %postload7485vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7490vector_func.i, label %postload7491vector_func.i

preload7490vector_func.i:                         ; preds = %postload7488vector_func.i
  %.sum8679vector_func.i = add i64 %dim_0_vector_tid.i, 165901
  %726 = getelementptr float addrspace(1)* %4, i64 %.sum8679vector_func.i
  %exData4546vector_func.i = extractelement <16 x float> %call.i45.i, i32 13
  store float %exData4546vector_func.i, float addrspace(1)* %726, align 4
  br label %postload7491vector_func.i

postload7491vector_func.i:                        ; preds = %preload7490vector_func.i, %postload7488vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7493vector_func.i, label %postload7494vector_func.i

preload7493vector_func.i:                         ; preds = %postload7491vector_func.i
  %.sum8678vector_func.i = add i64 %dim_0_vector_tid.i, 165902
  %727 = getelementptr float addrspace(1)* %4, i64 %.sum8678vector_func.i
  %exData4549vector_func.i = extractelement <16 x float> %call.i45.i, i32 14
  store float %exData4549vector_func.i, float addrspace(1)* %727, align 4
  br label %postload7494vector_func.i

postload7494vector_func.i:                        ; preds = %preload7493vector_func.i, %postload7491vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7496vector_func.i, label %postload7497vector_func.i

preload7496vector_func.i:                         ; preds = %postload7494vector_func.i
  %.sum8677vector_func.i = add i64 %dim_0_vector_tid.i, 165903
  %728 = getelementptr float addrspace(1)* %4, i64 %.sum8677vector_func.i
  %exData4552vector_func.i = extractelement <16 x float> %call.i45.i, i32 15
  store float %exData4552vector_func.i, float addrspace(1)* %728, align 4
  br label %postload7497vector_func.i

postload7497vector_func.i:                        ; preds = %preload7496vector_func.i, %postload7494vector_func.i
  %mul4411860vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000, float 0x40CC040B00000000>
  %add4421861vector_func.i = fadd <16 x float> %mul4411860vector_func.i, <float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000, float 0x400C1138E0000000>
  %mul4431862vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000, float 0x400CA2E280000000>
  %add4441863vector_func.i = fadd <16 x float> %add4421861vector_func.i, %mul4431862vector_func.i
  %mul.i1201864vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000, float 0xBD297510C0000000>
  %add.i1211865vector_func.i = fadd <16 x float> %mul.i1201864vector_func.i, <float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000, float 0x3DD4C6BD20000000>
  %mul1.i1221866vector_func.i = fmul <16 x float> %add.i1211865vector_func.i, %mul567vector_func.i
  %add2.i1231867vector_func.i = fadd <16 x float> %mul1.i1221866vector_func.i, <float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000, float 0x3E86BEE9A0000000>
  %mul3.i1241868vector_func.i = fmul <16 x float> %add2.i1231867vector_func.i, %mul567vector_func.i
  %add4.i1251869vector_func.i = fadd <16 x float> %mul3.i1241868vector_func.i, <float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000, float 0xBF34000480000000>
  %mul5.i1261870vector_func.i = fmul <16 x float> %add4.i1251869vector_func.i, %mul567vector_func.i
  %add4461871vector_func.i = fadd <16 x float> %add4441863vector_func.i, %mul5.i1261870vector_func.i
  %call.i46.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4461871vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7499vector_func.i, label %postload7500vector_func.i

preload7499vector_func.i:                         ; preds = %postload7497vector_func.i
  %extract1873vector_func.i = add i64 %dim_0_vector_tid.i, 179712
  %729 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1873vector_func.i
  %exData4556vector_func.i = extractelement <16 x float> %call.i46.i, i32 0
  store float %exData4556vector_func.i, float addrspace(1)* %729, align 4
  br label %postload7500vector_func.i

postload7500vector_func.i:                        ; preds = %preload7499vector_func.i, %postload7497vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7502vector_func.i, label %postload7503vector_func.i

preload7502vector_func.i:                         ; preds = %postload7500vector_func.i
  %.sum8676vector_func.i = add i64 %dim_0_vector_tid.i, 179713
  %730 = getelementptr float addrspace(1)* %4, i64 %.sum8676vector_func.i
  %exData4559vector_func.i = extractelement <16 x float> %call.i46.i, i32 1
  store float %exData4559vector_func.i, float addrspace(1)* %730, align 4
  br label %postload7503vector_func.i

postload7503vector_func.i:                        ; preds = %preload7502vector_func.i, %postload7500vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7505vector_func.i, label %postload7506vector_func.i

preload7505vector_func.i:                         ; preds = %postload7503vector_func.i
  %.sum8675vector_func.i = add i64 %dim_0_vector_tid.i, 179714
  %731 = getelementptr float addrspace(1)* %4, i64 %.sum8675vector_func.i
  %exData4562vector_func.i = extractelement <16 x float> %call.i46.i, i32 2
  store float %exData4562vector_func.i, float addrspace(1)* %731, align 4
  br label %postload7506vector_func.i

postload7506vector_func.i:                        ; preds = %preload7505vector_func.i, %postload7503vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7508vector_func.i, label %postload7509vector_func.i

preload7508vector_func.i:                         ; preds = %postload7506vector_func.i
  %.sum8674vector_func.i = add i64 %dim_0_vector_tid.i, 179715
  %732 = getelementptr float addrspace(1)* %4, i64 %.sum8674vector_func.i
  %exData4565vector_func.i = extractelement <16 x float> %call.i46.i, i32 3
  store float %exData4565vector_func.i, float addrspace(1)* %732, align 4
  br label %postload7509vector_func.i

postload7509vector_func.i:                        ; preds = %preload7508vector_func.i, %postload7506vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7511vector_func.i, label %postload7512vector_func.i

preload7511vector_func.i:                         ; preds = %postload7509vector_func.i
  %.sum8673vector_func.i = add i64 %dim_0_vector_tid.i, 179716
  %733 = getelementptr float addrspace(1)* %4, i64 %.sum8673vector_func.i
  %exData4568vector_func.i = extractelement <16 x float> %call.i46.i, i32 4
  store float %exData4568vector_func.i, float addrspace(1)* %733, align 4
  br label %postload7512vector_func.i

postload7512vector_func.i:                        ; preds = %preload7511vector_func.i, %postload7509vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7514vector_func.i, label %postload7515vector_func.i

preload7514vector_func.i:                         ; preds = %postload7512vector_func.i
  %.sum8672vector_func.i = add i64 %dim_0_vector_tid.i, 179717
  %734 = getelementptr float addrspace(1)* %4, i64 %.sum8672vector_func.i
  %exData4571vector_func.i = extractelement <16 x float> %call.i46.i, i32 5
  store float %exData4571vector_func.i, float addrspace(1)* %734, align 4
  br label %postload7515vector_func.i

postload7515vector_func.i:                        ; preds = %preload7514vector_func.i, %postload7512vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7517vector_func.i, label %postload7518vector_func.i

preload7517vector_func.i:                         ; preds = %postload7515vector_func.i
  %.sum8671vector_func.i = add i64 %dim_0_vector_tid.i, 179718
  %735 = getelementptr float addrspace(1)* %4, i64 %.sum8671vector_func.i
  %exData4574vector_func.i = extractelement <16 x float> %call.i46.i, i32 6
  store float %exData4574vector_func.i, float addrspace(1)* %735, align 4
  br label %postload7518vector_func.i

postload7518vector_func.i:                        ; preds = %preload7517vector_func.i, %postload7515vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7520vector_func.i, label %postload7521vector_func.i

preload7520vector_func.i:                         ; preds = %postload7518vector_func.i
  %.sum8670vector_func.i = add i64 %dim_0_vector_tid.i, 179719
  %736 = getelementptr float addrspace(1)* %4, i64 %.sum8670vector_func.i
  %exData4577vector_func.i = extractelement <16 x float> %call.i46.i, i32 7
  store float %exData4577vector_func.i, float addrspace(1)* %736, align 4
  br label %postload7521vector_func.i

postload7521vector_func.i:                        ; preds = %preload7520vector_func.i, %postload7518vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7523vector_func.i, label %postload7524vector_func.i

preload7523vector_func.i:                         ; preds = %postload7521vector_func.i
  %.sum8669vector_func.i = add i64 %dim_0_vector_tid.i, 179720
  %737 = getelementptr float addrspace(1)* %4, i64 %.sum8669vector_func.i
  %exData4580vector_func.i = extractelement <16 x float> %call.i46.i, i32 8
  store float %exData4580vector_func.i, float addrspace(1)* %737, align 4
  br label %postload7524vector_func.i

postload7524vector_func.i:                        ; preds = %preload7523vector_func.i, %postload7521vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7526vector_func.i, label %postload7527vector_func.i

preload7526vector_func.i:                         ; preds = %postload7524vector_func.i
  %.sum8668vector_func.i = add i64 %dim_0_vector_tid.i, 179721
  %738 = getelementptr float addrspace(1)* %4, i64 %.sum8668vector_func.i
  %exData4583vector_func.i = extractelement <16 x float> %call.i46.i, i32 9
  store float %exData4583vector_func.i, float addrspace(1)* %738, align 4
  br label %postload7527vector_func.i

postload7527vector_func.i:                        ; preds = %preload7526vector_func.i, %postload7524vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7529vector_func.i, label %postload7530vector_func.i

preload7529vector_func.i:                         ; preds = %postload7527vector_func.i
  %.sum8667vector_func.i = add i64 %dim_0_vector_tid.i, 179722
  %739 = getelementptr float addrspace(1)* %4, i64 %.sum8667vector_func.i
  %exData4586vector_func.i = extractelement <16 x float> %call.i46.i, i32 10
  store float %exData4586vector_func.i, float addrspace(1)* %739, align 4
  br label %postload7530vector_func.i

postload7530vector_func.i:                        ; preds = %preload7529vector_func.i, %postload7527vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7532vector_func.i, label %postload7533vector_func.i

preload7532vector_func.i:                         ; preds = %postload7530vector_func.i
  %.sum8666vector_func.i = add i64 %dim_0_vector_tid.i, 179723
  %740 = getelementptr float addrspace(1)* %4, i64 %.sum8666vector_func.i
  %exData4589vector_func.i = extractelement <16 x float> %call.i46.i, i32 11
  store float %exData4589vector_func.i, float addrspace(1)* %740, align 4
  br label %postload7533vector_func.i

postload7533vector_func.i:                        ; preds = %preload7532vector_func.i, %postload7530vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7535vector_func.i, label %postload7536vector_func.i

preload7535vector_func.i:                         ; preds = %postload7533vector_func.i
  %.sum8665vector_func.i = add i64 %dim_0_vector_tid.i, 179724
  %741 = getelementptr float addrspace(1)* %4, i64 %.sum8665vector_func.i
  %exData4592vector_func.i = extractelement <16 x float> %call.i46.i, i32 12
  store float %exData4592vector_func.i, float addrspace(1)* %741, align 4
  br label %postload7536vector_func.i

postload7536vector_func.i:                        ; preds = %preload7535vector_func.i, %postload7533vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7538vector_func.i, label %postload7539vector_func.i

preload7538vector_func.i:                         ; preds = %postload7536vector_func.i
  %.sum8664vector_func.i = add i64 %dim_0_vector_tid.i, 179725
  %742 = getelementptr float addrspace(1)* %4, i64 %.sum8664vector_func.i
  %exData4595vector_func.i = extractelement <16 x float> %call.i46.i, i32 13
  store float %exData4595vector_func.i, float addrspace(1)* %742, align 4
  br label %postload7539vector_func.i

postload7539vector_func.i:                        ; preds = %preload7538vector_func.i, %postload7536vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7541vector_func.i, label %postload7542vector_func.i

preload7541vector_func.i:                         ; preds = %postload7539vector_func.i
  %.sum8663vector_func.i = add i64 %dim_0_vector_tid.i, 179726
  %743 = getelementptr float addrspace(1)* %4, i64 %.sum8663vector_func.i
  %exData4598vector_func.i = extractelement <16 x float> %call.i46.i, i32 14
  store float %exData4598vector_func.i, float addrspace(1)* %743, align 4
  br label %postload7542vector_func.i

postload7542vector_func.i:                        ; preds = %preload7541vector_func.i, %postload7539vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7544vector_func.i, label %postload7545vector_func.i

preload7544vector_func.i:                         ; preds = %postload7542vector_func.i
  %.sum8662vector_func.i = add i64 %dim_0_vector_tid.i, 179727
  %744 = getelementptr float addrspace(1)* %4, i64 %.sum8662vector_func.i
  %exData4601vector_func.i = extractelement <16 x float> %call.i46.i, i32 15
  store float %exData4601vector_func.i, float addrspace(1)* %744, align 4
  br label %postload7545vector_func.i

postload7545vector_func.i:                        ; preds = %preload7544vector_func.i, %postload7542vector_func.i
  %mul4511890vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000, float 0x40E79E7F00000000>
  %add4521891vector_func.i = fadd <16 x float> %mul4511890vector_func.i, <float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000, float 0x4023CD56C0000000>
  %mul4531892vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000, float 0x4002DAAC20000000>
  %add4541893vector_func.i = fadd <16 x float> %add4521891vector_func.i, %mul4531892vector_func.i
  %mul.i1131894vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000, float 0xBD002DDB80000000>
  %add.i1141895vector_func.i = fadd <16 x float> %mul.i1131894vector_func.i, <float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000, float 0x3DEC2A6C00000000>
  %mul1.i1151896vector_func.i = fmul <16 x float> %add.i1141895vector_func.i, %mul567vector_func.i
  %add2.i1161897vector_func.i = fadd <16 x float> %mul1.i1151896vector_func.i, <float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000, float 0xBEB3EB3EA0000000>
  %mul3.i1171898vector_func.i = fmul <16 x float> %add2.i1161897vector_func.i, %mul567vector_func.i
  %add4.i1181899vector_func.i = fadd <16 x float> %mul3.i1171898vector_func.i, <float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000, float 0x3F72668420000000>
  %mul5.i1191900vector_func.i = fmul <16 x float> %add4.i1181899vector_func.i, %mul567vector_func.i
  %add4561901vector_func.i = fadd <16 x float> %add4541893vector_func.i, %mul5.i1191900vector_func.i
  %call.i47.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4561901vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7547vector_func.i, label %postload7548vector_func.i

preload7547vector_func.i:                         ; preds = %postload7545vector_func.i
  %extract1903vector_func.i = add i64 %dim_0_vector_tid.i, 193536
  %745 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1903vector_func.i
  %exData4605vector_func.i = extractelement <16 x float> %call.i47.i, i32 0
  store float %exData4605vector_func.i, float addrspace(1)* %745, align 4
  br label %postload7548vector_func.i

postload7548vector_func.i:                        ; preds = %preload7547vector_func.i, %postload7545vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7550vector_func.i, label %postload7551vector_func.i

preload7550vector_func.i:                         ; preds = %postload7548vector_func.i
  %.sum8661vector_func.i = add i64 %dim_0_vector_tid.i, 193537
  %746 = getelementptr float addrspace(1)* %4, i64 %.sum8661vector_func.i
  %exData4608vector_func.i = extractelement <16 x float> %call.i47.i, i32 1
  store float %exData4608vector_func.i, float addrspace(1)* %746, align 4
  br label %postload7551vector_func.i

postload7551vector_func.i:                        ; preds = %preload7550vector_func.i, %postload7548vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7553vector_func.i, label %postload7554vector_func.i

preload7553vector_func.i:                         ; preds = %postload7551vector_func.i
  %.sum8660vector_func.i = add i64 %dim_0_vector_tid.i, 193538
  %747 = getelementptr float addrspace(1)* %4, i64 %.sum8660vector_func.i
  %exData4611vector_func.i = extractelement <16 x float> %call.i47.i, i32 2
  store float %exData4611vector_func.i, float addrspace(1)* %747, align 4
  br label %postload7554vector_func.i

postload7554vector_func.i:                        ; preds = %preload7553vector_func.i, %postload7551vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7556vector_func.i, label %postload7557vector_func.i

preload7556vector_func.i:                         ; preds = %postload7554vector_func.i
  %.sum8659vector_func.i = add i64 %dim_0_vector_tid.i, 193539
  %748 = getelementptr float addrspace(1)* %4, i64 %.sum8659vector_func.i
  %exData4614vector_func.i = extractelement <16 x float> %call.i47.i, i32 3
  store float %exData4614vector_func.i, float addrspace(1)* %748, align 4
  br label %postload7557vector_func.i

postload7557vector_func.i:                        ; preds = %preload7556vector_func.i, %postload7554vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7559vector_func.i, label %postload7560vector_func.i

preload7559vector_func.i:                         ; preds = %postload7557vector_func.i
  %.sum8658vector_func.i = add i64 %dim_0_vector_tid.i, 193540
  %749 = getelementptr float addrspace(1)* %4, i64 %.sum8658vector_func.i
  %exData4617vector_func.i = extractelement <16 x float> %call.i47.i, i32 4
  store float %exData4617vector_func.i, float addrspace(1)* %749, align 4
  br label %postload7560vector_func.i

postload7560vector_func.i:                        ; preds = %preload7559vector_func.i, %postload7557vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7562vector_func.i, label %postload7563vector_func.i

preload7562vector_func.i:                         ; preds = %postload7560vector_func.i
  %.sum8657vector_func.i = add i64 %dim_0_vector_tid.i, 193541
  %750 = getelementptr float addrspace(1)* %4, i64 %.sum8657vector_func.i
  %exData4620vector_func.i = extractelement <16 x float> %call.i47.i, i32 5
  store float %exData4620vector_func.i, float addrspace(1)* %750, align 4
  br label %postload7563vector_func.i

postload7563vector_func.i:                        ; preds = %preload7562vector_func.i, %postload7560vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7565vector_func.i, label %postload7566vector_func.i

preload7565vector_func.i:                         ; preds = %postload7563vector_func.i
  %.sum8656vector_func.i = add i64 %dim_0_vector_tid.i, 193542
  %751 = getelementptr float addrspace(1)* %4, i64 %.sum8656vector_func.i
  %exData4623vector_func.i = extractelement <16 x float> %call.i47.i, i32 6
  store float %exData4623vector_func.i, float addrspace(1)* %751, align 4
  br label %postload7566vector_func.i

postload7566vector_func.i:                        ; preds = %preload7565vector_func.i, %postload7563vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7568vector_func.i, label %postload7569vector_func.i

preload7568vector_func.i:                         ; preds = %postload7566vector_func.i
  %.sum8655vector_func.i = add i64 %dim_0_vector_tid.i, 193543
  %752 = getelementptr float addrspace(1)* %4, i64 %.sum8655vector_func.i
  %exData4626vector_func.i = extractelement <16 x float> %call.i47.i, i32 7
  store float %exData4626vector_func.i, float addrspace(1)* %752, align 4
  br label %postload7569vector_func.i

postload7569vector_func.i:                        ; preds = %preload7568vector_func.i, %postload7566vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7571vector_func.i, label %postload7572vector_func.i

preload7571vector_func.i:                         ; preds = %postload7569vector_func.i
  %.sum8654vector_func.i = add i64 %dim_0_vector_tid.i, 193544
  %753 = getelementptr float addrspace(1)* %4, i64 %.sum8654vector_func.i
  %exData4629vector_func.i = extractelement <16 x float> %call.i47.i, i32 8
  store float %exData4629vector_func.i, float addrspace(1)* %753, align 4
  br label %postload7572vector_func.i

postload7572vector_func.i:                        ; preds = %preload7571vector_func.i, %postload7569vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7574vector_func.i, label %postload7575vector_func.i

preload7574vector_func.i:                         ; preds = %postload7572vector_func.i
  %.sum8653vector_func.i = add i64 %dim_0_vector_tid.i, 193545
  %754 = getelementptr float addrspace(1)* %4, i64 %.sum8653vector_func.i
  %exData4632vector_func.i = extractelement <16 x float> %call.i47.i, i32 9
  store float %exData4632vector_func.i, float addrspace(1)* %754, align 4
  br label %postload7575vector_func.i

postload7575vector_func.i:                        ; preds = %preload7574vector_func.i, %postload7572vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7577vector_func.i, label %postload7578vector_func.i

preload7577vector_func.i:                         ; preds = %postload7575vector_func.i
  %.sum8652vector_func.i = add i64 %dim_0_vector_tid.i, 193546
  %755 = getelementptr float addrspace(1)* %4, i64 %.sum8652vector_func.i
  %exData4635vector_func.i = extractelement <16 x float> %call.i47.i, i32 10
  store float %exData4635vector_func.i, float addrspace(1)* %755, align 4
  br label %postload7578vector_func.i

postload7578vector_func.i:                        ; preds = %preload7577vector_func.i, %postload7575vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7580vector_func.i, label %postload7581vector_func.i

preload7580vector_func.i:                         ; preds = %postload7578vector_func.i
  %.sum8651vector_func.i = add i64 %dim_0_vector_tid.i, 193547
  %756 = getelementptr float addrspace(1)* %4, i64 %.sum8651vector_func.i
  %exData4638vector_func.i = extractelement <16 x float> %call.i47.i, i32 11
  store float %exData4638vector_func.i, float addrspace(1)* %756, align 4
  br label %postload7581vector_func.i

postload7581vector_func.i:                        ; preds = %preload7580vector_func.i, %postload7578vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7583vector_func.i, label %postload7584vector_func.i

preload7583vector_func.i:                         ; preds = %postload7581vector_func.i
  %.sum8650vector_func.i = add i64 %dim_0_vector_tid.i, 193548
  %757 = getelementptr float addrspace(1)* %4, i64 %.sum8650vector_func.i
  %exData4641vector_func.i = extractelement <16 x float> %call.i47.i, i32 12
  store float %exData4641vector_func.i, float addrspace(1)* %757, align 4
  br label %postload7584vector_func.i

postload7584vector_func.i:                        ; preds = %preload7583vector_func.i, %postload7581vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7586vector_func.i, label %postload7587vector_func.i

preload7586vector_func.i:                         ; preds = %postload7584vector_func.i
  %.sum8649vector_func.i = add i64 %dim_0_vector_tid.i, 193549
  %758 = getelementptr float addrspace(1)* %4, i64 %.sum8649vector_func.i
  %exData4644vector_func.i = extractelement <16 x float> %call.i47.i, i32 13
  store float %exData4644vector_func.i, float addrspace(1)* %758, align 4
  br label %postload7587vector_func.i

postload7587vector_func.i:                        ; preds = %preload7586vector_func.i, %postload7584vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7589vector_func.i, label %postload7590vector_func.i

preload7589vector_func.i:                         ; preds = %postload7587vector_func.i
  %.sum8648vector_func.i = add i64 %dim_0_vector_tid.i, 193550
  %759 = getelementptr float addrspace(1)* %4, i64 %.sum8648vector_func.i
  %exData4647vector_func.i = extractelement <16 x float> %call.i47.i, i32 14
  store float %exData4647vector_func.i, float addrspace(1)* %759, align 4
  br label %postload7590vector_func.i

postload7590vector_func.i:                        ; preds = %preload7589vector_func.i, %postload7587vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7592vector_func.i, label %postload7593vector_func.i

preload7592vector_func.i:                         ; preds = %postload7590vector_func.i
  %.sum8647vector_func.i = add i64 %dim_0_vector_tid.i, 193551
  %760 = getelementptr float addrspace(1)* %4, i64 %.sum8647vector_func.i
  %exData4650vector_func.i = extractelement <16 x float> %call.i47.i, i32 15
  store float %exData4650vector_func.i, float addrspace(1)* %760, align 4
  br label %postload7593vector_func.i

postload7593vector_func.i:                        ; preds = %preload7592vector_func.i, %postload7590vector_func.i
  %mul4611920vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000, float 0x40ADFF2140000000>
  %sub4621921vector_func.i = fsub <16 x float> <float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000, float 0x400B27ACC0000000>, %mul4611920vector_func.i
  %mul4631922vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000, float 0x4010E27E80000000>
  %add4641923vector_func.i = fadd <16 x float> %sub4621921vector_func.i, %mul4631922vector_func.i
  %mul.i1061924vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000, float 0x3D4E8615E0000000>
  %add.i1071925vector_func.i = fadd <16 x float> %mul.i1061924vector_func.i, <float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000, float 0xBE130FC860000000>
  %mul1.i1081926vector_func.i = fmul <16 x float> %add.i1071925vector_func.i, %mul567vector_func.i
  %add2.i1091927vector_func.i = fadd <16 x float> %mul1.i1081926vector_func.i, <float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000, float 0x3EC34408C0000000>
  %mul3.i1101928vector_func.i = fmul <16 x float> %add2.i1091927vector_func.i, %mul567vector_func.i
  %add4.i1111929vector_func.i = fadd <16 x float> %mul3.i1101928vector_func.i, <float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000, float 0xBF5A930120000000>
  %mul5.i1121930vector_func.i = fmul <16 x float> %add4.i1111929vector_func.i, %mul567vector_func.i
  %add4661931vector_func.i = fadd <16 x float> %add4641923vector_func.i, %mul5.i1121930vector_func.i
  %call.i48.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4661931vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7595vector_func.i, label %postload7596vector_func.i

preload7595vector_func.i:                         ; preds = %postload7593vector_func.i
  %extract1933vector_func.i = add i64 %dim_0_vector_tid.i, 207360
  %761 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1933vector_func.i
  %exData4654vector_func.i = extractelement <16 x float> %call.i48.i, i32 0
  store float %exData4654vector_func.i, float addrspace(1)* %761, align 4
  br label %postload7596vector_func.i

postload7596vector_func.i:                        ; preds = %preload7595vector_func.i, %postload7593vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7598vector_func.i, label %postload7599vector_func.i

preload7598vector_func.i:                         ; preds = %postload7596vector_func.i
  %.sum8646vector_func.i = add i64 %dim_0_vector_tid.i, 207361
  %762 = getelementptr float addrspace(1)* %4, i64 %.sum8646vector_func.i
  %exData4657vector_func.i = extractelement <16 x float> %call.i48.i, i32 1
  store float %exData4657vector_func.i, float addrspace(1)* %762, align 4
  br label %postload7599vector_func.i

postload7599vector_func.i:                        ; preds = %preload7598vector_func.i, %postload7596vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7601vector_func.i, label %postload7602vector_func.i

preload7601vector_func.i:                         ; preds = %postload7599vector_func.i
  %.sum8645vector_func.i = add i64 %dim_0_vector_tid.i, 207362
  %763 = getelementptr float addrspace(1)* %4, i64 %.sum8645vector_func.i
  %exData4660vector_func.i = extractelement <16 x float> %call.i48.i, i32 2
  store float %exData4660vector_func.i, float addrspace(1)* %763, align 4
  br label %postload7602vector_func.i

postload7602vector_func.i:                        ; preds = %preload7601vector_func.i, %postload7599vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7604vector_func.i, label %postload7605vector_func.i

preload7604vector_func.i:                         ; preds = %postload7602vector_func.i
  %.sum8644vector_func.i = add i64 %dim_0_vector_tid.i, 207363
  %764 = getelementptr float addrspace(1)* %4, i64 %.sum8644vector_func.i
  %exData4663vector_func.i = extractelement <16 x float> %call.i48.i, i32 3
  store float %exData4663vector_func.i, float addrspace(1)* %764, align 4
  br label %postload7605vector_func.i

postload7605vector_func.i:                        ; preds = %preload7604vector_func.i, %postload7602vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7607vector_func.i, label %postload7608vector_func.i

preload7607vector_func.i:                         ; preds = %postload7605vector_func.i
  %.sum8643vector_func.i = add i64 %dim_0_vector_tid.i, 207364
  %765 = getelementptr float addrspace(1)* %4, i64 %.sum8643vector_func.i
  %exData4666vector_func.i = extractelement <16 x float> %call.i48.i, i32 4
  store float %exData4666vector_func.i, float addrspace(1)* %765, align 4
  br label %postload7608vector_func.i

postload7608vector_func.i:                        ; preds = %preload7607vector_func.i, %postload7605vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7610vector_func.i, label %postload7611vector_func.i

preload7610vector_func.i:                         ; preds = %postload7608vector_func.i
  %.sum8642vector_func.i = add i64 %dim_0_vector_tid.i, 207365
  %766 = getelementptr float addrspace(1)* %4, i64 %.sum8642vector_func.i
  %exData4669vector_func.i = extractelement <16 x float> %call.i48.i, i32 5
  store float %exData4669vector_func.i, float addrspace(1)* %766, align 4
  br label %postload7611vector_func.i

postload7611vector_func.i:                        ; preds = %preload7610vector_func.i, %postload7608vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7613vector_func.i, label %postload7614vector_func.i

preload7613vector_func.i:                         ; preds = %postload7611vector_func.i
  %.sum8641vector_func.i = add i64 %dim_0_vector_tid.i, 207366
  %767 = getelementptr float addrspace(1)* %4, i64 %.sum8641vector_func.i
  %exData4672vector_func.i = extractelement <16 x float> %call.i48.i, i32 6
  store float %exData4672vector_func.i, float addrspace(1)* %767, align 4
  br label %postload7614vector_func.i

postload7614vector_func.i:                        ; preds = %preload7613vector_func.i, %postload7611vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7616vector_func.i, label %postload7617vector_func.i

preload7616vector_func.i:                         ; preds = %postload7614vector_func.i
  %.sum8640vector_func.i = add i64 %dim_0_vector_tid.i, 207367
  %768 = getelementptr float addrspace(1)* %4, i64 %.sum8640vector_func.i
  %exData4675vector_func.i = extractelement <16 x float> %call.i48.i, i32 7
  store float %exData4675vector_func.i, float addrspace(1)* %768, align 4
  br label %postload7617vector_func.i

postload7617vector_func.i:                        ; preds = %preload7616vector_func.i, %postload7614vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7619vector_func.i, label %postload7620vector_func.i

preload7619vector_func.i:                         ; preds = %postload7617vector_func.i
  %.sum8639vector_func.i = add i64 %dim_0_vector_tid.i, 207368
  %769 = getelementptr float addrspace(1)* %4, i64 %.sum8639vector_func.i
  %exData4678vector_func.i = extractelement <16 x float> %call.i48.i, i32 8
  store float %exData4678vector_func.i, float addrspace(1)* %769, align 4
  br label %postload7620vector_func.i

postload7620vector_func.i:                        ; preds = %preload7619vector_func.i, %postload7617vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7622vector_func.i, label %postload7623vector_func.i

preload7622vector_func.i:                         ; preds = %postload7620vector_func.i
  %.sum8638vector_func.i = add i64 %dim_0_vector_tid.i, 207369
  %770 = getelementptr float addrspace(1)* %4, i64 %.sum8638vector_func.i
  %exData4681vector_func.i = extractelement <16 x float> %call.i48.i, i32 9
  store float %exData4681vector_func.i, float addrspace(1)* %770, align 4
  br label %postload7623vector_func.i

postload7623vector_func.i:                        ; preds = %preload7622vector_func.i, %postload7620vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7625vector_func.i, label %postload7626vector_func.i

preload7625vector_func.i:                         ; preds = %postload7623vector_func.i
  %.sum8637vector_func.i = add i64 %dim_0_vector_tid.i, 207370
  %771 = getelementptr float addrspace(1)* %4, i64 %.sum8637vector_func.i
  %exData4684vector_func.i = extractelement <16 x float> %call.i48.i, i32 10
  store float %exData4684vector_func.i, float addrspace(1)* %771, align 4
  br label %postload7626vector_func.i

postload7626vector_func.i:                        ; preds = %preload7625vector_func.i, %postload7623vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7628vector_func.i, label %postload7629vector_func.i

preload7628vector_func.i:                         ; preds = %postload7626vector_func.i
  %.sum8636vector_func.i = add i64 %dim_0_vector_tid.i, 207371
  %772 = getelementptr float addrspace(1)* %4, i64 %.sum8636vector_func.i
  %exData4687vector_func.i = extractelement <16 x float> %call.i48.i, i32 11
  store float %exData4687vector_func.i, float addrspace(1)* %772, align 4
  br label %postload7629vector_func.i

postload7629vector_func.i:                        ; preds = %preload7628vector_func.i, %postload7626vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7631vector_func.i, label %postload7632vector_func.i

preload7631vector_func.i:                         ; preds = %postload7629vector_func.i
  %.sum8635vector_func.i = add i64 %dim_0_vector_tid.i, 207372
  %773 = getelementptr float addrspace(1)* %4, i64 %.sum8635vector_func.i
  %exData4690vector_func.i = extractelement <16 x float> %call.i48.i, i32 12
  store float %exData4690vector_func.i, float addrspace(1)* %773, align 4
  br label %postload7632vector_func.i

postload7632vector_func.i:                        ; preds = %preload7631vector_func.i, %postload7629vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7634vector_func.i, label %postload7635vector_func.i

preload7634vector_func.i:                         ; preds = %postload7632vector_func.i
  %.sum8634vector_func.i = add i64 %dim_0_vector_tid.i, 207373
  %774 = getelementptr float addrspace(1)* %4, i64 %.sum8634vector_func.i
  %exData4693vector_func.i = extractelement <16 x float> %call.i48.i, i32 13
  store float %exData4693vector_func.i, float addrspace(1)* %774, align 4
  br label %postload7635vector_func.i

postload7635vector_func.i:                        ; preds = %preload7634vector_func.i, %postload7632vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7637vector_func.i, label %postload7638vector_func.i

preload7637vector_func.i:                         ; preds = %postload7635vector_func.i
  %.sum8633vector_func.i = add i64 %dim_0_vector_tid.i, 207374
  %775 = getelementptr float addrspace(1)* %4, i64 %.sum8633vector_func.i
  %exData4696vector_func.i = extractelement <16 x float> %call.i48.i, i32 14
  store float %exData4696vector_func.i, float addrspace(1)* %775, align 4
  br label %postload7638vector_func.i

postload7638vector_func.i:                        ; preds = %preload7637vector_func.i, %postload7635vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7640vector_func.i, label %postload7641vector_func.i

preload7640vector_func.i:                         ; preds = %postload7638vector_func.i
  %.sum8632vector_func.i = add i64 %dim_0_vector_tid.i, 207375
  %776 = getelementptr float addrspace(1)* %4, i64 %.sum8632vector_func.i
  %exData4699vector_func.i = extractelement <16 x float> %call.i48.i, i32 15
  store float %exData4699vector_func.i, float addrspace(1)* %776, align 4
  br label %postload7641vector_func.i

postload7641vector_func.i:                        ; preds = %preload7640vector_func.i, %postload7638vector_func.i
  %mul4711950vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000, float 0x40CBF27A80000000>
  %add4721951vector_func.i = fadd <16 x float> %mul4711950vector_func.i, <float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000, float 0x3FE34A3E40000000>
  %mul4731952vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000, float 0x40132CC5C0000000>
  %add4741953vector_func.i = fadd <16 x float> %add4721951vector_func.i, %mul4731952vector_func.i
  %mul.i991954vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000, float 0x3D672E8340000000>
  %add.i1001955vector_func.i = fadd <16 x float> %mul.i991954vector_func.i, <float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000, float 0xBE2B2679E0000000>
  %mul1.i1011956vector_func.i = fmul <16 x float> %add.i1001955vector_func.i, %mul567vector_func.i
  %add2.i1021957vector_func.i = fadd <16 x float> %mul1.i1011956vector_func.i, <float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000, float 0x3EDA170840000000>
  %mul3.i1031958vector_func.i = fmul <16 x float> %add2.i1021957vector_func.i, %mul567vector_func.i
  %add4.i1041959vector_func.i = fadd <16 x float> %mul3.i1031958vector_func.i, <float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000, float 0xBF744AD200000000>
  %mul5.i1051960vector_func.i = fmul <16 x float> %add4.i1041959vector_func.i, %mul567vector_func.i
  %add4761961vector_func.i = fadd <16 x float> %add4741953vector_func.i, %mul5.i1051960vector_func.i
  %call.i49.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4761961vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7643vector_func.i, label %postload7644vector_func.i

preload7643vector_func.i:                         ; preds = %postload7641vector_func.i
  %extract1963vector_func.i = add i64 %dim_0_vector_tid.i, 221184
  %777 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1963vector_func.i
  %exData4703vector_func.i = extractelement <16 x float> %call.i49.i, i32 0
  store float %exData4703vector_func.i, float addrspace(1)* %777, align 4
  br label %postload7644vector_func.i

postload7644vector_func.i:                        ; preds = %preload7643vector_func.i, %postload7641vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7646vector_func.i, label %postload7647vector_func.i

preload7646vector_func.i:                         ; preds = %postload7644vector_func.i
  %.sum8631vector_func.i = add i64 %dim_0_vector_tid.i, 221185
  %778 = getelementptr float addrspace(1)* %4, i64 %.sum8631vector_func.i
  %exData4706vector_func.i = extractelement <16 x float> %call.i49.i, i32 1
  store float %exData4706vector_func.i, float addrspace(1)* %778, align 4
  br label %postload7647vector_func.i

postload7647vector_func.i:                        ; preds = %preload7646vector_func.i, %postload7644vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7649vector_func.i, label %postload7650vector_func.i

preload7649vector_func.i:                         ; preds = %postload7647vector_func.i
  %.sum8630vector_func.i = add i64 %dim_0_vector_tid.i, 221186
  %779 = getelementptr float addrspace(1)* %4, i64 %.sum8630vector_func.i
  %exData4709vector_func.i = extractelement <16 x float> %call.i49.i, i32 2
  store float %exData4709vector_func.i, float addrspace(1)* %779, align 4
  br label %postload7650vector_func.i

postload7650vector_func.i:                        ; preds = %preload7649vector_func.i, %postload7647vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7652vector_func.i, label %postload7653vector_func.i

preload7652vector_func.i:                         ; preds = %postload7650vector_func.i
  %.sum8629vector_func.i = add i64 %dim_0_vector_tid.i, 221187
  %780 = getelementptr float addrspace(1)* %4, i64 %.sum8629vector_func.i
  %exData4712vector_func.i = extractelement <16 x float> %call.i49.i, i32 3
  store float %exData4712vector_func.i, float addrspace(1)* %780, align 4
  br label %postload7653vector_func.i

postload7653vector_func.i:                        ; preds = %preload7652vector_func.i, %postload7650vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7655vector_func.i, label %postload7656vector_func.i

preload7655vector_func.i:                         ; preds = %postload7653vector_func.i
  %.sum8628vector_func.i = add i64 %dim_0_vector_tid.i, 221188
  %781 = getelementptr float addrspace(1)* %4, i64 %.sum8628vector_func.i
  %exData4715vector_func.i = extractelement <16 x float> %call.i49.i, i32 4
  store float %exData4715vector_func.i, float addrspace(1)* %781, align 4
  br label %postload7656vector_func.i

postload7656vector_func.i:                        ; preds = %preload7655vector_func.i, %postload7653vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7658vector_func.i, label %postload7659vector_func.i

preload7658vector_func.i:                         ; preds = %postload7656vector_func.i
  %.sum8627vector_func.i = add i64 %dim_0_vector_tid.i, 221189
  %782 = getelementptr float addrspace(1)* %4, i64 %.sum8627vector_func.i
  %exData4718vector_func.i = extractelement <16 x float> %call.i49.i, i32 5
  store float %exData4718vector_func.i, float addrspace(1)* %782, align 4
  br label %postload7659vector_func.i

postload7659vector_func.i:                        ; preds = %preload7658vector_func.i, %postload7656vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7661vector_func.i, label %postload7662vector_func.i

preload7661vector_func.i:                         ; preds = %postload7659vector_func.i
  %.sum8626vector_func.i = add i64 %dim_0_vector_tid.i, 221190
  %783 = getelementptr float addrspace(1)* %4, i64 %.sum8626vector_func.i
  %exData4721vector_func.i = extractelement <16 x float> %call.i49.i, i32 6
  store float %exData4721vector_func.i, float addrspace(1)* %783, align 4
  br label %postload7662vector_func.i

postload7662vector_func.i:                        ; preds = %preload7661vector_func.i, %postload7659vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7664vector_func.i, label %postload7665vector_func.i

preload7664vector_func.i:                         ; preds = %postload7662vector_func.i
  %.sum8625vector_func.i = add i64 %dim_0_vector_tid.i, 221191
  %784 = getelementptr float addrspace(1)* %4, i64 %.sum8625vector_func.i
  %exData4724vector_func.i = extractelement <16 x float> %call.i49.i, i32 7
  store float %exData4724vector_func.i, float addrspace(1)* %784, align 4
  br label %postload7665vector_func.i

postload7665vector_func.i:                        ; preds = %preload7664vector_func.i, %postload7662vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7667vector_func.i, label %postload7668vector_func.i

preload7667vector_func.i:                         ; preds = %postload7665vector_func.i
  %.sum8624vector_func.i = add i64 %dim_0_vector_tid.i, 221192
  %785 = getelementptr float addrspace(1)* %4, i64 %.sum8624vector_func.i
  %exData4727vector_func.i = extractelement <16 x float> %call.i49.i, i32 8
  store float %exData4727vector_func.i, float addrspace(1)* %785, align 4
  br label %postload7668vector_func.i

postload7668vector_func.i:                        ; preds = %preload7667vector_func.i, %postload7665vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7670vector_func.i, label %postload7671vector_func.i

preload7670vector_func.i:                         ; preds = %postload7668vector_func.i
  %.sum8623vector_func.i = add i64 %dim_0_vector_tid.i, 221193
  %786 = getelementptr float addrspace(1)* %4, i64 %.sum8623vector_func.i
  %exData4730vector_func.i = extractelement <16 x float> %call.i49.i, i32 9
  store float %exData4730vector_func.i, float addrspace(1)* %786, align 4
  br label %postload7671vector_func.i

postload7671vector_func.i:                        ; preds = %preload7670vector_func.i, %postload7668vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7673vector_func.i, label %postload7674vector_func.i

preload7673vector_func.i:                         ; preds = %postload7671vector_func.i
  %.sum8622vector_func.i = add i64 %dim_0_vector_tid.i, 221194
  %787 = getelementptr float addrspace(1)* %4, i64 %.sum8622vector_func.i
  %exData4733vector_func.i = extractelement <16 x float> %call.i49.i, i32 10
  store float %exData4733vector_func.i, float addrspace(1)* %787, align 4
  br label %postload7674vector_func.i

postload7674vector_func.i:                        ; preds = %preload7673vector_func.i, %postload7671vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7676vector_func.i, label %postload7677vector_func.i

preload7676vector_func.i:                         ; preds = %postload7674vector_func.i
  %.sum8621vector_func.i = add i64 %dim_0_vector_tid.i, 221195
  %788 = getelementptr float addrspace(1)* %4, i64 %.sum8621vector_func.i
  %exData4736vector_func.i = extractelement <16 x float> %call.i49.i, i32 11
  store float %exData4736vector_func.i, float addrspace(1)* %788, align 4
  br label %postload7677vector_func.i

postload7677vector_func.i:                        ; preds = %preload7676vector_func.i, %postload7674vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7679vector_func.i, label %postload7680vector_func.i

preload7679vector_func.i:                         ; preds = %postload7677vector_func.i
  %.sum8620vector_func.i = add i64 %dim_0_vector_tid.i, 221196
  %789 = getelementptr float addrspace(1)* %4, i64 %.sum8620vector_func.i
  %exData4739vector_func.i = extractelement <16 x float> %call.i49.i, i32 12
  store float %exData4739vector_func.i, float addrspace(1)* %789, align 4
  br label %postload7680vector_func.i

postload7680vector_func.i:                        ; preds = %preload7679vector_func.i, %postload7677vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7682vector_func.i, label %postload7683vector_func.i

preload7682vector_func.i:                         ; preds = %postload7680vector_func.i
  %.sum8619vector_func.i = add i64 %dim_0_vector_tid.i, 221197
  %790 = getelementptr float addrspace(1)* %4, i64 %.sum8619vector_func.i
  %exData4742vector_func.i = extractelement <16 x float> %call.i49.i, i32 13
  store float %exData4742vector_func.i, float addrspace(1)* %790, align 4
  br label %postload7683vector_func.i

postload7683vector_func.i:                        ; preds = %preload7682vector_func.i, %postload7680vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7685vector_func.i, label %postload7686vector_func.i

preload7685vector_func.i:                         ; preds = %postload7683vector_func.i
  %.sum8618vector_func.i = add i64 %dim_0_vector_tid.i, 221198
  %791 = getelementptr float addrspace(1)* %4, i64 %.sum8618vector_func.i
  %exData4745vector_func.i = extractelement <16 x float> %call.i49.i, i32 14
  store float %exData4745vector_func.i, float addrspace(1)* %791, align 4
  br label %postload7686vector_func.i

postload7686vector_func.i:                        ; preds = %preload7685vector_func.i, %postload7683vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7688vector_func.i, label %postload7689vector_func.i

preload7688vector_func.i:                         ; preds = %postload7686vector_func.i
  %.sum8617vector_func.i = add i64 %dim_0_vector_tid.i, 221199
  %792 = getelementptr float addrspace(1)* %4, i64 %.sum8617vector_func.i
  %exData4748vector_func.i = extractelement <16 x float> %call.i49.i, i32 15
  store float %exData4748vector_func.i, float addrspace(1)* %792, align 4
  br label %postload7689vector_func.i

postload7689vector_func.i:                        ; preds = %preload7688vector_func.i, %postload7686vector_func.i
  %mul4811980vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000, float 0x408E94CF00000000>
  %sub4821981vector_func.i = fsub <16 x float> <float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000, float 0x402A4DEA20000000>, %mul4811980vector_func.i
  %mul4831982vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000, float 0x4000D98180000000>
  %add4841983vector_func.i = fadd <16 x float> %sub4821981vector_func.i, %mul4831982vector_func.i
  %mul.i921984vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000, float 0x3D3D362C60000000>
  %add.i931985vector_func.i = fadd <16 x float> %mul.i921984vector_func.i, <float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000, float 0xBE051FDD40000000>
  %mul1.i941986vector_func.i = fmul <16 x float> %add.i931985vector_func.i, %mul567vector_func.i
  %add2.i951987vector_func.i = fadd <16 x float> %mul1.i941986vector_func.i, <float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000, float 0x3EADDADAA0000000>
  %mul3.i961988vector_func.i = fmul <16 x float> %add2.i951987vector_func.i, %mul567vector_func.i
  %add4.i971989vector_func.i = fadd <16 x float> %mul3.i961988vector_func.i, <float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000, float 0x3F6D8F2600000000>
  %mul5.i981990vector_func.i = fmul <16 x float> %add4.i971989vector_func.i, %mul567vector_func.i
  %add4861991vector_func.i = fadd <16 x float> %add4841983vector_func.i, %mul5.i981990vector_func.i
  %call.i50.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4861991vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7691vector_func.i, label %postload7692vector_func.i

preload7691vector_func.i:                         ; preds = %postload7689vector_func.i
  %extract1993vector_func.i = add i64 %dim_0_vector_tid.i, 235008
  %793 = getelementptr inbounds float addrspace(1)* %4, i64 %extract1993vector_func.i
  %exData4752vector_func.i = extractelement <16 x float> %call.i50.i, i32 0
  store float %exData4752vector_func.i, float addrspace(1)* %793, align 4
  br label %postload7692vector_func.i

postload7692vector_func.i:                        ; preds = %preload7691vector_func.i, %postload7689vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7694vector_func.i, label %postload7695vector_func.i

preload7694vector_func.i:                         ; preds = %postload7692vector_func.i
  %.sum8616vector_func.i = add i64 %dim_0_vector_tid.i, 235009
  %794 = getelementptr float addrspace(1)* %4, i64 %.sum8616vector_func.i
  %exData4755vector_func.i = extractelement <16 x float> %call.i50.i, i32 1
  store float %exData4755vector_func.i, float addrspace(1)* %794, align 4
  br label %postload7695vector_func.i

postload7695vector_func.i:                        ; preds = %preload7694vector_func.i, %postload7692vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7697vector_func.i, label %postload7698vector_func.i

preload7697vector_func.i:                         ; preds = %postload7695vector_func.i
  %.sum8615vector_func.i = add i64 %dim_0_vector_tid.i, 235010
  %795 = getelementptr float addrspace(1)* %4, i64 %.sum8615vector_func.i
  %exData4758vector_func.i = extractelement <16 x float> %call.i50.i, i32 2
  store float %exData4758vector_func.i, float addrspace(1)* %795, align 4
  br label %postload7698vector_func.i

postload7698vector_func.i:                        ; preds = %preload7697vector_func.i, %postload7695vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7700vector_func.i, label %postload7701vector_func.i

preload7700vector_func.i:                         ; preds = %postload7698vector_func.i
  %.sum8614vector_func.i = add i64 %dim_0_vector_tid.i, 235011
  %796 = getelementptr float addrspace(1)* %4, i64 %.sum8614vector_func.i
  %exData4761vector_func.i = extractelement <16 x float> %call.i50.i, i32 3
  store float %exData4761vector_func.i, float addrspace(1)* %796, align 4
  br label %postload7701vector_func.i

postload7701vector_func.i:                        ; preds = %preload7700vector_func.i, %postload7698vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7703vector_func.i, label %postload7704vector_func.i

preload7703vector_func.i:                         ; preds = %postload7701vector_func.i
  %.sum8613vector_func.i = add i64 %dim_0_vector_tid.i, 235012
  %797 = getelementptr float addrspace(1)* %4, i64 %.sum8613vector_func.i
  %exData4764vector_func.i = extractelement <16 x float> %call.i50.i, i32 4
  store float %exData4764vector_func.i, float addrspace(1)* %797, align 4
  br label %postload7704vector_func.i

postload7704vector_func.i:                        ; preds = %preload7703vector_func.i, %postload7701vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7751vector_func.i, label %postload7752vector_func.i

preload7751vector_func.i:                         ; preds = %postload7704vector_func.i
  %.sum8612vector_func.i = add i64 %dim_0_vector_tid.i, 235013
  %798 = getelementptr float addrspace(1)* %4, i64 %.sum8612vector_func.i
  %exData4767vector_func.i = extractelement <16 x float> %call.i50.i, i32 5
  store float %exData4767vector_func.i, float addrspace(1)* %798, align 4
  br label %postload7752vector_func.i

postload7752vector_func.i:                        ; preds = %preload7751vector_func.i, %postload7704vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7754vector_func.i, label %postload7755vector_func.i

preload7754vector_func.i:                         ; preds = %postload7752vector_func.i
  %.sum8611vector_func.i = add i64 %dim_0_vector_tid.i, 235014
  %799 = getelementptr float addrspace(1)* %4, i64 %.sum8611vector_func.i
  %exData4770vector_func.i = extractelement <16 x float> %call.i50.i, i32 6
  store float %exData4770vector_func.i, float addrspace(1)* %799, align 4
  br label %postload7755vector_func.i

postload7755vector_func.i:                        ; preds = %preload7754vector_func.i, %postload7752vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7757vector_func.i, label %postload7758vector_func.i

preload7757vector_func.i:                         ; preds = %postload7755vector_func.i
  %.sum8610vector_func.i = add i64 %dim_0_vector_tid.i, 235015
  %800 = getelementptr float addrspace(1)* %4, i64 %.sum8610vector_func.i
  %exData4773vector_func.i = extractelement <16 x float> %call.i50.i, i32 7
  store float %exData4773vector_func.i, float addrspace(1)* %800, align 4
  br label %postload7758vector_func.i

postload7758vector_func.i:                        ; preds = %preload7757vector_func.i, %postload7755vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7760vector_func.i, label %postload7761vector_func.i

preload7760vector_func.i:                         ; preds = %postload7758vector_func.i
  %.sum8609vector_func.i = add i64 %dim_0_vector_tid.i, 235016
  %801 = getelementptr float addrspace(1)* %4, i64 %.sum8609vector_func.i
  %exData4776vector_func.i = extractelement <16 x float> %call.i50.i, i32 8
  store float %exData4776vector_func.i, float addrspace(1)* %801, align 4
  br label %postload7761vector_func.i

postload7761vector_func.i:                        ; preds = %preload7760vector_func.i, %postload7758vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7763vector_func.i, label %postload7764vector_func.i

preload7763vector_func.i:                         ; preds = %postload7761vector_func.i
  %.sum8608vector_func.i = add i64 %dim_0_vector_tid.i, 235017
  %802 = getelementptr float addrspace(1)* %4, i64 %.sum8608vector_func.i
  %exData4779vector_func.i = extractelement <16 x float> %call.i50.i, i32 9
  store float %exData4779vector_func.i, float addrspace(1)* %802, align 4
  br label %postload7764vector_func.i

postload7764vector_func.i:                        ; preds = %preload7763vector_func.i, %postload7761vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7766vector_func.i, label %postload7767vector_func.i

preload7766vector_func.i:                         ; preds = %postload7764vector_func.i
  %.sum8607vector_func.i = add i64 %dim_0_vector_tid.i, 235018
  %803 = getelementptr float addrspace(1)* %4, i64 %.sum8607vector_func.i
  %exData4782vector_func.i = extractelement <16 x float> %call.i50.i, i32 10
  store float %exData4782vector_func.i, float addrspace(1)* %803, align 4
  br label %postload7767vector_func.i

postload7767vector_func.i:                        ; preds = %preload7766vector_func.i, %postload7764vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7769vector_func.i, label %postload7770vector_func.i

preload7769vector_func.i:                         ; preds = %postload7767vector_func.i
  %.sum8606vector_func.i = add i64 %dim_0_vector_tid.i, 235019
  %804 = getelementptr float addrspace(1)* %4, i64 %.sum8606vector_func.i
  %exData4785vector_func.i = extractelement <16 x float> %call.i50.i, i32 11
  store float %exData4785vector_func.i, float addrspace(1)* %804, align 4
  br label %postload7770vector_func.i

postload7770vector_func.i:                        ; preds = %preload7769vector_func.i, %postload7767vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7772vector_func.i, label %postload7773vector_func.i

preload7772vector_func.i:                         ; preds = %postload7770vector_func.i
  %.sum8605vector_func.i = add i64 %dim_0_vector_tid.i, 235020
  %805 = getelementptr float addrspace(1)* %4, i64 %.sum8605vector_func.i
  %exData4788vector_func.i = extractelement <16 x float> %call.i50.i, i32 12
  store float %exData4788vector_func.i, float addrspace(1)* %805, align 4
  br label %postload7773vector_func.i

postload7773vector_func.i:                        ; preds = %preload7772vector_func.i, %postload7770vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7775vector_func.i, label %postload7776vector_func.i

preload7775vector_func.i:                         ; preds = %postload7773vector_func.i
  %.sum8604vector_func.i = add i64 %dim_0_vector_tid.i, 235021
  %806 = getelementptr float addrspace(1)* %4, i64 %.sum8604vector_func.i
  %exData4791vector_func.i = extractelement <16 x float> %call.i50.i, i32 13
  store float %exData4791vector_func.i, float addrspace(1)* %806, align 4
  br label %postload7776vector_func.i

postload7776vector_func.i:                        ; preds = %preload7775vector_func.i, %postload7773vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7778vector_func.i, label %postload7779vector_func.i

preload7778vector_func.i:                         ; preds = %postload7776vector_func.i
  %.sum8603vector_func.i = add i64 %dim_0_vector_tid.i, 235022
  %807 = getelementptr float addrspace(1)* %4, i64 %.sum8603vector_func.i
  %exData4794vector_func.i = extractelement <16 x float> %call.i50.i, i32 14
  store float %exData4794vector_func.i, float addrspace(1)* %807, align 4
  br label %postload7779vector_func.i

postload7779vector_func.i:                        ; preds = %preload7778vector_func.i, %postload7776vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7781vector_func.i, label %postload7782vector_func.i

preload7781vector_func.i:                         ; preds = %postload7779vector_func.i
  %.sum8602vector_func.i = add i64 %dim_0_vector_tid.i, 235023
  %808 = getelementptr float addrspace(1)* %4, i64 %.sum8602vector_func.i
  %exData4797vector_func.i = extractelement <16 x float> %call.i50.i, i32 15
  store float %exData4797vector_func.i, float addrspace(1)* %808, align 4
  br label %postload7782vector_func.i

postload7782vector_func.i:                        ; preds = %preload7781vector_func.i, %postload7779vector_func.i
  %mul4912010vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000, float 0x40D9CF3EC0000000>
  %sub4922011vector_func.i = fsub <16 x float> <float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000, float 0x402BE12100000000>, %mul4912010vector_func.i
  %mul4932012vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000, float 0x3FE9E0B720000000>
  %add4942013vector_func.i = fadd <16 x float> %sub4922011vector_func.i, %mul4932012vector_func.i
  %mul.i852014vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000, float 0xBD5DE8C6E0000000>
  %add.i862015vector_func.i = fadd <16 x float> %mul.i852014vector_func.i, <float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000, float 0x3E240DD900000000>
  %mul1.i872016vector_func.i = fmul <16 x float> %add.i862015vector_func.i, %mul567vector_func.i
  %add2.i882017vector_func.i = fadd <16 x float> %mul1.i872016vector_func.i, <float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000, float 0xBED8D40C20000000>
  %mul3.i892018vector_func.i = fmul <16 x float> %add2.i882017vector_func.i, %mul567vector_func.i
  %add4.i902019vector_func.i = fadd <16 x float> %mul3.i892018vector_func.i, <float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000, float 0x3F87EC1800000000>
  %mul5.i912020vector_func.i = fmul <16 x float> %add4.i902019vector_func.i, %mul567vector_func.i
  %add4962021vector_func.i = fadd <16 x float> %add4942013vector_func.i, %mul5.i912020vector_func.i
  %call.i51.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add4962021vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7784vector_func.i, label %postload7785vector_func.i

preload7784vector_func.i:                         ; preds = %postload7782vector_func.i
  %extract2023vector_func.i = add i64 %dim_0_vector_tid.i, 248832
  %809 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2023vector_func.i
  %exData4801vector_func.i = extractelement <16 x float> %call.i51.i, i32 0
  store float %exData4801vector_func.i, float addrspace(1)* %809, align 4
  br label %postload7785vector_func.i

postload7785vector_func.i:                        ; preds = %preload7784vector_func.i, %postload7782vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7787vector_func.i, label %postload7788vector_func.i

preload7787vector_func.i:                         ; preds = %postload7785vector_func.i
  %.sum8601vector_func.i = add i64 %dim_0_vector_tid.i, 248833
  %810 = getelementptr float addrspace(1)* %4, i64 %.sum8601vector_func.i
  %exData4804vector_func.i = extractelement <16 x float> %call.i51.i, i32 1
  store float %exData4804vector_func.i, float addrspace(1)* %810, align 4
  br label %postload7788vector_func.i

postload7788vector_func.i:                        ; preds = %preload7787vector_func.i, %postload7785vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7790vector_func.i, label %postload7791vector_func.i

preload7790vector_func.i:                         ; preds = %postload7788vector_func.i
  %.sum8600vector_func.i = add i64 %dim_0_vector_tid.i, 248834
  %811 = getelementptr float addrspace(1)* %4, i64 %.sum8600vector_func.i
  %exData4807vector_func.i = extractelement <16 x float> %call.i51.i, i32 2
  store float %exData4807vector_func.i, float addrspace(1)* %811, align 4
  br label %postload7791vector_func.i

postload7791vector_func.i:                        ; preds = %preload7790vector_func.i, %postload7788vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7793vector_func.i, label %postload7794vector_func.i

preload7793vector_func.i:                         ; preds = %postload7791vector_func.i
  %.sum8599vector_func.i = add i64 %dim_0_vector_tid.i, 248835
  %812 = getelementptr float addrspace(1)* %4, i64 %.sum8599vector_func.i
  %exData4810vector_func.i = extractelement <16 x float> %call.i51.i, i32 3
  store float %exData4810vector_func.i, float addrspace(1)* %812, align 4
  br label %postload7794vector_func.i

postload7794vector_func.i:                        ; preds = %preload7793vector_func.i, %postload7791vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7796vector_func.i, label %postload7797vector_func.i

preload7796vector_func.i:                         ; preds = %postload7794vector_func.i
  %.sum8598vector_func.i = add i64 %dim_0_vector_tid.i, 248836
  %813 = getelementptr float addrspace(1)* %4, i64 %.sum8598vector_func.i
  %exData4813vector_func.i = extractelement <16 x float> %call.i51.i, i32 4
  store float %exData4813vector_func.i, float addrspace(1)* %813, align 4
  br label %postload7797vector_func.i

postload7797vector_func.i:                        ; preds = %preload7796vector_func.i, %postload7794vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7799vector_func.i, label %postload7800vector_func.i

preload7799vector_func.i:                         ; preds = %postload7797vector_func.i
  %.sum8597vector_func.i = add i64 %dim_0_vector_tid.i, 248837
  %814 = getelementptr float addrspace(1)* %4, i64 %.sum8597vector_func.i
  %exData4816vector_func.i = extractelement <16 x float> %call.i51.i, i32 5
  store float %exData4816vector_func.i, float addrspace(1)* %814, align 4
  br label %postload7800vector_func.i

postload7800vector_func.i:                        ; preds = %preload7799vector_func.i, %postload7797vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7802vector_func.i, label %postload7803vector_func.i

preload7802vector_func.i:                         ; preds = %postload7800vector_func.i
  %.sum8596vector_func.i = add i64 %dim_0_vector_tid.i, 248838
  %815 = getelementptr float addrspace(1)* %4, i64 %.sum8596vector_func.i
  %exData4819vector_func.i = extractelement <16 x float> %call.i51.i, i32 6
  store float %exData4819vector_func.i, float addrspace(1)* %815, align 4
  br label %postload7803vector_func.i

postload7803vector_func.i:                        ; preds = %preload7802vector_func.i, %postload7800vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7805vector_func.i, label %postload7806vector_func.i

preload7805vector_func.i:                         ; preds = %postload7803vector_func.i
  %.sum8595vector_func.i = add i64 %dim_0_vector_tid.i, 248839
  %816 = getelementptr float addrspace(1)* %4, i64 %.sum8595vector_func.i
  %exData4822vector_func.i = extractelement <16 x float> %call.i51.i, i32 7
  store float %exData4822vector_func.i, float addrspace(1)* %816, align 4
  br label %postload7806vector_func.i

postload7806vector_func.i:                        ; preds = %preload7805vector_func.i, %postload7803vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7808vector_func.i, label %postload7809vector_func.i

preload7808vector_func.i:                         ; preds = %postload7806vector_func.i
  %.sum8594vector_func.i = add i64 %dim_0_vector_tid.i, 248840
  %817 = getelementptr float addrspace(1)* %4, i64 %.sum8594vector_func.i
  %exData4825vector_func.i = extractelement <16 x float> %call.i51.i, i32 8
  store float %exData4825vector_func.i, float addrspace(1)* %817, align 4
  br label %postload7809vector_func.i

postload7809vector_func.i:                        ; preds = %preload7808vector_func.i, %postload7806vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7811vector_func.i, label %postload7812vector_func.i

preload7811vector_func.i:                         ; preds = %postload7809vector_func.i
  %.sum8593vector_func.i = add i64 %dim_0_vector_tid.i, 248841
  %818 = getelementptr float addrspace(1)* %4, i64 %.sum8593vector_func.i
  %exData4828vector_func.i = extractelement <16 x float> %call.i51.i, i32 9
  store float %exData4828vector_func.i, float addrspace(1)* %818, align 4
  br label %postload7812vector_func.i

postload7812vector_func.i:                        ; preds = %preload7811vector_func.i, %postload7809vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7814vector_func.i, label %postload7815vector_func.i

preload7814vector_func.i:                         ; preds = %postload7812vector_func.i
  %.sum8592vector_func.i = add i64 %dim_0_vector_tid.i, 248842
  %819 = getelementptr float addrspace(1)* %4, i64 %.sum8592vector_func.i
  %exData4831vector_func.i = extractelement <16 x float> %call.i51.i, i32 10
  store float %exData4831vector_func.i, float addrspace(1)* %819, align 4
  br label %postload7815vector_func.i

postload7815vector_func.i:                        ; preds = %preload7814vector_func.i, %postload7812vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7817vector_func.i, label %postload7818vector_func.i

preload7817vector_func.i:                         ; preds = %postload7815vector_func.i
  %.sum8591vector_func.i = add i64 %dim_0_vector_tid.i, 248843
  %820 = getelementptr float addrspace(1)* %4, i64 %.sum8591vector_func.i
  %exData4834vector_func.i = extractelement <16 x float> %call.i51.i, i32 11
  store float %exData4834vector_func.i, float addrspace(1)* %820, align 4
  br label %postload7818vector_func.i

postload7818vector_func.i:                        ; preds = %preload7817vector_func.i, %postload7815vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7820vector_func.i, label %postload7821vector_func.i

preload7820vector_func.i:                         ; preds = %postload7818vector_func.i
  %.sum8590vector_func.i = add i64 %dim_0_vector_tid.i, 248844
  %821 = getelementptr float addrspace(1)* %4, i64 %.sum8590vector_func.i
  %exData4837vector_func.i = extractelement <16 x float> %call.i51.i, i32 12
  store float %exData4837vector_func.i, float addrspace(1)* %821, align 4
  br label %postload7821vector_func.i

postload7821vector_func.i:                        ; preds = %preload7820vector_func.i, %postload7818vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7823vector_func.i, label %postload7824vector_func.i

preload7823vector_func.i:                         ; preds = %postload7821vector_func.i
  %.sum8589vector_func.i = add i64 %dim_0_vector_tid.i, 248845
  %822 = getelementptr float addrspace(1)* %4, i64 %.sum8589vector_func.i
  %exData4840vector_func.i = extractelement <16 x float> %call.i51.i, i32 13
  store float %exData4840vector_func.i, float addrspace(1)* %822, align 4
  br label %postload7824vector_func.i

postload7824vector_func.i:                        ; preds = %preload7823vector_func.i, %postload7821vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7826vector_func.i, label %postload7827vector_func.i

preload7826vector_func.i:                         ; preds = %postload7824vector_func.i
  %.sum8588vector_func.i = add i64 %dim_0_vector_tid.i, 248846
  %823 = getelementptr float addrspace(1)* %4, i64 %.sum8588vector_func.i
  %exData4843vector_func.i = extractelement <16 x float> %call.i51.i, i32 14
  store float %exData4843vector_func.i, float addrspace(1)* %823, align 4
  br label %postload7827vector_func.i

postload7827vector_func.i:                        ; preds = %preload7826vector_func.i, %postload7824vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7829vector_func.i, label %postload7830vector_func.i

preload7829vector_func.i:                         ; preds = %postload7827vector_func.i
  %.sum8587vector_func.i = add i64 %dim_0_vector_tid.i, 248847
  %824 = getelementptr float addrspace(1)* %4, i64 %.sum8587vector_func.i
  %exData4846vector_func.i = extractelement <16 x float> %call.i51.i, i32 15
  store float %exData4846vector_func.i, float addrspace(1)* %824, align 4
  br label %postload7830vector_func.i

postload7830vector_func.i:                        ; preds = %preload7829vector_func.i, %postload7827vector_func.i
  %mul5012040vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000, float 0x40E7BDB960000000>
  %sub5022041vector_func.i = fsub <16 x float> <float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000, float 0x4017AE7B00000000>, %mul5012040vector_func.i
  %mul5032042vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000, float 0x400A409C60000000>
  %add5042043vector_func.i = fadd <16 x float> %sub5022041vector_func.i, %mul5032042vector_func.i
  %mul.i782044vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000, float 0x3D2BA34D60000000>
  %add.i792045vector_func.i = fadd <16 x float> %mul.i782044vector_func.i, <float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000, float 0xBDDBBA1D20000000>
  %mul1.i802046vector_func.i = fmul <16 x float> %add.i792045vector_func.i, %mul567vector_func.i
  %add2.i812047vector_func.i = fadd <16 x float> %mul1.i802046vector_func.i, <float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000, float 0xBE9AAE7FE0000000>
  %mul3.i822048vector_func.i = fmul <16 x float> %add2.i812047vector_func.i, %mul567vector_func.i
  %add4.i832049vector_func.i = fadd <16 x float> %mul3.i822048vector_func.i, <float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000, float 0x3F6C935E60000000>
  %mul5.i842050vector_func.i = fmul <16 x float> %add4.i832049vector_func.i, %mul567vector_func.i
  %add5062051vector_func.i = fadd <16 x float> %add5042043vector_func.i, %mul5.i842050vector_func.i
  %call.i52.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5062051vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7832vector_func.i, label %postload7833vector_func.i

preload7832vector_func.i:                         ; preds = %postload7830vector_func.i
  %extract2053vector_func.i = add i64 %dim_0_vector_tid.i, 262656
  %825 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2053vector_func.i
  %exData4850vector_func.i = extractelement <16 x float> %call.i52.i, i32 0
  store float %exData4850vector_func.i, float addrspace(1)* %825, align 4
  br label %postload7833vector_func.i

postload7833vector_func.i:                        ; preds = %preload7832vector_func.i, %postload7830vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7835vector_func.i, label %postload7836vector_func.i

preload7835vector_func.i:                         ; preds = %postload7833vector_func.i
  %.sum8586vector_func.i = add i64 %dim_0_vector_tid.i, 262657
  %826 = getelementptr float addrspace(1)* %4, i64 %.sum8586vector_func.i
  %exData4853vector_func.i = extractelement <16 x float> %call.i52.i, i32 1
  store float %exData4853vector_func.i, float addrspace(1)* %826, align 4
  br label %postload7836vector_func.i

postload7836vector_func.i:                        ; preds = %preload7835vector_func.i, %postload7833vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7838vector_func.i, label %postload7839vector_func.i

preload7838vector_func.i:                         ; preds = %postload7836vector_func.i
  %.sum8585vector_func.i = add i64 %dim_0_vector_tid.i, 262658
  %827 = getelementptr float addrspace(1)* %4, i64 %.sum8585vector_func.i
  %exData4856vector_func.i = extractelement <16 x float> %call.i52.i, i32 2
  store float %exData4856vector_func.i, float addrspace(1)* %827, align 4
  br label %postload7839vector_func.i

postload7839vector_func.i:                        ; preds = %preload7838vector_func.i, %postload7836vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7841vector_func.i, label %postload7842vector_func.i

preload7841vector_func.i:                         ; preds = %postload7839vector_func.i
  %.sum8584vector_func.i = add i64 %dim_0_vector_tid.i, 262659
  %828 = getelementptr float addrspace(1)* %4, i64 %.sum8584vector_func.i
  %exData4859vector_func.i = extractelement <16 x float> %call.i52.i, i32 3
  store float %exData4859vector_func.i, float addrspace(1)* %828, align 4
  br label %postload7842vector_func.i

postload7842vector_func.i:                        ; preds = %preload7841vector_func.i, %postload7839vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7844vector_func.i, label %postload7845vector_func.i

preload7844vector_func.i:                         ; preds = %postload7842vector_func.i
  %.sum8583vector_func.i = add i64 %dim_0_vector_tid.i, 262660
  %829 = getelementptr float addrspace(1)* %4, i64 %.sum8583vector_func.i
  %exData4862vector_func.i = extractelement <16 x float> %call.i52.i, i32 4
  store float %exData4862vector_func.i, float addrspace(1)* %829, align 4
  br label %postload7845vector_func.i

postload7845vector_func.i:                        ; preds = %preload7844vector_func.i, %postload7842vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7847vector_func.i, label %postload7848vector_func.i

preload7847vector_func.i:                         ; preds = %postload7845vector_func.i
  %.sum8582vector_func.i = add i64 %dim_0_vector_tid.i, 262661
  %830 = getelementptr float addrspace(1)* %4, i64 %.sum8582vector_func.i
  %exData4865vector_func.i = extractelement <16 x float> %call.i52.i, i32 5
  store float %exData4865vector_func.i, float addrspace(1)* %830, align 4
  br label %postload7848vector_func.i

postload7848vector_func.i:                        ; preds = %preload7847vector_func.i, %postload7845vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7850vector_func.i, label %postload7851vector_func.i

preload7850vector_func.i:                         ; preds = %postload7848vector_func.i
  %.sum8581vector_func.i = add i64 %dim_0_vector_tid.i, 262662
  %831 = getelementptr float addrspace(1)* %4, i64 %.sum8581vector_func.i
  %exData4868vector_func.i = extractelement <16 x float> %call.i52.i, i32 6
  store float %exData4868vector_func.i, float addrspace(1)* %831, align 4
  br label %postload7851vector_func.i

postload7851vector_func.i:                        ; preds = %preload7850vector_func.i, %postload7848vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7853vector_func.i, label %postload7854vector_func.i

preload7853vector_func.i:                         ; preds = %postload7851vector_func.i
  %.sum8580vector_func.i = add i64 %dim_0_vector_tid.i, 262663
  %832 = getelementptr float addrspace(1)* %4, i64 %.sum8580vector_func.i
  %exData4871vector_func.i = extractelement <16 x float> %call.i52.i, i32 7
  store float %exData4871vector_func.i, float addrspace(1)* %832, align 4
  br label %postload7854vector_func.i

postload7854vector_func.i:                        ; preds = %preload7853vector_func.i, %postload7851vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7856vector_func.i, label %postload7857vector_func.i

preload7856vector_func.i:                         ; preds = %postload7854vector_func.i
  %.sum8579vector_func.i = add i64 %dim_0_vector_tid.i, 262664
  %833 = getelementptr float addrspace(1)* %4, i64 %.sum8579vector_func.i
  %exData4874vector_func.i = extractelement <16 x float> %call.i52.i, i32 8
  store float %exData4874vector_func.i, float addrspace(1)* %833, align 4
  br label %postload7857vector_func.i

postload7857vector_func.i:                        ; preds = %preload7856vector_func.i, %postload7854vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7859vector_func.i, label %postload7860vector_func.i

preload7859vector_func.i:                         ; preds = %postload7857vector_func.i
  %.sum8578vector_func.i = add i64 %dim_0_vector_tid.i, 262665
  %834 = getelementptr float addrspace(1)* %4, i64 %.sum8578vector_func.i
  %exData4877vector_func.i = extractelement <16 x float> %call.i52.i, i32 9
  store float %exData4877vector_func.i, float addrspace(1)* %834, align 4
  br label %postload7860vector_func.i

postload7860vector_func.i:                        ; preds = %preload7859vector_func.i, %postload7857vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7862vector_func.i, label %postload7863vector_func.i

preload7862vector_func.i:                         ; preds = %postload7860vector_func.i
  %.sum8577vector_func.i = add i64 %dim_0_vector_tid.i, 262666
  %835 = getelementptr float addrspace(1)* %4, i64 %.sum8577vector_func.i
  %exData4880vector_func.i = extractelement <16 x float> %call.i52.i, i32 10
  store float %exData4880vector_func.i, float addrspace(1)* %835, align 4
  br label %postload7863vector_func.i

postload7863vector_func.i:                        ; preds = %preload7862vector_func.i, %postload7860vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7865vector_func.i, label %postload7866vector_func.i

preload7865vector_func.i:                         ; preds = %postload7863vector_func.i
  %.sum8576vector_func.i = add i64 %dim_0_vector_tid.i, 262667
  %836 = getelementptr float addrspace(1)* %4, i64 %.sum8576vector_func.i
  %exData4883vector_func.i = extractelement <16 x float> %call.i52.i, i32 11
  store float %exData4883vector_func.i, float addrspace(1)* %836, align 4
  br label %postload7866vector_func.i

postload7866vector_func.i:                        ; preds = %preload7865vector_func.i, %postload7863vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7868vector_func.i, label %postload7869vector_func.i

preload7868vector_func.i:                         ; preds = %postload7866vector_func.i
  %.sum8575vector_func.i = add i64 %dim_0_vector_tid.i, 262668
  %837 = getelementptr float addrspace(1)* %4, i64 %.sum8575vector_func.i
  %exData4886vector_func.i = extractelement <16 x float> %call.i52.i, i32 12
  store float %exData4886vector_func.i, float addrspace(1)* %837, align 4
  br label %postload7869vector_func.i

postload7869vector_func.i:                        ; preds = %preload7868vector_func.i, %postload7866vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7871vector_func.i, label %postload7872vector_func.i

preload7871vector_func.i:                         ; preds = %postload7869vector_func.i
  %.sum8574vector_func.i = add i64 %dim_0_vector_tid.i, 262669
  %838 = getelementptr float addrspace(1)* %4, i64 %.sum8574vector_func.i
  %exData4889vector_func.i = extractelement <16 x float> %call.i52.i, i32 13
  store float %exData4889vector_func.i, float addrspace(1)* %838, align 4
  br label %postload7872vector_func.i

postload7872vector_func.i:                        ; preds = %preload7871vector_func.i, %postload7869vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7874vector_func.i, label %postload7875vector_func.i

preload7874vector_func.i:                         ; preds = %postload7872vector_func.i
  %.sum8573vector_func.i = add i64 %dim_0_vector_tid.i, 262670
  %839 = getelementptr float addrspace(1)* %4, i64 %.sum8573vector_func.i
  %exData4892vector_func.i = extractelement <16 x float> %call.i52.i, i32 14
  store float %exData4892vector_func.i, float addrspace(1)* %839, align 4
  br label %postload7875vector_func.i

postload7875vector_func.i:                        ; preds = %preload7874vector_func.i, %postload7872vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7877vector_func.i, label %postload7878vector_func.i

preload7877vector_func.i:                         ; preds = %postload7875vector_func.i
  %.sum8572vector_func.i = add i64 %dim_0_vector_tid.i, 262671
  %840 = getelementptr float addrspace(1)* %4, i64 %.sum8572vector_func.i
  %exData4895vector_func.i = extractelement <16 x float> %call.i52.i, i32 15
  store float %exData4895vector_func.i, float addrspace(1)* %840, align 4
  br label %postload7878vector_func.i

postload7878vector_func.i:                        ; preds = %preload7877vector_func.i, %postload7875vector_func.i
  %mul5112070vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000, float 0x40E1057B20000000>
  %sub5122071vector_func.i = fsub <16 x float> <float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000, float 0x4021056580000000>, %mul5112070vector_func.i
  %mul5132072vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000, float 0x4009B321A0000000>
  %add5142073vector_func.i = fadd <16 x float> %sub5122071vector_func.i, %mul5132072vector_func.i
  %mul.i712074vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000, float 0x3D69E31600000000>
  %add.i722075vector_func.i = fadd <16 x float> %mul.i712074vector_func.i, <float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000, float 0xBE299A2640000000>
  %mul1.i732076vector_func.i = fmul <16 x float> %add.i722075vector_func.i, %mul567vector_func.i
  %add2.i742077vector_func.i = fadd <16 x float> %mul1.i732076vector_func.i, <float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000, float 0x3ED21EBBA0000000>
  %mul3.i752078vector_func.i = fmul <16 x float> %add2.i742077vector_func.i, %mul567vector_func.i
  %add4.i762079vector_func.i = fadd <16 x float> %mul3.i752078vector_func.i, <float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000, float 0x3F48D17F20000000>
  %mul5.i772080vector_func.i = fmul <16 x float> %add4.i762079vector_func.i, %mul567vector_func.i
  %add5162081vector_func.i = fadd <16 x float> %add5142073vector_func.i, %mul5.i772080vector_func.i
  %call.i53.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5162081vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7880vector_func.i, label %postload7881vector_func.i

preload7880vector_func.i:                         ; preds = %postload7878vector_func.i
  %extract2083vector_func.i = add i64 %dim_0_vector_tid.i, 276480
  %841 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2083vector_func.i
  %exData4899vector_func.i = extractelement <16 x float> %call.i53.i, i32 0
  store float %exData4899vector_func.i, float addrspace(1)* %841, align 4
  br label %postload7881vector_func.i

postload7881vector_func.i:                        ; preds = %preload7880vector_func.i, %postload7878vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7883vector_func.i, label %postload7884vector_func.i

preload7883vector_func.i:                         ; preds = %postload7881vector_func.i
  %.sum8571vector_func.i = add i64 %dim_0_vector_tid.i, 276481
  %842 = getelementptr float addrspace(1)* %4, i64 %.sum8571vector_func.i
  %exData4902vector_func.i = extractelement <16 x float> %call.i53.i, i32 1
  store float %exData4902vector_func.i, float addrspace(1)* %842, align 4
  br label %postload7884vector_func.i

postload7884vector_func.i:                        ; preds = %preload7883vector_func.i, %postload7881vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7886vector_func.i, label %postload7887vector_func.i

preload7886vector_func.i:                         ; preds = %postload7884vector_func.i
  %.sum8570vector_func.i = add i64 %dim_0_vector_tid.i, 276482
  %843 = getelementptr float addrspace(1)* %4, i64 %.sum8570vector_func.i
  %exData4905vector_func.i = extractelement <16 x float> %call.i53.i, i32 2
  store float %exData4905vector_func.i, float addrspace(1)* %843, align 4
  br label %postload7887vector_func.i

postload7887vector_func.i:                        ; preds = %preload7886vector_func.i, %postload7884vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7889vector_func.i, label %postload7890vector_func.i

preload7889vector_func.i:                         ; preds = %postload7887vector_func.i
  %.sum8569vector_func.i = add i64 %dim_0_vector_tid.i, 276483
  %844 = getelementptr float addrspace(1)* %4, i64 %.sum8569vector_func.i
  %exData4908vector_func.i = extractelement <16 x float> %call.i53.i, i32 3
  store float %exData4908vector_func.i, float addrspace(1)* %844, align 4
  br label %postload7890vector_func.i

postload7890vector_func.i:                        ; preds = %preload7889vector_func.i, %postload7887vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7892vector_func.i, label %postload7893vector_func.i

preload7892vector_func.i:                         ; preds = %postload7890vector_func.i
  %.sum8568vector_func.i = add i64 %dim_0_vector_tid.i, 276484
  %845 = getelementptr float addrspace(1)* %4, i64 %.sum8568vector_func.i
  %exData4911vector_func.i = extractelement <16 x float> %call.i53.i, i32 4
  store float %exData4911vector_func.i, float addrspace(1)* %845, align 4
  br label %postload7893vector_func.i

postload7893vector_func.i:                        ; preds = %preload7892vector_func.i, %postload7890vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7895vector_func.i, label %postload7896vector_func.i

preload7895vector_func.i:                         ; preds = %postload7893vector_func.i
  %.sum8567vector_func.i = add i64 %dim_0_vector_tid.i, 276485
  %846 = getelementptr float addrspace(1)* %4, i64 %.sum8567vector_func.i
  %exData4914vector_func.i = extractelement <16 x float> %call.i53.i, i32 5
  store float %exData4914vector_func.i, float addrspace(1)* %846, align 4
  br label %postload7896vector_func.i

postload7896vector_func.i:                        ; preds = %preload7895vector_func.i, %postload7893vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7898vector_func.i, label %postload7899vector_func.i

preload7898vector_func.i:                         ; preds = %postload7896vector_func.i
  %.sum8566vector_func.i = add i64 %dim_0_vector_tid.i, 276486
  %847 = getelementptr float addrspace(1)* %4, i64 %.sum8566vector_func.i
  %exData4917vector_func.i = extractelement <16 x float> %call.i53.i, i32 6
  store float %exData4917vector_func.i, float addrspace(1)* %847, align 4
  br label %postload7899vector_func.i

postload7899vector_func.i:                        ; preds = %preload7898vector_func.i, %postload7896vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7901vector_func.i, label %postload7902vector_func.i

preload7901vector_func.i:                         ; preds = %postload7899vector_func.i
  %.sum8565vector_func.i = add i64 %dim_0_vector_tid.i, 276487
  %848 = getelementptr float addrspace(1)* %4, i64 %.sum8565vector_func.i
  %exData4920vector_func.i = extractelement <16 x float> %call.i53.i, i32 7
  store float %exData4920vector_func.i, float addrspace(1)* %848, align 4
  br label %postload7902vector_func.i

postload7902vector_func.i:                        ; preds = %preload7901vector_func.i, %postload7899vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7904vector_func.i, label %postload7905vector_func.i

preload7904vector_func.i:                         ; preds = %postload7902vector_func.i
  %.sum8564vector_func.i = add i64 %dim_0_vector_tid.i, 276488
  %849 = getelementptr float addrspace(1)* %4, i64 %.sum8564vector_func.i
  %exData4923vector_func.i = extractelement <16 x float> %call.i53.i, i32 8
  store float %exData4923vector_func.i, float addrspace(1)* %849, align 4
  br label %postload7905vector_func.i

postload7905vector_func.i:                        ; preds = %preload7904vector_func.i, %postload7902vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7907vector_func.i, label %postload7908vector_func.i

preload7907vector_func.i:                         ; preds = %postload7905vector_func.i
  %.sum8563vector_func.i = add i64 %dim_0_vector_tid.i, 276489
  %850 = getelementptr float addrspace(1)* %4, i64 %.sum8563vector_func.i
  %exData4926vector_func.i = extractelement <16 x float> %call.i53.i, i32 9
  store float %exData4926vector_func.i, float addrspace(1)* %850, align 4
  br label %postload7908vector_func.i

postload7908vector_func.i:                        ; preds = %preload7907vector_func.i, %postload7905vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7910vector_func.i, label %postload7911vector_func.i

preload7910vector_func.i:                         ; preds = %postload7908vector_func.i
  %.sum8562vector_func.i = add i64 %dim_0_vector_tid.i, 276490
  %851 = getelementptr float addrspace(1)* %4, i64 %.sum8562vector_func.i
  %exData4929vector_func.i = extractelement <16 x float> %call.i53.i, i32 10
  store float %exData4929vector_func.i, float addrspace(1)* %851, align 4
  br label %postload7911vector_func.i

postload7911vector_func.i:                        ; preds = %preload7910vector_func.i, %postload7908vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7913vector_func.i, label %postload7914vector_func.i

preload7913vector_func.i:                         ; preds = %postload7911vector_func.i
  %.sum8561vector_func.i = add i64 %dim_0_vector_tid.i, 276491
  %852 = getelementptr float addrspace(1)* %4, i64 %.sum8561vector_func.i
  %exData4932vector_func.i = extractelement <16 x float> %call.i53.i, i32 11
  store float %exData4932vector_func.i, float addrspace(1)* %852, align 4
  br label %postload7914vector_func.i

postload7914vector_func.i:                        ; preds = %preload7913vector_func.i, %postload7911vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7916vector_func.i, label %postload7917vector_func.i

preload7916vector_func.i:                         ; preds = %postload7914vector_func.i
  %.sum8560vector_func.i = add i64 %dim_0_vector_tid.i, 276492
  %853 = getelementptr float addrspace(1)* %4, i64 %.sum8560vector_func.i
  %exData4935vector_func.i = extractelement <16 x float> %call.i53.i, i32 12
  store float %exData4935vector_func.i, float addrspace(1)* %853, align 4
  br label %postload7917vector_func.i

postload7917vector_func.i:                        ; preds = %preload7916vector_func.i, %postload7914vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7919vector_func.i, label %postload7920vector_func.i

preload7919vector_func.i:                         ; preds = %postload7917vector_func.i
  %.sum8559vector_func.i = add i64 %dim_0_vector_tid.i, 276493
  %854 = getelementptr float addrspace(1)* %4, i64 %.sum8559vector_func.i
  %exData4938vector_func.i = extractelement <16 x float> %call.i53.i, i32 13
  store float %exData4938vector_func.i, float addrspace(1)* %854, align 4
  br label %postload7920vector_func.i

postload7920vector_func.i:                        ; preds = %preload7919vector_func.i, %postload7917vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7922vector_func.i, label %postload7923vector_func.i

preload7922vector_func.i:                         ; preds = %postload7920vector_func.i
  %.sum8558vector_func.i = add i64 %dim_0_vector_tid.i, 276494
  %855 = getelementptr float addrspace(1)* %4, i64 %.sum8558vector_func.i
  %exData4941vector_func.i = extractelement <16 x float> %call.i53.i, i32 14
  store float %exData4941vector_func.i, float addrspace(1)* %855, align 4
  br label %postload7923vector_func.i

postload7923vector_func.i:                        ; preds = %preload7922vector_func.i, %postload7920vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7925vector_func.i, label %postload7926vector_func.i

preload7925vector_func.i:                         ; preds = %postload7923vector_func.i
  %.sum8557vector_func.i = add i64 %dim_0_vector_tid.i, 276495
  %856 = getelementptr float addrspace(1)* %4, i64 %.sum8557vector_func.i
  %exData4944vector_func.i = extractelement <16 x float> %call.i53.i, i32 15
  store float %exData4944vector_func.i, float addrspace(1)* %856, align 4
  br label %postload7926vector_func.i

postload7926vector_func.i:                        ; preds = %preload7925vector_func.i, %postload7923vector_func.i
  %mul5212100vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000, float 0x40B3E1C6A0000000>
  %sub5222101vector_func.i = fsub <16 x float> <float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000, float 0x401063AAC0000000>, %mul5212100vector_func.i
  %mul5232102vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000, float 0x400FAC71E0000000>
  %add5242103vector_func.i = fadd <16 x float> %sub5222101vector_func.i, %mul5232102vector_func.i
  %mul.i642104vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000, float 0x3D77BD4180000000>
  %add.i652105vector_func.i = fadd <16 x float> %mul.i642104vector_func.i, <float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000, float 0xBE38C0BFC0000000>
  %mul1.i662106vector_func.i = fmul <16 x float> %add.i652105vector_func.i, %mul567vector_func.i
  %add2.i672107vector_func.i = fadd <16 x float> %mul1.i662106vector_func.i, <float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000, float 0x3EE3F52280000000>
  %mul3.i682108vector_func.i = fmul <16 x float> %add2.i672107vector_func.i, %mul567vector_func.i
  %add4.i692109vector_func.i = fadd <16 x float> %mul3.i682108vector_func.i, <float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000, float 0xBF6F0244A0000000>
  %mul5.i702110vector_func.i = fmul <16 x float> %add4.i692109vector_func.i, %mul567vector_func.i
  %add5262111vector_func.i = fadd <16 x float> %add5242103vector_func.i, %mul5.i702110vector_func.i
  %call.i54.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5262111vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7928vector_func.i, label %postload7929vector_func.i

preload7928vector_func.i:                         ; preds = %postload7926vector_func.i
  %extract2113vector_func.i = add i64 %dim_0_vector_tid.i, 290304
  %857 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2113vector_func.i
  %exData4948vector_func.i = extractelement <16 x float> %call.i54.i, i32 0
  store float %exData4948vector_func.i, float addrspace(1)* %857, align 4
  br label %postload7929vector_func.i

postload7929vector_func.i:                        ; preds = %preload7928vector_func.i, %postload7926vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7931vector_func.i, label %postload7932vector_func.i

preload7931vector_func.i:                         ; preds = %postload7929vector_func.i
  %.sum8556vector_func.i = add i64 %dim_0_vector_tid.i, 290305
  %858 = getelementptr float addrspace(1)* %4, i64 %.sum8556vector_func.i
  %exData4951vector_func.i = extractelement <16 x float> %call.i54.i, i32 1
  store float %exData4951vector_func.i, float addrspace(1)* %858, align 4
  br label %postload7932vector_func.i

postload7932vector_func.i:                        ; preds = %preload7931vector_func.i, %postload7929vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7934vector_func.i, label %postload7935vector_func.i

preload7934vector_func.i:                         ; preds = %postload7932vector_func.i
  %.sum8555vector_func.i = add i64 %dim_0_vector_tid.i, 290306
  %859 = getelementptr float addrspace(1)* %4, i64 %.sum8555vector_func.i
  %exData4954vector_func.i = extractelement <16 x float> %call.i54.i, i32 2
  store float %exData4954vector_func.i, float addrspace(1)* %859, align 4
  br label %postload7935vector_func.i

postload7935vector_func.i:                        ; preds = %preload7934vector_func.i, %postload7932vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7937vector_func.i, label %postload7938vector_func.i

preload7937vector_func.i:                         ; preds = %postload7935vector_func.i
  %.sum8554vector_func.i = add i64 %dim_0_vector_tid.i, 290307
  %860 = getelementptr float addrspace(1)* %4, i64 %.sum8554vector_func.i
  %exData4957vector_func.i = extractelement <16 x float> %call.i54.i, i32 3
  store float %exData4957vector_func.i, float addrspace(1)* %860, align 4
  br label %postload7938vector_func.i

postload7938vector_func.i:                        ; preds = %preload7937vector_func.i, %postload7935vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7940vector_func.i, label %postload7941vector_func.i

preload7940vector_func.i:                         ; preds = %postload7938vector_func.i
  %.sum8553vector_func.i = add i64 %dim_0_vector_tid.i, 290308
  %861 = getelementptr float addrspace(1)* %4, i64 %.sum8553vector_func.i
  %exData4960vector_func.i = extractelement <16 x float> %call.i54.i, i32 4
  store float %exData4960vector_func.i, float addrspace(1)* %861, align 4
  br label %postload7941vector_func.i

postload7941vector_func.i:                        ; preds = %preload7940vector_func.i, %postload7938vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7943vector_func.i, label %postload7944vector_func.i

preload7943vector_func.i:                         ; preds = %postload7941vector_func.i
  %.sum8552vector_func.i = add i64 %dim_0_vector_tid.i, 290309
  %862 = getelementptr float addrspace(1)* %4, i64 %.sum8552vector_func.i
  %exData4963vector_func.i = extractelement <16 x float> %call.i54.i, i32 5
  store float %exData4963vector_func.i, float addrspace(1)* %862, align 4
  br label %postload7944vector_func.i

postload7944vector_func.i:                        ; preds = %preload7943vector_func.i, %postload7941vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7946vector_func.i, label %postload7947vector_func.i

preload7946vector_func.i:                         ; preds = %postload7944vector_func.i
  %.sum8551vector_func.i = add i64 %dim_0_vector_tid.i, 290310
  %863 = getelementptr float addrspace(1)* %4, i64 %.sum8551vector_func.i
  %exData4966vector_func.i = extractelement <16 x float> %call.i54.i, i32 6
  store float %exData4966vector_func.i, float addrspace(1)* %863, align 4
  br label %postload7947vector_func.i

postload7947vector_func.i:                        ; preds = %preload7946vector_func.i, %postload7944vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7949vector_func.i, label %postload7950vector_func.i

preload7949vector_func.i:                         ; preds = %postload7947vector_func.i
  %.sum8550vector_func.i = add i64 %dim_0_vector_tid.i, 290311
  %864 = getelementptr float addrspace(1)* %4, i64 %.sum8550vector_func.i
  %exData4969vector_func.i = extractelement <16 x float> %call.i54.i, i32 7
  store float %exData4969vector_func.i, float addrspace(1)* %864, align 4
  br label %postload7950vector_func.i

postload7950vector_func.i:                        ; preds = %preload7949vector_func.i, %postload7947vector_func.i
  br i1 %exmask3942vector_func.i, label %preload7952vector_func.i, label %postload7953vector_func.i

preload7952vector_func.i:                         ; preds = %postload7950vector_func.i
  %.sum8549vector_func.i = add i64 %dim_0_vector_tid.i, 290312
  %865 = getelementptr float addrspace(1)* %4, i64 %.sum8549vector_func.i
  %exData4972vector_func.i = extractelement <16 x float> %call.i54.i, i32 8
  store float %exData4972vector_func.i, float addrspace(1)* %865, align 4
  br label %postload7953vector_func.i

postload7953vector_func.i:                        ; preds = %preload7952vector_func.i, %postload7950vector_func.i
  br i1 %exmask3945vector_func.i, label %preload7955vector_func.i, label %postload7956vector_func.i

preload7955vector_func.i:                         ; preds = %postload7953vector_func.i
  %.sum8548vector_func.i = add i64 %dim_0_vector_tid.i, 290313
  %866 = getelementptr float addrspace(1)* %4, i64 %.sum8548vector_func.i
  %exData4975vector_func.i = extractelement <16 x float> %call.i54.i, i32 9
  store float %exData4975vector_func.i, float addrspace(1)* %866, align 4
  br label %postload7956vector_func.i

postload7956vector_func.i:                        ; preds = %preload7955vector_func.i, %postload7953vector_func.i
  br i1 %exmask3948vector_func.i, label %preload7958vector_func.i, label %postload7959vector_func.i

preload7958vector_func.i:                         ; preds = %postload7956vector_func.i
  %.sum8547vector_func.i = add i64 %dim_0_vector_tid.i, 290314
  %867 = getelementptr float addrspace(1)* %4, i64 %.sum8547vector_func.i
  %exData4978vector_func.i = extractelement <16 x float> %call.i54.i, i32 10
  store float %exData4978vector_func.i, float addrspace(1)* %867, align 4
  br label %postload7959vector_func.i

postload7959vector_func.i:                        ; preds = %preload7958vector_func.i, %postload7956vector_func.i
  br i1 %exmask3951vector_func.i, label %preload7961vector_func.i, label %postload7962vector_func.i

preload7961vector_func.i:                         ; preds = %postload7959vector_func.i
  %.sum8546vector_func.i = add i64 %dim_0_vector_tid.i, 290315
  %868 = getelementptr float addrspace(1)* %4, i64 %.sum8546vector_func.i
  %exData4981vector_func.i = extractelement <16 x float> %call.i54.i, i32 11
  store float %exData4981vector_func.i, float addrspace(1)* %868, align 4
  br label %postload7962vector_func.i

postload7962vector_func.i:                        ; preds = %preload7961vector_func.i, %postload7959vector_func.i
  br i1 %exmask3954vector_func.i, label %preload7964vector_func.i, label %postload7965vector_func.i

preload7964vector_func.i:                         ; preds = %postload7962vector_func.i
  %.sum8545vector_func.i = add i64 %dim_0_vector_tid.i, 290316
  %869 = getelementptr float addrspace(1)* %4, i64 %.sum8545vector_func.i
  %exData4984vector_func.i = extractelement <16 x float> %call.i54.i, i32 12
  store float %exData4984vector_func.i, float addrspace(1)* %869, align 4
  br label %postload7965vector_func.i

postload7965vector_func.i:                        ; preds = %preload7964vector_func.i, %postload7962vector_func.i
  br i1 %exmask3957vector_func.i, label %preload7967vector_func.i, label %postload7968vector_func.i

preload7967vector_func.i:                         ; preds = %postload7965vector_func.i
  %.sum8544vector_func.i = add i64 %dim_0_vector_tid.i, 290317
  %870 = getelementptr float addrspace(1)* %4, i64 %.sum8544vector_func.i
  %exData4987vector_func.i = extractelement <16 x float> %call.i54.i, i32 13
  store float %exData4987vector_func.i, float addrspace(1)* %870, align 4
  br label %postload7968vector_func.i

postload7968vector_func.i:                        ; preds = %preload7967vector_func.i, %postload7965vector_func.i
  br i1 %exmask3960vector_func.i, label %preload7970vector_func.i, label %postload7971vector_func.i

preload7970vector_func.i:                         ; preds = %postload7968vector_func.i
  %.sum8543vector_func.i = add i64 %dim_0_vector_tid.i, 290318
  %871 = getelementptr float addrspace(1)* %4, i64 %.sum8543vector_func.i
  %exData4990vector_func.i = extractelement <16 x float> %call.i54.i, i32 14
  store float %exData4990vector_func.i, float addrspace(1)* %871, align 4
  br label %postload7971vector_func.i

postload7971vector_func.i:                        ; preds = %preload7970vector_func.i, %postload7968vector_func.i
  br i1 %exmask3963vector_func.i, label %preload7973vector_func.i, label %postload7974vector_func.i

preload7973vector_func.i:                         ; preds = %postload7971vector_func.i
  %.sum8542vector_func.i = add i64 %dim_0_vector_tid.i, 290319
  %872 = getelementptr float addrspace(1)* %4, i64 %.sum8542vector_func.i
  %exData4993vector_func.i = extractelement <16 x float> %call.i54.i, i32 15
  store float %exData4993vector_func.i, float addrspace(1)* %872, align 4
  br label %postload7974vector_func.i

postload7974vector_func.i:                        ; preds = %preload7973vector_func.i, %postload7971vector_func.i
  %mul5312130vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000, float 0x40C914D040000000>
  %sub5322131vector_func.i = fsub <16 x float> <float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000, float 0x4012D42EA0000000>, %mul5312130vector_func.i
  %mul5332132vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000, float 0x401139D220000000>
  %add5342133vector_func.i = fadd <16 x float> %sub5322131vector_func.i, %mul5332132vector_func.i
  %mul.i572134vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000, float 0x3D74469A00000000>
  %add.i582135vector_func.i = fadd <16 x float> %mul.i572134vector_func.i, <float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000, float 0xBE35718E40000000>
  %mul1.i592136vector_func.i = fmul <16 x float> %add.i582135vector_func.i, %mul567vector_func.i
  %add2.i602137vector_func.i = fadd <16 x float> %mul1.i592136vector_func.i, <float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000, float 0x3EE1605BC0000000>
  %mul3.i612138vector_func.i = fmul <16 x float> %add2.i602137vector_func.i, %mul567vector_func.i
  %add4.i622139vector_func.i = fadd <16 x float> %mul3.i612138vector_func.i, <float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000, float 0xBF6125F4E0000000>
  %mul5.i632140vector_func.i = fmul <16 x float> %add4.i622139vector_func.i, %mul567vector_func.i
  %add5362141vector_func.i = fadd <16 x float> %add5342133vector_func.i, %mul5.i632140vector_func.i
  %call.i55.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5362141vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload7976vector_func.i, label %postload7977vector_func.i

preload7976vector_func.i:                         ; preds = %postload7974vector_func.i
  %extract2143vector_func.i = add i64 %dim_0_vector_tid.i, 304128
  %873 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2143vector_func.i
  %exData4997vector_func.i = extractelement <16 x float> %call.i55.i, i32 0
  store float %exData4997vector_func.i, float addrspace(1)* %873, align 4
  br label %postload7977vector_func.i

postload7977vector_func.i:                        ; preds = %preload7976vector_func.i, %postload7974vector_func.i
  br i1 %exmask3921vector_func.i, label %preload7979vector_func.i, label %postload7980vector_func.i

preload7979vector_func.i:                         ; preds = %postload7977vector_func.i
  %.sum8541vector_func.i = add i64 %dim_0_vector_tid.i, 304129
  %874 = getelementptr float addrspace(1)* %4, i64 %.sum8541vector_func.i
  %exData5000vector_func.i = extractelement <16 x float> %call.i55.i, i32 1
  store float %exData5000vector_func.i, float addrspace(1)* %874, align 4
  br label %postload7980vector_func.i

postload7980vector_func.i:                        ; preds = %preload7979vector_func.i, %postload7977vector_func.i
  br i1 %exmask3924vector_func.i, label %preload7982vector_func.i, label %postload7983vector_func.i

preload7982vector_func.i:                         ; preds = %postload7980vector_func.i
  %.sum8540vector_func.i = add i64 %dim_0_vector_tid.i, 304130
  %875 = getelementptr float addrspace(1)* %4, i64 %.sum8540vector_func.i
  %exData5003vector_func.i = extractelement <16 x float> %call.i55.i, i32 2
  store float %exData5003vector_func.i, float addrspace(1)* %875, align 4
  br label %postload7983vector_func.i

postload7983vector_func.i:                        ; preds = %preload7982vector_func.i, %postload7980vector_func.i
  br i1 %exmask3927vector_func.i, label %preload7985vector_func.i, label %postload7986vector_func.i

preload7985vector_func.i:                         ; preds = %postload7983vector_func.i
  %.sum8539vector_func.i = add i64 %dim_0_vector_tid.i, 304131
  %876 = getelementptr float addrspace(1)* %4, i64 %.sum8539vector_func.i
  %exData5006vector_func.i = extractelement <16 x float> %call.i55.i, i32 3
  store float %exData5006vector_func.i, float addrspace(1)* %876, align 4
  br label %postload7986vector_func.i

postload7986vector_func.i:                        ; preds = %preload7985vector_func.i, %postload7983vector_func.i
  br i1 %exmask3930vector_func.i, label %preload7988vector_func.i, label %postload7989vector_func.i

preload7988vector_func.i:                         ; preds = %postload7986vector_func.i
  %.sum8538vector_func.i = add i64 %dim_0_vector_tid.i, 304132
  %877 = getelementptr float addrspace(1)* %4, i64 %.sum8538vector_func.i
  %exData5009vector_func.i = extractelement <16 x float> %call.i55.i, i32 4
  store float %exData5009vector_func.i, float addrspace(1)* %877, align 4
  br label %postload7989vector_func.i

postload7989vector_func.i:                        ; preds = %preload7988vector_func.i, %postload7986vector_func.i
  br i1 %exmask3933vector_func.i, label %preload7991vector_func.i, label %postload7992vector_func.i

preload7991vector_func.i:                         ; preds = %postload7989vector_func.i
  %.sum8537vector_func.i = add i64 %dim_0_vector_tid.i, 304133
  %878 = getelementptr float addrspace(1)* %4, i64 %.sum8537vector_func.i
  %exData5012vector_func.i = extractelement <16 x float> %call.i55.i, i32 5
  store float %exData5012vector_func.i, float addrspace(1)* %878, align 4
  br label %postload7992vector_func.i

postload7992vector_func.i:                        ; preds = %preload7991vector_func.i, %postload7989vector_func.i
  br i1 %exmask3936vector_func.i, label %preload7994vector_func.i, label %postload7995vector_func.i

preload7994vector_func.i:                         ; preds = %postload7992vector_func.i
  %.sum8536vector_func.i = add i64 %dim_0_vector_tid.i, 304134
  %879 = getelementptr float addrspace(1)* %4, i64 %.sum8536vector_func.i
  %exData5015vector_func.i = extractelement <16 x float> %call.i55.i, i32 6
  store float %exData5015vector_func.i, float addrspace(1)* %879, align 4
  br label %postload7995vector_func.i

postload7995vector_func.i:                        ; preds = %preload7994vector_func.i, %postload7992vector_func.i
  br i1 %exmask3939vector_func.i, label %preload7997vector_func.i, label %postload7998vector_func.i

preload7997vector_func.i:                         ; preds = %postload7995vector_func.i
  %.sum8535vector_func.i = add i64 %dim_0_vector_tid.i, 304135
  %880 = getelementptr float addrspace(1)* %4, i64 %.sum8535vector_func.i
  %exData5018vector_func.i = extractelement <16 x float> %call.i55.i, i32 7
  store float %exData5018vector_func.i, float addrspace(1)* %880, align 4
  br label %postload7998vector_func.i

postload7998vector_func.i:                        ; preds = %preload7997vector_func.i, %postload7995vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8000vector_func.i, label %postload8001vector_func.i

preload8000vector_func.i:                         ; preds = %postload7998vector_func.i
  %.sum8534vector_func.i = add i64 %dim_0_vector_tid.i, 304136
  %881 = getelementptr float addrspace(1)* %4, i64 %.sum8534vector_func.i
  %exData5021vector_func.i = extractelement <16 x float> %call.i55.i, i32 8
  store float %exData5021vector_func.i, float addrspace(1)* %881, align 4
  br label %postload8001vector_func.i

postload8001vector_func.i:                        ; preds = %preload8000vector_func.i, %postload7998vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8003vector_func.i, label %postload8004vector_func.i

preload8003vector_func.i:                         ; preds = %postload8001vector_func.i
  %.sum8533vector_func.i = add i64 %dim_0_vector_tid.i, 304137
  %882 = getelementptr float addrspace(1)* %4, i64 %.sum8533vector_func.i
  %exData5024vector_func.i = extractelement <16 x float> %call.i55.i, i32 9
  store float %exData5024vector_func.i, float addrspace(1)* %882, align 4
  br label %postload8004vector_func.i

postload8004vector_func.i:                        ; preds = %preload8003vector_func.i, %postload8001vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8006vector_func.i, label %postload8007vector_func.i

preload8006vector_func.i:                         ; preds = %postload8004vector_func.i
  %.sum8532vector_func.i = add i64 %dim_0_vector_tid.i, 304138
  %883 = getelementptr float addrspace(1)* %4, i64 %.sum8532vector_func.i
  %exData5027vector_func.i = extractelement <16 x float> %call.i55.i, i32 10
  store float %exData5027vector_func.i, float addrspace(1)* %883, align 4
  br label %postload8007vector_func.i

postload8007vector_func.i:                        ; preds = %preload8006vector_func.i, %postload8004vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8009vector_func.i, label %postload8010vector_func.i

preload8009vector_func.i:                         ; preds = %postload8007vector_func.i
  %.sum8531vector_func.i = add i64 %dim_0_vector_tid.i, 304139
  %884 = getelementptr float addrspace(1)* %4, i64 %.sum8531vector_func.i
  %exData5030vector_func.i = extractelement <16 x float> %call.i55.i, i32 11
  store float %exData5030vector_func.i, float addrspace(1)* %884, align 4
  br label %postload8010vector_func.i

postload8010vector_func.i:                        ; preds = %preload8009vector_func.i, %postload8007vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8012vector_func.i, label %postload8013vector_func.i

preload8012vector_func.i:                         ; preds = %postload8010vector_func.i
  %.sum8530vector_func.i = add i64 %dim_0_vector_tid.i, 304140
  %885 = getelementptr float addrspace(1)* %4, i64 %.sum8530vector_func.i
  %exData5033vector_func.i = extractelement <16 x float> %call.i55.i, i32 12
  store float %exData5033vector_func.i, float addrspace(1)* %885, align 4
  br label %postload8013vector_func.i

postload8013vector_func.i:                        ; preds = %preload8012vector_func.i, %postload8010vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8015vector_func.i, label %postload8016vector_func.i

preload8015vector_func.i:                         ; preds = %postload8013vector_func.i
  %.sum8529vector_func.i = add i64 %dim_0_vector_tid.i, 304141
  %886 = getelementptr float addrspace(1)* %4, i64 %.sum8529vector_func.i
  %exData5036vector_func.i = extractelement <16 x float> %call.i55.i, i32 13
  store float %exData5036vector_func.i, float addrspace(1)* %886, align 4
  br label %postload8016vector_func.i

postload8016vector_func.i:                        ; preds = %preload8015vector_func.i, %postload8013vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8018vector_func.i, label %postload8019vector_func.i

preload8018vector_func.i:                         ; preds = %postload8016vector_func.i
  %.sum8528vector_func.i = add i64 %dim_0_vector_tid.i, 304142
  %887 = getelementptr float addrspace(1)* %4, i64 %.sum8528vector_func.i
  %exData5039vector_func.i = extractelement <16 x float> %call.i55.i, i32 14
  store float %exData5039vector_func.i, float addrspace(1)* %887, align 4
  br label %postload8019vector_func.i

postload8019vector_func.i:                        ; preds = %preload8018vector_func.i, %postload8016vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8021vector_func.i, label %postload8022vector_func.i

preload8021vector_func.i:                         ; preds = %postload8019vector_func.i
  %.sum8527vector_func.i = add i64 %dim_0_vector_tid.i, 304143
  %888 = getelementptr float addrspace(1)* %4, i64 %.sum8527vector_func.i
  %exData5042vector_func.i = extractelement <16 x float> %call.i55.i, i32 15
  store float %exData5042vector_func.i, float addrspace(1)* %888, align 4
  br label %postload8022vector_func.i

postload8022vector_func.i:                        ; preds = %preload8021vector_func.i, %postload8019vector_func.i
  %mul5412160vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000, float 0x40C6811A40000000>
  %add5422161vector_func.i = fadd <16 x float> %mul5412160vector_func.i, <float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000, float 0x400555A760000000>
  %mul5432162vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000, float 0x40112A6B40000000>
  %add5442163vector_func.i = fadd <16 x float> %add5422161vector_func.i, %mul5432162vector_func.i
  %mul.i502164vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000, float 0x3D77A24400000000>
  %add.i512165vector_func.i = fadd <16 x float> %mul.i502164vector_func.i, <float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000, float 0xBE395B6420000000>
  %mul1.i522166vector_func.i = fmul <16 x float> %add.i512165vector_func.i, %mul567vector_func.i
  %add2.i532167vector_func.i = fadd <16 x float> %mul1.i522166vector_func.i, <float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000, float 0x3EE4F3AEE0000000>
  %mul3.i542168vector_func.i = fmul <16 x float> %add2.i532167vector_func.i, %mul567vector_func.i
  %add4.i552169vector_func.i = fadd <16 x float> %mul3.i542168vector_func.i, <float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000, float 0xBF6688C920000000>
  %mul5.i562170vector_func.i = fmul <16 x float> %add4.i552169vector_func.i, %mul567vector_func.i
  %add5462171vector_func.i = fadd <16 x float> %add5442163vector_func.i, %mul5.i562170vector_func.i
  %call.i56.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5462171vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8024vector_func.i, label %postload8025vector_func.i

preload8024vector_func.i:                         ; preds = %postload8022vector_func.i
  %extract2173vector_func.i = add i64 %dim_0_vector_tid.i, 317952
  %889 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2173vector_func.i
  %exData5046vector_func.i = extractelement <16 x float> %call.i56.i, i32 0
  store float %exData5046vector_func.i, float addrspace(1)* %889, align 4
  br label %postload8025vector_func.i

postload8025vector_func.i:                        ; preds = %preload8024vector_func.i, %postload8022vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8027vector_func.i, label %postload8028vector_func.i

preload8027vector_func.i:                         ; preds = %postload8025vector_func.i
  %.sum8526vector_func.i = add i64 %dim_0_vector_tid.i, 317953
  %890 = getelementptr float addrspace(1)* %4, i64 %.sum8526vector_func.i
  %exData5049vector_func.i = extractelement <16 x float> %call.i56.i, i32 1
  store float %exData5049vector_func.i, float addrspace(1)* %890, align 4
  br label %postload8028vector_func.i

postload8028vector_func.i:                        ; preds = %preload8027vector_func.i, %postload8025vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8030vector_func.i, label %postload8031vector_func.i

preload8030vector_func.i:                         ; preds = %postload8028vector_func.i
  %.sum8525vector_func.i = add i64 %dim_0_vector_tid.i, 317954
  %891 = getelementptr float addrspace(1)* %4, i64 %.sum8525vector_func.i
  %exData5052vector_func.i = extractelement <16 x float> %call.i56.i, i32 2
  store float %exData5052vector_func.i, float addrspace(1)* %891, align 4
  br label %postload8031vector_func.i

postload8031vector_func.i:                        ; preds = %preload8030vector_func.i, %postload8028vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8033vector_func.i, label %postload8034vector_func.i

preload8033vector_func.i:                         ; preds = %postload8031vector_func.i
  %.sum8524vector_func.i = add i64 %dim_0_vector_tid.i, 317955
  %892 = getelementptr float addrspace(1)* %4, i64 %.sum8524vector_func.i
  %exData5055vector_func.i = extractelement <16 x float> %call.i56.i, i32 3
  store float %exData5055vector_func.i, float addrspace(1)* %892, align 4
  br label %postload8034vector_func.i

postload8034vector_func.i:                        ; preds = %preload8033vector_func.i, %postload8031vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8036vector_func.i, label %postload8037vector_func.i

preload8036vector_func.i:                         ; preds = %postload8034vector_func.i
  %.sum8523vector_func.i = add i64 %dim_0_vector_tid.i, 317956
  %893 = getelementptr float addrspace(1)* %4, i64 %.sum8523vector_func.i
  %exData5058vector_func.i = extractelement <16 x float> %call.i56.i, i32 4
  store float %exData5058vector_func.i, float addrspace(1)* %893, align 4
  br label %postload8037vector_func.i

postload8037vector_func.i:                        ; preds = %preload8036vector_func.i, %postload8034vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8039vector_func.i, label %postload8040vector_func.i

preload8039vector_func.i:                         ; preds = %postload8037vector_func.i
  %.sum8522vector_func.i = add i64 %dim_0_vector_tid.i, 317957
  %894 = getelementptr float addrspace(1)* %4, i64 %.sum8522vector_func.i
  %exData5061vector_func.i = extractelement <16 x float> %call.i56.i, i32 5
  store float %exData5061vector_func.i, float addrspace(1)* %894, align 4
  br label %postload8040vector_func.i

postload8040vector_func.i:                        ; preds = %preload8039vector_func.i, %postload8037vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8042vector_func.i, label %postload8043vector_func.i

preload8042vector_func.i:                         ; preds = %postload8040vector_func.i
  %.sum8521vector_func.i = add i64 %dim_0_vector_tid.i, 317958
  %895 = getelementptr float addrspace(1)* %4, i64 %.sum8521vector_func.i
  %exData5064vector_func.i = extractelement <16 x float> %call.i56.i, i32 6
  store float %exData5064vector_func.i, float addrspace(1)* %895, align 4
  br label %postload8043vector_func.i

postload8043vector_func.i:                        ; preds = %preload8042vector_func.i, %postload8040vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8045vector_func.i, label %postload8046vector_func.i

preload8045vector_func.i:                         ; preds = %postload8043vector_func.i
  %.sum8520vector_func.i = add i64 %dim_0_vector_tid.i, 317959
  %896 = getelementptr float addrspace(1)* %4, i64 %.sum8520vector_func.i
  %exData5067vector_func.i = extractelement <16 x float> %call.i56.i, i32 7
  store float %exData5067vector_func.i, float addrspace(1)* %896, align 4
  br label %postload8046vector_func.i

postload8046vector_func.i:                        ; preds = %preload8045vector_func.i, %postload8043vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8048vector_func.i, label %postload8049vector_func.i

preload8048vector_func.i:                         ; preds = %postload8046vector_func.i
  %.sum8519vector_func.i = add i64 %dim_0_vector_tid.i, 317960
  %897 = getelementptr float addrspace(1)* %4, i64 %.sum8519vector_func.i
  %exData5070vector_func.i = extractelement <16 x float> %call.i56.i, i32 8
  store float %exData5070vector_func.i, float addrspace(1)* %897, align 4
  br label %postload8049vector_func.i

postload8049vector_func.i:                        ; preds = %preload8048vector_func.i, %postload8046vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8051vector_func.i, label %postload8052vector_func.i

preload8051vector_func.i:                         ; preds = %postload8049vector_func.i
  %.sum8518vector_func.i = add i64 %dim_0_vector_tid.i, 317961
  %898 = getelementptr float addrspace(1)* %4, i64 %.sum8518vector_func.i
  %exData5073vector_func.i = extractelement <16 x float> %call.i56.i, i32 9
  store float %exData5073vector_func.i, float addrspace(1)* %898, align 4
  br label %postload8052vector_func.i

postload8052vector_func.i:                        ; preds = %preload8051vector_func.i, %postload8049vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8054vector_func.i, label %postload8055vector_func.i

preload8054vector_func.i:                         ; preds = %postload8052vector_func.i
  %.sum8517vector_func.i = add i64 %dim_0_vector_tid.i, 317962
  %899 = getelementptr float addrspace(1)* %4, i64 %.sum8517vector_func.i
  %exData5076vector_func.i = extractelement <16 x float> %call.i56.i, i32 10
  store float %exData5076vector_func.i, float addrspace(1)* %899, align 4
  br label %postload8055vector_func.i

postload8055vector_func.i:                        ; preds = %preload8054vector_func.i, %postload8052vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8057vector_func.i, label %postload8058vector_func.i

preload8057vector_func.i:                         ; preds = %postload8055vector_func.i
  %.sum8516vector_func.i = add i64 %dim_0_vector_tid.i, 317963
  %900 = getelementptr float addrspace(1)* %4, i64 %.sum8516vector_func.i
  %exData5079vector_func.i = extractelement <16 x float> %call.i56.i, i32 11
  store float %exData5079vector_func.i, float addrspace(1)* %900, align 4
  br label %postload8058vector_func.i

postload8058vector_func.i:                        ; preds = %preload8057vector_func.i, %postload8055vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8060vector_func.i, label %postload8061vector_func.i

preload8060vector_func.i:                         ; preds = %postload8058vector_func.i
  %.sum8515vector_func.i = add i64 %dim_0_vector_tid.i, 317964
  %901 = getelementptr float addrspace(1)* %4, i64 %.sum8515vector_func.i
  %exData5082vector_func.i = extractelement <16 x float> %call.i56.i, i32 12
  store float %exData5082vector_func.i, float addrspace(1)* %901, align 4
  br label %postload8061vector_func.i

postload8061vector_func.i:                        ; preds = %preload8060vector_func.i, %postload8058vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8063vector_func.i, label %postload8064vector_func.i

preload8063vector_func.i:                         ; preds = %postload8061vector_func.i
  %.sum8514vector_func.i = add i64 %dim_0_vector_tid.i, 317965
  %902 = getelementptr float addrspace(1)* %4, i64 %.sum8514vector_func.i
  %exData5085vector_func.i = extractelement <16 x float> %call.i56.i, i32 13
  store float %exData5085vector_func.i, float addrspace(1)* %902, align 4
  br label %postload8064vector_func.i

postload8064vector_func.i:                        ; preds = %preload8063vector_func.i, %postload8061vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8066vector_func.i, label %postload8067vector_func.i

preload8066vector_func.i:                         ; preds = %postload8064vector_func.i
  %.sum8513vector_func.i = add i64 %dim_0_vector_tid.i, 317966
  %903 = getelementptr float addrspace(1)* %4, i64 %.sum8513vector_func.i
  %exData5088vector_func.i = extractelement <16 x float> %call.i56.i, i32 14
  store float %exData5088vector_func.i, float addrspace(1)* %903, align 4
  br label %postload8067vector_func.i

postload8067vector_func.i:                        ; preds = %preload8066vector_func.i, %postload8064vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8069vector_func.i, label %postload8070vector_func.i

preload8069vector_func.i:                         ; preds = %postload8067vector_func.i
  %.sum8512vector_func.i = add i64 %dim_0_vector_tid.i, 317967
  %904 = getelementptr float addrspace(1)* %4, i64 %.sum8512vector_func.i
  %exData5091vector_func.i = extractelement <16 x float> %call.i56.i, i32 15
  store float %exData5091vector_func.i, float addrspace(1)* %904, align 4
  br label %postload8070vector_func.i

postload8070vector_func.i:                        ; preds = %preload8069vector_func.i, %postload8067vector_func.i
  %mul5512190vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000, float 0x40D396DCC0000000>
  %sub5522191vector_func.i = fsub <16 x float> <float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000, float 0x4028FB17E0000000>, %mul5512190vector_func.i
  %mul5532192vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000, float 0x4002038680000000>
  %add5542193vector_func.i = fadd <16 x float> %sub5522191vector_func.i, %mul5532192vector_func.i
  %mul.i432194vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000, float 0xBD51D37B00000000>
  %add.i442195vector_func.i = fadd <16 x float> %mul.i432194vector_func.i, <float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000, float 0x3E18BBA200000000>
  %mul1.i452196vector_func.i = fmul <16 x float> %add.i442195vector_func.i, %mul567vector_func.i
  %add2.i462197vector_func.i = fadd <16 x float> %mul1.i452196vector_func.i, <float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000, float 0xBED0967CE0000000>
  %mul3.i472198vector_func.i = fmul <16 x float> %add2.i462197vector_func.i, %mul567vector_func.i
  %add4.i482199vector_func.i = fadd <16 x float> %mul3.i472198vector_func.i, <float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000, float 0x3F82142860000000>
  %mul5.i492200vector_func.i = fmul <16 x float> %add4.i482199vector_func.i, %mul567vector_func.i
  %add5562201vector_func.i = fadd <16 x float> %add5542193vector_func.i, %mul5.i492200vector_func.i
  %call.i57.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5562201vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8072vector_func.i, label %postload8073vector_func.i

preload8072vector_func.i:                         ; preds = %postload8070vector_func.i
  %extract2203vector_func.i = add i64 %dim_0_vector_tid.i, 331776
  %905 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2203vector_func.i
  %exData5095vector_func.i = extractelement <16 x float> %call.i57.i, i32 0
  store float %exData5095vector_func.i, float addrspace(1)* %905, align 4
  br label %postload8073vector_func.i

postload8073vector_func.i:                        ; preds = %preload8072vector_func.i, %postload8070vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8075vector_func.i, label %postload8076vector_func.i

preload8075vector_func.i:                         ; preds = %postload8073vector_func.i
  %.sum8511vector_func.i = add i64 %dim_0_vector_tid.i, 331777
  %906 = getelementptr float addrspace(1)* %4, i64 %.sum8511vector_func.i
  %exData5098vector_func.i = extractelement <16 x float> %call.i57.i, i32 1
  store float %exData5098vector_func.i, float addrspace(1)* %906, align 4
  br label %postload8076vector_func.i

postload8076vector_func.i:                        ; preds = %preload8075vector_func.i, %postload8073vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8078vector_func.i, label %postload8079vector_func.i

preload8078vector_func.i:                         ; preds = %postload8076vector_func.i
  %.sum8510vector_func.i = add i64 %dim_0_vector_tid.i, 331778
  %907 = getelementptr float addrspace(1)* %4, i64 %.sum8510vector_func.i
  %exData5101vector_func.i = extractelement <16 x float> %call.i57.i, i32 2
  store float %exData5101vector_func.i, float addrspace(1)* %907, align 4
  br label %postload8079vector_func.i

postload8079vector_func.i:                        ; preds = %preload8078vector_func.i, %postload8076vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8081vector_func.i, label %postload8082vector_func.i

preload8081vector_func.i:                         ; preds = %postload8079vector_func.i
  %.sum8509vector_func.i = add i64 %dim_0_vector_tid.i, 331779
  %908 = getelementptr float addrspace(1)* %4, i64 %.sum8509vector_func.i
  %exData5104vector_func.i = extractelement <16 x float> %call.i57.i, i32 3
  store float %exData5104vector_func.i, float addrspace(1)* %908, align 4
  br label %postload8082vector_func.i

postload8082vector_func.i:                        ; preds = %preload8081vector_func.i, %postload8079vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8084vector_func.i, label %postload8085vector_func.i

preload8084vector_func.i:                         ; preds = %postload8082vector_func.i
  %.sum8508vector_func.i = add i64 %dim_0_vector_tid.i, 331780
  %909 = getelementptr float addrspace(1)* %4, i64 %.sum8508vector_func.i
  %exData5107vector_func.i = extractelement <16 x float> %call.i57.i, i32 4
  store float %exData5107vector_func.i, float addrspace(1)* %909, align 4
  br label %postload8085vector_func.i

postload8085vector_func.i:                        ; preds = %preload8084vector_func.i, %postload8082vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8087vector_func.i, label %postload8088vector_func.i

preload8087vector_func.i:                         ; preds = %postload8085vector_func.i
  %.sum8507vector_func.i = add i64 %dim_0_vector_tid.i, 331781
  %910 = getelementptr float addrspace(1)* %4, i64 %.sum8507vector_func.i
  %exData5110vector_func.i = extractelement <16 x float> %call.i57.i, i32 5
  store float %exData5110vector_func.i, float addrspace(1)* %910, align 4
  br label %postload8088vector_func.i

postload8088vector_func.i:                        ; preds = %preload8087vector_func.i, %postload8085vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8090vector_func.i, label %postload8091vector_func.i

preload8090vector_func.i:                         ; preds = %postload8088vector_func.i
  %.sum8506vector_func.i = add i64 %dim_0_vector_tid.i, 331782
  %911 = getelementptr float addrspace(1)* %4, i64 %.sum8506vector_func.i
  %exData5113vector_func.i = extractelement <16 x float> %call.i57.i, i32 6
  store float %exData5113vector_func.i, float addrspace(1)* %911, align 4
  br label %postload8091vector_func.i

postload8091vector_func.i:                        ; preds = %preload8090vector_func.i, %postload8088vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8093vector_func.i, label %postload8094vector_func.i

preload8093vector_func.i:                         ; preds = %postload8091vector_func.i
  %.sum8505vector_func.i = add i64 %dim_0_vector_tid.i, 331783
  %912 = getelementptr float addrspace(1)* %4, i64 %.sum8505vector_func.i
  %exData5116vector_func.i = extractelement <16 x float> %call.i57.i, i32 7
  store float %exData5116vector_func.i, float addrspace(1)* %912, align 4
  br label %postload8094vector_func.i

postload8094vector_func.i:                        ; preds = %preload8093vector_func.i, %postload8091vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8096vector_func.i, label %postload8097vector_func.i

preload8096vector_func.i:                         ; preds = %postload8094vector_func.i
  %.sum8504vector_func.i = add i64 %dim_0_vector_tid.i, 331784
  %913 = getelementptr float addrspace(1)* %4, i64 %.sum8504vector_func.i
  %exData5119vector_func.i = extractelement <16 x float> %call.i57.i, i32 8
  store float %exData5119vector_func.i, float addrspace(1)* %913, align 4
  br label %postload8097vector_func.i

postload8097vector_func.i:                        ; preds = %preload8096vector_func.i, %postload8094vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8099vector_func.i, label %postload8100vector_func.i

preload8099vector_func.i:                         ; preds = %postload8097vector_func.i
  %.sum8503vector_func.i = add i64 %dim_0_vector_tid.i, 331785
  %914 = getelementptr float addrspace(1)* %4, i64 %.sum8503vector_func.i
  %exData5122vector_func.i = extractelement <16 x float> %call.i57.i, i32 9
  store float %exData5122vector_func.i, float addrspace(1)* %914, align 4
  br label %postload8100vector_func.i

postload8100vector_func.i:                        ; preds = %preload8099vector_func.i, %postload8097vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8102vector_func.i, label %postload8103vector_func.i

preload8102vector_func.i:                         ; preds = %postload8100vector_func.i
  %.sum8502vector_func.i = add i64 %dim_0_vector_tid.i, 331786
  %915 = getelementptr float addrspace(1)* %4, i64 %.sum8502vector_func.i
  %exData5125vector_func.i = extractelement <16 x float> %call.i57.i, i32 10
  store float %exData5125vector_func.i, float addrspace(1)* %915, align 4
  br label %postload8103vector_func.i

postload8103vector_func.i:                        ; preds = %preload8102vector_func.i, %postload8100vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8105vector_func.i, label %postload8106vector_func.i

preload8105vector_func.i:                         ; preds = %postload8103vector_func.i
  %.sum8501vector_func.i = add i64 %dim_0_vector_tid.i, 331787
  %916 = getelementptr float addrspace(1)* %4, i64 %.sum8501vector_func.i
  %exData5128vector_func.i = extractelement <16 x float> %call.i57.i, i32 11
  store float %exData5128vector_func.i, float addrspace(1)* %916, align 4
  br label %postload8106vector_func.i

postload8106vector_func.i:                        ; preds = %preload8105vector_func.i, %postload8103vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8108vector_func.i, label %postload8109vector_func.i

preload8108vector_func.i:                         ; preds = %postload8106vector_func.i
  %.sum8500vector_func.i = add i64 %dim_0_vector_tid.i, 331788
  %917 = getelementptr float addrspace(1)* %4, i64 %.sum8500vector_func.i
  %exData5131vector_func.i = extractelement <16 x float> %call.i57.i, i32 12
  store float %exData5131vector_func.i, float addrspace(1)* %917, align 4
  br label %postload8109vector_func.i

postload8109vector_func.i:                        ; preds = %preload8108vector_func.i, %postload8106vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8111vector_func.i, label %postload8112vector_func.i

preload8111vector_func.i:                         ; preds = %postload8109vector_func.i
  %.sum8499vector_func.i = add i64 %dim_0_vector_tid.i, 331789
  %918 = getelementptr float addrspace(1)* %4, i64 %.sum8499vector_func.i
  %exData5134vector_func.i = extractelement <16 x float> %call.i57.i, i32 13
  store float %exData5134vector_func.i, float addrspace(1)* %918, align 4
  br label %postload8112vector_func.i

postload8112vector_func.i:                        ; preds = %preload8111vector_func.i, %postload8109vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8114vector_func.i, label %postload8115vector_func.i

preload8114vector_func.i:                         ; preds = %postload8112vector_func.i
  %.sum8498vector_func.i = add i64 %dim_0_vector_tid.i, 331790
  %919 = getelementptr float addrspace(1)* %4, i64 %.sum8498vector_func.i
  %exData5137vector_func.i = extractelement <16 x float> %call.i57.i, i32 14
  store float %exData5137vector_func.i, float addrspace(1)* %919, align 4
  br label %postload8115vector_func.i

postload8115vector_func.i:                        ; preds = %preload8114vector_func.i, %postload8112vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8117vector_func.i, label %postload8118vector_func.i

preload8117vector_func.i:                         ; preds = %postload8115vector_func.i
  %.sum8497vector_func.i = add i64 %dim_0_vector_tid.i, 331791
  %920 = getelementptr float addrspace(1)* %4, i64 %.sum8497vector_func.i
  %exData5140vector_func.i = extractelement <16 x float> %call.i57.i, i32 15
  store float %exData5140vector_func.i, float addrspace(1)* %920, align 4
  br label %postload8118vector_func.i

postload8118vector_func.i:                        ; preds = %preload8117vector_func.i, %postload8115vector_func.i
  %mul5612220vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000, float 0x40BB82EB00000000>
  %add5622221vector_func.i = fadd <16 x float> %mul5612220vector_func.i, <float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000, float 0x40286E6960000000>
  %mul5632222vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000, float 0x4001163160000000>
  %add5642223vector_func.i = fadd <16 x float> %add5622221vector_func.i, %mul5632222vector_func.i
  %mul.i362224vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000, float 0xBD3C5A4680000000>
  %add.i372225vector_func.i = fadd <16 x float> %mul.i362224vector_func.i, <float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000, float 0x3E0AC134E0000000>
  %mul1.i382226vector_func.i = fmul <16 x float> %add.i372225vector_func.i, %mul567vector_func.i
  %add2.i392227vector_func.i = fadd <16 x float> %mul1.i382226vector_func.i, <float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000, float 0xBEC851D2A0000000>
  %mul3.i402228vector_func.i = fmul <16 x float> %add2.i392227vector_func.i, %mul567vector_func.i
  %add4.i412229vector_func.i = fadd <16 x float> %mul3.i402228vector_func.i, <float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000, float 0x3F828DC0E0000000>
  %mul5.i422230vector_func.i = fmul <16 x float> %add4.i412229vector_func.i, %mul567vector_func.i
  %add5662231vector_func.i = fadd <16 x float> %add5642223vector_func.i, %mul5.i422230vector_func.i
  %call.i58.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5662231vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8120vector_func.i, label %postload8121vector_func.i

preload8120vector_func.i:                         ; preds = %postload8118vector_func.i
  %extract2233vector_func.i = add i64 %dim_0_vector_tid.i, 345600
  %921 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2233vector_func.i
  %exData5144vector_func.i = extractelement <16 x float> %call.i58.i, i32 0
  store float %exData5144vector_func.i, float addrspace(1)* %921, align 4
  br label %postload8121vector_func.i

postload8121vector_func.i:                        ; preds = %preload8120vector_func.i, %postload8118vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8123vector_func.i, label %postload8124vector_func.i

preload8123vector_func.i:                         ; preds = %postload8121vector_func.i
  %.sum8496vector_func.i = add i64 %dim_0_vector_tid.i, 345601
  %922 = getelementptr float addrspace(1)* %4, i64 %.sum8496vector_func.i
  %exData5147vector_func.i = extractelement <16 x float> %call.i58.i, i32 1
  store float %exData5147vector_func.i, float addrspace(1)* %922, align 4
  br label %postload8124vector_func.i

postload8124vector_func.i:                        ; preds = %preload8123vector_func.i, %postload8121vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8126vector_func.i, label %postload8127vector_func.i

preload8126vector_func.i:                         ; preds = %postload8124vector_func.i
  %.sum8495vector_func.i = add i64 %dim_0_vector_tid.i, 345602
  %923 = getelementptr float addrspace(1)* %4, i64 %.sum8495vector_func.i
  %exData5150vector_func.i = extractelement <16 x float> %call.i58.i, i32 2
  store float %exData5150vector_func.i, float addrspace(1)* %923, align 4
  br label %postload8127vector_func.i

postload8127vector_func.i:                        ; preds = %preload8126vector_func.i, %postload8124vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8129vector_func.i, label %postload8130vector_func.i

preload8129vector_func.i:                         ; preds = %postload8127vector_func.i
  %.sum8494vector_func.i = add i64 %dim_0_vector_tid.i, 345603
  %924 = getelementptr float addrspace(1)* %4, i64 %.sum8494vector_func.i
  %exData5153vector_func.i = extractelement <16 x float> %call.i58.i, i32 3
  store float %exData5153vector_func.i, float addrspace(1)* %924, align 4
  br label %postload8130vector_func.i

postload8130vector_func.i:                        ; preds = %preload8129vector_func.i, %postload8127vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8132vector_func.i, label %postload8133vector_func.i

preload8132vector_func.i:                         ; preds = %postload8130vector_func.i
  %.sum8493vector_func.i = add i64 %dim_0_vector_tid.i, 345604
  %925 = getelementptr float addrspace(1)* %4, i64 %.sum8493vector_func.i
  %exData5156vector_func.i = extractelement <16 x float> %call.i58.i, i32 4
  store float %exData5156vector_func.i, float addrspace(1)* %925, align 4
  br label %postload8133vector_func.i

postload8133vector_func.i:                        ; preds = %preload8132vector_func.i, %postload8130vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8135vector_func.i, label %postload8136vector_func.i

preload8135vector_func.i:                         ; preds = %postload8133vector_func.i
  %.sum8492vector_func.i = add i64 %dim_0_vector_tid.i, 345605
  %926 = getelementptr float addrspace(1)* %4, i64 %.sum8492vector_func.i
  %exData5159vector_func.i = extractelement <16 x float> %call.i58.i, i32 5
  store float %exData5159vector_func.i, float addrspace(1)* %926, align 4
  br label %postload8136vector_func.i

postload8136vector_func.i:                        ; preds = %preload8135vector_func.i, %postload8133vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8138vector_func.i, label %postload8139vector_func.i

preload8138vector_func.i:                         ; preds = %postload8136vector_func.i
  %.sum8491vector_func.i = add i64 %dim_0_vector_tid.i, 345606
  %927 = getelementptr float addrspace(1)* %4, i64 %.sum8491vector_func.i
  %exData5162vector_func.i = extractelement <16 x float> %call.i58.i, i32 6
  store float %exData5162vector_func.i, float addrspace(1)* %927, align 4
  br label %postload8139vector_func.i

postload8139vector_func.i:                        ; preds = %preload8138vector_func.i, %postload8136vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8141vector_func.i, label %postload8142vector_func.i

preload8141vector_func.i:                         ; preds = %postload8139vector_func.i
  %.sum8490vector_func.i = add i64 %dim_0_vector_tid.i, 345607
  %928 = getelementptr float addrspace(1)* %4, i64 %.sum8490vector_func.i
  %exData5165vector_func.i = extractelement <16 x float> %call.i58.i, i32 7
  store float %exData5165vector_func.i, float addrspace(1)* %928, align 4
  br label %postload8142vector_func.i

postload8142vector_func.i:                        ; preds = %preload8141vector_func.i, %postload8139vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8144vector_func.i, label %postload8145vector_func.i

preload8144vector_func.i:                         ; preds = %postload8142vector_func.i
  %.sum8489vector_func.i = add i64 %dim_0_vector_tid.i, 345608
  %929 = getelementptr float addrspace(1)* %4, i64 %.sum8489vector_func.i
  %exData5168vector_func.i = extractelement <16 x float> %call.i58.i, i32 8
  store float %exData5168vector_func.i, float addrspace(1)* %929, align 4
  br label %postload8145vector_func.i

postload8145vector_func.i:                        ; preds = %preload8144vector_func.i, %postload8142vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8147vector_func.i, label %postload8148vector_func.i

preload8147vector_func.i:                         ; preds = %postload8145vector_func.i
  %.sum8488vector_func.i = add i64 %dim_0_vector_tid.i, 345609
  %930 = getelementptr float addrspace(1)* %4, i64 %.sum8488vector_func.i
  %exData5171vector_func.i = extractelement <16 x float> %call.i58.i, i32 9
  store float %exData5171vector_func.i, float addrspace(1)* %930, align 4
  br label %postload8148vector_func.i

postload8148vector_func.i:                        ; preds = %preload8147vector_func.i, %postload8145vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8150vector_func.i, label %postload8151vector_func.i

preload8150vector_func.i:                         ; preds = %postload8148vector_func.i
  %.sum8487vector_func.i = add i64 %dim_0_vector_tid.i, 345610
  %931 = getelementptr float addrspace(1)* %4, i64 %.sum8487vector_func.i
  %exData5174vector_func.i = extractelement <16 x float> %call.i58.i, i32 10
  store float %exData5174vector_func.i, float addrspace(1)* %931, align 4
  br label %postload8151vector_func.i

postload8151vector_func.i:                        ; preds = %preload8150vector_func.i, %postload8148vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8153vector_func.i, label %postload8154vector_func.i

preload8153vector_func.i:                         ; preds = %postload8151vector_func.i
  %.sum8486vector_func.i = add i64 %dim_0_vector_tid.i, 345611
  %932 = getelementptr float addrspace(1)* %4, i64 %.sum8486vector_func.i
  %exData5177vector_func.i = extractelement <16 x float> %call.i58.i, i32 11
  store float %exData5177vector_func.i, float addrspace(1)* %932, align 4
  br label %postload8154vector_func.i

postload8154vector_func.i:                        ; preds = %preload8153vector_func.i, %postload8151vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8156vector_func.i, label %postload8157vector_func.i

preload8156vector_func.i:                         ; preds = %postload8154vector_func.i
  %.sum8485vector_func.i = add i64 %dim_0_vector_tid.i, 345612
  %933 = getelementptr float addrspace(1)* %4, i64 %.sum8485vector_func.i
  %exData5180vector_func.i = extractelement <16 x float> %call.i58.i, i32 12
  store float %exData5180vector_func.i, float addrspace(1)* %933, align 4
  br label %postload8157vector_func.i

postload8157vector_func.i:                        ; preds = %preload8156vector_func.i, %postload8154vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8159vector_func.i, label %postload8160vector_func.i

preload8159vector_func.i:                         ; preds = %postload8157vector_func.i
  %.sum8484vector_func.i = add i64 %dim_0_vector_tid.i, 345613
  %934 = getelementptr float addrspace(1)* %4, i64 %.sum8484vector_func.i
  %exData5183vector_func.i = extractelement <16 x float> %call.i58.i, i32 13
  store float %exData5183vector_func.i, float addrspace(1)* %934, align 4
  br label %postload8160vector_func.i

postload8160vector_func.i:                        ; preds = %preload8159vector_func.i, %postload8157vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8162vector_func.i, label %postload8163vector_func.i

preload8162vector_func.i:                         ; preds = %postload8160vector_func.i
  %.sum8483vector_func.i = add i64 %dim_0_vector_tid.i, 345614
  %935 = getelementptr float addrspace(1)* %4, i64 %.sum8483vector_func.i
  %exData5186vector_func.i = extractelement <16 x float> %call.i58.i, i32 14
  store float %exData5186vector_func.i, float addrspace(1)* %935, align 4
  br label %postload8163vector_func.i

postload8163vector_func.i:                        ; preds = %preload8162vector_func.i, %postload8160vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8165vector_func.i, label %postload8166vector_func.i

preload8165vector_func.i:                         ; preds = %postload8163vector_func.i
  %.sum8482vector_func.i = add i64 %dim_0_vector_tid.i, 345615
  %936 = getelementptr float addrspace(1)* %4, i64 %.sum8482vector_func.i
  %exData5189vector_func.i = extractelement <16 x float> %call.i58.i, i32 15
  store float %exData5189vector_func.i, float addrspace(1)* %936, align 4
  br label %postload8166vector_func.i

postload8166vector_func.i:                        ; preds = %preload8165vector_func.i, %postload8163vector_func.i
  %mul5712250vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000, float 0x4097C5E800000000>
  %sub5722251vector_func.i = fsub <16 x float> <float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000, float 0x4023249580000000>, %mul5712250vector_func.i
  %mul5732252vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000, float 0x400B45C280000000>
  %add5742253vector_func.i = fadd <16 x float> %sub5722251vector_func.i, %mul5732252vector_func.i
  %mul.i292254vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000, float 0x3D442D6C00000000>
  %add.i302255vector_func.i = fadd <16 x float> %mul.i292254vector_func.i, <float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000, float 0x3E047F4C00000000>
  %mul1.i312256vector_func.i = fmul <16 x float> %add.i302255vector_func.i, %mul567vector_func.i
  %add2.i322257vector_func.i = fadd <16 x float> %mul1.i312256vector_func.i, <float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000, float 0x3E9527EEA0000000>
  %mul3.i332258vector_func.i = fmul <16 x float> %add2.i322257vector_func.i, %mul567vector_func.i
  %add4.i342259vector_func.i = fadd <16 x float> %mul3.i332258vector_func.i, <float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000, float 0x3F75FE1B00000000>
  %mul5.i352260vector_func.i = fmul <16 x float> %add4.i342259vector_func.i, %mul567vector_func.i
  %add5762261vector_func.i = fadd <16 x float> %add5742253vector_func.i, %mul5.i352260vector_func.i
  %call.i59.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5762261vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8168vector_func.i, label %postload8169vector_func.i

preload8168vector_func.i:                         ; preds = %postload8166vector_func.i
  %extract2263vector_func.i = add i64 %dim_0_vector_tid.i, 359424
  %937 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2263vector_func.i
  %exData5193vector_func.i = extractelement <16 x float> %call.i59.i, i32 0
  store float %exData5193vector_func.i, float addrspace(1)* %937, align 4
  br label %postload8169vector_func.i

postload8169vector_func.i:                        ; preds = %preload8168vector_func.i, %postload8166vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8171vector_func.i, label %postload8172vector_func.i

preload8171vector_func.i:                         ; preds = %postload8169vector_func.i
  %.sum8481vector_func.i = add i64 %dim_0_vector_tid.i, 359425
  %938 = getelementptr float addrspace(1)* %4, i64 %.sum8481vector_func.i
  %exData5196vector_func.i = extractelement <16 x float> %call.i59.i, i32 1
  store float %exData5196vector_func.i, float addrspace(1)* %938, align 4
  br label %postload8172vector_func.i

postload8172vector_func.i:                        ; preds = %preload8171vector_func.i, %postload8169vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8174vector_func.i, label %postload8175vector_func.i

preload8174vector_func.i:                         ; preds = %postload8172vector_func.i
  %.sum8480vector_func.i = add i64 %dim_0_vector_tid.i, 359426
  %939 = getelementptr float addrspace(1)* %4, i64 %.sum8480vector_func.i
  %exData5199vector_func.i = extractelement <16 x float> %call.i59.i, i32 2
  store float %exData5199vector_func.i, float addrspace(1)* %939, align 4
  br label %postload8175vector_func.i

postload8175vector_func.i:                        ; preds = %preload8174vector_func.i, %postload8172vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8177vector_func.i, label %postload8178vector_func.i

preload8177vector_func.i:                         ; preds = %postload8175vector_func.i
  %.sum8479vector_func.i = add i64 %dim_0_vector_tid.i, 359427
  %940 = getelementptr float addrspace(1)* %4, i64 %.sum8479vector_func.i
  %exData5202vector_func.i = extractelement <16 x float> %call.i59.i, i32 3
  store float %exData5202vector_func.i, float addrspace(1)* %940, align 4
  br label %postload8178vector_func.i

postload8178vector_func.i:                        ; preds = %preload8177vector_func.i, %postload8175vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8180vector_func.i, label %postload8181vector_func.i

preload8180vector_func.i:                         ; preds = %postload8178vector_func.i
  %.sum8478vector_func.i = add i64 %dim_0_vector_tid.i, 359428
  %941 = getelementptr float addrspace(1)* %4, i64 %.sum8478vector_func.i
  %exData5205vector_func.i = extractelement <16 x float> %call.i59.i, i32 4
  store float %exData5205vector_func.i, float addrspace(1)* %941, align 4
  br label %postload8181vector_func.i

postload8181vector_func.i:                        ; preds = %preload8180vector_func.i, %postload8178vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8183vector_func.i, label %postload8184vector_func.i

preload8183vector_func.i:                         ; preds = %postload8181vector_func.i
  %.sum8477vector_func.i = add i64 %dim_0_vector_tid.i, 359429
  %942 = getelementptr float addrspace(1)* %4, i64 %.sum8477vector_func.i
  %exData5208vector_func.i = extractelement <16 x float> %call.i59.i, i32 5
  store float %exData5208vector_func.i, float addrspace(1)* %942, align 4
  br label %postload8184vector_func.i

postload8184vector_func.i:                        ; preds = %preload8183vector_func.i, %postload8181vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8186vector_func.i, label %postload8187vector_func.i

preload8186vector_func.i:                         ; preds = %postload8184vector_func.i
  %.sum8476vector_func.i = add i64 %dim_0_vector_tid.i, 359430
  %943 = getelementptr float addrspace(1)* %4, i64 %.sum8476vector_func.i
  %exData5211vector_func.i = extractelement <16 x float> %call.i59.i, i32 6
  store float %exData5211vector_func.i, float addrspace(1)* %943, align 4
  br label %postload8187vector_func.i

postload8187vector_func.i:                        ; preds = %preload8186vector_func.i, %postload8184vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8189vector_func.i, label %postload8190vector_func.i

preload8189vector_func.i:                         ; preds = %postload8187vector_func.i
  %.sum8475vector_func.i = add i64 %dim_0_vector_tid.i, 359431
  %944 = getelementptr float addrspace(1)* %4, i64 %.sum8475vector_func.i
  %exData5214vector_func.i = extractelement <16 x float> %call.i59.i, i32 7
  store float %exData5214vector_func.i, float addrspace(1)* %944, align 4
  br label %postload8190vector_func.i

postload8190vector_func.i:                        ; preds = %preload8189vector_func.i, %postload8187vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8192vector_func.i, label %postload8193vector_func.i

preload8192vector_func.i:                         ; preds = %postload8190vector_func.i
  %.sum8474vector_func.i = add i64 %dim_0_vector_tid.i, 359432
  %945 = getelementptr float addrspace(1)* %4, i64 %.sum8474vector_func.i
  %exData5217vector_func.i = extractelement <16 x float> %call.i59.i, i32 8
  store float %exData5217vector_func.i, float addrspace(1)* %945, align 4
  br label %postload8193vector_func.i

postload8193vector_func.i:                        ; preds = %preload8192vector_func.i, %postload8190vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8195vector_func.i, label %postload8196vector_func.i

preload8195vector_func.i:                         ; preds = %postload8193vector_func.i
  %.sum8473vector_func.i = add i64 %dim_0_vector_tid.i, 359433
  %946 = getelementptr float addrspace(1)* %4, i64 %.sum8473vector_func.i
  %exData5220vector_func.i = extractelement <16 x float> %call.i59.i, i32 9
  store float %exData5220vector_func.i, float addrspace(1)* %946, align 4
  br label %postload8196vector_func.i

postload8196vector_func.i:                        ; preds = %preload8195vector_func.i, %postload8193vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8198vector_func.i, label %postload8199vector_func.i

preload8198vector_func.i:                         ; preds = %postload8196vector_func.i
  %.sum8472vector_func.i = add i64 %dim_0_vector_tid.i, 359434
  %947 = getelementptr float addrspace(1)* %4, i64 %.sum8472vector_func.i
  %exData5223vector_func.i = extractelement <16 x float> %call.i59.i, i32 10
  store float %exData5223vector_func.i, float addrspace(1)* %947, align 4
  br label %postload8199vector_func.i

postload8199vector_func.i:                        ; preds = %preload8198vector_func.i, %postload8196vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8201vector_func.i, label %postload8202vector_func.i

preload8201vector_func.i:                         ; preds = %postload8199vector_func.i
  %.sum8471vector_func.i = add i64 %dim_0_vector_tid.i, 359435
  %948 = getelementptr float addrspace(1)* %4, i64 %.sum8471vector_func.i
  %exData5226vector_func.i = extractelement <16 x float> %call.i59.i, i32 11
  store float %exData5226vector_func.i, float addrspace(1)* %948, align 4
  br label %postload8202vector_func.i

postload8202vector_func.i:                        ; preds = %preload8201vector_func.i, %postload8199vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8204vector_func.i, label %postload8205vector_func.i

preload8204vector_func.i:                         ; preds = %postload8202vector_func.i
  %.sum8470vector_func.i = add i64 %dim_0_vector_tid.i, 359436
  %949 = getelementptr float addrspace(1)* %4, i64 %.sum8470vector_func.i
  %exData5229vector_func.i = extractelement <16 x float> %call.i59.i, i32 12
  store float %exData5229vector_func.i, float addrspace(1)* %949, align 4
  br label %postload8205vector_func.i

postload8205vector_func.i:                        ; preds = %preload8204vector_func.i, %postload8202vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8207vector_func.i, label %postload8208vector_func.i

preload8207vector_func.i:                         ; preds = %postload8205vector_func.i
  %.sum8469vector_func.i = add i64 %dim_0_vector_tid.i, 359437
  %950 = getelementptr float addrspace(1)* %4, i64 %.sum8469vector_func.i
  %exData5232vector_func.i = extractelement <16 x float> %call.i59.i, i32 13
  store float %exData5232vector_func.i, float addrspace(1)* %950, align 4
  br label %postload8208vector_func.i

postload8208vector_func.i:                        ; preds = %preload8207vector_func.i, %postload8205vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8210vector_func.i, label %postload8211vector_func.i

preload8210vector_func.i:                         ; preds = %postload8208vector_func.i
  %.sum8468vector_func.i = add i64 %dim_0_vector_tid.i, 359438
  %951 = getelementptr float addrspace(1)* %4, i64 %.sum8468vector_func.i
  %exData5235vector_func.i = extractelement <16 x float> %call.i59.i, i32 14
  store float %exData5235vector_func.i, float addrspace(1)* %951, align 4
  br label %postload8211vector_func.i

postload8211vector_func.i:                        ; preds = %preload8210vector_func.i, %postload8208vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8213vector_func.i, label %postload8214vector_func.i

preload8213vector_func.i:                         ; preds = %postload8211vector_func.i
  %.sum8467vector_func.i = add i64 %dim_0_vector_tid.i, 359439
  %952 = getelementptr float addrspace(1)* %4, i64 %.sum8467vector_func.i
  %exData5238vector_func.i = extractelement <16 x float> %call.i59.i, i32 15
  store float %exData5238vector_func.i, float addrspace(1)* %952, align 4
  br label %postload8214vector_func.i

postload8214vector_func.i:                        ; preds = %preload8213vector_func.i, %postload8211vector_func.i
  %mul5812280vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000, float 0x40D5113840000000>
  %add5822281vector_func.i = fadd <16 x float> %mul5812280vector_func.i, <float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000, float 0x4010697D00000000>
  %mul5832282vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000, float 0x4012EAF760000000>
  %add5842283vector_func.i = fadd <16 x float> %add5822281vector_func.i, %mul5832282vector_func.i
  %mul.i222284vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000, float 0x3D734A7280000000>
  %add.i232285vector_func.i = fadd <16 x float> %mul.i222284vector_func.i, <float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000, float 0xBE3490B360000000>
  %mul1.i242286vector_func.i = fmul <16 x float> %add.i232285vector_func.i, %mul567vector_func.i
  %add2.i252287vector_func.i = fadd <16 x float> %mul1.i242286vector_func.i, <float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000, float 0x3EE09D5A40000000>
  %mul3.i262288vector_func.i = fmul <16 x float> %add2.i252287vector_func.i, %mul567vector_func.i
  %add4.i272289vector_func.i = fadd <16 x float> %mul3.i262288vector_func.i, <float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000, float 0xBF5A28CE40000000>
  %mul5.i282290vector_func.i = fmul <16 x float> %add4.i272289vector_func.i, %mul567vector_func.i
  %add5862291vector_func.i = fadd <16 x float> %add5842283vector_func.i, %mul5.i282290vector_func.i
  %call.i60.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5862291vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8216vector_func.i, label %postload8217vector_func.i

preload8216vector_func.i:                         ; preds = %postload8214vector_func.i
  %extract2293vector_func.i = add i64 %dim_0_vector_tid.i, 373248
  %953 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2293vector_func.i
  %exData5242vector_func.i = extractelement <16 x float> %call.i60.i, i32 0
  store float %exData5242vector_func.i, float addrspace(1)* %953, align 4
  br label %postload8217vector_func.i

postload8217vector_func.i:                        ; preds = %preload8216vector_func.i, %postload8214vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8219vector_func.i, label %postload8220vector_func.i

preload8219vector_func.i:                         ; preds = %postload8217vector_func.i
  %.sum8466vector_func.i = add i64 %dim_0_vector_tid.i, 373249
  %954 = getelementptr float addrspace(1)* %4, i64 %.sum8466vector_func.i
  %exData5245vector_func.i = extractelement <16 x float> %call.i60.i, i32 1
  store float %exData5245vector_func.i, float addrspace(1)* %954, align 4
  br label %postload8220vector_func.i

postload8220vector_func.i:                        ; preds = %preload8219vector_func.i, %postload8217vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8222vector_func.i, label %postload8223vector_func.i

preload8222vector_func.i:                         ; preds = %postload8220vector_func.i
  %.sum8465vector_func.i = add i64 %dim_0_vector_tid.i, 373250
  %955 = getelementptr float addrspace(1)* %4, i64 %.sum8465vector_func.i
  %exData5248vector_func.i = extractelement <16 x float> %call.i60.i, i32 2
  store float %exData5248vector_func.i, float addrspace(1)* %955, align 4
  br label %postload8223vector_func.i

postload8223vector_func.i:                        ; preds = %preload8222vector_func.i, %postload8220vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8225vector_func.i, label %postload8226vector_func.i

preload8225vector_func.i:                         ; preds = %postload8223vector_func.i
  %.sum8464vector_func.i = add i64 %dim_0_vector_tid.i, 373251
  %956 = getelementptr float addrspace(1)* %4, i64 %.sum8464vector_func.i
  %exData5251vector_func.i = extractelement <16 x float> %call.i60.i, i32 3
  store float %exData5251vector_func.i, float addrspace(1)* %956, align 4
  br label %postload8226vector_func.i

postload8226vector_func.i:                        ; preds = %preload8225vector_func.i, %postload8223vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8228vector_func.i, label %postload8229vector_func.i

preload8228vector_func.i:                         ; preds = %postload8226vector_func.i
  %.sum8463vector_func.i = add i64 %dim_0_vector_tid.i, 373252
  %957 = getelementptr float addrspace(1)* %4, i64 %.sum8463vector_func.i
  %exData5254vector_func.i = extractelement <16 x float> %call.i60.i, i32 4
  store float %exData5254vector_func.i, float addrspace(1)* %957, align 4
  br label %postload8229vector_func.i

postload8229vector_func.i:                        ; preds = %preload8228vector_func.i, %postload8226vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8231vector_func.i, label %postload8232vector_func.i

preload8231vector_func.i:                         ; preds = %postload8229vector_func.i
  %.sum8462vector_func.i = add i64 %dim_0_vector_tid.i, 373253
  %958 = getelementptr float addrspace(1)* %4, i64 %.sum8462vector_func.i
  %exData5257vector_func.i = extractelement <16 x float> %call.i60.i, i32 5
  store float %exData5257vector_func.i, float addrspace(1)* %958, align 4
  br label %postload8232vector_func.i

postload8232vector_func.i:                        ; preds = %preload8231vector_func.i, %postload8229vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8234vector_func.i, label %postload8235vector_func.i

preload8234vector_func.i:                         ; preds = %postload8232vector_func.i
  %.sum8461vector_func.i = add i64 %dim_0_vector_tid.i, 373254
  %959 = getelementptr float addrspace(1)* %4, i64 %.sum8461vector_func.i
  %exData5260vector_func.i = extractelement <16 x float> %call.i60.i, i32 6
  store float %exData5260vector_func.i, float addrspace(1)* %959, align 4
  br label %postload8235vector_func.i

postload8235vector_func.i:                        ; preds = %preload8234vector_func.i, %postload8232vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8237vector_func.i, label %postload8238vector_func.i

preload8237vector_func.i:                         ; preds = %postload8235vector_func.i
  %.sum8460vector_func.i = add i64 %dim_0_vector_tid.i, 373255
  %960 = getelementptr float addrspace(1)* %4, i64 %.sum8460vector_func.i
  %exData5263vector_func.i = extractelement <16 x float> %call.i60.i, i32 7
  store float %exData5263vector_func.i, float addrspace(1)* %960, align 4
  br label %postload8238vector_func.i

postload8238vector_func.i:                        ; preds = %preload8237vector_func.i, %postload8235vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8240vector_func.i, label %postload8241vector_func.i

preload8240vector_func.i:                         ; preds = %postload8238vector_func.i
  %.sum8459vector_func.i = add i64 %dim_0_vector_tid.i, 373256
  %961 = getelementptr float addrspace(1)* %4, i64 %.sum8459vector_func.i
  %exData5266vector_func.i = extractelement <16 x float> %call.i60.i, i32 8
  store float %exData5266vector_func.i, float addrspace(1)* %961, align 4
  br label %postload8241vector_func.i

postload8241vector_func.i:                        ; preds = %preload8240vector_func.i, %postload8238vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8243vector_func.i, label %postload8244vector_func.i

preload8243vector_func.i:                         ; preds = %postload8241vector_func.i
  %.sum8458vector_func.i = add i64 %dim_0_vector_tid.i, 373257
  %962 = getelementptr float addrspace(1)* %4, i64 %.sum8458vector_func.i
  %exData5269vector_func.i = extractelement <16 x float> %call.i60.i, i32 9
  store float %exData5269vector_func.i, float addrspace(1)* %962, align 4
  br label %postload8244vector_func.i

postload8244vector_func.i:                        ; preds = %preload8243vector_func.i, %postload8241vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8246vector_func.i, label %postload8247vector_func.i

preload8246vector_func.i:                         ; preds = %postload8244vector_func.i
  %.sum8457vector_func.i = add i64 %dim_0_vector_tid.i, 373258
  %963 = getelementptr float addrspace(1)* %4, i64 %.sum8457vector_func.i
  %exData5272vector_func.i = extractelement <16 x float> %call.i60.i, i32 10
  store float %exData5272vector_func.i, float addrspace(1)* %963, align 4
  br label %postload8247vector_func.i

postload8247vector_func.i:                        ; preds = %preload8246vector_func.i, %postload8244vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8249vector_func.i, label %postload8250vector_func.i

preload8249vector_func.i:                         ; preds = %postload8247vector_func.i
  %.sum8456vector_func.i = add i64 %dim_0_vector_tid.i, 373259
  %964 = getelementptr float addrspace(1)* %4, i64 %.sum8456vector_func.i
  %exData5275vector_func.i = extractelement <16 x float> %call.i60.i, i32 11
  store float %exData5275vector_func.i, float addrspace(1)* %964, align 4
  br label %postload8250vector_func.i

postload8250vector_func.i:                        ; preds = %preload8249vector_func.i, %postload8247vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8252vector_func.i, label %postload8253vector_func.i

preload8252vector_func.i:                         ; preds = %postload8250vector_func.i
  %.sum8455vector_func.i = add i64 %dim_0_vector_tid.i, 373260
  %965 = getelementptr float addrspace(1)* %4, i64 %.sum8455vector_func.i
  %exData5278vector_func.i = extractelement <16 x float> %call.i60.i, i32 12
  store float %exData5278vector_func.i, float addrspace(1)* %965, align 4
  br label %postload8253vector_func.i

postload8253vector_func.i:                        ; preds = %preload8252vector_func.i, %postload8250vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8255vector_func.i, label %postload8256vector_func.i

preload8255vector_func.i:                         ; preds = %postload8253vector_func.i
  %.sum8454vector_func.i = add i64 %dim_0_vector_tid.i, 373261
  %966 = getelementptr float addrspace(1)* %4, i64 %.sum8454vector_func.i
  %exData5281vector_func.i = extractelement <16 x float> %call.i60.i, i32 13
  store float %exData5281vector_func.i, float addrspace(1)* %966, align 4
  br label %postload8256vector_func.i

postload8256vector_func.i:                        ; preds = %preload8255vector_func.i, %postload8253vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8258vector_func.i, label %postload8259vector_func.i

preload8258vector_func.i:                         ; preds = %postload8256vector_func.i
  %.sum8453vector_func.i = add i64 %dim_0_vector_tid.i, 373262
  %967 = getelementptr float addrspace(1)* %4, i64 %.sum8453vector_func.i
  %exData5284vector_func.i = extractelement <16 x float> %call.i60.i, i32 14
  store float %exData5284vector_func.i, float addrspace(1)* %967, align 4
  br label %postload8259vector_func.i

postload8259vector_func.i:                        ; preds = %preload8258vector_func.i, %postload8256vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8261vector_func.i, label %postload8262vector_func.i

preload8261vector_func.i:                         ; preds = %postload8259vector_func.i
  %.sum8452vector_func.i = add i64 %dim_0_vector_tid.i, 373263
  %968 = getelementptr float addrspace(1)* %4, i64 %.sum8452vector_func.i
  %exData5287vector_func.i = extractelement <16 x float> %call.i60.i, i32 15
  store float %exData5287vector_func.i, float addrspace(1)* %968, align 4
  br label %postload8262vector_func.i

postload8262vector_func.i:                        ; preds = %preload8261vector_func.i, %postload8259vector_func.i
  %mul5912310vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000, float 0x40D2CB6840000000>
  %sub5922311vector_func.i = fsub <16 x float> <float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000, float 0x40312C57C0000000>, %mul5912310vector_func.i
  %mul5932312vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000, float 0x3FF5CF9980000000>
  %add5942313vector_func.i = fadd <16 x float> %sub5922311vector_func.i, %mul5932312vector_func.i
  %mul.i152314vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000, float 0x3D6BE0A940000000>
  %add.i162315vector_func.i = fadd <16 x float> %mul.i152314vector_func.i, <float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000, float 0xBE27E07860000000>
  %mul1.i172316vector_func.i = fmul <16 x float> %add.i162315vector_func.i, %mul567vector_func.i
  %add2.i182317vector_func.i = fadd <16 x float> %mul1.i172316vector_func.i, <float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000, float 0x3EC178DF40000000>
  %mul3.i192318vector_func.i = fmul <16 x float> %add2.i182317vector_func.i, %mul567vector_func.i
  %add4.i202319vector_func.i = fadd <16 x float> %mul3.i192318vector_func.i, <float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000, float 0x3F844A1300000000>
  %mul5.i212320vector_func.i = fmul <16 x float> %add4.i202319vector_func.i, %mul567vector_func.i
  %add5962321vector_func.i = fadd <16 x float> %add5942313vector_func.i, %mul5.i212320vector_func.i
  %call.i61.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add5962321vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8264vector_func.i, label %postload8265vector_func.i

preload8264vector_func.i:                         ; preds = %postload8262vector_func.i
  %extract2323vector_func.i = add i64 %dim_0_vector_tid.i, 387072
  %969 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2323vector_func.i
  %exData5291vector_func.i = extractelement <16 x float> %call.i61.i, i32 0
  store float %exData5291vector_func.i, float addrspace(1)* %969, align 4
  br label %postload8265vector_func.i

postload8265vector_func.i:                        ; preds = %preload8264vector_func.i, %postload8262vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8267vector_func.i, label %postload8268vector_func.i

preload8267vector_func.i:                         ; preds = %postload8265vector_func.i
  %.sum8451vector_func.i = add i64 %dim_0_vector_tid.i, 387073
  %970 = getelementptr float addrspace(1)* %4, i64 %.sum8451vector_func.i
  %exData5294vector_func.i = extractelement <16 x float> %call.i61.i, i32 1
  store float %exData5294vector_func.i, float addrspace(1)* %970, align 4
  br label %postload8268vector_func.i

postload8268vector_func.i:                        ; preds = %preload8267vector_func.i, %postload8265vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8270vector_func.i, label %postload8271vector_func.i

preload8270vector_func.i:                         ; preds = %postload8268vector_func.i
  %.sum8450vector_func.i = add i64 %dim_0_vector_tid.i, 387074
  %971 = getelementptr float addrspace(1)* %4, i64 %.sum8450vector_func.i
  %exData5297vector_func.i = extractelement <16 x float> %call.i61.i, i32 2
  store float %exData5297vector_func.i, float addrspace(1)* %971, align 4
  br label %postload8271vector_func.i

postload8271vector_func.i:                        ; preds = %preload8270vector_func.i, %postload8268vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8273vector_func.i, label %postload8274vector_func.i

preload8273vector_func.i:                         ; preds = %postload8271vector_func.i
  %.sum8449vector_func.i = add i64 %dim_0_vector_tid.i, 387075
  %972 = getelementptr float addrspace(1)* %4, i64 %.sum8449vector_func.i
  %exData5300vector_func.i = extractelement <16 x float> %call.i61.i, i32 3
  store float %exData5300vector_func.i, float addrspace(1)* %972, align 4
  br label %postload8274vector_func.i

postload8274vector_func.i:                        ; preds = %preload8273vector_func.i, %postload8271vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8276vector_func.i, label %postload8277vector_func.i

preload8276vector_func.i:                         ; preds = %postload8274vector_func.i
  %.sum8448vector_func.i = add i64 %dim_0_vector_tid.i, 387076
  %973 = getelementptr float addrspace(1)* %4, i64 %.sum8448vector_func.i
  %exData5303vector_func.i = extractelement <16 x float> %call.i61.i, i32 4
  store float %exData5303vector_func.i, float addrspace(1)* %973, align 4
  br label %postload8277vector_func.i

postload8277vector_func.i:                        ; preds = %preload8276vector_func.i, %postload8274vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8279vector_func.i, label %postload8280vector_func.i

preload8279vector_func.i:                         ; preds = %postload8277vector_func.i
  %.sum8447vector_func.i = add i64 %dim_0_vector_tid.i, 387077
  %974 = getelementptr float addrspace(1)* %4, i64 %.sum8447vector_func.i
  %exData5306vector_func.i = extractelement <16 x float> %call.i61.i, i32 5
  store float %exData5306vector_func.i, float addrspace(1)* %974, align 4
  br label %postload8280vector_func.i

postload8280vector_func.i:                        ; preds = %preload8279vector_func.i, %postload8277vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8282vector_func.i, label %postload8283vector_func.i

preload8282vector_func.i:                         ; preds = %postload8280vector_func.i
  %.sum8446vector_func.i = add i64 %dim_0_vector_tid.i, 387078
  %975 = getelementptr float addrspace(1)* %4, i64 %.sum8446vector_func.i
  %exData5309vector_func.i = extractelement <16 x float> %call.i61.i, i32 6
  store float %exData5309vector_func.i, float addrspace(1)* %975, align 4
  br label %postload8283vector_func.i

postload8283vector_func.i:                        ; preds = %preload8282vector_func.i, %postload8280vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8285vector_func.i, label %postload8286vector_func.i

preload8285vector_func.i:                         ; preds = %postload8283vector_func.i
  %.sum8445vector_func.i = add i64 %dim_0_vector_tid.i, 387079
  %976 = getelementptr float addrspace(1)* %4, i64 %.sum8445vector_func.i
  %exData5312vector_func.i = extractelement <16 x float> %call.i61.i, i32 7
  store float %exData5312vector_func.i, float addrspace(1)* %976, align 4
  br label %postload8286vector_func.i

postload8286vector_func.i:                        ; preds = %preload8285vector_func.i, %postload8283vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8288vector_func.i, label %postload8289vector_func.i

preload8288vector_func.i:                         ; preds = %postload8286vector_func.i
  %.sum8444vector_func.i = add i64 %dim_0_vector_tid.i, 387080
  %977 = getelementptr float addrspace(1)* %4, i64 %.sum8444vector_func.i
  %exData5315vector_func.i = extractelement <16 x float> %call.i61.i, i32 8
  store float %exData5315vector_func.i, float addrspace(1)* %977, align 4
  br label %postload8289vector_func.i

postload8289vector_func.i:                        ; preds = %preload8288vector_func.i, %postload8286vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8291vector_func.i, label %postload8292vector_func.i

preload8291vector_func.i:                         ; preds = %postload8289vector_func.i
  %.sum8443vector_func.i = add i64 %dim_0_vector_tid.i, 387081
  %978 = getelementptr float addrspace(1)* %4, i64 %.sum8443vector_func.i
  %exData5318vector_func.i = extractelement <16 x float> %call.i61.i, i32 9
  store float %exData5318vector_func.i, float addrspace(1)* %978, align 4
  br label %postload8292vector_func.i

postload8292vector_func.i:                        ; preds = %preload8291vector_func.i, %postload8289vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8294vector_func.i, label %postload8295vector_func.i

preload8294vector_func.i:                         ; preds = %postload8292vector_func.i
  %.sum8442vector_func.i = add i64 %dim_0_vector_tid.i, 387082
  %979 = getelementptr float addrspace(1)* %4, i64 %.sum8442vector_func.i
  %exData5321vector_func.i = extractelement <16 x float> %call.i61.i, i32 10
  store float %exData5321vector_func.i, float addrspace(1)* %979, align 4
  br label %postload8295vector_func.i

postload8295vector_func.i:                        ; preds = %preload8294vector_func.i, %postload8292vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8297vector_func.i, label %postload8298vector_func.i

preload8297vector_func.i:                         ; preds = %postload8295vector_func.i
  %.sum8441vector_func.i = add i64 %dim_0_vector_tid.i, 387083
  %980 = getelementptr float addrspace(1)* %4, i64 %.sum8441vector_func.i
  %exData5324vector_func.i = extractelement <16 x float> %call.i61.i, i32 11
  store float %exData5324vector_func.i, float addrspace(1)* %980, align 4
  br label %postload8298vector_func.i

postload8298vector_func.i:                        ; preds = %preload8297vector_func.i, %postload8295vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8300vector_func.i, label %postload8301vector_func.i

preload8300vector_func.i:                         ; preds = %postload8298vector_func.i
  %.sum8440vector_func.i = add i64 %dim_0_vector_tid.i, 387084
  %981 = getelementptr float addrspace(1)* %4, i64 %.sum8440vector_func.i
  %exData5327vector_func.i = extractelement <16 x float> %call.i61.i, i32 12
  store float %exData5327vector_func.i, float addrspace(1)* %981, align 4
  br label %postload8301vector_func.i

postload8301vector_func.i:                        ; preds = %preload8300vector_func.i, %postload8298vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8303vector_func.i, label %postload8304vector_func.i

preload8303vector_func.i:                         ; preds = %postload8301vector_func.i
  %.sum8439vector_func.i = add i64 %dim_0_vector_tid.i, 387085
  %982 = getelementptr float addrspace(1)* %4, i64 %.sum8439vector_func.i
  %exData5330vector_func.i = extractelement <16 x float> %call.i61.i, i32 13
  store float %exData5330vector_func.i, float addrspace(1)* %982, align 4
  br label %postload8304vector_func.i

postload8304vector_func.i:                        ; preds = %preload8303vector_func.i, %postload8301vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8306vector_func.i, label %postload8307vector_func.i

preload8306vector_func.i:                         ; preds = %postload8304vector_func.i
  %.sum8438vector_func.i = add i64 %dim_0_vector_tid.i, 387086
  %983 = getelementptr float addrspace(1)* %4, i64 %.sum8438vector_func.i
  %exData5333vector_func.i = extractelement <16 x float> %call.i61.i, i32 14
  store float %exData5333vector_func.i, float addrspace(1)* %983, align 4
  br label %postload8307vector_func.i

postload8307vector_func.i:                        ; preds = %preload8306vector_func.i, %postload8304vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8309vector_func.i, label %postload8310vector_func.i

preload8309vector_func.i:                         ; preds = %postload8307vector_func.i
  %.sum8437vector_func.i = add i64 %dim_0_vector_tid.i, 387087
  %984 = getelementptr float addrspace(1)* %4, i64 %.sum8437vector_func.i
  %exData5336vector_func.i = extractelement <16 x float> %call.i61.i, i32 15
  store float %exData5336vector_func.i, float addrspace(1)* %984, align 4
  br label %postload8310vector_func.i

postload8310vector_func.i:                        ; preds = %preload8309vector_func.i, %postload8307vector_func.i
  %mul6012340vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000, float 0x4090CB4DE0000000>
  %sub6022341vector_func.i = fsub <16 x float> <float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000, float 0x4030253500000000>, %mul6012340vector_func.i
  %mul6032342vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000, float 0x3FF7E495E0000000>
  %add6042343vector_func.i = fadd <16 x float> %sub6022341vector_func.i, %mul6032342vector_func.i
  %mul.i82344vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000, float 0x3D592F7C20000000>
  %add.i92345vector_func.i = fadd <16 x float> %mul.i82344vector_func.i, <float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000, float 0xBE17E4A080000000>
  %mul1.i102346vector_func.i = fmul <16 x float> %add.i92345vector_func.i, %mul567vector_func.i
  %add2.i112347vector_func.i = fadd <16 x float> %mul1.i102346vector_func.i, <float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000, float 0x3EA9178B60000000>
  %mul3.i122348vector_func.i = fmul <16 x float> %add2.i112347vector_func.i, %mul567vector_func.i
  %add4.i132349vector_func.i = fadd <16 x float> %mul3.i122348vector_func.i, <float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000, float 0x3F856D6900000000>
  %mul5.i142350vector_func.i = fmul <16 x float> %add4.i132349vector_func.i, %mul567vector_func.i
  %add6062351vector_func.i = fadd <16 x float> %add6042343vector_func.i, %mul5.i142350vector_func.i
  %call.i62.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add6062351vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8312vector_func.i, label %postload8313vector_func.i

preload8312vector_func.i:                         ; preds = %postload8310vector_func.i
  %extract2353vector_func.i = add i64 %dim_0_vector_tid.i, 400896
  %985 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2353vector_func.i
  %exData5340vector_func.i = extractelement <16 x float> %call.i62.i, i32 0
  store float %exData5340vector_func.i, float addrspace(1)* %985, align 4
  br label %postload8313vector_func.i

postload8313vector_func.i:                        ; preds = %preload8312vector_func.i, %postload8310vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8315vector_func.i, label %postload8316vector_func.i

preload8315vector_func.i:                         ; preds = %postload8313vector_func.i
  %.sum8436vector_func.i = add i64 %dim_0_vector_tid.i, 400897
  %986 = getelementptr float addrspace(1)* %4, i64 %.sum8436vector_func.i
  %exData5343vector_func.i = extractelement <16 x float> %call.i62.i, i32 1
  store float %exData5343vector_func.i, float addrspace(1)* %986, align 4
  br label %postload8316vector_func.i

postload8316vector_func.i:                        ; preds = %preload8315vector_func.i, %postload8313vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8318vector_func.i, label %postload8319vector_func.i

preload8318vector_func.i:                         ; preds = %postload8316vector_func.i
  %.sum8435vector_func.i = add i64 %dim_0_vector_tid.i, 400898
  %987 = getelementptr float addrspace(1)* %4, i64 %.sum8435vector_func.i
  %exData5346vector_func.i = extractelement <16 x float> %call.i62.i, i32 2
  store float %exData5346vector_func.i, float addrspace(1)* %987, align 4
  br label %postload8319vector_func.i

postload8319vector_func.i:                        ; preds = %preload8318vector_func.i, %postload8316vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8321vector_func.i, label %postload8322vector_func.i

preload8321vector_func.i:                         ; preds = %postload8319vector_func.i
  %.sum8434vector_func.i = add i64 %dim_0_vector_tid.i, 400899
  %988 = getelementptr float addrspace(1)* %4, i64 %.sum8434vector_func.i
  %exData5349vector_func.i = extractelement <16 x float> %call.i62.i, i32 3
  store float %exData5349vector_func.i, float addrspace(1)* %988, align 4
  br label %postload8322vector_func.i

postload8322vector_func.i:                        ; preds = %preload8321vector_func.i, %postload8319vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8324vector_func.i, label %postload8325vector_func.i

preload8324vector_func.i:                         ; preds = %postload8322vector_func.i
  %.sum8433vector_func.i = add i64 %dim_0_vector_tid.i, 400900
  %989 = getelementptr float addrspace(1)* %4, i64 %.sum8433vector_func.i
  %exData5352vector_func.i = extractelement <16 x float> %call.i62.i, i32 4
  store float %exData5352vector_func.i, float addrspace(1)* %989, align 4
  br label %postload8325vector_func.i

postload8325vector_func.i:                        ; preds = %preload8324vector_func.i, %postload8322vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8327vector_func.i, label %postload8328vector_func.i

preload8327vector_func.i:                         ; preds = %postload8325vector_func.i
  %.sum8432vector_func.i = add i64 %dim_0_vector_tid.i, 400901
  %990 = getelementptr float addrspace(1)* %4, i64 %.sum8432vector_func.i
  %exData5355vector_func.i = extractelement <16 x float> %call.i62.i, i32 5
  store float %exData5355vector_func.i, float addrspace(1)* %990, align 4
  br label %postload8328vector_func.i

postload8328vector_func.i:                        ; preds = %preload8327vector_func.i, %postload8325vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8330vector_func.i, label %postload8331vector_func.i

preload8330vector_func.i:                         ; preds = %postload8328vector_func.i
  %.sum8431vector_func.i = add i64 %dim_0_vector_tid.i, 400902
  %991 = getelementptr float addrspace(1)* %4, i64 %.sum8431vector_func.i
  %exData5358vector_func.i = extractelement <16 x float> %call.i62.i, i32 6
  store float %exData5358vector_func.i, float addrspace(1)* %991, align 4
  br label %postload8331vector_func.i

postload8331vector_func.i:                        ; preds = %preload8330vector_func.i, %postload8328vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8333vector_func.i, label %postload8334vector_func.i

preload8333vector_func.i:                         ; preds = %postload8331vector_func.i
  %.sum8430vector_func.i = add i64 %dim_0_vector_tid.i, 400903
  %992 = getelementptr float addrspace(1)* %4, i64 %.sum8430vector_func.i
  %exData5361vector_func.i = extractelement <16 x float> %call.i62.i, i32 7
  store float %exData5361vector_func.i, float addrspace(1)* %992, align 4
  br label %postload8334vector_func.i

postload8334vector_func.i:                        ; preds = %preload8333vector_func.i, %postload8331vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8336vector_func.i, label %postload8337vector_func.i

preload8336vector_func.i:                         ; preds = %postload8334vector_func.i
  %.sum8429vector_func.i = add i64 %dim_0_vector_tid.i, 400904
  %993 = getelementptr float addrspace(1)* %4, i64 %.sum8429vector_func.i
  %exData5364vector_func.i = extractelement <16 x float> %call.i62.i, i32 8
  store float %exData5364vector_func.i, float addrspace(1)* %993, align 4
  br label %postload8337vector_func.i

postload8337vector_func.i:                        ; preds = %preload8336vector_func.i, %postload8334vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8339vector_func.i, label %postload8340vector_func.i

preload8339vector_func.i:                         ; preds = %postload8337vector_func.i
  %.sum8428vector_func.i = add i64 %dim_0_vector_tid.i, 400905
  %994 = getelementptr float addrspace(1)* %4, i64 %.sum8428vector_func.i
  %exData5367vector_func.i = extractelement <16 x float> %call.i62.i, i32 9
  store float %exData5367vector_func.i, float addrspace(1)* %994, align 4
  br label %postload8340vector_func.i

postload8340vector_func.i:                        ; preds = %preload8339vector_func.i, %postload8337vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8342vector_func.i, label %postload8343vector_func.i

preload8342vector_func.i:                         ; preds = %postload8340vector_func.i
  %.sum8427vector_func.i = add i64 %dim_0_vector_tid.i, 400906
  %995 = getelementptr float addrspace(1)* %4, i64 %.sum8427vector_func.i
  %exData5370vector_func.i = extractelement <16 x float> %call.i62.i, i32 10
  store float %exData5370vector_func.i, float addrspace(1)* %995, align 4
  br label %postload8343vector_func.i

postload8343vector_func.i:                        ; preds = %preload8342vector_func.i, %postload8340vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8345vector_func.i, label %postload8346vector_func.i

preload8345vector_func.i:                         ; preds = %postload8343vector_func.i
  %.sum8426vector_func.i = add i64 %dim_0_vector_tid.i, 400907
  %996 = getelementptr float addrspace(1)* %4, i64 %.sum8426vector_func.i
  %exData5373vector_func.i = extractelement <16 x float> %call.i62.i, i32 11
  store float %exData5373vector_func.i, float addrspace(1)* %996, align 4
  br label %postload8346vector_func.i

postload8346vector_func.i:                        ; preds = %preload8345vector_func.i, %postload8343vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8348vector_func.i, label %postload8349vector_func.i

preload8348vector_func.i:                         ; preds = %postload8346vector_func.i
  %.sum8425vector_func.i = add i64 %dim_0_vector_tid.i, 400908
  %997 = getelementptr float addrspace(1)* %4, i64 %.sum8425vector_func.i
  %exData5376vector_func.i = extractelement <16 x float> %call.i62.i, i32 12
  store float %exData5376vector_func.i, float addrspace(1)* %997, align 4
  br label %postload8349vector_func.i

postload8349vector_func.i:                        ; preds = %preload8348vector_func.i, %postload8346vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8351vector_func.i, label %postload8352vector_func.i

preload8351vector_func.i:                         ; preds = %postload8349vector_func.i
  %.sum8424vector_func.i = add i64 %dim_0_vector_tid.i, 400909
  %998 = getelementptr float addrspace(1)* %4, i64 %.sum8424vector_func.i
  %exData5379vector_func.i = extractelement <16 x float> %call.i62.i, i32 13
  store float %exData5379vector_func.i, float addrspace(1)* %998, align 4
  br label %postload8352vector_func.i

postload8352vector_func.i:                        ; preds = %preload8351vector_func.i, %postload8349vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8354vector_func.i, label %postload8355vector_func.i

preload8354vector_func.i:                         ; preds = %postload8352vector_func.i
  %.sum8423vector_func.i = add i64 %dim_0_vector_tid.i, 400910
  %999 = getelementptr float addrspace(1)* %4, i64 %.sum8423vector_func.i
  %exData5382vector_func.i = extractelement <16 x float> %call.i62.i, i32 14
  store float %exData5382vector_func.i, float addrspace(1)* %999, align 4
  br label %postload8355vector_func.i

postload8355vector_func.i:                        ; preds = %preload8354vector_func.i, %postload8352vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8357vector_func.i, label %postload8358vector_func.i

preload8357vector_func.i:                         ; preds = %postload8355vector_func.i
  %.sum8422vector_func.i = add i64 %dim_0_vector_tid.i, 400911
  %1000 = getelementptr float addrspace(1)* %4, i64 %.sum8422vector_func.i
  %exData5385vector_func.i = extractelement <16 x float> %call.i62.i, i32 15
  store float %exData5385vector_func.i, float addrspace(1)* %1000, align 4
  br label %postload8358vector_func.i

postload8358vector_func.i:                        ; preds = %preload8357vector_func.i, %postload8355vector_func.i
  %mul6112370vector_func.i = fmul <16 x float> %div568vector_func.i, <float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000, float 0x40C4242C40000000>
  %sub6122371vector_func.i = fsub <16 x float> <float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000, float 0x403522D320000000>, %mul6112370vector_func.i
  %mul6132372vector_func.i = fmul <16 x float> %sub569vector_func.i, <float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000, float 0x3FF0C92F40000000>
  %add6142373vector_func.i = fadd <16 x float> %sub6122371vector_func.i, %mul6132372vector_func.i
  %mul.i12374vector_func.i = fmul <16 x float> %mul567vector_func.i, <float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000, float 0x3D607CC860000000>
  %add.i22375vector_func.i = fadd <16 x float> %mul.i12374vector_func.i, <float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000, float 0xBE1C0DB120000000>
  %mul1.i32376vector_func.i = fmul <16 x float> %add.i22375vector_func.i, %mul567vector_func.i
  %add2.i42377vector_func.i = fadd <16 x float> %mul1.i32376vector_func.i, <float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000, float 0x3E9A54F4A0000000>
  %mul3.i52378vector_func.i = fmul <16 x float> %add2.i42377vector_func.i, %mul567vector_func.i
  %add4.i62379vector_func.i = fadd <16 x float> %mul3.i52378vector_func.i, <float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000, float 0x3F8AA218A0000000>
  %mul5.i72380vector_func.i = fmul <16 x float> %add4.i62379vector_func.i, %mul567vector_func.i
  %add6162381vector_func.i = fadd <16 x float> %add6142373vector_func.i, %mul5.i72380vector_func.i
  %call.i63.i = call x86_svmlcc <16 x float> @__ocl_svml_b1_expf16(<16 x float> %add6162381vector_func.i) nounwind readnone
  br i1 %exmask3918vector_func.i, label %preload8360vector_func.i, label %postload8361vector_func.i

preload8360vector_func.i:                         ; preds = %postload8358vector_func.i
  %extract2383vector_func.i = add i64 %dim_0_vector_tid.i, 414720
  %1001 = getelementptr inbounds float addrspace(1)* %4, i64 %extract2383vector_func.i
  %exData5389vector_func.i = extractelement <16 x float> %call.i63.i, i32 0
  store float %exData5389vector_func.i, float addrspace(1)* %1001, align 4
  br label %postload8361vector_func.i

postload8361vector_func.i:                        ; preds = %preload8360vector_func.i, %postload8358vector_func.i
  br i1 %exmask3921vector_func.i, label %preload8363vector_func.i, label %postload8364vector_func.i

preload8363vector_func.i:                         ; preds = %postload8361vector_func.i
  %.sum8421vector_func.i = add i64 %dim_0_vector_tid.i, 414721
  %1002 = getelementptr float addrspace(1)* %4, i64 %.sum8421vector_func.i
  %exData5392vector_func.i = extractelement <16 x float> %call.i63.i, i32 1
  store float %exData5392vector_func.i, float addrspace(1)* %1002, align 4
  br label %postload8364vector_func.i

postload8364vector_func.i:                        ; preds = %preload8363vector_func.i, %postload8361vector_func.i
  br i1 %exmask3924vector_func.i, label %preload8366vector_func.i, label %postload8367vector_func.i

preload8366vector_func.i:                         ; preds = %postload8364vector_func.i
  %.sum8420vector_func.i = add i64 %dim_0_vector_tid.i, 414722
  %1003 = getelementptr float addrspace(1)* %4, i64 %.sum8420vector_func.i
  %exData5395vector_func.i = extractelement <16 x float> %call.i63.i, i32 2
  store float %exData5395vector_func.i, float addrspace(1)* %1003, align 4
  br label %postload8367vector_func.i

postload8367vector_func.i:                        ; preds = %preload8366vector_func.i, %postload8364vector_func.i
  br i1 %exmask3927vector_func.i, label %preload8369vector_func.i, label %postload8370vector_func.i

preload8369vector_func.i:                         ; preds = %postload8367vector_func.i
  %.sum8419vector_func.i = add i64 %dim_0_vector_tid.i, 414723
  %1004 = getelementptr float addrspace(1)* %4, i64 %.sum8419vector_func.i
  %exData5398vector_func.i = extractelement <16 x float> %call.i63.i, i32 3
  store float %exData5398vector_func.i, float addrspace(1)* %1004, align 4
  br label %postload8370vector_func.i

postload8370vector_func.i:                        ; preds = %preload8369vector_func.i, %postload8367vector_func.i
  br i1 %exmask3930vector_func.i, label %preload8372vector_func.i, label %postload8373vector_func.i

preload8372vector_func.i:                         ; preds = %postload8370vector_func.i
  %.sum8418vector_func.i = add i64 %dim_0_vector_tid.i, 414724
  %1005 = getelementptr float addrspace(1)* %4, i64 %.sum8418vector_func.i
  %exData5401vector_func.i = extractelement <16 x float> %call.i63.i, i32 4
  store float %exData5401vector_func.i, float addrspace(1)* %1005, align 4
  br label %postload8373vector_func.i

postload8373vector_func.i:                        ; preds = %preload8372vector_func.i, %postload8370vector_func.i
  br i1 %exmask3933vector_func.i, label %preload8375vector_func.i, label %postload8376vector_func.i

preload8375vector_func.i:                         ; preds = %postload8373vector_func.i
  %.sum8417vector_func.i = add i64 %dim_0_vector_tid.i, 414725
  %1006 = getelementptr float addrspace(1)* %4, i64 %.sum8417vector_func.i
  %exData5404vector_func.i = extractelement <16 x float> %call.i63.i, i32 5
  store float %exData5404vector_func.i, float addrspace(1)* %1006, align 4
  br label %postload8376vector_func.i

postload8376vector_func.i:                        ; preds = %preload8375vector_func.i, %postload8373vector_func.i
  br i1 %exmask3936vector_func.i, label %preload8378vector_func.i, label %postload8379vector_func.i

preload8378vector_func.i:                         ; preds = %postload8376vector_func.i
  %.sum8416vector_func.i = add i64 %dim_0_vector_tid.i, 414726
  %1007 = getelementptr float addrspace(1)* %4, i64 %.sum8416vector_func.i
  %exData5407vector_func.i = extractelement <16 x float> %call.i63.i, i32 6
  store float %exData5407vector_func.i, float addrspace(1)* %1007, align 4
  br label %postload8379vector_func.i

postload8379vector_func.i:                        ; preds = %preload8378vector_func.i, %postload8376vector_func.i
  br i1 %exmask3939vector_func.i, label %preload8381vector_func.i, label %postload8382vector_func.i

preload8381vector_func.i:                         ; preds = %postload8379vector_func.i
  %.sum8415vector_func.i = add i64 %dim_0_vector_tid.i, 414727
  %1008 = getelementptr float addrspace(1)* %4, i64 %.sum8415vector_func.i
  %exData5410vector_func.i = extractelement <16 x float> %call.i63.i, i32 7
  store float %exData5410vector_func.i, float addrspace(1)* %1008, align 4
  br label %postload8382vector_func.i

postload8382vector_func.i:                        ; preds = %preload8381vector_func.i, %postload8379vector_func.i
  br i1 %exmask3942vector_func.i, label %preload8384vector_func.i, label %postload8385vector_func.i

preload8384vector_func.i:                         ; preds = %postload8382vector_func.i
  %.sum8414vector_func.i = add i64 %dim_0_vector_tid.i, 414728
  %1009 = getelementptr float addrspace(1)* %4, i64 %.sum8414vector_func.i
  %exData5413vector_func.i = extractelement <16 x float> %call.i63.i, i32 8
  store float %exData5413vector_func.i, float addrspace(1)* %1009, align 4
  br label %postload8385vector_func.i

postload8385vector_func.i:                        ; preds = %preload8384vector_func.i, %postload8382vector_func.i
  br i1 %exmask3945vector_func.i, label %preload8387vector_func.i, label %postload8388vector_func.i

preload8387vector_func.i:                         ; preds = %postload8385vector_func.i
  %.sum8413vector_func.i = add i64 %dim_0_vector_tid.i, 414729
  %1010 = getelementptr float addrspace(1)* %4, i64 %.sum8413vector_func.i
  %exData5416vector_func.i = extractelement <16 x float> %call.i63.i, i32 9
  store float %exData5416vector_func.i, float addrspace(1)* %1010, align 4
  br label %postload8388vector_func.i

postload8388vector_func.i:                        ; preds = %preload8387vector_func.i, %postload8385vector_func.i
  br i1 %exmask3948vector_func.i, label %preload8390vector_func.i, label %postload8391vector_func.i

preload8390vector_func.i:                         ; preds = %postload8388vector_func.i
  %.sum8412vector_func.i = add i64 %dim_0_vector_tid.i, 414730
  %1011 = getelementptr float addrspace(1)* %4, i64 %.sum8412vector_func.i
  %exData5419vector_func.i = extractelement <16 x float> %call.i63.i, i32 10
  store float %exData5419vector_func.i, float addrspace(1)* %1011, align 4
  br label %postload8391vector_func.i

postload8391vector_func.i:                        ; preds = %preload8390vector_func.i, %postload8388vector_func.i
  br i1 %exmask3951vector_func.i, label %preload8393vector_func.i, label %postload8394vector_func.i

preload8393vector_func.i:                         ; preds = %postload8391vector_func.i
  %.sum8411vector_func.i = add i64 %dim_0_vector_tid.i, 414731
  %1012 = getelementptr float addrspace(1)* %4, i64 %.sum8411vector_func.i
  %exData5422vector_func.i = extractelement <16 x float> %call.i63.i, i32 11
  store float %exData5422vector_func.i, float addrspace(1)* %1012, align 4
  br label %postload8394vector_func.i

postload8394vector_func.i:                        ; preds = %preload8393vector_func.i, %postload8391vector_func.i
  br i1 %exmask3954vector_func.i, label %preload8396vector_func.i, label %postload8397vector_func.i

preload8396vector_func.i:                         ; preds = %postload8394vector_func.i
  %.sum8410vector_func.i = add i64 %dim_0_vector_tid.i, 414732
  %1013 = getelementptr float addrspace(1)* %4, i64 %.sum8410vector_func.i
  %exData5425vector_func.i = extractelement <16 x float> %call.i63.i, i32 12
  store float %exData5425vector_func.i, float addrspace(1)* %1013, align 4
  br label %postload8397vector_func.i

postload8397vector_func.i:                        ; preds = %preload8396vector_func.i, %postload8394vector_func.i
  br i1 %exmask3957vector_func.i, label %preload8399vector_func.i, label %postload8400vector_func.i

preload8399vector_func.i:                         ; preds = %postload8397vector_func.i
  %.sum8409vector_func.i = add i64 %dim_0_vector_tid.i, 414733
  %1014 = getelementptr float addrspace(1)* %4, i64 %.sum8409vector_func.i
  %exData5428vector_func.i = extractelement <16 x float> %call.i63.i, i32 13
  store float %exData5428vector_func.i, float addrspace(1)* %1014, align 4
  br label %postload8400vector_func.i

postload8400vector_func.i:                        ; preds = %preload8399vector_func.i, %postload8397vector_func.i
  br i1 %exmask3960vector_func.i, label %preload8402vector_func.i, label %postload8403vector_func.i

preload8402vector_func.i:                         ; preds = %postload8400vector_func.i
  %.sum8408vector_func.i = add i64 %dim_0_vector_tid.i, 414734
  %1015 = getelementptr float addrspace(1)* %4, i64 %.sum8408vector_func.i
  %exData5431vector_func.i = extractelement <16 x float> %call.i63.i, i32 14
  store float %exData5431vector_func.i, float addrspace(1)* %1015, align 4
  br label %postload8403vector_func.i

postload8403vector_func.i:                        ; preds = %preload8402vector_func.i, %postload8400vector_func.i
  br i1 %exmask3963vector_func.i, label %preload8405vector_func.i, label %if.endvector_func.i

preload8405vector_func.i:                         ; preds = %postload8403vector_func.i
  %.sumvector_func.i = add i64 %dim_0_vector_tid.i, 414735
  %1016 = getelementptr float addrspace(1)* %4, i64 %.sumvector_func.i
  %exData5434vector_func.i = extractelement <16 x float> %call.i63.i, i32 15
  store float %exData5434vector_func.i, float addrspace(1)* %1016, align 4
  br label %if.endvector_func.i

if.endvector_func.i:                              ; preds = %preload8405vector_func.i, %postload8403vector_func.i
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %if.endvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %19
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %21
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %1017 = icmp eq i64 %15, %num.vector.wi.i
  br i1 %1017, label %__rdsmh_kernel_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %if.end.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %if.end.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %if.end.i ]
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %dim_0_tid.i
  %1018 = load float addrspace(1)* %arrayidx.i, align 4
  %mul.i = fmul float %1018, %7
  %call.i64.i = call x86_svmlcc float @__ocl_svml_b1_logf1(float %mul.i) nounwind readnone
  %div.i = fdiv float 1.000000e+00, %mul.i
  %sub.i = fadd float %call.i64.i, -1.000000e+00
  %cmp.i = fcmp ogt float %mul.i, 1.000000e+03
  br i1 %cmp.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %scalar_kernel_entry.i
  %mul2.i = fmul float %div.i, 0x408DB14580000000
  %add.i = fadd float %mul2.i, 0xC009A3E340000000
  %mul3.i = fmul float %sub.i, 0x400AB2BF60000000
  %add4.i = fadd float %add.i, %mul3.i
  %mul.i.i = fmul float %mul.i, 0x3CD2099320000000
  %add.i.i = fadd float %mul.i.i, 0xBDB073F440000000
  %mul1.i.i = fmul float %add.i.i, %mul.i
  %add2.i.i = fadd float %mul1.i.i, 0x3E765866C0000000
  %mul3.i.i = fmul float %add2.i.i, %mul.i
  %add4.i.i = fadd float %mul3.i.i, 0xBEF9E6B000000000
  %mul5.i.i = fmul float %add4.i.i, %mul.i
  %add6.i = fadd float %add4.i, %mul5.i.i
  %call.i65.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add6.i) nounwind readnone
  %arrayidx10.i = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_tid.i
  store float %call.i65.i, float addrspace(1)* %arrayidx10.i, align 4
  %mul11.i = fmul float %div.i, 0x40D8E06A40000000
  %sub12.i = fsub float 0xBFDC9673E0000000, %mul11.i
  %mul13.i = fmul float %sub.i, 2.500000e+00
  %add14.i = fadd float %sub12.i, %mul13.i
  %mul.i421.i = fmul float %mul.i, 0x3B3E1D3B00000000
  %add.i422.i = fadd float %mul.i421.i, 0xBC1D1DB540000000
  %mul1.i423.i = fmul float %add.i422.i, %mul.i
  %add2.i424.i = fadd float %mul1.i423.i, 0x3CE840F100000000
  %mul3.i425.i = fmul float %add2.i424.i, %mul.i
  %add4.i426.i = fadd float %mul3.i425.i, 0xBDA961A6E0000000
  %mul5.i427.i = fmul float %add4.i426.i, %mul.i
  %add16.i = fadd float %add14.i, %mul5.i427.i
  %call.i66.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add16.i) nounwind readnone
  %add19.i = add i64 %dim_0_tid.i, 13824
  %arrayidx20.i = getelementptr inbounds float addrspace(1)* %4, i64 %add19.i
  store float %call.i66.i, float addrspace(1)* %arrayidx20.i, align 4
  %mul21.i = fmul float %div.i, 0x40DC886500000000
  %sub22.i = fsub float 0x40132329A0000000, %mul21.i
  %mul23.i = fmul float %sub.i, 0x40048E2C80000000
  %add24.i = fadd float %sub22.i, %mul23.i
  %mul.i414.i = fmul float %mul.i, 0x3C91B3C360000000
  %add.i415.i = fadd float %mul.i414.i, 0xBD6D5F5860000000
  %mul1.i416.i = fmul float %add.i415.i, %mul.i
  %add2.i417.i = fadd float %mul1.i416.i, 0x3E3E0722E0000000
  %mul3.i418.i = fmul float %add2.i417.i, %mul.i
  %add4.i419.i = fadd float %mul3.i418.i, 0xBF0689A000000000
  %mul5.i420.i = fmul float %add4.i419.i, %mul.i
  %add26.i = fadd float %add24.i, %mul5.i420.i
  %call.i67.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add26.i) nounwind readnone
  %add29.i = add i64 %dim_0_tid.i, 27648
  %arrayidx30.i = getelementptr inbounds float addrspace(1)* %4, i64 %add29.i
  store float %call.i67.i, float addrspace(1)* %arrayidx30.i, align 4
  %mul31.i = fmul float %div.i, 0x409101D4C0000000
  %add32.i = fadd float %mul31.i, 0x4015D01BE0000000
  %mul33.i = fmul float %sub.i, 0x400A42A340000000
  %add34.i = fadd float %add32.i, %mul33.i
  %mul.i407.i = fmul float %mul.i, 0xBCD3852C00000000
  %add.i408.i = fadd float %mul.i407.i, 0x3DB33164A0000000
  %mul1.i409.i = fmul float %add.i408.i, %mul.i
  %add2.i410.i = fadd float %mul1.i409.i, 0xBE80F496E0000000
  %mul3.i411.i = fmul float %add2.i410.i, %mul.i
  %add4.i412.i = fadd float %mul3.i411.i, 0x3F484C8520000000
  %mul5.i413.i = fmul float %add4.i412.i, %mul.i
  %add36.i = fadd float %add34.i, %mul5.i413.i
  %call.i68.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add36.i) nounwind readnone
  %add39.i = add i64 %dim_0_tid.i, 41472
  %arrayidx40.i = getelementptr inbounds float addrspace(1)* %4, i64 %add39.i
  store float %call.i68.i, float addrspace(1)* %arrayidx40.i, align 4
  %mul41.i = fmul float %div.i, 0x40AE255060000000
  %sub42.i = fsub float 0x4011E82300000000, %mul41.i
  %mul43.i = fmul float %sub.i, 0x4008BE3BE0000000
  %add44.i = fadd float %sub42.i, %mul43.i
  %mul.i400.i = fmul float %mul.i, 0x3CC526B0A0000000
  %add.i401.i = fadd float %mul.i400.i, 0xBDA01DC620000000
  %mul1.i402.i = fmul float %add.i401.i, %mul.i
  %add2.i403.i = fadd float %mul1.i402.i, 0x3E56A39500000000
  %mul3.i404.i = fmul float %add2.i403.i, %mul.i
  %add4.i405.i = fadd float %mul3.i404.i, 0x3F31F88FE0000000
  %mul5.i406.i = fmul float %add4.i405.i, %mul.i
  %add46.i = fadd float %add44.i, %mul5.i406.i
  %call.i69.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add46.i) nounwind readnone
  %add49.i = add i64 %dim_0_tid.i, 55296
  %arrayidx50.i = getelementptr inbounds float addrspace(1)* %4, i64 %add49.i
  store float %call.i69.i, float addrspace(1)* %arrayidx50.i, align 4
  %mul51.i = fmul float %div.i, 0x40DD4D1300000000
  %add52.i = fadd float %mul51.i, 0x4013DDF900000000
  %mul53.i = fmul float %sub.i, 0x4008459DE0000000
  %add54.i = fadd float %add52.i, %mul53.i
  %mul.i393.i = fmul float %mul.i, 0x3CCE4CE6E0000000
  %add.i394.i = fadd float %mul.i393.i, 0xBDA1C87B60000000
  %mul1.i395.i = fmul float %add.i394.i, %mul.i
  %add2.i396.i = fadd float %mul1.i395.i, 0xBE5D5CA6E0000000
  %mul3.i397.i = fmul float %add2.i396.i, %mul.i
  %add4.i398.i = fadd float %mul3.i397.i, 0x3F51D55400000000
  %mul5.i399.i = fmul float %add4.i398.i, %mul.i
  %add56.i = fadd float %add54.i, %mul5.i399.i
  %call.i70.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add56.i) nounwind readnone
  %add59.i = add i64 %dim_0_tid.i, 69120
  %arrayidx60.i = getelementptr inbounds float addrspace(1)* %4, i64 %add59.i
  store float %call.i70.i, float addrspace(1)* %arrayidx60.i, align 4
  %mul61.i = fmul float %div.i, 0x405BF6D460000000
  %sub62.i = fsub float 0x400E47E3A0000000, %mul61.i
  %mul63.i = fmul float %sub.i, 0x4010119FC0000000
  %add64.i = fadd float %sub62.i, %mul63.i
  %mul.i386.i = fmul float %mul.i, 0xBCC3706720000000
  %add.i387.i = fadd float %mul.i386.i, 0x3DA4EF9520000000
  %mul1.i388.i = fmul float %add.i387.i, %mul.i
  %add2.i389.i = fadd float %mul1.i388.i, 0xBE7C597160000000
  %mul3.i390.i = fmul float %add2.i389.i, %mul.i
  %add4.i391.i = fadd float %mul3.i390.i, 0x3F52593E40000000
  %mul5.i392.i = fmul float %add4.i391.i, %mul.i
  %add66.i = fadd float %add64.i, %mul5.i392.i
  %call.i71.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add66.i) nounwind readnone
  %add69.i = add i64 %dim_0_tid.i, 82944
  %arrayidx70.i = getelementptr inbounds float addrspace(1)* %4, i64 %add69.i
  store float %call.i71.i, float addrspace(1)* %arrayidx70.i, align 4
  %mul71.i = fmul float %div.i, 0x40D1717260000000
  %add72.i = fadd float %mul71.i, 0x40075449E0000000
  %mul73.i = fmul float %sub.i, 0x4010A8F680000000
  %add74.i = fadd float %add72.i, %mul73.i
  %mul.i379.i = fmul float %mul.i, 0xBCD9EEB6A0000000
  %add.i380.i = fadd float %mul.i379.i, 0x3DC10150C0000000
  %mul1.i381.i = fmul float %add.i380.i, %mul.i
  %add2.i382.i = fadd float %mul1.i381.i, 0xBE95444740000000
  %mul3.i383.i = fmul float %add2.i382.i, %mul.i
  %add4.i384.i = fadd float %mul3.i383.i, 0x3F641ABE40000000
  %mul5.i385.i = fmul float %add4.i384.i, %mul.i
  %add76.i = fadd float %add74.i, %mul5.i385.i
  %call.i72.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add76.i) nounwind readnone
  %add79.i = add i64 %dim_0_tid.i, 96768
  %arrayidx80.i = getelementptr inbounds float addrspace(1)* %4, i64 %add79.i
  store float %call.i72.i, float addrspace(1)* %arrayidx80.i, align 4
  %mul81.i = fmul float %div.i, 0x40F1564700000000
  %sub82.i = fsub float 0x4015F09EA0000000, %mul81.i
  %mul83.i = fmul float %sub.i, 0x4007071880000000
  %add84.i = fadd float %sub82.i, %mul83.i
  %mul.i372.i = fmul float %mul.i, 0x3CCFB83A80000000
  %add.i373.i = fadd float %mul.i372.i, 0xBDA7F2E4A0000000
  %mul1.i374.i = fmul float %add.i373.i, %mul.i
  %add2.i375.i = fadd float %mul1.i374.i, 0x3E59D97C80000000
  %mul3.i376.i = fmul float %add2.i375.i, %mul.i
  %add4.i377.i = fadd float %mul3.i376.i, 0x3F3FD09D40000000
  %mul5.i378.i = fmul float %add4.i377.i, %mul.i
  %add86.i = fadd float %add84.i, %mul5.i378.i
  %call.i73.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add86.i) nounwind readnone
  %add89.i = add i64 %dim_0_tid.i, 110592
  %arrayidx90.i = getelementptr inbounds float addrspace(1)* %4, i64 %add89.i
  store float %call.i73.i, float addrspace(1)* %arrayidx90.i, align 4
  %mul91.i = fmul float %div.i, 0x40E696F360000000
  %sub92.i = fsub float 0x4018AF4D40000000, %mul91.i
  %mul93.i = fmul float %sub.i, 0x4006FE28C0000000
  %add94.i = fadd float %sub92.i, %mul93.i
  %mul.i365.i = fmul float %mul.i, 0xBCD0E8B400000000
  %add.i366.i = fadd float %mul.i365.i, 0x3DB7D6D600000000
  %mul1.i367.i = fmul float %add.i366.i, %mul.i
  %add2.i368.i = fadd float %mul1.i367.i, 0xBE8F8480A0000000
  %mul3.i369.i = fmul float %add2.i368.i, %mul.i
  %add4.i370.i = fadd float %mul3.i369.i, 0x3F5DF40300000000
  %mul5.i371.i = fmul float %add4.i370.i, %mul.i
  %add96.i = fadd float %add94.i, %mul5.i371.i
  %call.i74.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add96.i) nounwind readnone
  %add99.i = add i64 %dim_0_tid.i, 124416
  %arrayidx100.i = getelementptr inbounds float addrspace(1)* %4, i64 %add99.i
  store float %call.i74.i, float addrspace(1)* %arrayidx100.i, align 4
  %mul101.i = fmul float %div.i, 5.092600e+04
  %sub102.i = fsub float 0x402140C4E0000000, %mul101.i
  %mul103.i = fmul float %sub.i, 0x4002561840000000
  %add104.i = fadd float %sub102.i, %mul103.i
  %mul.i358.i = fmul float %mul.i, 0xBCDE995380000000
  %add.i359.i = fadd float %mul.i358.i, 0x3DC32540E0000000
  %mul1.i360.i = fmul float %add.i359.i, %mul.i
  %add2.i361.i = fadd float %mul1.i360.i, 0xBE9680C0A0000000
  %mul3.i362.i = fmul float %add2.i361.i, %mul.i
  %add4.i363.i = fadd float %mul3.i362.i, 0x3F63120D00000000
  %mul5.i364.i = fmul float %add4.i363.i, %mul.i
  %add106.i = fadd float %add104.i, %mul5.i364.i
  %call.i75.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add106.i) nounwind readnone
  %add109.i = add i64 %dim_0_tid.i, 138240
  %arrayidx110.i = getelementptr inbounds float addrspace(1)* %4, i64 %add109.i
  store float %call.i75.i, float addrspace(1)* %arrayidx110.i, align 4
  %mul111.i = fmul float %div.i, 0x40D061E560000000
  %sub112.i = fsub float 0x4020F5CC00000000, %mul111.i
  %mul113.i = fmul float %sub.i, 0x4002492660000000
  %add114.i = fadd float %sub112.i, %mul113.i
  %mul.i351.i = fmul float %mul.i, 0xBCE509EC60000000
  %add.i352.i = fadd float %mul.i351.i, 0x3DCB4A4360000000
  %mul1.i353.i = fmul float %add.i352.i, %mul.i
  %add2.i354.i = fadd float %mul1.i353.i, 0xBEA0B48FA0000000
  %mul3.i355.i = fmul float %add2.i354.i, %mul.i
  %add4.i356.i = fadd float %mul3.i355.i, 0x3F6DA79600000000
  %mul5.i357.i = fmul float %add4.i356.i, %mul.i
  %add116.i = fadd float %add114.i, %mul5.i357.i
  %call.i76.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add116.i) nounwind readnone
  %add119.i = add i64 %dim_0_tid.i, 152064
  %arrayidx120.i = getelementptr inbounds float addrspace(1)* %4, i64 %add119.i
  store float %call.i76.i, float addrspace(1)* %arrayidx120.i, align 4
  %mul121.i = fmul float %div.i, 0x40C27E2C20000000
  %add122.i = fadd float %mul121.i, 0x40326FF420000000
  %mul123.i = fmul float %sub.i, 0x3FB32977C0000000
  %add124.i = fadd float %add122.i, %mul123.i
  %mul.i344.i = fmul float %mul.i, 0xBCF6ED3FA0000000
  %add.i345.i = fadd float %mul.i344.i, 0x3DDC034F60000000
  %mul1.i346.i = fmul float %add.i345.i, %mul.i
  %add2.i347.i = fadd float %mul1.i346.i, 0xBEB007BD60000000
  %mul3.i348.i = fmul float %add2.i347.i, %mul.i
  %add4.i349.i = fadd float %mul3.i348.i, 0x3F7B6CB680000000
  %mul5.i350.i = fmul float %add4.i349.i, %mul.i
  %add126.i = fadd float %add124.i, %mul5.i350.i
  %call.i77.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add126.i) nounwind readnone
  %add129.i = add i64 %dim_0_tid.i, 165888
  %arrayidx130.i = getelementptr inbounds float addrspace(1)* %4, i64 %add129.i
  store float %call.i77.i, float addrspace(1)* %arrayidx130.i, align 4
  %mul131.i = fmul float %div.i, 0x40CBA3EFA0000000
  %add132.i = fadd float %mul131.i, 0x401F465620000000
  %mul133.i = fmul float %sub.i, 0x4005B8B340000000
  %add134.i = fadd float %add132.i, %mul133.i
  %mul.i337.i = fmul float %mul.i, 0xBCD257CBE0000000
  %add.i338.i = fadd float %mul.i337.i, 0x3DB5142E40000000
  %mul1.i339.i = fmul float %add.i338.i, %mul.i
  %add2.i340.i = fadd float %mul1.i339.i, 0xBE8657E620000000
  %mul3.i341.i = fmul float %add2.i340.i, %mul.i
  %add4.i342.i = fadd float %mul3.i341.i, 0x3F50E56F00000000
  %mul5.i343.i = fmul float %add4.i342.i, %mul.i
  %add136.i = fadd float %add134.i, %mul5.i343.i
  %call.i78.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add136.i) nounwind readnone
  %add139.i = add i64 %dim_0_tid.i, 179712
  %arrayidx140.i = getelementptr inbounds float addrspace(1)* %4, i64 %add139.i
  store float %call.i78.i, float addrspace(1)* %arrayidx140.i, align 4
  %mul141.i = fmul float %div.i, 0x40E7CEE540000000
  %add142.i = fadd float %mul141.i, 0x40022C50A0000000
  %mul143.i = fmul float %sub.i, 0x400EDC1420000000
  %add144.i = fadd float %add142.i, %mul143.i
  %mul.i330.i = fmul float %mul.i, 0xBCE542C280000000
  %add.i331.i = fadd float %mul.i330.i, 0x3DC7FB8EC0000000
  %mul1.i332.i = fmul float %add.i331.i, %mul.i
  %add2.i333.i = fadd float %mul1.i332.i, 0xBE98C5B3E0000000
  %mul3.i334.i = fmul float %add2.i333.i, %mul.i
  %add4.i335.i = fadd float %mul3.i334.i, 0x3F6214CD80000000
  %mul5.i336.i = fmul float %add4.i335.i, %mul.i
  %add146.i = fadd float %add144.i, %mul5.i336.i
  %call.i79.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add146.i) nounwind readnone
  %add149.i = add i64 %dim_0_tid.i, 193536
  %arrayidx150.i = getelementptr inbounds float addrspace(1)* %4, i64 %add149.i
  store float %call.i79.i, float addrspace(1)* %arrayidx150.i, align 4
  %mul151.i = fmul float %div.i, 0x40AF57D620000000
  %sub152.i = fsub float 0x402398C0A0000000, %mul151.i
  %mul153.i = fmul float %sub.i, 0x40062D69C0000000
  %add154.i = fadd float %sub152.i, %mul153.i
  %mul.i323.i = fmul float %mul.i, 0xBCE806EFC0000000
  %add.i324.i = fadd float %mul.i323.i, 0x3DCAFDC320000000
  %mul1.i325.i = fmul float %add.i324.i, %mul.i
  %add2.i326.i = fadd float %mul1.i325.i, 0xBE9BC9C5A0000000
  %mul3.i327.i = fmul float %add2.i326.i, %mul.i
  %add4.i328.i = fadd float %mul3.i327.i, 0x3F644DBE80000000
  %mul5.i329.i = fmul float %add4.i328.i, %mul.i
  %add156.i = fadd float %add154.i, %mul5.i329.i
  %call.i80.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add156.i) nounwind readnone
  %add159.i = add i64 %dim_0_tid.i, 207360
  %arrayidx160.i = getelementptr inbounds float addrspace(1)* %4, i64 %add159.i
  store float %call.i80.i, float addrspace(1)* %arrayidx160.i, align 4
  %mul161.i = fmul float %div.i, 0x40CB55EA80000000
  %add162.i = fadd float %mul161.i, 0x402B5009A0000000
  %mul163.i = fmul float %sub.i, 0x3FFC2BC960000000
  %add164.i = fadd float %add162.i, %mul163.i
  %mul.i316.i = fmul float %mul.i, 0xBCF3E714C0000000
  %add.i317.i = fadd float %mul.i316.i, 0x3DD70DA9C0000000
  %mul1.i318.i = fmul float %add.i317.i, %mul.i
  %add2.i319.i = fadd float %mul1.i318.i, 0xBEA8BB9FC0000000
  %mul3.i320.i = fmul float %add2.i319.i, %mul.i
  %add4.i321.i = fadd float %mul3.i320.i, 0x3F72D77340000000
  %mul5.i322.i = fmul float %add4.i321.i, %mul.i
  %add166.i = fadd float %add164.i, %mul5.i322.i
  %call.i81.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add166.i) nounwind readnone
  %add169.i = add i64 %dim_0_tid.i, 221184
  %arrayidx170.i = getelementptr inbounds float addrspace(1)* %4, i64 %add169.i
  store float %call.i81.i, float addrspace(1)* %arrayidx170.i, align 4
  %mul171.i = fmul float %div.i, 0x405FF54800000000
  %sub172.i = fsub float 0x40076FC500000000, %mul171.i
  %mul173.i = fmul float %sub.i, 0x400E2A98A0000000
  %add174.i = fadd float %sub172.i, %mul173.i
  %mul.i309.i = fmul float %mul.i, 0xBCD3075C60000000
  %add.i310.i = fadd float %mul.i309.i, 0x3DC21213E0000000
  %mul1.i311.i = fmul float %add.i310.i, %mul.i
  %add2.i312.i = fadd float %mul1.i311.i, 0xBE9DB60E20000000
  %mul3.i313.i = fmul float %add2.i312.i, %mul.i
  %add4.i314.i = fadd float %mul3.i313.i, 0x3F701EEE80000000
  %mul5.i315.i = fmul float %add4.i314.i, %mul.i
  %add176.i = fadd float %add174.i, %mul5.i315.i
  %call.i82.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add176.i) nounwind readnone
  %add179.i = add i64 %dim_0_tid.i, 235008
  %arrayidx180.i = getelementptr inbounds float addrspace(1)* %4, i64 %add179.i
  store float %call.i82.i, float addrspace(1)* %arrayidx180.i, align 4
  %mul181.i = fmul float %div.i, 2.593600e+04
  %sub182.i = fsub float 0xBFF3AF3B60000000, %mul181.i
  %mul183.i = fmul float %sub.i, 0x4010971C80000000
  %add184.i = fadd float %sub182.i, %mul183.i
  %mul.i302.i = fmul float %mul.i, 0xBCE044C220000000
  %add.i303.i = fadd float %mul.i302.i, 0x3DC569DE40000000
  %mul1.i304.i = fmul float %add.i303.i, %mul.i
  %add2.i305.i = fadd float %mul1.i304.i, 0xBE9A8A7DA0000000
  %mul3.i306.i = fmul float %add2.i305.i, %mul.i
  %add4.i307.i = fadd float %mul3.i306.i, 0x3F686B42C0000000
  %mul5.i308.i = fmul float %add4.i307.i, %mul.i
  %add186.i = fadd float %add184.i, %mul5.i308.i
  %call.i83.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add186.i) nounwind readnone
  %add189.i = add i64 %dim_0_tid.i, 248832
  %arrayidx190.i = getelementptr inbounds float addrspace(1)* %4, i64 %add189.i
  store float %call.i83.i, float addrspace(1)* %arrayidx190.i, align 4
  %mul191.i = fmul float %div.i, 0x40E7979600000000
  %sub192.i = fsub float 0x3FE47CD260000000, %mul191.i
  %mul193.i = fmul float %sub.i, 0x40111CB500000000
  %add194.i = fadd float %sub192.i, %mul193.i
  %mul.i295.i = fmul float %mul.i, 0xBCCAD12160000000
  %add.i296.i = fadd float %mul.i295.i, 0x3DB7549E80000000
  %mul1.i297.i = fmul float %add.i296.i, %mul.i
  %add2.i298.i = fadd float %mul1.i297.i, 0xBE923B7CA0000000
  %mul3.i299.i = fmul float %add2.i298.i, %mul.i
  %add4.i300.i = fadd float %mul3.i299.i, 0x3F637B5240000000
  %mul5.i301.i = fmul float %add4.i300.i, %mul.i
  %add196.i = fadd float %add194.i, %mul5.i301.i
  %call.i84.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add196.i) nounwind readnone
  %add199.i = add i64 %dim_0_tid.i, 262656
  %arrayidx200.i = getelementptr inbounds float addrspace(1)* %4, i64 %add199.i
  store float %call.i84.i, float addrspace(1)* %arrayidx200.i, align 4
  %mul201.i = fmul float %div.i, 0x40E0E69C00000000
  %sub202.i = fsub float 0x401F263840000000, %mul201.i
  %mul203.i = fmul float %sub.i, 0x4008224040000000
  %add204.i = fadd float %sub202.i, %mul203.i
  %mul.i288.i = fmul float %mul.i, 0xBCF36C9740000000
  %add.i289.i = fadd float %mul.i288.i, 0x3DD74F7660000000
  %mul1.i290.i = fmul float %add.i289.i, %mul.i
  %add2.i291.i = fadd float %mul1.i290.i, 0xBEAA2D5400000000
  %mul3.i292.i = fmul float %add2.i291.i, %mul.i
  %add4.i293.i = fadd float %mul3.i292.i, 0x3F752803E0000000
  %mul5.i294.i = fmul float %add4.i293.i, %mul.i
  %add206.i = fadd float %add204.i, %mul5.i294.i
  %call.i85.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add206.i) nounwind readnone
  %add209.i = add i64 %dim_0_tid.i, 276480
  %arrayidx210.i = getelementptr inbounds float addrspace(1)* %4, i64 %add209.i
  store float %call.i85.i, float addrspace(1)* %arrayidx210.i, align 4
  %mul211.i = fmul float %div.i, 0x40B34BE2E0000000
  %sub212.i = fsub float 0x40249C5960000000, %mul211.i
  %mul213.i = fmul float %sub.i, 0x400049F4A0000000
  %add214.i = fadd float %sub212.i, %mul213.i
  %mul.i281.i = fmul float %mul.i, 0xBCFC4E7600000000
  %add.i282.i = fadd float %mul.i281.i, 0x3DE0DC9F20000000
  %mul1.i283.i = fmul float %add.i282.i, %mul.i
  %add2.i284.i = fadd float %mul1.i283.i, 0xBEB2C3C340000000
  %mul3.i285.i = fmul float %add2.i284.i, %mul.i
  %add4.i286.i = fadd float %mul3.i285.i, 0x3F7DFE6A60000000
  %mul5.i287.i = fmul float %add4.i286.i, %mul.i
  %add216.i = fadd float %add214.i, %mul5.i287.i
  %call.i86.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add216.i) nounwind readnone
  %add219.i = add i64 %dim_0_tid.i, 290304
  %arrayidx220.i = getelementptr inbounds float addrspace(1)* %4, i64 %add219.i
  store float %call.i86.i, float addrspace(1)* %arrayidx220.i, align 4
  %mul221.i = fmul float %div.i, 0x40C91CC280000000
  %sub222.i = fsub float 0x402AECC440000000, %mul221.i
  %mul223.i = fmul float %sub.i, 0x3FFF4645C0000000
  %add224.i = fadd float %sub222.i, %mul223.i
  %mul.i274.i = fmul float %mul.i, 0xBD00D92000000000
  %add.i275.i = fadd float %mul.i274.i, 0x3DE4116FE0000000
  %mul1.i276.i = fmul float %add.i275.i, %mul.i
  %add2.i277.i = fadd float %mul1.i276.i, 0xBEB651C940000000
  %mul3.i278.i = fmul float %add2.i277.i, %mul.i
  %add4.i279.i = fadd float %mul3.i278.i, 0x3F81D09720000000
  %mul5.i280.i = fmul float %add4.i279.i, %mul.i
  %add226.i = fadd float %add224.i, %mul5.i280.i
  %call.i87.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add226.i) nounwind readnone
  %add229.i = add i64 %dim_0_tid.i, 304128
  %arrayidx230.i = getelementptr inbounds float addrspace(1)* %4, i64 %add229.i
  store float %call.i87.i, float addrspace(1)* %arrayidx230.i, align 4
  %mul231.i = fmul float %div.i, 0x40C6513260000000
  %add232.i = fadd float %mul231.i, 0x402E3B3160000000
  %mul233.i = fmul float %sub.i, 0x3FF1266D40000000
  %add234.i = fadd float %add232.i, %mul233.i
  %mul.i267.i = fmul float %mul.i, 0xBD056475E0000000
  %add.i268.i = fadd float %mul.i267.i, 0x3DE95BDE60000000
  %mul1.i269.i = fmul float %add.i268.i, %mul.i
  %add2.i270.i = fadd float %mul1.i269.i, 0xBEBC089BE0000000
  %mul3.i271.i = fmul float %add2.i270.i, %mul.i
  %add4.i272.i = fadd float %mul3.i271.i, 0x3F8634A9C0000000
  %mul5.i273.i = fmul float %add4.i272.i, %mul.i
  %add236.i = fadd float %add234.i, %mul5.i273.i
  %call.i88.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add236.i) nounwind readnone
  %add239.i = add i64 %dim_0_tid.i, 317952
  %arrayidx240.i = getelementptr inbounds float addrspace(1)* %4, i64 %add239.i
  store float %call.i88.i, float addrspace(1)* %arrayidx240.i, align 4
  %mul241.i = fmul float %div.i, 0x40D2DFCDC0000000
  %sub242.i = fsub float 0xC00F712BE0000000, %mul241.i
  %mul243.i = fmul float %sub.i, 0x4016834860000000
  %add244.i = fadd float %sub242.i, %mul243.i
  %mul.i260.i = fmul float %mul.i, 0xBCD17B2440000000
  %add.i261.i = fadd float %mul.i260.i, 0x3DBA3A9900000000
  %mul1.i262.i = fmul float %add.i261.i, %mul.i
  %add2.i263.i = fadd float %mul1.i262.i, 0xBE91D28EA0000000
  %mul3.i264.i = fmul float %add2.i263.i, %mul.i
  %add4.i265.i = fadd float %mul3.i264.i, 0x3F60BBCA20000000
  %mul5.i266.i = fmul float %add4.i265.i, %mul.i
  %add246.i = fadd float %add244.i, %mul5.i266.i
  %call.i89.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add246.i) nounwind readnone
  %add249.i = add i64 %dim_0_tid.i, 331776
  %arrayidx250.i = getelementptr inbounds float addrspace(1)* %4, i64 %add249.i
  store float %call.i89.i, float addrspace(1)* %arrayidx250.i, align 4
  %mul251.i = fmul float %div.i, 0x40BD7F0DA0000000
  %add252.i = fadd float %mul251.i, 0x3FE43B5E80000000
  %mul253.i = fmul float %sub.i, 0x40120B9180000000
  %add254.i = fadd float %add252.i, %mul253.i
  %mul.i253.i = fmul float %mul.i, 0xBCF1E5EE20000000
  %add.i254.i = fadd float %mul.i253.i, 0x3DD5268EC0000000
  %mul1.i255.i = fmul float %add.i254.i, %mul.i
  %add2.i256.i = fadd float %mul1.i255.i, 0xBEA75123E0000000
  %mul3.i257.i = fmul float %add2.i256.i, %mul.i
  %add4.i258.i = fadd float %mul3.i257.i, 0x3F72707A60000000
  %mul5.i259.i = fmul float %add4.i258.i, %mul.i
  %add256.i = fadd float %add254.i, %mul5.i259.i
  %call.i90.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add256.i) nounwind readnone
  %add259.i = add i64 %dim_0_tid.i, 345600
  %arrayidx260.i = getelementptr inbounds float addrspace(1)* %4, i64 %add259.i
  store float %call.i90.i, float addrspace(1)* %arrayidx260.i, align 4
  %mul261.i = fmul float %div.i, 0x407EA52600000000
  %sub262.i = fsub float 0xC01420DBA0000000, %mul261.i
  %mul263.i = fmul float %sub.i, 0x4017E71600000000
  %add264.i = fadd float %sub262.i, %mul263.i
  %mul.i246.i = fmul float %mul.i, 0xBCD3998DC0000000
  %add.i247.i = fadd float %mul.i246.i, 0x3DC2A5B400000000
  %mul1.i248.i = fmul float %add.i247.i, %mul.i
  %add2.i249.i = fadd float %mul1.i248.i, 0xBE9EAFDA00000000
  %mul3.i250.i = fmul float %add2.i249.i, %mul.i
  %add4.i251.i = fadd float %mul3.i250.i, 0x3F70A6C580000000
  %mul5.i252.i = fmul float %add4.i251.i, %mul.i
  %add266.i = fadd float %add264.i, %mul5.i252.i
  %call.i91.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add266.i) nounwind readnone
  %add269.i = add i64 %dim_0_tid.i, 359424
  %arrayidx270.i = getelementptr inbounds float addrspace(1)* %4, i64 %add269.i
  store float %call.i91.i, float addrspace(1)* %arrayidx270.i, align 4
  %mul271.i = fmul float %div.i, 0x40D61047C0000000
  %add272.i = fadd float %mul271.i, 0xC00BD8A960000000
  %mul273.i = fmul float %sub.i, 0x40159DCF40000000
  %add274.i = fadd float %add272.i, %mul273.i
  %mul.i239.i = fmul float %mul.i, 0xBCE2753BA0000000
  %add.i240.i = fadd float %mul.i239.i, 0x3DCF52CE40000000
  %mul1.i241.i = fmul float %add.i240.i, %mul.i
  %add2.i242.i = fadd float %mul1.i241.i, 0xBEA7A2A060000000
  %mul3.i243.i = fmul float %add2.i242.i, %mul.i
  %add4.i244.i = fadd float %mul3.i243.i, 0x3F78024260000000
  %mul5.i245.i = fmul float %add4.i244.i, %mul.i
  %add276.i = fadd float %add274.i, %mul5.i245.i
  %call.i92.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add276.i) nounwind readnone
  %add279.i = add i64 %dim_0_tid.i, 373248
  %arrayidx280.i = getelementptr inbounds float addrspace(1)* %4, i64 %add279.i
  store float %call.i92.i, float addrspace(1)* %arrayidx280.i, align 4
  %mul281.i = fmul float %div.i, 0x40D1129CC0000000
  %sub282.i = fsub float 0xC0267C7100000000, %mul281.i
  %mul283.i = fmul float %sub.i, 0x401A00CE80000000
  %add284.i = fadd float %sub282.i, %mul283.i
  %mul.i232.i = fmul float %mul.i, 0xBCF4591FA0000000
  %add.i233.i = fadd float %mul.i232.i, 0x3DD961D9C0000000
  %mul1.i234.i = fmul float %add.i233.i, %mul.i
  %add2.i235.i = fadd float %mul1.i234.i, 0xBEAFC12CE0000000
  %mul3.i236.i = fmul float %add2.i235.i, %mul.i
  %add4.i237.i = fadd float %mul3.i236.i, 0x3F7D5648E0000000
  %mul5.i238.i = fmul float %add4.i237.i, %mul.i
  %add286.i = fadd float %add284.i, %mul5.i238.i
  %call.i93.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add286.i) nounwind readnone
  %add289.i = add i64 %dim_0_tid.i, 387072
  %arrayidx290.i = getelementptr inbounds float addrspace(1)* %4, i64 %add289.i
  store float %call.i93.i, float addrspace(1)* %arrayidx290.i, align 4
  %mul291.i = fmul float %div.i, 0x408CDC9000000000
  %add292.i = fadd float %mul291.i, 0xC02AA06F60000000
  %mul293.i = fmul float %sub.i, 0x401AEDD4C0000000
  %add294.i = fadd float %add292.i, %mul293.i
  %mul.i225.i = fmul float %mul.i, 0xBCE0F62340000000
  %add.i226.i = fadd float %mul.i225.i, 0x3DD0852CA0000000
  %mul1.i227.i = fmul float %add.i226.i, %mul.i
  %add2.i228.i = fadd float %mul1.i227.i, 0xBEABAE8D20000000
  %mul3.i229.i = fmul float %add2.i228.i, %mul.i
  %add4.i230.i = fadd float %mul3.i229.i, 0x3F7E884380000000
  %mul5.i231.i = fmul float %add4.i230.i, %mul.i
  %add296.i = fadd float %add294.i, %mul5.i231.i
  %call.i94.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add296.i) nounwind readnone
  %add299.i = add i64 %dim_0_tid.i, 400896
  %arrayidx300.i = getelementptr inbounds float addrspace(1)* %4, i64 %add299.i
  store float %call.i94.i, float addrspace(1)* %arrayidx300.i, align 4
  %mul301.i = fmul float %div.i, 0x40BF283940000000
  %sub302.i = fsub float 0xC02F07D500000000, %mul301.i
  %mul303.i = fmul float %sub.i, 0x401ED6C820000000
  %add304.i = fadd float %sub302.i, %mul303.i
  %mul.i218.i = fmul float %mul.i, 0xBCE1809100000000
  %add.i219.i = fadd float %mul.i218.i, 0x3DD16223E0000000
  %mul1.i220.i = fmul float %add.i219.i, %mul.i
  %add2.i221.i = fadd float %mul1.i220.i, 0xBEAD7BB920000000
  %mul3.i222.i = fmul float %add2.i221.i, %mul.i
  %add4.i223.i = fadd float %mul3.i222.i, 0x3F806A8EC0000000
  %mul5.i224.i = fmul float %add4.i223.i, %mul.i
  %add306.i = fadd float %add304.i, %mul5.i224.i
  %call.i95.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add306.i) nounwind readnone
  %add309.i = add i64 %dim_0_tid.i, 414720
  %arrayidx310.i = getelementptr inbounds float addrspace(1)* %4, i64 %add309.i
  store float %call.i95.i, float addrspace(1)* %arrayidx310.i, align 4
  br label %if.end.i

if.else.i:                                        ; preds = %scalar_kernel_entry.i
  %mul311.i = fmul float %div.i, 0x408CAF7B40000000
  %add312.i = fadd float %mul311.i, 0x3FE5DB3840000000
  %mul313.i = fmul float %sub.i, 0x4002C130A0000000
  %add314.i = fadd float %add312.i, %mul313.i
  %mul.i211.i = fmul float %mul.i, 0xBD59F3D0E0000000
  %add.i212.i = fadd float %mul.i211.i, 0x3E1CDBB200000000
  %mul1.i213.i = fmul float %add.i212.i, %mul.i
  %add2.i214.i = fadd float %mul1.i213.i, 0xBECB3B8080000000
  %mul3.i215.i = fmul float %add2.i214.i, %mul.i
  %add4.i216.i = fadd float %mul3.i215.i, 0x3F70581760000000
  %mul5.i217.i = fmul float %add4.i216.i, %mul.i
  %add316.i = fadd float %add314.i, %mul5.i217.i
  %call.i96.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add316.i) nounwind readnone
  %arrayidx320.i = getelementptr inbounds float addrspace(1)* %4, i64 %dim_0_tid.i
  store float %call.i96.i, float addrspace(1)* %arrayidx320.i, align 4
  %mul321.i = fmul float %div.i, 0x40D8E06A40000000
  %sub322.i = fsub float 0xBFDC9673A0000000, %mul321.i
  %mul323.i = fmul float %sub.i, 2.500000e+00
  %add324.i = fadd float %sub322.i, %mul323.i
  %mul.i204.i = fmul float %mul.i, 0xBB4C09FB40000000
  %add.i205.i = fadd float %mul.i204.i, 0x3C0C4B8820000000
  %mul1.i206.i = fmul float %add.i205.i, %mul.i
  %add2.i207.i = fadd float %mul1.i206.i, 0xBCB7F85EA0000000
  %mul3.i208.i = fmul float %add2.i207.i, %mul.i
  %add4.i209.i = fadd float %mul3.i208.i, 0x3D58D112C0000000
  %mul5.i210.i = fmul float %add4.i209.i, %mul.i
  %add326.i = fadd float %add324.i, %mul5.i210.i
  %call.i97.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add326.i) nounwind readnone
  %add329.i = add i64 %dim_0_tid.i, 13824
  %arrayidx330.i = getelementptr inbounds float addrspace(1)* %4, i64 %add329.i
  store float %call.i97.i, float addrspace(1)* %arrayidx330.i, align 4
  %mul331.i = fmul float %div.i, 0x40DC7090A0000000
  %sub332.i = fsub float 0x40006A5C20000000, %mul331.i
  %mul333.i = fmul float %sub.i, 0x4009589C60000000
  %add334.i = fadd float %sub332.i, %mul333.i
  %mul.i197.i = fmul float %mul.i, 0x3D3DBBA8A0000000
  %add.i198.i = fadd float %mul.i197.i, 0xBE018BEB80000000
  %mul1.i199.i = fmul float %add.i198.i, %mul.i
  %add2.i200.i = fadd float %mul1.i199.i, 0x3EB2934A60000000
  %mul3.i201.i = fmul float %add2.i200.i, %mul.i
  %add4.i202.i = fadd float %mul3.i201.i, 0xBF5ADD3AE0000000
  %mul5.i203.i = fmul float %add4.i202.i, %mul.i
  %add336.i = fadd float %add334.i, %mul5.i203.i
  %call.i98.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add336.i) nounwind readnone
  %add339.i = add i64 %dim_0_tid.i, 27648
  %arrayidx340.i = getelementptr inbounds float addrspace(1)* %4, i64 %add339.i
  store float %call.i98.i, float addrspace(1)* %arrayidx340.i, align 4
  %mul341.i = fmul float %div.i, 0x40909FC640000000
  %add342.i = fadd float %mul341.i, 0x400D42EB80000000
  %mul343.i = fmul float %sub.i, 0x400E427880000000
  %add344.i = fadd float %add342.i, %mul343.i
  %mul.i190.i = fmul float %mul.i, 0x3D46D361A0000000
  %add.i191.i = fadd float %mul.i190.i, 0xBE0BB876E0000000
  %mul1.i192.i = fmul float %add.i191.i, %mul.i
  %add2.i193.i = fadd float %mul1.i192.i, 0x3EBB88F920000000
  %mul3.i194.i = fmul float %add2.i193.i, %mul.i
  %add4.i195.i = fadd float %mul3.i194.i, 0xBF588C9B60000000
  %mul5.i196.i = fmul float %add4.i195.i, %mul.i
  %add346.i = fadd float %add344.i, %mul5.i196.i
  %call.i99.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add346.i) nounwind readnone
  %add349.i = add i64 %dim_0_tid.i, 41472
  %arrayidx350.i = getelementptr inbounds float addrspace(1)* %4, i64 %add349.i
  store float %call.i99.i, float addrspace(1)* %arrayidx350.i, align 4
  %mul351.i = fmul float %div.i, 0x40AC3E2940000000
  %sub352.i = fsub float 0xBFBA9ADBE0000000, %mul351.i
  %mul353.i = fmul float %sub.i, 0x400FEFA5C0000000
  %add354.i = fadd float %sub352.i, %mul353.i
  %mul.i183.i = fmul float %mul.i, 0x3D3332BDC0000000
  %add.i184.i = fadd float %mul.i183.i, 0xBDF639CD40000000
  %mul1.i185.i = fmul float %add.i184.i, %mul.i
  %add2.i186.i = fadd float %mul1.i185.i, 0x3EA9D34C60000000
  %mul3.i187.i = fmul float %add2.i186.i, %mul.i
  %add4.i188.i = fadd float %mul3.i187.i, 0xBF53ABED80000000
  %mul5.i189.i = fmul float %add4.i188.i, %mul.i
  %add356.i = fadd float %add354.i, %mul5.i189.i
  %call.i100.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add356.i) nounwind readnone
  %add359.i = add i64 %dim_0_tid.i, 55296
  %arrayidx360.i = getelementptr inbounds float addrspace(1)* %4, i64 %add359.i
  store float %call.i100.i, float addrspace(1)* %arrayidx360.i, align 4
  %mul361.i = fmul float %div.i, 0x40DD956E80000000
  %add362.i = fadd float %mul361.i, 0xBFEB2B45A0000000
  %mul363.i = fmul float %sub.i, 0x4010CB6860000000
  %add364.i = fadd float %add362.i, %mul363.i
  %mul.i176.i = fmul float %mul.i, 0x3D38F03960000000
  %add.i177.i = fadd float %mul.i176.i, 0xBDFF6D7340000000
  %mul1.i178.i = fmul float %add.i177.i, %mul.i
  %add2.i179.i = fadd float %mul1.i178.i, 0x3EB23B7C60000000
  %mul3.i180.i = fmul float %add2.i179.i, %mul.i
  %add4.i181.i = fadd float %mul3.i180.i, 0xBF50AEB640000000
  %mul5.i182.i = fmul float %add4.i181.i, %mul.i
  %add366.i = fadd float %add364.i, %mul5.i182.i
  %call.i101.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add366.i) nounwind readnone
  %add369.i = add i64 %dim_0_tid.i, 69120
  %arrayidx370.i = getelementptr inbounds float addrspace(1)* %4, i64 %add369.i
  store float %call.i101.i, float addrspace(1)* %arrayidx370.i, align 4
  %mul371.i = fmul float %div.i, 0x40726CEDC0000000
  %sub372.i = fsub float 0x400DBBB980000000, %mul371.i
  %mul373.i = fmul float %sub.i, 0x4011350A80000000
  %add374.i = fadd float %sub372.i, %mul373.i
  %mul.i169.i = fmul float %mul.i, 0x3D6058DBA0000000
  %add.i170.i = fadd float %mul.i169.i, 0xBE2160B200000000
  %mul1.i171.i = fmul float %add.i170.i, %mul.i
  %add2.i172.i = fadd float %mul1.i171.i, 0x3ECD94D8C0000000
  %mul3.i173.i = fmul float %add2.i172.i, %mul.i
  %add4.i174.i = fadd float %mul3.i173.i, 0xBF6373D060000000
  %mul5.i175.i = fmul float %add4.i174.i, %mul.i
  %add376.i = fadd float %add374.i, %mul5.i175.i
  %call.i102.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add376.i) nounwind readnone
  %add379.i = add i64 %dim_0_tid.i, 82944
  %arrayidx380.i = getelementptr inbounds float addrspace(1)* %4, i64 %add379.i
  store float %call.i102.i, float addrspace(1)* %arrayidx380.i, align 4
  %mul381.i = fmul float %div.i, 0x40D149A540000000
  %add382.i = fadd float %mul381.i, 0x400B7AFBE0000000
  %mul383.i = fmul float %sub.i, 0x40111ABD40000000
  %add384.i = fadd float %add382.i, %mul383.i
  %mul.i162.i = fmul float %mul.i, 0x3D5E584C60000000
  %add.i163.i = fadd float %mul.i162.i, 0xBE1EE41580000000
  %mul1.i164.i = fmul float %add.i163.i, %mul.i
  %add2.i165.i = fadd float %mul1.i164.i, 0x3EC7652DA0000000
  %mul3.i166.i = fmul float %add2.i165.i, %mul.i
  %add4.i167.i = fadd float %mul3.i166.i, 0xBF31C98640000000
  %mul5.i168.i = fmul float %add4.i167.i, %mul.i
  %add386.i = fadd float %add384.i, %mul5.i168.i
  %call.i103.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add386.i) nounwind readnone
  %add389.i = add i64 %dim_0_tid.i, 96768
  %arrayidx390.i = getelementptr inbounds float addrspace(1)* %4, i64 %add389.i
  store float %call.i103.i, float addrspace(1)* %arrayidx390.i, align 4
  %mul391.i = fmul float %div.i, 0x40F148D4C0000000
  %sub392.i = fsub float 0x4000AC0E00000000, %mul391.i
  %mul393.i = fmul float %sub.i, 0x400BEB2500000000
  %add394.i = fadd float %sub392.i, %mul393.i
  %mul.i155.i = fmul float %mul.i, 0xBD33C9F9C0000000
  %add.i156.i = fadd float %mul.i155.i, 0x3DF21BCB80000000
  %mul1.i157.i = fmul float %add.i156.i, %mul.i
  %add2.i158.i = fadd float %mul1.i157.i, 0xBE92E41B40000000
  %mul3.i159.i = fmul float %add2.i158.i, %mul.i
  %add4.i160.i = fadd float %mul3.i159.i, 0x3F25390F00000000
  %mul5.i161.i = fmul float %add4.i160.i, %mul.i
  %add396.i = fadd float %add394.i, %mul5.i161.i
  %call.i104.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add396.i) nounwind readnone
  %add399.i = add i64 %dim_0_tid.i, 110592
  %arrayidx400.i = getelementptr inbounds float addrspace(1)* %4, i64 %add399.i
  store float %call.i104.i, float addrspace(1)* %arrayidx400.i, align 4
  %mul401.i = fmul float %div.i, 0x40E6768140000000
  %sub402.i = fsub float 0x3FF9002160000000, %mul401.i
  %mul403.i = fmul float %sub.i, 0x400E19F740000000
  %add404.i = fadd float %sub402.i, %mul403.i
  %mul.i148.i = fmul float %mul.i, 0x3D37BF8FA0000000
  %add.i149.i = fadd float %mul.i148.i, 0xBDF60D7F00000000
  %mul1.i150.i = fmul float %add.i149.i, %mul.i
  %add2.i151.i = fadd float %mul1.i150.i, 0x3E9F42AA40000000
  %mul3.i152.i = fmul float %add2.i151.i, %mul.i
  %add4.i153.i = fadd float %mul3.i152.i, 0x3F3FBF7D20000000
  %mul5.i154.i = fmul float %add4.i153.i, %mul.i
  %add406.i = fadd float %add404.i, %mul5.i154.i
  %call.i105.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add406.i) nounwind readnone
  %add409.i = add i64 %dim_0_tid.i, 124416
  %arrayidx410.i = getelementptr inbounds float addrspace(1)* %4, i64 %add409.i
  store float %call.i105.i, float addrspace(1)* %arrayidx410.i, align 4
  %mul411.i = fmul float %div.i, 0x40E8A81A20000000
  %sub412.i = fsub float 0xBFE89C9F60000000, %mul411.i
  %mul413.i = fmul float %sub.i, 0x4010CB5EE0000000
  %add414.i = fadd float %sub412.i, %mul413.i
  %mul.i141.i = fmul float %mul.i, 0x3D3B58ED20000000
  %add.i142.i = fadd float %mul.i141.i, 0xBE03267920000000
  %mul1.i143.i = fmul float %add.i142.i, %mul.i
  %add2.i144.i = fadd float %mul1.i143.i, 0x3EB7056240000000
  %mul3.i145.i = fmul float %add2.i144.i, %mul.i
  %add4.i146.i = fadd float %mul3.i145.i, 0xBF53632660000000
  %mul5.i147.i = fmul float %add4.i146.i, %mul.i
  %add416.i = fadd float %add414.i, %mul5.i147.i
  %call.i106.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add416.i) nounwind readnone
  %add419.i = add i64 %dim_0_tid.i, 138240
  %arrayidx420.i = getelementptr inbounds float addrspace(1)* %4, i64 %add419.i
  store float %call.i106.i, float addrspace(1)* %arrayidx420.i, align 4
  %mul421.i = fmul float %div.i, 0x40D00F3FE0000000
  %sub422.i = fsub float 0x3FF9AC4BA0000000, %mul421.i
  %mul423.i = fmul float %sub.i, 0x400D638360000000
  %add424.i = fadd float %sub422.i, %mul423.i
  %mul.i134.i = fmul float %mul.i, 0x3D41E69B20000000
  %add.i135.i = fadd float %mul.i134.i, 0xBE03AC9FC0000000
  %mul1.i136.i = fmul float %add.i135.i, %mul.i
  %add2.i137.i = fadd float %mul1.i136.i, 0x3EB005D9A0000000
  %mul3.i138.i = fmul float %add2.i137.i, %mul.i
  %add4.i139.i = fadd float %mul3.i138.i, 0x3F50794580000000
  %mul5.i140.i = fmul float %add4.i139.i, %mul.i
  %add426.i = fadd float %add424.i, %mul5.i140.i
  %call.i107.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add426.i) nounwind readnone
  %add429.i = add i64 %dim_0_tid.i, 152064
  %arrayidx430.i = getelementptr inbounds float addrspace(1)* %4, i64 %add429.i
  store float %call.i107.i, float addrspace(1)* %arrayidx430.i, align 4
  %mul431.i = fmul float %div.i, 0x40C40352E0000000
  %add432.i = fadd float %mul431.i, 0xC01290B1E0000000
  %mul433.i = fmul float %sub.i, 0x4014997920000000
  %add434.i = fadd float %add432.i, %mul433.i
  %mul.i127.i = fmul float %mul.i, 0x3D6D533A80000000
  %add.i128.i = fadd float %mul.i127.i, 0xBE31598140000000
  %mul1.i129.i = fmul float %add.i128.i, %mul.i
  %add2.i130.i = fadd float %mul1.i129.i, 0x3EE1308EA0000000
  %mul3.i131.i = fmul float %add2.i130.i, %mul.i
  %add4.i132.i = fadd float %mul3.i131.i, 0xBF7BFF87C0000000
  %mul5.i133.i = fmul float %add4.i132.i, %mul.i
  %add436.i = fadd float %add434.i, %mul5.i133.i
  %call.i108.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add436.i) nounwind readnone
  %add439.i = add i64 %dim_0_tid.i, 165888
  %arrayidx440.i = getelementptr inbounds float addrspace(1)* %4, i64 %add439.i
  store float %call.i108.i, float addrspace(1)* %arrayidx440.i, align 4
  %mul441.i = fmul float %div.i, 0x40CC040B00000000
  %add442.i = fadd float %mul441.i, 0x400C1138E0000000
  %mul443.i = fmul float %sub.i, 0x400CA2E280000000
  %add444.i = fadd float %add442.i, %mul443.i
  %mul.i120.i = fmul float %mul.i, 0xBD297510C0000000
  %add.i121.i = fadd float %mul.i120.i, 0x3DD4C6BD20000000
  %mul1.i122.i = fmul float %add.i121.i, %mul.i
  %add2.i123.i = fadd float %mul1.i122.i, 0x3E86BEE9A0000000
  %mul3.i124.i = fmul float %add2.i123.i, %mul.i
  %add4.i125.i = fadd float %mul3.i124.i, 0xBF34000480000000
  %mul5.i126.i = fmul float %add4.i125.i, %mul.i
  %add446.i = fadd float %add444.i, %mul5.i126.i
  %call.i109.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add446.i) nounwind readnone
  %add449.i = add i64 %dim_0_tid.i, 179712
  %arrayidx450.i = getelementptr inbounds float addrspace(1)* %4, i64 %add449.i
  store float %call.i109.i, float addrspace(1)* %arrayidx450.i, align 4
  %mul451.i = fmul float %div.i, 0x40E79E7F00000000
  %add452.i = fadd float %mul451.i, 0x4023CD56C0000000
  %mul453.i = fmul float %sub.i, 0x4002DAAC20000000
  %add454.i = fadd float %add452.i, %mul453.i
  %mul.i113.i = fmul float %mul.i, 0xBD002DDB80000000
  %add.i114.i = fadd float %mul.i113.i, 0x3DEC2A6C00000000
  %mul1.i115.i = fmul float %add.i114.i, %mul.i
  %add2.i116.i = fadd float %mul1.i115.i, 0xBEB3EB3EA0000000
  %mul3.i117.i = fmul float %add2.i116.i, %mul.i
  %add4.i118.i = fadd float %mul3.i117.i, 0x3F72668420000000
  %mul5.i119.i = fmul float %add4.i118.i, %mul.i
  %add456.i = fadd float %add454.i, %mul5.i119.i
  %call.i110.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add456.i) nounwind readnone
  %add459.i = add i64 %dim_0_tid.i, 193536
  %arrayidx460.i = getelementptr inbounds float addrspace(1)* %4, i64 %add459.i
  store float %call.i110.i, float addrspace(1)* %arrayidx460.i, align 4
  %mul461.i = fmul float %div.i, 0x40ADFF2140000000
  %sub462.i = fsub float 0x400B27ACC0000000, %mul461.i
  %mul463.i = fmul float %sub.i, 0x4010E27E80000000
  %add464.i = fadd float %sub462.i, %mul463.i
  %mul.i106.i = fmul float %mul.i, 0x3D4E8615E0000000
  %add.i107.i = fadd float %mul.i106.i, 0xBE130FC860000000
  %mul1.i108.i = fmul float %add.i107.i, %mul.i
  %add2.i109.i = fadd float %mul1.i108.i, 0x3EC34408C0000000
  %mul3.i110.i = fmul float %add2.i109.i, %mul.i
  %add4.i111.i = fadd float %mul3.i110.i, 0xBF5A930120000000
  %mul5.i112.i = fmul float %add4.i111.i, %mul.i
  %add466.i = fadd float %add464.i, %mul5.i112.i
  %call.i111.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add466.i) nounwind readnone
  %add469.i = add i64 %dim_0_tid.i, 207360
  %arrayidx470.i = getelementptr inbounds float addrspace(1)* %4, i64 %add469.i
  store float %call.i111.i, float addrspace(1)* %arrayidx470.i, align 4
  %mul471.i = fmul float %div.i, 0x40CBF27A80000000
  %add472.i = fadd float %mul471.i, 0x3FE34A3E40000000
  %mul473.i = fmul float %sub.i, 0x40132CC5C0000000
  %add474.i = fadd float %add472.i, %mul473.i
  %mul.i99.i = fmul float %mul.i, 0x3D672E8340000000
  %add.i100.i = fadd float %mul.i99.i, 0xBE2B2679E0000000
  %mul1.i101.i = fmul float %add.i100.i, %mul.i
  %add2.i102.i = fadd float %mul1.i101.i, 0x3EDA170840000000
  %mul3.i103.i = fmul float %add2.i102.i, %mul.i
  %add4.i104.i = fadd float %mul3.i103.i, 0xBF744AD200000000
  %mul5.i105.i = fmul float %add4.i104.i, %mul.i
  %add476.i = fadd float %add474.i, %mul5.i105.i
  %call.i112.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add476.i) nounwind readnone
  %add479.i = add i64 %dim_0_tid.i, 221184
  %arrayidx480.i = getelementptr inbounds float addrspace(1)* %4, i64 %add479.i
  store float %call.i112.i, float addrspace(1)* %arrayidx480.i, align 4
  %mul481.i = fmul float %div.i, 0x408E94CF00000000
  %sub482.i = fsub float 0x402A4DEA20000000, %mul481.i
  %mul483.i = fmul float %sub.i, 0x4000D98180000000
  %add484.i = fadd float %sub482.i, %mul483.i
  %mul.i92.i = fmul float %mul.i, 0x3D3D362C60000000
  %add.i93.i = fadd float %mul.i92.i, 0xBE051FDD40000000
  %mul1.i94.i = fmul float %add.i93.i, %mul.i
  %add2.i95.i = fadd float %mul1.i94.i, 0x3EADDADAA0000000
  %mul3.i96.i = fmul float %add2.i95.i, %mul.i
  %add4.i97.i = fadd float %mul3.i96.i, 0x3F6D8F2600000000
  %mul5.i98.i = fmul float %add4.i97.i, %mul.i
  %add486.i = fadd float %add484.i, %mul5.i98.i
  %call.i113.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add486.i) nounwind readnone
  %add489.i = add i64 %dim_0_tid.i, 235008
  %arrayidx490.i = getelementptr inbounds float addrspace(1)* %4, i64 %add489.i
  store float %call.i113.i, float addrspace(1)* %arrayidx490.i, align 4
  %mul491.i = fmul float %div.i, 0x40D9CF3EC0000000
  %sub492.i = fsub float 0x402BE12100000000, %mul491.i
  %mul493.i = fmul float %sub.i, 0x3FE9E0B720000000
  %add494.i = fadd float %sub492.i, %mul493.i
  %mul.i85.i = fmul float %mul.i, 0xBD5DE8C6E0000000
  %add.i86.i = fadd float %mul.i85.i, 0x3E240DD900000000
  %mul1.i87.i = fmul float %add.i86.i, %mul.i
  %add2.i88.i = fadd float %mul1.i87.i, 0xBED8D40C20000000
  %mul3.i89.i = fmul float %add2.i88.i, %mul.i
  %add4.i90.i = fadd float %mul3.i89.i, 0x3F87EC1800000000
  %mul5.i91.i = fmul float %add4.i90.i, %mul.i
  %add496.i = fadd float %add494.i, %mul5.i91.i
  %call.i114.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add496.i) nounwind readnone
  %add499.i = add i64 %dim_0_tid.i, 248832
  %arrayidx500.i = getelementptr inbounds float addrspace(1)* %4, i64 %add499.i
  store float %call.i114.i, float addrspace(1)* %arrayidx500.i, align 4
  %mul501.i = fmul float %div.i, 0x40E7BDB960000000
  %sub502.i = fsub float 0x4017AE7B00000000, %mul501.i
  %mul503.i = fmul float %sub.i, 0x400A409C60000000
  %add504.i = fadd float %sub502.i, %mul503.i
  %mul.i78.i = fmul float %mul.i, 0x3D2BA34D60000000
  %add.i79.i = fadd float %mul.i78.i, 0xBDDBBA1D20000000
  %mul1.i80.i = fmul float %add.i79.i, %mul.i
  %add2.i81.i = fadd float %mul1.i80.i, 0xBE9AAE7FE0000000
  %mul3.i82.i = fmul float %add2.i81.i, %mul.i
  %add4.i83.i = fadd float %mul3.i82.i, 0x3F6C935E60000000
  %mul5.i84.i = fmul float %add4.i83.i, %mul.i
  %add506.i = fadd float %add504.i, %mul5.i84.i
  %call.i115.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add506.i) nounwind readnone
  %add509.i = add i64 %dim_0_tid.i, 262656
  %arrayidx510.i = getelementptr inbounds float addrspace(1)* %4, i64 %add509.i
  store float %call.i115.i, float addrspace(1)* %arrayidx510.i, align 4
  %mul511.i = fmul float %div.i, 0x40E1057B20000000
  %sub512.i = fsub float 0x4021056580000000, %mul511.i
  %mul513.i = fmul float %sub.i, 0x4009B321A0000000
  %add514.i = fadd float %sub512.i, %mul513.i
  %mul.i71.i = fmul float %mul.i, 0x3D69E31600000000
  %add.i72.i = fadd float %mul.i71.i, 0xBE299A2640000000
  %mul1.i73.i = fmul float %add.i72.i, %mul.i
  %add2.i74.i = fadd float %mul1.i73.i, 0x3ED21EBBA0000000
  %mul3.i75.i = fmul float %add2.i74.i, %mul.i
  %add4.i76.i = fadd float %mul3.i75.i, 0x3F48D17F20000000
  %mul5.i77.i = fmul float %add4.i76.i, %mul.i
  %add516.i = fadd float %add514.i, %mul5.i77.i
  %call.i116.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add516.i) nounwind readnone
  %add519.i = add i64 %dim_0_tid.i, 276480
  %arrayidx520.i = getelementptr inbounds float addrspace(1)* %4, i64 %add519.i
  store float %call.i116.i, float addrspace(1)* %arrayidx520.i, align 4
  %mul521.i = fmul float %div.i, 0x40B3E1C6A0000000
  %sub522.i = fsub float 0x401063AAC0000000, %mul521.i
  %mul523.i = fmul float %sub.i, 0x400FAC71E0000000
  %add524.i = fadd float %sub522.i, %mul523.i
  %mul.i64.i = fmul float %mul.i, 0x3D77BD4180000000
  %add.i65.i = fadd float %mul.i64.i, 0xBE38C0BFC0000000
  %mul1.i66.i = fmul float %add.i65.i, %mul.i
  %add2.i67.i = fadd float %mul1.i66.i, 0x3EE3F52280000000
  %mul3.i68.i = fmul float %add2.i67.i, %mul.i
  %add4.i69.i = fadd float %mul3.i68.i, 0xBF6F0244A0000000
  %mul5.i70.i = fmul float %add4.i69.i, %mul.i
  %add526.i = fadd float %add524.i, %mul5.i70.i
  %call.i117.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add526.i) nounwind readnone
  %add529.i = add i64 %dim_0_tid.i, 290304
  %arrayidx530.i = getelementptr inbounds float addrspace(1)* %4, i64 %add529.i
  store float %call.i117.i, float addrspace(1)* %arrayidx530.i, align 4
  %mul531.i = fmul float %div.i, 0x40C914D040000000
  %sub532.i = fsub float 0x4012D42EA0000000, %mul531.i
  %mul533.i = fmul float %sub.i, 0x401139D220000000
  %add534.i = fadd float %sub532.i, %mul533.i
  %mul.i57.i = fmul float %mul.i, 0x3D74469A00000000
  %add.i58.i = fadd float %mul.i57.i, 0xBE35718E40000000
  %mul1.i59.i = fmul float %add.i58.i, %mul.i
  %add2.i60.i = fadd float %mul1.i59.i, 0x3EE1605BC0000000
  %mul3.i61.i = fmul float %add2.i60.i, %mul.i
  %add4.i62.i = fadd float %mul3.i61.i, 0xBF6125F4E0000000
  %mul5.i63.i = fmul float %add4.i62.i, %mul.i
  %add536.i = fadd float %add534.i, %mul5.i63.i
  %call.i118.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add536.i) nounwind readnone
  %add539.i = add i64 %dim_0_tid.i, 304128
  %arrayidx540.i = getelementptr inbounds float addrspace(1)* %4, i64 %add539.i
  store float %call.i118.i, float addrspace(1)* %arrayidx540.i, align 4
  %mul541.i = fmul float %div.i, 0x40C6811A40000000
  %add542.i = fadd float %mul541.i, 0x400555A760000000
  %mul543.i = fmul float %sub.i, 0x40112A6B40000000
  %add544.i = fadd float %add542.i, %mul543.i
  %mul.i50.i = fmul float %mul.i, 0x3D77A24400000000
  %add.i51.i = fadd float %mul.i50.i, 0xBE395B6420000000
  %mul1.i52.i = fmul float %add.i51.i, %mul.i
  %add2.i53.i = fadd float %mul1.i52.i, 0x3EE4F3AEE0000000
  %mul3.i54.i = fmul float %add2.i53.i, %mul.i
  %add4.i55.i = fadd float %mul3.i54.i, 0xBF6688C920000000
  %mul5.i56.i = fmul float %add4.i55.i, %mul.i
  %add546.i = fadd float %add544.i, %mul5.i56.i
  %call.i119.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add546.i) nounwind readnone
  %add549.i = add i64 %dim_0_tid.i, 317952
  %arrayidx550.i = getelementptr inbounds float addrspace(1)* %4, i64 %add549.i
  store float %call.i119.i, float addrspace(1)* %arrayidx550.i, align 4
  %mul551.i = fmul float %div.i, 0x40D396DCC0000000
  %sub552.i = fsub float 0x4028FB17E0000000, %mul551.i
  %mul553.i = fmul float %sub.i, 0x4002038680000000
  %add554.i = fadd float %sub552.i, %mul553.i
  %mul.i43.i = fmul float %mul.i, 0xBD51D37B00000000
  %add.i44.i = fadd float %mul.i43.i, 0x3E18BBA200000000
  %mul1.i45.i = fmul float %add.i44.i, %mul.i
  %add2.i46.i = fadd float %mul1.i45.i, 0xBED0967CE0000000
  %mul3.i47.i = fmul float %add2.i46.i, %mul.i
  %add4.i48.i = fadd float %mul3.i47.i, 0x3F82142860000000
  %mul5.i49.i = fmul float %add4.i48.i, %mul.i
  %add556.i = fadd float %add554.i, %mul5.i49.i
  %call.i120.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add556.i) nounwind readnone
  %add559.i = add i64 %dim_0_tid.i, 331776
  %arrayidx560.i = getelementptr inbounds float addrspace(1)* %4, i64 %add559.i
  store float %call.i120.i, float addrspace(1)* %arrayidx560.i, align 4
  %mul561.i = fmul float %div.i, 0x40BB82EB00000000
  %add562.i = fadd float %mul561.i, 0x40286E6960000000
  %mul563.i = fmul float %sub.i, 0x4001163160000000
  %add564.i = fadd float %add562.i, %mul563.i
  %mul.i36.i = fmul float %mul.i, 0xBD3C5A4680000000
  %add.i37.i = fadd float %mul.i36.i, 0x3E0AC134E0000000
  %mul1.i38.i = fmul float %add.i37.i, %mul.i
  %add2.i39.i = fadd float %mul1.i38.i, 0xBEC851D2A0000000
  %mul3.i40.i = fmul float %add2.i39.i, %mul.i
  %add4.i41.i = fadd float %mul3.i40.i, 0x3F828DC0E0000000
  %mul5.i42.i = fmul float %add4.i41.i, %mul.i
  %add566.i = fadd float %add564.i, %mul5.i42.i
  %call.i121.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add566.i) nounwind readnone
  %add569.i = add i64 %dim_0_tid.i, 345600
  %arrayidx570.i = getelementptr inbounds float addrspace(1)* %4, i64 %add569.i
  store float %call.i121.i, float addrspace(1)* %arrayidx570.i, align 4
  %mul571.i = fmul float %div.i, 0x4097C5E800000000
  %sub572.i = fsub float 0x4023249580000000, %mul571.i
  %mul573.i = fmul float %sub.i, 0x400B45C280000000
  %add574.i = fadd float %sub572.i, %mul573.i
  %mul.i29.i = fmul float %mul.i, 0x3D442D6C00000000
  %add.i30.i = fadd float %mul.i29.i, 0x3E047F4C00000000
  %mul1.i31.i = fmul float %add.i30.i, %mul.i
  %add2.i32.i = fadd float %mul1.i31.i, 0x3E9527EEA0000000
  %mul3.i33.i = fmul float %add2.i32.i, %mul.i
  %add4.i34.i = fadd float %mul3.i33.i, 0x3F75FE1B00000000
  %mul5.i35.i = fmul float %add4.i34.i, %mul.i
  %add576.i = fadd float %add574.i, %mul5.i35.i
  %call.i122.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add576.i) nounwind readnone
  %add579.i = add i64 %dim_0_tid.i, 359424
  %arrayidx580.i = getelementptr inbounds float addrspace(1)* %4, i64 %add579.i
  store float %call.i122.i, float addrspace(1)* %arrayidx580.i, align 4
  %mul581.i = fmul float %div.i, 0x40D5113840000000
  %add582.i = fadd float %mul581.i, 0x4010697D00000000
  %mul583.i = fmul float %sub.i, 0x4012EAF760000000
  %add584.i = fadd float %add582.i, %mul583.i
  %mul.i22.i = fmul float %mul.i, 0x3D734A7280000000
  %add.i23.i = fadd float %mul.i22.i, 0xBE3490B360000000
  %mul1.i24.i = fmul float %add.i23.i, %mul.i
  %add2.i25.i = fadd float %mul1.i24.i, 0x3EE09D5A40000000
  %mul3.i26.i = fmul float %add2.i25.i, %mul.i
  %add4.i27.i = fadd float %mul3.i26.i, 0xBF5A28CE40000000
  %mul5.i28.i = fmul float %add4.i27.i, %mul.i
  %add586.i = fadd float %add584.i, %mul5.i28.i
  %call.i123.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add586.i) nounwind readnone
  %add589.i = add i64 %dim_0_tid.i, 373248
  %arrayidx590.i = getelementptr inbounds float addrspace(1)* %4, i64 %add589.i
  store float %call.i123.i, float addrspace(1)* %arrayidx590.i, align 4
  %mul591.i = fmul float %div.i, 0x40D2CB6840000000
  %sub592.i = fsub float 0x40312C57C0000000, %mul591.i
  %mul593.i = fmul float %sub.i, 0x3FF5CF9980000000
  %add594.i = fadd float %sub592.i, %mul593.i
  %mul.i15.i = fmul float %mul.i, 0x3D6BE0A940000000
  %add.i16.i = fadd float %mul.i15.i, 0xBE27E07860000000
  %mul1.i17.i = fmul float %add.i16.i, %mul.i
  %add2.i18.i = fadd float %mul1.i17.i, 0x3EC178DF40000000
  %mul3.i19.i = fmul float %add2.i18.i, %mul.i
  %add4.i20.i = fadd float %mul3.i19.i, 0x3F844A1300000000
  %mul5.i21.i = fmul float %add4.i20.i, %mul.i
  %add596.i = fadd float %add594.i, %mul5.i21.i
  %call.i124.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add596.i) nounwind readnone
  %add599.i = add i64 %dim_0_tid.i, 387072
  %arrayidx600.i = getelementptr inbounds float addrspace(1)* %4, i64 %add599.i
  store float %call.i124.i, float addrspace(1)* %arrayidx600.i, align 4
  %mul601.i = fmul float %div.i, 0x4090CB4DE0000000
  %sub602.i = fsub float 0x4030253500000000, %mul601.i
  %mul603.i = fmul float %sub.i, 0x3FF7E495E0000000
  %add604.i = fadd float %sub602.i, %mul603.i
  %mul.i8.i = fmul float %mul.i, 0x3D592F7C20000000
  %add.i9.i = fadd float %mul.i8.i, 0xBE17E4A080000000
  %mul1.i10.i = fmul float %add.i9.i, %mul.i
  %add2.i11.i = fadd float %mul1.i10.i, 0x3EA9178B60000000
  %mul3.i12.i = fmul float %add2.i11.i, %mul.i
  %add4.i13.i = fadd float %mul3.i12.i, 0x3F856D6900000000
  %mul5.i14.i = fmul float %add4.i13.i, %mul.i
  %add606.i = fadd float %add604.i, %mul5.i14.i
  %call.i125.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add606.i) nounwind readnone
  %add609.i = add i64 %dim_0_tid.i, 400896
  %arrayidx610.i = getelementptr inbounds float addrspace(1)* %4, i64 %add609.i
  store float %call.i125.i, float addrspace(1)* %arrayidx610.i, align 4
  %mul611.i = fmul float %div.i, 0x40C4242C40000000
  %sub612.i = fsub float 0x403522D320000000, %mul611.i
  %mul613.i = fmul float %sub.i, 0x3FF0C92F40000000
  %add614.i = fadd float %sub612.i, %mul613.i
  %mul.i1.i = fmul float %mul.i, 0x3D607CC860000000
  %add.i2.i = fadd float %mul.i1.i, 0xBE1C0DB120000000
  %mul1.i3.i = fmul float %add.i2.i, %mul.i
  %add2.i4.i = fadd float %mul1.i3.i, 0x3E9A54F4A0000000
  %mul3.i5.i = fmul float %add2.i4.i, %mul.i
  %add4.i6.i = fadd float %mul3.i5.i, 0x3F8AA218A0000000
  %mul5.i7.i = fmul float %add4.i6.i, %mul.i
  %add616.i = fadd float %add614.i, %mul5.i7.i
  %call.i.i = call x86_svmlcc float @__ocl_svml_b1_expf1(float %add616.i) nounwind readnone
  %add619.i = add i64 %dim_0_tid.i, 414720
  %arrayidx620.i = getelementptr inbounds float addrspace(1)* %4, i64 %add619.i
  store float %call.i.i, float addrspace(1)* %arrayidx620.i, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.else.i, %if.then.i
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %if.end.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %19
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %21
  br i1 %dim_2_cmp.to.max.i, label %__rdsmh_kernel_separated_args.exit, label %dim_1_pre_head.i

__rdsmh_kernel_separated_args.exit:               ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__rdsmh_kernel_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"rdsmh_kernel"}
!4 = metadata !{void (i8*)* @rdsmh_kernel}
