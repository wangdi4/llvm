; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone,vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"
%opencl.image2d_ro_t.4 = type opaque
%opencl.image2d_wo_t.5 = type opaque
; CHECK: declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #[[SCALAR_ALL_ATTR:.*]]

; Function Attrs: convergent nounwind
define spir_kernel void @a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b, %opencl.image2d_ro_t.4 addrspace(1)* %c, %opencl.image2d_wo_t.5 addrspace(1)* %d) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !recommended_vector_length !15 {
entry:
; CHECK-LABEL: vector.body
; CHECK: [[VEC_IND:%.*]] = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, {{.*}} ], [ [[VEC_IND_NEXT:%.*]], {{.*}} ]
; CHECK-NEXT: [[VEC_IND_I64:%.*]] = sext <4 x i32> [[VEC_IND]] to <4 x i64>
; CHECK-NEXT: [[VGID:%.*]] = add nuw <4 x i64> [[VEC_IND_I64]], [[GID_BROADCAST:%.*]]
;
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %slid = tail call i32 @_Z22get_sub_group_local_idv() #3
  %slid.reverse = sub nuw i32 16, %slid
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %1 = trunc i32 %0 to i16
; CHECK: [[WIDE_LOAD_TRUNCi16:%.*]] = trunc <4 x i32> %wide.load to <4 x i16>
  %2 = trunc i32 %0 to i8
; CHECK: [[WIDE_LOAD_TRUNCi8:%.*]] = trunc <4 x i32> %wide.load to <4 x i8>
  %3 = zext i32 %0 to i64
; CHECK: [[WIDE_LOAD_TRUNCi64:%.*]] = zext <4 x i32> %wide.load to <4 x i64>

  %val = call i32 @_Z23intel_sub_group_shuffleij(i32 %0, i32 %slid.reverse)
  %val2 = call i32 @_Z23intel_sub_group_shufflejj(i32 %0, i32 %slid.reverse)
; CHECK: call {{(spir_func )?}}<4 x i32> @_Z23intel_sub_group_shuffleDv4_iDv4_jS0_(<4 x i32> %wide.load, <4 x i32> [[SLID_REVERSE:%.*]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: call {{(spir_func )?}}<4 x i32> @_Z23intel_sub_group_shuffleDv4_jS_S_(<4 x i32> %wide.load, <4 x i32> [[SLID_REVERSE]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %vec2 = insertelement <3 x i32> undef, i32 %0, i32 0
  %val3 = call <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32> %vec2, i32 %slid.reverse)
; CHECK: call {{(spir_func )?}}<12 x i32> @_Z23intel_sub_group_shuffleDv12_iDv4_jS0_(<12 x i32> %wide.insert, <4 x i32> [[SLID_REVERSE]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %vec3 = insertelement <16 x i16> undef, i16 %1, i32 0
  %val4 = call <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16> %vec3, i32 %slid.reverse)
; CHECK: call {{(spir_func )?}}<64 x i16> @_Z23intel_sub_group_shuffleDv64_sDv4_jS0_(<64 x i16> %wide.insert{{[0-9]+}}, <4 x i32> [[SLID_REVERSE]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %call1 = tail call spir_func i32 @_Z13sub_group_alli(i32 %0) #4
  %call2 = tail call spir_func i32 @_Z16get_sub_group_idv() #4
; CHECK: [[VECTOR_ALL:%.*]] = call {{(spir_func )?}}<4 x i32> @_Z13sub_group_allDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: [[UNIFORM_SUB_GROUP_ID:%.*]] = {{(tail )?}}call spir_func i32 @_Z16get_sub_group_idv()

  %call3 = tail call spir_func i64 @_Z19sub_group_broadcastlj(i64 %3, i32 0) #4
  %call4 = tail call spir_func i32 @_Z19sub_group_broadcastij(i32 %0, i32 0) #4
  %call5 = tail call spir_func i16 @_Z25intel_sub_group_broadcastsj(i16 %1, i32 0) #4
  %call6 = tail call spir_func i8  @_Z25intel_sub_group_broadcastcj(i8 %2, i32 0) #4
; CHECK: = call {{(spir_func )?}}<4 x i64> @_Z19sub_group_broadcastDv4_ljDv4_j(<4 x i64> [[WIDE_LOAD_TRUNCi64]], i32 0, <4 x i64> <i64 -1, i64 -1, i64 -1, i64 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i32> @_Z19sub_group_broadcastDv4_ijDv4_j(<4 x i32> %wide.load, i32 0, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i16> @_Z25intel_sub_group_broadcastDv4_sjDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], i32 0, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i8> @_Z25intel_sub_group_broadcastDv4_cjDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], i32 0, <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call7 = tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 %0) #4
  %call8 = tail call spir_func i16 @_Z26intel_sub_group_reduce_mins(i16 %1) #4
  %call9 = tail call spir_func i8  @_Z26intel_sub_group_reduce_maxc(i8  %2) #4
; CHECK: = call {{(spir_func )?}}<4 x i32> @_Z20sub_group_reduce_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i16> @_Z26intel_sub_group_reduce_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i8>  @_Z26intel_sub_group_reduce_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call10 = tail call spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32 %0) #4
  %call11 = tail call spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16 %1) #4
  %call12 = tail call spir_func i8  @_Z34intel_sub_group_scan_exclusive_maxc(i8  %2) #4
; CHECK: = call {{(spir_func )?}}<4 x i32> @_Z28sub_group_scan_exclusive_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i16> @_Z34intel_sub_group_scan_exclusive_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i8> @_Z34intel_sub_group_scan_exclusive_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call13 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 %0) #4
  %call14 = tail call spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16 %1) #4
  %call15 = tail call spir_func i8  @_Z34intel_sub_group_scan_inclusive_maxc(i8  %2) #4
; CHECK: = call {{(spir_func )?}}<4 x i32> @_Z28sub_group_scan_inclusive_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i16> @_Z34intel_sub_group_scan_inclusive_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call {{(spir_func )?}}<4 x i8> @_Z34intel_sub_group_scan_inclusive_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call16 = tail call spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64 %call, i64 42, i32 %slid.reverse)
; CHECK: = call {{(spir_func )?}}<4 x i64> @_Z28intel_sub_group_shuffle_downDv4_lS_Dv4_jS0_(

  %call17 = tail call spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32> <i32 41, i32 42, i32 43, i32 44>, i32 %slid.reverse)
; CHECK: = call {{(spir_func )?}}<16 x i32> @_Z28intel_sub_group_shuffle_downDv16_iS_Dv4_jS0_(

  %blk_read = call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)* %a)
; CHECK: = call <8 x i32> @_Z29intel_sub_group_block_read2_4PU3AS1KjDv4_j(i32 addrspace(1)* [[LOAD_a:%.*]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %blk_read.x2 = mul <2 x i32> %blk_read, <i32 2, i32 2>
  call void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(i32 addrspace(1)* %b, <2 x i32> %blk_read.x2)
; CHECK: call void @_Z30intel_sub_group_block_write2_4PU3AS1jDv8_jDv4_j(i32 addrspace(1)* [[LOAD_b:%.*]], <8 x i32> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %short_ptr = bitcast i32 addrspace(1)* %a to i16 addrspace(1)*
  %blk_read_short = call <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(i16 addrspace(1)* %short_ptr)
; CHECK: call <8 x i16> @_Z32intel_sub_group_block_read_us2_4PU3AS1KtDv4_j(i16 addrspace(1)* {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %blk_read_short.x2 = mul <2 x i16> %blk_read_short, <i16 2, i16 2>
  call void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(i16 addrspace(1)* %short_ptr, <2 x i16> %blk_read_short.x2)
; CHECK: call void @_Z33intel_sub_group_block_write_us2_4PU3AS1tDv8_tDv4_j(i16 addrspace(1)* {{%.*}}, <8 x i16> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %blk_img_read = call <2 x i32> @_Z27intel_sub_group_block_read214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)* %c, <2 x i32> <i32 0, i32 0>)
; CHECK: = call <8 x i32> @_Z29intel_sub_group_block_read2_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.4 addrspace(1)* %load.c, <2 x i32> zeroinitializer, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
   call void @_Z28intel_sub_group_block_write214ocl_image2d_woDv2_iDv2_j(%opencl.image2d_wo_t.5 addrspace(1)* %d, <2 x i32> <i32 0, i32 0>, <2 x i32> <i32 4, i32 4>)
; CHECK: call void @_Z30intel_sub_group_block_write2_414ocl_image2d_woDv2_iDv8_jDv4_j(%opencl.image2d_wo_t.5 addrspace(1)* %load.d, <2 x i32> zeroinitializer, <8 x i32> <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %blk_img_read1 = call <2 x i16> @_Z30intel_sub_group_block_read_us214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)* %c, <2 x i32> <i32 0, i32 0>)
; CHECK: = call <8 x i16> @_Z32intel_sub_group_block_read_us2_414ocl_image2d_roDv2_iDv4_j(%opencl.image2d_ro_t.4 addrspace(1)* %load.c, <2 x i32> zeroinitializer, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
   call void @_Z31intel_sub_group_block_write_us214ocl_image2d_woDv2_iDv2_t(%opencl.image2d_wo_t.5 addrspace(1)* %d, <2 x i32> <i32 0, i32 0>, <2 x i16> <i16 4, i16 4>)
; CHECK: call void @_Z33intel_sub_group_block_write_us2_414ocl_image2d_woDv2_iDv8_tDv4_j(%opencl.image2d_wo_t.5 addrspace(1)* %load.d, <2 x i32> zeroinitializer, <8 x i16> <i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4>, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %call18 = call <4 x i32> @_Z22intel_sub_group_balloti(i32 %0)
; CHECK: call <16 x i32> @_Z22intel_sub_group_ballotDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %tobool = icmp ne i32 %0, 0
  %call19 = call i32 @intel_sub_group_ballot(i1 zeroext %tobool)
; CHECK: call <4 x i32> @intel_sub_group_ballot_vf4(<4 x i8> zeroext {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %mul = mul i32 %call4, 1000
  %conv = zext i32 %mul to i64
  %add = add i64 %call, %conv
  %arrayidx4 = getelementptr inbounds i32, i32 addrspace(1)* %b, i64 %add
  store i32 %call1, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !10
; CHECK: store <4 x i32> [[VECTOR_ALL]]

  %cmp = icmp eq i32 %slid, 0
; CHECK: [[VICMP:%.*]] = icmp eq <4 x i32> [[VEC_IND]], zeroinitializer
; CHECK: [[MASK:%.*]] = xor <4 x i1> [[VICMP]], <i1 true, i1 true, i1 true, i1 true>
; CHECK: [[MASKEXT:%.*]] = sext <4 x i1> [[MASK]] to <4 x i32>
  br i1 %cmp, label %slid.zero, label %slid.nonzero

slid.zero:
  br label %end

slid.nonzero:
  %masked_load = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %masked_shuffle = call i32 @_Z23intel_sub_group_shuffleij(i32 %0, i32 4)
; CHECK: call {{(spir_func )?}}<4 x i32> @_Z23intel_sub_group_shuffleDv4_iDv4_jS0_(<4 x i32> %wide.load, <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32> [[MASKEXT]])
  br label %end

; This test previously had a large number of CHECK lines against function call
; attributes with hardcoded numbers, which was brittle to attribute changes.
; These CHECK lines are added to make sure the attributes being checked are the
; intended ones even if the numbering changes.
; CHECK: [[V54:%.*]] = load i32, i32 addrspace(1)* %arrayidx, align 4
; CHECK: [[V55:%.*]] = trunc i32 [[V54]] to i16
; CHECK: [[V56:%.*]] = trunc i32 [[V54]] to i8
; CHECK: [[V57:%.*]] = zext i32 [[V54]] to i64
; CHECK: %val2 = call i32 @_Z23intel_sub_group_shufflejj(i32 [[V54]], i32 %slid.reverse) [[AT12:#.*]]
; CHECK:  %val3 = call <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32> %vec2, i32 %slid.reverse) [[AT13:#.*]]
; CHECK:  %val4 = call <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16> %vec3, i32 %slid.reverse) [[AT14:#.*]]
; CHECK:  %call1 = tail call spir_func i32 @_Z13sub_group_alli(i32 [[V54]]) [[AT15:#.*]]
; CHECK:  %call3 = tail call spir_func i64 @_Z19sub_group_broadcastlj(i64 [[V57]], i32 0) [[AT16:#.*]]
; CHECK:  %call4 = tail call spir_func i32 @_Z19sub_group_broadcastij(i32 [[V54]], i32 0) [[AT17:#.*]]
; CHECK:  %call5 = tail call spir_func i16 @_Z25intel_sub_group_broadcastsj(i16 [[V55]], i32 0) [[AT18:#.*]]
; CHECK:  %call6 = tail call spir_func i8 @_Z25intel_sub_group_broadcastcj(i8 [[V56]], i32 0) [[AT19:#.*]]
; CHECK:  %call7 = tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 [[V54]]) [[AT20:#.*]]
; CHECK:  %call8 = tail call spir_func i16 @_Z26intel_sub_group_reduce_mins(i16 [[V55]]) [[AT21:#.*]]
; CHECK:  %call9 = tail call spir_func i8 @_Z26intel_sub_group_reduce_maxc(i8 [[V56]]) [[AT22:#.*]]
; CHECK:  %call10 = tail call spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32 [[V54]]) [[AT23:#.*]]
; CHECK:  %call11 = tail call spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16 [[V55]]) [[AT24:#.*]]
; CHECK:  %call12 = tail call spir_func i8 @_Z34intel_sub_group_scan_exclusive_maxc(i8 [[V56]]) [[AT25:#.*]]
; CHECK:  %call13 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 [[V54]]) [[AT26:#.*]]
; CHECK:  %call14 = tail call spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16 [[V55]]) [[AT27:#.*]]
; CHECK:  %call15 = tail call spir_func i8 @_Z34intel_sub_group_scan_inclusive_maxc(i8 [[V56]]) [[AT28:#.*]]
; CHECK:  %call16 = tail call spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64 %add1, i64 42, i32 %slid.reverse) [[AT29:#.*]]
; CHECK:  %call17 = tail call spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32> <i32 41, i32 42, i32 43, i32 44>, i32 %slid.reverse) [[AT30:#.*]]
; CHECK:  %blk_read = call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)* %load.a) [[AT31:#.*]]
; CHECK: call void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(i32 addrspace(1)* %load.b, <2 x i32> %blk_read.x2) [[AT32:#.*]]
; CHECK: %blk_read_short = call <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(i16 addrspace(1)* %short_ptr) [[AT33:#.*]]
; CHECK: call void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(i16 addrspace(1)* %short_ptr, <2 x i16> %blk_read_short.x2) [[AT34:#.*]]
; CHECK: %blk_img_read = call <2 x i32> @_Z27intel_sub_group_block_read214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)* %load.c, <2 x i32> zeroinitializer) [[AT35:#.*]]
; CHECK: call void @_Z28intel_sub_group_block_write214ocl_image2d_woDv2_iDv2_j(%opencl.image2d_wo_t.5 addrspace(1)* %load.d, <2 x i32> zeroinitializer, <2 x i32> <i32 4, i32 4>) [[AT36:#.*]]
; CHECK: %blk_img_read1 = call <2 x i16> @_Z30intel_sub_group_block_read_us214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)* %load.c, <2 x i32> zeroinitializer) [[AT37:#.*]]
; CHECK: call void @_Z31intel_sub_group_block_write_us214ocl_image2d_woDv2_iDv2_t(%opencl.image2d_wo_t.5 addrspace(1)* %load.d, <2 x i32> zeroinitializer, <2 x i16> <i16 4, i16 4>) [[AT38:#.*]]
; CHECK: %call18 = call <4 x i32> @_Z22intel_sub_group_balloti(i32 [[V54]]) [[AT39:#.*]]
; CHECK: %call19 = call i32 @intel_sub_group_ballot(i1 zeroext %tobool) [[AT40:#.*]]
; CHECK: %masked_shuffle = call i32 @_Z23intel_sub_group_shuffleij(i32 [[V54]], i32 4) [[AT11:#.*]]

end:
  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i64 @_Z19sub_group_broadcastlj(i64, i32) local_unnamed_addr #1
declare spir_func i32 @_Z19sub_group_broadcastij(i32, i32) local_unnamed_addr #1
declare spir_func i16 @_Z25intel_sub_group_broadcastsj(i16, i32) local_unnamed_addr #1
declare spir_func i8 @_Z25intel_sub_group_broadcastcj(i8, i32) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z20sub_group_reduce_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z26intel_sub_group_reduce_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z26intel_sub_group_reduce_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z34intel_sub_group_scan_exclusive_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z34intel_sub_group_scan_exclusive_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z34intel_sub_group_scan_inclusive_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z34intel_sub_group_scan_inclusive_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent
declare spir_func i32 @_Z16get_sub_group_idv() local_unnamed_addr #1

; Function Attrs: convergent
declare <4 x i32> @_Z22intel_sub_group_balloti(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @intel_sub_group_ballot(i1 zeroext) local_unnamed_addr #1

declare spir_func i32 @_Z23intel_sub_group_shuffleij(i32, i32) local_unnamed_addr #1
declare spir_func i32 @_Z23intel_sub_group_shufflejj(i32, i32) local_unnamed_addr #1
declare spir_func <3 x i32> @_Z23intel_sub_group_shuffleDv3_ij(<3 x i32>, i32) local_unnamed_addr #1
declare spir_func <16 x i16> @_Z23intel_sub_group_shuffleDv16_sj(<16 x i16>, i32) local_unnamed_addr #1
declare spir_func i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #1
declare spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64, i64, i32) local_unnamed_addr #1

declare spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32>, <4 x i32>, i32) local_unnamed_addr #1

declare <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)*) local_unnamed_addr #1
declare void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(i32 addrspace(1)*, <2 x i32>) local_unnamed_addr #1
declare <2 x i16> @_Z30intel_sub_group_block_read_us2PU3AS1Kt(i16 addrspace(1)*) local_unnamed_addr #1
declare void @_Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(i16 addrspace(1)*, <2 x i16>) local_unnamed_addr #1

declare <2 x i32> @_Z27intel_sub_group_block_read214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)*, <2 x i32>) local_unnamed_addr #1
declare void @_Z28intel_sub_group_block_write214ocl_image2d_woDv2_iDv2_j(%opencl.image2d_wo_t.5 addrspace(1)*, <2 x i32>, <2 x i32>) local_unnamed_addr #1
declare <2 x i16> @_Z30intel_sub_group_block_read_us214ocl_image2d_roDv2_i(%opencl.image2d_ro_t.4 addrspace(1)*, <2 x i32>) local_unnamed_addr #1
declare void @_Z31intel_sub_group_block_write_us214ocl_image2d_woDv2_iDv2_t(%opencl.image2d_wo_t.5 addrspace(1)*, <2 x i32>, <2 x i16>) local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

; CHECK-DAG: attributes [[AT11]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z23intel_sub_group_shuffleij(_Z23intel_sub_group_shuffleDv4_iDv4_jS0_)" }
; CHECK-DAG: attributes [[AT12]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z23intel_sub_group_shufflejj(_Z23intel_sub_group_shuffleDv4_jS_S_)" }
; CHECK-DAG: attributes [[AT13]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z23intel_sub_group_shuffleDv3_ij(_Z23intel_sub_group_shuffleDv12_iDv4_jS0_)" }
; CHECK-DAG: attributes [[AT14]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z23intel_sub_group_shuffleDv16_sj(_Z23intel_sub_group_shuffleDv64_sDv4_jS0_)" }
; CHECK-DAG: attributes [[AT15]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z13sub_group_alli(_Z13sub_group_allDv4_iDv4_j)" }
; CHECK-DAG: attributes [[AT16]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z19sub_group_broadcastlj(_Z19sub_group_broadcastDv4_lDv4_jS0_),_ZGVbM4vu__Z19sub_group_broadcastlj(_Z19sub_group_broadcastDv4_ljDv4_j)" }
; CHECK-DAG: attributes [[AT17]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z19sub_group_broadcastij(_Z19sub_group_broadcastDv4_iDv4_jS0_),_ZGVbM4vu__Z19sub_group_broadcastij(_Z19sub_group_broadcastDv4_ijDv4_j)" }
; CHECK-DAG: attributes [[AT18]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z25intel_sub_group_broadcastsj(_Z25intel_sub_group_broadcastDv4_sDv4_jS0_),_ZGVbM4vu__Z25intel_sub_group_broadcastsj(_Z25intel_sub_group_broadcastDv4_sjDv4_j)" }
; CHECK-DAG: attributes [[AT19]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vv__Z25intel_sub_group_broadcastcj(_Z25intel_sub_group_broadcastDv4_cDv4_jS0_),_ZGVbM4vu__Z25intel_sub_group_broadcastcj(_Z25intel_sub_group_broadcastDv4_cjDv4_j)" }
; CHECK-DAG: attributes [[AT20]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z20sub_group_reduce_addi(_Z20sub_group_reduce_addDv4_iDv4_j)" }
; CHECK-DAG: attributes [[AT21]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z26intel_sub_group_reduce_mins(_Z26intel_sub_group_reduce_minDv4_sDv4_j)" }
; CHECK-DAG: attributes [[AT22]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z26intel_sub_group_reduce_maxc(_Z26intel_sub_group_reduce_maxDv4_cDv4_j)" }
; CHECK-DAG: attributes [[AT23]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z28sub_group_scan_exclusive_addi(_Z28sub_group_scan_exclusive_addDv4_iDv4_j)" }
; CHECK-DAG: attributes [[AT24]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z34intel_sub_group_scan_exclusive_mins(_Z34intel_sub_group_scan_exclusive_minDv4_sDv4_j)" }
; CHECK-DAG: attributes [[AT25]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z34intel_sub_group_scan_exclusive_maxc(_Z34intel_sub_group_scan_exclusive_maxDv4_cDv4_j)" }
; CHECK-DAG: attributes [[AT26]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z28sub_group_scan_inclusive_addi(_Z28sub_group_scan_inclusive_addDv4_iDv4_j)" }
; CHECK-DAG: attributes [[AT27]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z34intel_sub_group_scan_inclusive_mins(_Z34intel_sub_group_scan_inclusive_minDv4_sDv4_j)" }
; CHECK-DAG: attributes [[AT28]] = { convergent nounwind "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z34intel_sub_group_scan_inclusive_maxc(_Z34intel_sub_group_scan_inclusive_maxDv4_cDv4_j)" }
; CHECK-DAG: attributes [[AT29]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vvv__Z28intel_sub_group_shuffle_downllj(_Z28intel_sub_group_shuffle_downDv4_lS_Dv4_jS0_)" }
; CHECK-DAG: attributes [[AT30]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4vvv__Z28intel_sub_group_shuffle_downDv4_iS_j(_Z28intel_sub_group_shuffle_downDv16_iS_Dv4_jS0_)" }
; CHECK-DAG: attributes [[AT31]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4u__Z27intel_sub_group_block_read2PU3AS1Kj(_Z29intel_sub_group_block_read2_4PU3AS1KjDv4_j),_ZGVbM4v__Z27intel_sub_group_block_read2PU3AS1Kj(_Z29intel_sub_group_block_read2_4Dv4_PU3AS1KjDv4_j)" }
; CHECK-DAG: attributes [[AT32]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uv__Z28intel_sub_group_block_write2PU3AS1jDv2_j(_Z30intel_sub_group_block_write2_4PU3AS1jDv8_jDv4_j),_ZGVbM4vv__Z28intel_sub_group_block_write2PU3AS1jDv2_j(_Z30intel_sub_group_block_write2_4Dv4_PU3AS1jDv8_jDv4_j)" }
; CHECK-DAG: attributes [[AT33]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4u__Z30intel_sub_group_block_read_us2PU3AS1Kt(_Z32intel_sub_group_block_read_us2_4PU3AS1KtDv4_j),_ZGVbM4v__Z30intel_sub_group_block_read_us2PU3AS1Kt(_Z32intel_sub_group_block_read_us2_4Dv4_PU3AS1KtDv4_j)" }
; CHECK-DAG: attributes [[AT34]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uv__Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(_Z33intel_sub_group_block_write_us2_4PU3AS1tDv8_tDv4_j),_ZGVbM4vv__Z31intel_sub_group_block_write_us2PU3AS1tDv2_t(_Z33intel_sub_group_block_write_us2_4Dv4_PU3AS1tDv8_tDv4_j)" }
; CHECK-DAG: attributes [[AT35]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uu__Z27intel_sub_group_block_read214ocl_image2d_roDv2_i(_Z29intel_sub_group_block_read2_414ocl_image2d_roDv2_iDv4_j),_ZGVbM4vv__Z27intel_sub_group_block_read214ocl_image2d_roDv2_i(_Z29intel_sub_group_block_read2_4Dv4_14ocl_image2d_roDv8_iDv4_j)" }
; CHECK-DAG: attributes [[AT36]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uuv__Z28intel_sub_group_block_write214ocl_image2d_woDv2_iDv2_j(_Z30intel_sub_group_block_write2_414ocl_image2d_woDv2_iDv8_jDv4_j),_ZGVbM4vvv__Z28intel_sub_group_block_write214ocl_image2d_woDv2_iDv2_j(_Z30intel_sub_group_block_write2_4Dv4_14ocl_image2d_woDv8_iDv8_jDv4_j)" }
; CHECK-DAG: attributes [[AT37]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uu__Z30intel_sub_group_block_read_us214ocl_image2d_roDv2_i(_Z32intel_sub_group_block_read_us2_414ocl_image2d_roDv2_iDv4_j),_ZGVbM4vv__Z30intel_sub_group_block_read_us214ocl_image2d_roDv2_i(_Z32intel_sub_group_block_read_us2_4Dv4_14ocl_image2d_roDv8_iDv4_j)" }
; CHECK-DAG: attributes [[AT38]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4uuv__Z31intel_sub_group_block_write_us214ocl_image2d_woDv2_iDv2_t(_Z33intel_sub_group_block_write_us2_414ocl_image2d_woDv2_iDv8_tDv4_j),_ZGVbM4vvv__Z31intel_sub_group_block_write_us214ocl_image2d_woDv2_iDv2_t(_Z33intel_sub_group_block_write_us2_4Dv4_14ocl_image2d_woDv8_iDv8_tDv4_j)" }
; CHECK-DAG: attributes [[AT39]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v__Z22intel_sub_group_balloti(_Z22intel_sub_group_ballotDv4_iDv4_j)" }
; CHECK-DAG: attributes [[AT40]] = { "has-vplan-mask" "kernel-call-once" "vector-variants"="_ZGVbM4v_intel_sub_group_balloti(intel_sub_group_ballot_vf4)" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!sycl.kernels = !{!14}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!4 = !{i32 1, i32 1}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int*"}
!7 = !{!"", !""}
!8 = !{i1 false, i1 false}
!9 = !{i32 0, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, %opencl.image2d_ro_t.4 addrspace(1)*, %opencl.image2d_wo_t.5 addrspace(1)*)* @a}
!15 = !{i32 4}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uuuu_a {{.*}} br
; DEBUGIFY-NOT: WARNING
