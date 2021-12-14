; CMPLRLLVM-28784
; This is the core loop from mandelbrot-dpd, after the vectorizer runs.
; This code is very sensitive to performance, so we reproduce it entirely in
; this test case.

; The two linked blocks new.loop.latch10 and VPlannedBB22 have 4 vector
; selects total.
; 2 of these selects are "select .... <1,1,1,1..." and
; "select .... <true,true,...>" patterns which can be replaced with boolean.
; instcombine must first freeze the operand of these selects (same operand).
; The select => or/and transform only works if the operand is not poison.
; So we want to see:
;
; %BROADCASTXX = freeze ...
; %CMP = icmp %BROADCASTXX...
;  ...
;  = shufflevector %CMP ....
; and %CMP should not be used in any selects.

; RUN: opt -passes="instcombine" -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi2EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi2EEE.cl::sycl::detail::array" = type { [2 x i64] }

; Function Attrs: nofree nosync nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

; Function Attrs: nounwind
define void @_ZGVeN16uuuuu__ZTSN2cl4sycl6detail19__pf_kernel_wrapperI7manSYCLEE(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range") %_arg_, i32 addrspace(1)* noalias %_arg_1, %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range") %_arg_2, %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range") %_arg_3, %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* byval(%"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range") %_arg_4) local_unnamed_addr #1 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !no_barrier_path !12 !vectorized_kernel !13 !kernel_has_sub_groups !14 !scalarized_kernel !15 !vectorized_width !16 !ocl_recommended_vector_length !16 !vectorization_dimension !17 !can_unite_workgroups !14 {

; CHECK:       new.loop.latch10:
; CHECK-NEXT:    br label %VPlannedBB22
; CHECK:       VPlannedBB22:
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT16:%.*]] = insertelement <16 x i32> poison, i32 [[TMP21:%.*]], i64 0
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT16_FR:%.*]] = freeze <16 x i32> [[BROADCAST_SPLATINSERT16]]
; CHECK-NEXT:    [[TMP22:%.*]] = icmp ne <16 x i32> [[BROADCAST_SPLATINSERT16_FR]], <i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000>
; CHECK-NEXT:    [[PREDBLEND20:%.*]] = select <16 x i1> [[TMP15:%.*]], <16 x i32> <i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000>, <16 x i32> [[BROADCAST_SPLAT19:%.*]]
; CHECK-NEXT:    [[TMP23:%.*]] = select <16 x i1> [[VEC_PHI6:%.*]], <16 x i32> [[PREDBLEND20]], <16 x i32> [[VEC_PHI7:%.*]]
; CHECK-NEXT:    [[DOTNOT:%.*]] = shufflevector <16 x i1> [[TMP22]], <16 x i1> poison, <16 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP24:%.*]] = and <16 x i1> [[TMP15]], [[DOTNOT]]
; CHECK-NEXT:    [[TMP25:%.*]] = bitcast <16 x i1> [[TMP24]] to i16
; CHECK-NEXT:    [[TMP26:%.*]] = icmp eq i16 [[TMP25]], 0
; CHECK-NEXT:    br i1 [[TMP26]], label [[VPLANNEDBB25:%.*]], label [[VPLANNEDBB3:%.*]]

entry:
  %alloca._arg_1 = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %_arg_1, i32 addrspace(1)** %alloca._arg_1, align 8
  %0 = call i64 @_Z13get_global_idj(i32 2) #2
  %1 = call i64 @_Z13get_global_idj(i32 1) #2
  %2 = call i64 @_Z13get_global_idj(i32 0) #2
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load._arg_1 = load i32 addrspace(1)*, i32 addrspace(1)** %alloca._arg_1, align 8
  %agg.tmp5.sroa.2.0..sroa_idx4 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* %_arg_3, i64 0, i32 0, i32 0, i64 1
  %agg.tmp5.sroa.2.0.copyload = load i64, i64* %agg.tmp5.sroa.2.0..sroa_idx4, align 8
  %agg.tmp6.sroa.0.sroa.0.0..sroa_idx = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* %_arg_4, i64 0, i32 0, i32 0, i64 0
  %agg.tmp6.sroa.0.sroa.0.0.copyload = load i64, i64* %agg.tmp6.sroa.0.sroa.0.0..sroa_idx, align 8
  %agg.tmp6.sroa.0.sroa.2.0..sroa_idx31 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"* %_arg_4, i64 0, i32 0, i32 0, i64 1
  %agg.tmp6.sroa.0.sroa.2.0.copyload = load i64, i64* %agg.tmp6.sroa.0.sroa.2.0..sroa_idx31, align 8
  %conv.i.i = trunc i64 %1 to i32
  %conv.i7.i.i = sitofp i32 %conv.i.i to float
  %mul.i8.i.i = fmul fast float %conv.i7.i.i, 0x3F35D867C0000000
  %add.i9.i.i = fadd fast float %mul.i8.i.i, -1.500000e+00
  %add6.i.i.i.i = add i64 %agg.tmp6.sroa.0.sroa.0.0.copyload, %1
  %mul.1.i.i.i.i = mul i64 %agg.tmp5.sroa.2.0.copyload, %add6.i.i.i.i
  %add.1.i.i.i.i = add i64 %agg.tmp6.sroa.0.sroa.2.0.copyload, %mul.1.i.i.i.i
  br label %VPlannedBB

VPlannedBB:                                       ; preds = %simd.loop.preheader
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %VPlannedBB
  br i1 false, label %scalar.ph, label %vector.ph

vector.ph:                                        ; preds = %VPlannedBB1
  %broadcast.splatinsert = insertelement <16 x i64> poison, i64 %2, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert13 = insertelement <16 x float> poison, float %add.i9.i.i, i32 0
  %broadcast.splat14 = shufflevector <16 x float> %broadcast.splatinsert13, <16 x float> poison, <16 x i32> zeroinitializer
  %broadcast.splatinsert27 = insertelement <16 x i64> poison, i64 %add.1.i.i.i.i, i32 0
  %broadcast.splat28 = shufflevector <16 x i64> %broadcast.splatinsert27, <16 x i64> poison, <16 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %VPlannedBB29, %vector.ph
  %uni.phi = phi i32 [ 0, %vector.ph ], [ %31, %VPlannedBB29 ]
  %vec.phi = phi <16 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>, %vector.ph ], [ %30, %VPlannedBB29 ]
  %3 = sext <16 x i32> %vec.phi to <16 x i64>
  %4 = add nuw <16 x i64> %3, %broadcast.splat
  %5 = trunc <16 x i64> %4 to <16 x i32>
  %6 = sitofp <16 x i32> %5 to <16 x float>
  %7 = fmul fast <16 x float> %6, <float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000, float 0x3F35D867C0000000>
  %8 = fadd fast <16 x float> %7, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %VPlannedBB22, %vector.body
  %vec.phi4 = phi <32 x float> [ zeroinitializer, %vector.body ], [ %wide.insert15, %VPlannedBB22 ]
  %uni.phi5 = phi i32 [ 0, %vector.body ], [ %21, %VPlannedBB22 ]
  %vec.phi6 = phi <16 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %vector.body ], [ %25, %VPlannedBB22 ]
  %vec.phi7 = phi <16 x i32> [ undef, %vector.body ], [ %23, %VPlannedBB22 ]
  %broadcast.splatinsert18 = insertelement <16 x i32> poison, i32 %uni.phi5, i32 0
  %broadcast.splat19 = shufflevector <16 x i32> %broadcast.splatinsert18, <16 x i32> poison, <16 x i32> zeroinitializer
  br label %VPlannedBB8

VPlannedBB8:                                      ; preds = %VPlannedBB3
  %wide.extract = shufflevector <32 x float> %vec.phi4, <32 x float> undef, <16 x i32> <i32 0, i32 2, i32 4, i32 6, i32 8, i32 10, i32 12, i32 14, i32 16, i32 18, i32 20, i32 22, i32 24, i32 26, i32 28, i32 30>
  %wide.extract9 = shufflevector <32 x float> %vec.phi4, <32 x float> undef, <16 x i32> <i32 1, i32 3, i32 5, i32 7, i32 9, i32 11, i32 13, i32 15, i32 17, i32 19, i32 21, i32 23, i32 25, i32 27, i32 29, i32 31>
  %9 = fmul fast <16 x float> %wide.extract, %wide.extract
  %10 = fmul fast <16 x float> %wide.extract9, %wide.extract9
  %11 = fadd fast <16 x float> %10, %9
  %12 = fcmp ult <16 x float> %11, <float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00>
  %13 = xor <16 x i1> %12, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %VPlannedBB10

VPlannedBB10:                                     ; preds = %VPlannedBB8
  %14 = and <16 x i1> %vec.phi6, %13
  %15 = and <16 x i1> %vec.phi6, %12
  br label %VPlannedBB11

VPlannedBB11:                                     ; preds = %VPlannedBB10
  br label %VPlannedBB12

VPlannedBB12:                                     ; preds = %VPlannedBB11
  %16 = fsub fast <16 x float> %9, %10
  %17 = fmul fast <16 x float> %wide.extract, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %18 = fmul fast <16 x float> %17, %wide.extract9
  %19 = fadd fast <16 x float> %16, %broadcast.splat14
  %20 = fadd fast <16 x float> %18, %8
  %wide.insert = shufflevector <16 x float> %19, <16 x float> undef, <32 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef, i32 8, i32 undef, i32 9, i32 undef, i32 10, i32 undef, i32 11, i32 undef, i32 12, i32 undef, i32 13, i32 undef, i32 14, i32 undef, i32 15, i32 undef>
  %extended. = shufflevector <16 x float> %20, <16 x float> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %wide.insert15 = shufflevector <32 x float> %wide.insert, <32 x float> %extended., <32 x i32> <i32 0, i32 32, i32 2, i32 33, i32 4, i32 34, i32 6, i32 35, i32 8, i32 36, i32 10, i32 37, i32 12, i32 38, i32 14, i32 39, i32 16, i32 40, i32 18, i32 41, i32 20, i32 42, i32 22, i32 43, i32 24, i32 44, i32 26, i32 45, i32 28, i32 46, i32 30, i32 47>
  %21 = add i32 %uni.phi5, 1
  %broadcast.splatinsert16 = insertelement <16 x i32> poison, i32 %21, i32 0
  %broadcast.splat17 = shufflevector <16 x i32> %broadcast.splatinsert16, <16 x i32> poison, <16 x i32> zeroinitializer
  %22 = icmp eq <16 x i32> %broadcast.splat17, <i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000>
  br label %new.loop.latch10

new.loop.latch10:                                 ; preds = %VPlannedBB12
  %predblend = select <16 x i1> %15, <16 x i32> zeroinitializer, <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %predblend20 = select <16 x i1> %15, <16 x i32> <i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000, i32 10000>, <16 x i32> %broadcast.splat19
  %predblend21 = select <16 x i1> %15, <16 x i1> %22, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %VPlannedBB22

VPlannedBB22:                                     ; preds = %new.loop.latch10
  %23 = select <16 x i1> %vec.phi6, <16 x i32> %predblend20, <16 x i32> %vec.phi7
  %24 = xor <16 x i1> %predblend21, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %25 = and <16 x i1> %24, %vec.phi6
  %26 = bitcast <16 x i1> %25 to i16
  %27 = icmp eq i16 %26, 0
  %broadcast.splatinsert23 = insertelement <16 x i1> poison, i1 %27, i32 0
  %broadcast.splat24 = shufflevector <16 x i1> %broadcast.splatinsert23, <16 x i1> poison, <16 x i32> zeroinitializer
  %broadcast.splat24.extract.0. = extractelement <16 x i1> %broadcast.splat24, i32 0
  br i1 %broadcast.splat24.extract.0., label %VPlannedBB25, label %VPlannedBB3

VPlannedBB25:                                     ; preds = %VPlannedBB22
  %vec.phi26 = phi <16 x i32> [ %23, %VPlannedBB22 ]
  %28 = add <16 x i64> %broadcast.splat28, %4
  %.extract.0. = extractelement <16 x i64> %28, i32 0
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %load._arg_1, i64 %.extract.0.
  %29 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*
  store <16 x i32> %vec.phi26, <16 x i32> addrspace(1)* %29, align 4
  br label %VPlannedBB29

VPlannedBB29:                                     ; preds = %VPlannedBB25
  %30 = add nuw <16 x i32> %vec.phi, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %31 = add nuw i32 %uni.phi, 16
  %32 = icmp ult i32 %31, 16
  br i1 false, label %vector.body, label %VPlannedBB30, !llvm.loop !18

VPlannedBB30:                                     ; preds = %VPlannedBB29
  br label %middle.block

middle.block:                                     ; preds = %VPlannedBB30
  br i1 false, label %scalar.ph, label %VPlannedBB31

scalar.ph:                                        ; preds = %middle.block, %VPlannedBB1
  %uni.phi32 = phi i32 [ 16, %middle.block ], [ 0, %VPlannedBB1 ]
  br label %VPlannedBB33

VPlannedBB33:                                     ; preds = %scalar.ph
  br label %simd.loop

VPlannedBB31:                                     ; preds = %simd.loop.exit, %middle.block
  %uni.phi34 = phi i32 [ %indvar, %simd.loop.exit ], [ 16, %middle.block ]
  br label %VPlannedBB35

VPlannedBB35:                                     ; preds = %VPlannedBB31
  br label %simd.end.region

simd.loop:                                        ; preds = %simd.loop.exit, %VPlannedBB33
  %index = phi i32 [ %uni.phi32, %VPlannedBB33 ], [ %indvar, %simd.loop.exit ]
  %33 = sext i32 %index to i64
  %add = add nuw i64 %33, %2
  %conv3.i.i = trunc i64 %add to i32
  %conv.i.i.i = sitofp i32 %conv3.i.i to float
  %mul.i5.i.i = fmul fast float %conv.i.i.i, 0x3F35D867C0000000
  %add.i6.i.i = fadd fast float %mul.i5.i.i, -1.000000e+00
  br label %for.body.i.i.i

for.body.i.i.i:                                   ; preds = %if.end.i.i.i, %simd.loop
  %z.0.i.i.i = phi <2 x float> [ zeroinitializer, %simd.loop ], [ %z.4.vec.insert30.i.i.i, %if.end.i.i.i ]
  %count.014.i.i.i = phi i32 [ 0, %simd.loop ], [ %inc.i.i.i, %if.end.i.i.i ]
  %z.0.vec.extract.i.i.i = extractelement <2 x float> %z.0.i.i.i, i32 0
  %z.4.vec.extract.i.i.i = extractelement <2 x float> %z.0.i.i.i, i32 1
  %mul.i.i.i = fmul fast float %z.0.vec.extract.i.i.i, %z.0.vec.extract.i.i.i
  %mul2.i.i.i = fmul fast float %z.4.vec.extract.i.i.i, %z.4.vec.extract.i.i.i
  %add.i.i.i = fadd fast float %mul2.i.i.i, %mul.i.i.i
  %cmp3.i.i.i = fcmp ult float %add.i.i.i, 4.000000e+00
  br i1 %cmp3.i.i.i, label %if.end.i.i.i, label %_ZZZN10MandelSYCLILi6000ELi6000ELi10000EE8EvaluateEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi2EEEE_clES7_.exit.i

if.end.i.i.i:                                     ; preds = %for.body.i.i.i
  %sub.i.i.i.i = fsub fast float %mul.i.i.i, %mul2.i.i.i
  %mul7.i.i.i.i = fmul fast float %z.0.vec.extract.i.i.i, 2.000000e+00
  %mul8.i.i.i.i = fmul fast float %mul7.i.i.i.i, %z.4.vec.extract.i.i.i
  %add.r.i.i.i.i.i = fadd fast float %sub.i.i.i.i, %add.i9.i.i
  %add.i.i.i.i.i.i = fadd fast float %mul8.i.i.i.i, %add.i6.i.i
  %z.0.vec.insert26.i.i.i = insertelement <2 x float> poison, float %add.r.i.i.i.i.i, i32 0
  %z.4.vec.insert30.i.i.i = insertelement <2 x float> %z.0.vec.insert26.i.i.i, float %add.i.i.i.i.i.i, i32 1
  %inc.i.i.i = add nuw nsw i32 %count.014.i.i.i, 1
  %exitcond.not.i.i.i = icmp eq i32 %inc.i.i.i, 10000
  br i1 %exitcond.not.i.i.i, label %_ZZZN10MandelSYCLILi6000ELi6000ELi10000EE8EvaluateEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi2EEEE_clES7_.exit.i, label %for.body.i.i.i

_ZZZN10MandelSYCLILi6000ELi6000ELi10000EE8EvaluateEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi2EEEE_clES7_.exit.i: ; preds = %if.end.i.i.i, %for.body.i.i.i
  %count.0.lcssa.i.i.i = phi i32 [ %count.014.i.i.i, %for.body.i.i.i ], [ 10000, %if.end.i.i.i ]
  %add6.1.i.i.i.i = add i64 %add.1.i.i.i.i, %add
  %arrayidx.i4.i.i = getelementptr inbounds i32, i32 addrspace(1)* %load._arg_1, i64 %add6.1.i.i.i.i
  store i32 %count.0.lcssa.i.i.i, i32 addrspace(1)* %arrayidx.i4.i.i, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %_ZZZN10MandelSYCLILi6000ELi6000ELi10000EE8EvaluateEvENKUlRN2cl4sycl7handlerEE_clES4_ENKUlNS2_2idILi2EEEE_clES7_.exit.i
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 16
  br i1 %vl.cond, label %simd.loop, label %VPlannedBB31, !llvm.loop !20

simd.end.region:                                  ; preds = %VPlannedBB35
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

attributes #0 = { nofree nosync nounwind readnone "prefer-vector-width"="512" }
attributes #1 = { nounwind "prefer-vector-width"="512" "vector-variants"="_ZGVeN16uuuuu__ZTSN2cl4sycl6detail19__pf_kernel_wrapperI7manSYCLEE" }
attributes #2 = { nounwind readnone }

!spirv.MemoryModel = !{!0}
!spirv.Source = !{!1}
!opencl.spir.version = !{!2}
!opencl.ocl.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!5}
!spirv.Generator = !{!6}
!opencl.kernels = !{!7}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 100000}
!2 = !{i32 1, i32 2}
!3 = !{i32 1, i32 0}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{i16 6, i16 14}
!7 = distinct !{null, null}
!8 = !{i32 0, i32 1, i32 0, i32 0, i32 0}
!9 = !{!"none", !"none", !"none", !"none", !"none"}
!10 = !{!"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"int*", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi2EEE.cl::sycl::range"}
!11 = !{!"", !"", !"", !"", !""}
!12 = !{i1 true}
!13 = !{null}
!14 = !{i1 false}
!15 = distinct !{null}
!16 = !{i32 16}
!17 = !{i32 0}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.isvectorized", i32 1}
!20 = distinct !{!20, !21, !22, !19}
!21 = !{!"llvm.loop.unroll.disable"}
!22 = !{!"llvm.loop.vectorize.enable", i32 1}
