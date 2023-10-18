; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s

; This test is used to check reduction with complex array.
; Test src:
;
; program complex_alloc_array_reduction
;   implicit none
;   integer :: ir, nxxs, ipol, jpol, npol
;   complex(8), allocatable :: psic_all_nc(:,:)
;
;   npol = 2
;   nxxs = 10
;   allocate(psic_all_nc(nxxs,npol))
;
;   print *, "About to enter parallel region"
; !$omp parallel do reduction(+:psic_all_nc)
;   do ir = 1, nxxs
;      print *, "In parallel region"
;      do jpol = 1, npol
;         psic_all_nc(ir, 1) = psic_all_nc(ir,1) + 1
;      end do
;   end do
; !$omp end parallel do
;
; print *, "Done with test"
;
; end program complex_alloc_array_reduction

; Check for the fast reduction struct type
; CHECK: %struct.fast_red_t = type <{ %"QNCA_a0$%complex_128bit*$rank2$" }>

; Check that a global is created to store the number of elements in the dv.
; CHECK: [[NUM_ELEMENTS_GV:[^ ]+]] = private thread_local global i64 0

; Check that the fast reduction callback loads the num_elements global for the F90 DV.
; CHECK-LABEL: define internal void @MAIN{{[^ ]+}}tree_reduce{{[^ ]+}}(ptr %dst, ptr %src)
; CHECK: [[NUM_ELEMENTS_LOAD:[^ ]+]] = load i64, ptr [[NUM_ELEMENTS_GV]], align 8

; Now check the code generated inside the outlined function for the parallel region.
; CHECK-LABEL: define internal void @MAIN{{[^ ]*}}DIR.OMP.PARALLEL{{[^ ]*}}

; Check for the allocation of local dope vector, and the fast reduction struct.
; CHECK: [[FAST_RED_STR:[^ ]+]] = alloca %struct.fast_red_t, align 16
; CHECK: [[PRIV_DV:[^ ]+]] = alloca %"QNCA_a0$%complex_128bit*$rank2$", align 16

; Check that the dope vector init call is emitted for PRIV_DV.
; CHECK: [[PRIV_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[PRIV_DV]], ptr @{{.*}})
; CHECK: [[NUM_ELEMENTS:[^ ]+]] = udiv i64 [[PRIV_DV_ARR_SIZE]], 16

; Check that local data is allocated and stored to the addr0 field of the dope vector.
; CHECK: [[PRIV_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$%complex_128bit*$rank2$", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[PRIV_DV_DATA:[^ ]+]] = alloca %complex_128bit, i64 [[NUM_ELEMENTS]], align 16
; CHECK: store ptr [[PRIV_DV_DATA]], ptr [[PRIV_DV_ADDR0]]
; Check that num_elements is stored to a global so that it can be accessed from the reduction callback.
; CHECK: store i64 [[NUM_ELEMENTS]], ptr [[NUM_ELEMENTS_GV]], align 8

; Check that the GEP for the dope vector in the fast-reduction struct is computed:
; CHECK: [[FAST_RED_DV:[^ ]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[FAST_RED_STR]], i32 0, i32 0

; Check that PRIV_DV is used for the code inside the parallel region.
; CHECK: [[PRIV_DV_ADDR1:[^ ]+]] = getelementptr inbounds %"QNCA_a0$%complex_128bit*$rank2$", ptr [[PRIV_DV]], i32 0, i32 0
; CHECK: [[PRIV_DV_DATA1:[^ ]+]] = load ptr, ptr [[PRIV_DV_ADDR1]], align 8
; CHECK: %{{.*}} = getelementptr %complex_128bit, ptr [[PRIV_DV_DATA1]], i64 [[NUM_ELEMENTS]]

; Check that the copy of the dope vector in the fast reduction struct is initialized.
; CHECK: [[FAST_RED_DV_ARR_SIZE:[^ ]+]] = call i64 @_f90_dope_vector_init2(ptr [[FAST_RED_DV]], ptr [[PRIV_DV]])
; CHECK: [[NUM_ELEMENTS_1:[^ ]+]] = udiv i64 [[FAST_RED_DV_ARR_SIZE]], 16
; CHECK: [[FAST_RED_DV_ADDR0:[^ ]+]] = getelementptr inbounds %"QNCA_a0$%complex_128bit*$rank2$", ptr [[FAST_RED_DV]], i32 0, i32 0
; CHECK: [[FAST_RED_DV_DATA:[^ ]+]] = alloca %complex_128bit, i64 [[NUM_ELEMENTS_1]], align 16
; CHECK: store ptr [[FAST_RED_DV_DATA]], ptr [[FAST_RED_DV_ADDR0]]

; Check for calls to kmpc_[end_]reduce, and that FAST_RED_DV and Orig (%"complex_alloc_array_reduction_$PSIC_ALL_NC") are used between those calls, but not PRIV_DV.
; CHECK: {{[^ ]+}} = call i32 @__kmpc_reduce(ptr {{[^ ,]+}}, i32 {{[^ ,]+}}, i32 1, i32 96, ptr {{[^ ,]+}}, ptr @MAIN{{[^ ,]*}}tree_reduce{{[^ ,]*}}, ptr {{[^ ,]+}})
; CHECK: {{[^ ]+}} = getelementptr inbounds %"QNCA_a0$%complex_128bit*$rank2$", ptr [[FAST_RED_DV]], i32 0, i32 0
; CHECK: {{.+}} = load ptr, ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", align 8
; CHECK: call void @__kmpc_end_reduce(ptr {{.+}}, i32 {{.+}}, ptr {{.+}})
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%complex_128bit*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%complex_128bit = type { double, double }

@0 = internal unnamed_addr constant [14 x i8] c"Done with test"
@1 = internal unnamed_addr constant [18 x i8] c"In parallel region"
@2 = internal unnamed_addr constant [30 x i8] c"About to enter parallel region"
@"complex_alloc_array_reduction_$PSIC_ALL_NC" = internal global %"QNCA_a0$%complex_128bit*$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@3 = internal unnamed_addr constant i32 2

define void @MAIN__() {
alloca_0:
  %"var$2" = alloca [8 x i64], align 16
  %"complex_alloc_array_reduction_$NPOL" = alloca i32, align 8
  %"complex_alloc_array_reduction_$JPOL" = alloca i32, align 8
  %"complex_alloc_array_reduction_$IPOL" = alloca i32, align 8
  %"complex_alloc_array_reduction_$NXXS" = alloca i32, align 8
  %"complex_alloc_array_reduction_$IR" = alloca i32, align 8
  %"var$3" = alloca i64, align 8
  %"var$4" = alloca i64, align 8
  %"var$5" = alloca i32, align 4
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { i64, ptr }, align 8
  %"var$6" = alloca i32, align 4
  %"(&)val$156" = alloca [4 x i8], align 1
  %argblock157 = alloca { i64, ptr }, align 8
  %"var$7" = alloca i32, align 4
  %"var$8" = alloca i32, align 4
  %"(&)val$262" = alloca [4 x i8], align 1
  %argblock263 = alloca { i64, ptr }, align 8
  %strlit = load [14 x i8], ptr @0, align 1
  %strlit1 = load [18 x i8], ptr @1, align 1
  %strlit2 = load [30 x i8], ptr @2, align 1
  %func_result = call i32 @for_set_reentrancy(ptr @3)
  store i32 2, ptr %"complex_alloc_array_reduction_$NPOL", align 1
  store i32 10, ptr %"complex_alloc_array_reduction_$NXXS", align 1
  %val_fetch107 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 3), align 1
  %and108 = and i64 %val_fetch107, 256
  %lshr109 = lshr i64 %and108, 8
  %shl110 = shl i64 %lshr109, 8
  %or111 = or i64 133, %shl110
  %and112 = and i64 %val_fetch107, 1030792151040
  %lshr113 = lshr i64 %and112, 36
  %and114 = and i64 %or111, -1030792151041
  %shl115 = shl i64 %lshr113, 36
  %or116 = or i64 %and114, %shl115
  store i64 %or116, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 3), align 1
  store i64 16, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 1), align 1
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 4), align 1
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 2), align 1
  %"complex_alloc_array_reduction_$NXXS_fetch" = load i32, ptr %"complex_alloc_array_reduction_$NXXS", align 1
  %int_sext117 = sext i32 %"complex_alloc_array_reduction_$NXXS_fetch" to i64
  %"val$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, ptr %"val$[]", align 1
  %sub = sub nsw i64 %int_sext117, 1
  %add = add nsw i64 1, %sub
  %rel = icmp sle i64 1, %int_sext117
  %rel6 = icmp sle i64 1, %int_sext117
  %rel8 = icmp ne i1 %rel6, false
  %slct = select i1 %rel8, i64 %add, i64 0
  %"val$[]11" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 %slct, ptr %"val$[]11", align 1
  %"complex_alloc_array_reduction_$NPOL_fetch" = load i32, ptr %"complex_alloc_array_reduction_$NPOL", align 1
  %int_sext = sext i32 %"complex_alloc_array_reduction_$NPOL_fetch" to i64
  %"val$[]14" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  store i64 1, ptr %"val$[]14", align 1
  %sub16 = sub nsw i64 %int_sext, 1
  %add18 = add nsw i64 1, %sub16
  %rel20 = icmp sle i64 1, %int_sext
  %rel22 = icmp sle i64 1, %int_sext
  %rel24 = icmp ne i1 %rel22, false
  %slct26 = select i1 %rel24, i64 %add18, i64 0
  %"val$[]29" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 0), i32 1)
  store i64 %slct26, ptr %"val$[]29", align 1
  %"val$[]32" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 16, ptr %"val$[]32", align 1
  %mul = mul nsw i64 16, %slct
  %"val$[]35" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  store i64 %mul, ptr %"val$[]35", align 1
  %func_result37 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr %"var$4", i32 3, i64 %slct, i64 %slct26, i64 16)
  %"var$4_fetch" = load i64, ptr %"var$4", align 1
  %val_fetch = load i64, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 3), align 1
  %val_fetch39 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 3), align 1
  %and = and i64 %val_fetch39, -68451041281
  %or = or i64 %and, 1073741824
  store i64 %or, ptr getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 3), align 1
  %and41 = and i64 %val_fetch, 1
  %shl = shl i64 %and41, 1
  %int_zext = trunc i64 %shl to i32
  %or43 = or i32 0, %int_zext
  %and45 = and i32 %func_result37, 1
  %and47 = and i32 %or43, -17
  %shl49 = shl i32 %and45, 4
  %or51 = or i32 %and47, %shl49
  %and53 = and i64 %val_fetch, 256
  %lshr = lshr i64 %and53, 8
  %and55 = and i32 %or51, -2097153
  %shl57 = shl i64 %lshr, 21
  %int_zext59 = trunc i64 %shl57 to i32
  %or61 = or i32 %and55, %int_zext59
  %and63 = and i64 %val_fetch, 1030792151040
  %lshr65 = lshr i64 %and63, 36
  %and67 = and i32 %or61, -31457281
  %shl69 = shl i64 %lshr65, 21
  %int_zext71 = trunc i64 %shl69 to i32
  %or73 = or i32 %and67, %int_zext71
  %and75 = and i64 %val_fetch, 1099511627776
  %lshr77 = lshr i64 %and75, 40
  %and79 = and i32 %or73, -33554433
  %shl81 = shl i64 %lshr77, 25
  %int_zext83 = trunc i64 %shl81 to i32
  %or85 = or i32 %and79, %int_zext83
  %and87 = and i32 %or85, -2031617
  %or89 = or i32 %and87, 262144
  %func_result91 = call i32 @for_alloc_allocatable(i64 %"var$4_fetch", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 %or89)
  %"val$[]94" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"val$[]94_fetch" = load i64, ptr %"val$[]94", align 1
  %mul96 = mul nsw i64 %"val$[]94_fetch", 16
  %"val$[]99" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  %"val$[]99_fetch" = load i64, ptr %"val$[]99", align 1
  %"val$[]102" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  %"val$[]102_fetch" = load i64, ptr %"val$[]102", align 1
  %mul104 = mul nsw i64 %"val$[]99_fetch", %"val$[]102_fetch"
  %add106 = add nsw i64 %mul96, %mul104
  store [4 x i8] c"8\04\01\00", ptr %"(&)val$", align 1
  %BLKFIELD_ = getelementptr inbounds { i64, ptr }, ptr %argblock, i32 0, i32 0
  store i64 30, ptr %BLKFIELD_, align 1
  %BLKFIELD_119 = getelementptr inbounds { i64, ptr }, ptr %argblock, i32 0, i32 1
  store ptr @2, ptr %BLKFIELD_119, align 1
  %"(i8*)var$2$" = bitcast ptr %"var$2" to ptr
  %"(i8*)(&)val$$" = bitcast ptr %"(&)val$" to ptr
  %"(i8*)argblock$" = bitcast ptr %argblock to ptr
  %func_result121 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr %"(i8*)var$2$", i32 -1, i64 1239157112576, ptr %"(i8*)(&)val$$", ptr %"(i8*)argblock$")
  %"complex_alloc_array_reduction_$NXXS_fetch123" = load i32, ptr %"complex_alloc_array_reduction_$NXXS", align 1
  %temp = alloca i32, align 1
  %temp125 = alloca i32, align 1
  %temp127 = alloca i32, align 1
  store i32 1, ptr %temp, align 1
  store i32 %"complex_alloc_array_reduction_$NXXS_fetch123", ptr %temp125, align 1
  store i32 1, ptr %temp127, align 1
  %temp_fetch = load i32, ptr %temp, align 1
  store i32 %temp_fetch, ptr %"complex_alloc_array_reduction_$IR", align 1
  %temp129 = alloca i32, align 1
  %temp131 = alloca i32, align 1
  %temp133 = alloca i32, align 1
  store i32 0, ptr %temp129, align 1
  store i32 0, ptr %temp131, align 1
  %temp127_fetch = load i32, ptr %temp127, align 1
  %temp_fetch135 = load i32, ptr %temp, align 1
  %temp125_fetch = load i32, ptr %temp125, align 1
  %sub137 = sub nsw i32 %temp125_fetch, %temp_fetch135
  %div = sdiv i32 %sub137, %temp127_fetch
  store i32 %div, ptr %temp133, align 1
  br label %bb33

bb35:                                             ; preds = %bb33
  store i32 0, ptr %temp131, align 1
  %temp_fetch139 = load i32, ptr %temp, align 1
  %temp127_fetch141 = load i32, ptr %temp127, align 1
  %temp131_fetch = load i32, ptr %temp131, align 1
  %mul143 = mul nsw i32 %temp131_fetch, %temp127_fetch141
  %add145 = add nsw i32 %mul143, %temp_fetch139
  store i32 %add145, ptr %"complex_alloc_array_reduction_$IR", align 1
  br label %bb39

bb39:                                             ; preds = %bb45, %bb35
  %temp_fetch147 = load i32, ptr %temp, align 1
  %temp127_fetch149 = load i32, ptr %temp127, align 1
  %temp131_fetch151 = load i32, ptr %temp131, align 1
  %mul153 = mul nsw i32 %temp131_fetch151, %temp127_fetch149
  %add155 = add nsw i32 %mul153, %temp_fetch147
  store i32 %add155, ptr %"complex_alloc_array_reduction_$IR", align 1
  store [4 x i8] c"8\04\01\00", ptr %"(&)val$156", align 1
  %BLKFIELD_159 = getelementptr inbounds { i64, ptr }, ptr %argblock157, i32 0, i32 0
  store i64 18, ptr %BLKFIELD_159, align 1
  %BLKFIELD_161 = getelementptr inbounds { i64, ptr }, ptr %argblock157, i32 0, i32 1
  store ptr @1, ptr %BLKFIELD_161, align 1
  %"(i8*)var$2$163" = bitcast ptr %"var$2" to ptr
  %"(i8*)(&)val$156$" = bitcast ptr %"(&)val$156" to ptr
  %"(i8*)argblock157$" = bitcast ptr %argblock157 to ptr
  %func_result165 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr %"(i8*)var$2$163", i32 -1, i64 1239157112576, ptr %"(i8*)(&)val$156$", ptr %"(i8*)argblock157$")
  %"complex_alloc_array_reduction_$NPOL_fetch167" = load i32, ptr %"complex_alloc_array_reduction_$NPOL", align 1
  store i32 %"complex_alloc_array_reduction_$NPOL_fetch167", ptr %"var$7", align 1
  store i32 1, ptr %"complex_alloc_array_reduction_$JPOL", align 1
  %"var$7_fetch" = load i32, ptr %"var$7", align 1
  %rel169 = icmp slt i32 %"var$7_fetch", 1
  br i1 %rel169, label %bb45, label %bb46

bb46:                                             ; preds = %bb39
  br label %bb44

bb44:                                             ; preds = %bb44, %bb46
  %val_fetch240 = load ptr, ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", align 1
  %"val$[]172" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"val$[]172_fetch" = load i64, ptr %"val$[]172", align 1
  %"complex_alloc_array_reduction_$IR_fetch" = load i32, ptr %"complex_alloc_array_reduction_$IR", align 1
  %int_sext174 = sext i32 %"complex_alloc_array_reduction_$IR_fetch" to i64
  %"val$[]177" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  %"val$[]177_fetch" = load i64, ptr %"val$[]177", align 1
  %"val$[]180" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  %"val$[]180_fetch" = load i64, ptr %"val$[]180", align 1
  %"val$[]183" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"val$[]183_fetch" = load i64, ptr %"val$[]183", align 1
  %mul185 = mul nsw i64 %"val$[]183_fetch", 16
  %"val$[]188" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  %"val$[]188_fetch" = load i64, ptr %"val$[]188", align 1
  %"val$[]191" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  %"val$[]191_fetch" = load i64, ptr %"val$[]191", align 1
  %mul193 = mul nsw i64 %"val$[]188_fetch", %"val$[]191_fetch"
  %add195 = add nsw i64 %mul185, %mul193
  %"val_fetch[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"val$[]180_fetch", i64 %"val$[]177_fetch", ptr elementtype(%complex_128bit) %val_fetch240, i64 1)
  %"val_fetch[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]172_fetch", i64 16, ptr elementtype(%complex_128bit) %"val_fetch[]", i64 %int_sext174)
  %"val_fetch[][]_fetch" = load %complex_128bit, ptr %"val_fetch[][]", align 1
  %"val_fetch[][]_fetch_comp_0" = extractvalue %complex_128bit %"val_fetch[][]_fetch", 0
  %add201 = fadd reassoc ninf nsz arcp contract afn double %"val_fetch[][]_fetch_comp_0", 1.000000e+00
  %"val_fetch[][]_fetch_comp_1" = extractvalue %complex_128bit %"val_fetch[][]_fetch", 1
  %add203 = fadd reassoc ninf nsz arcp contract afn double %"val_fetch[][]_fetch_comp_1", 0.000000e+00
  %insertval = insertvalue %complex_128bit zeroinitializer, double %add201, 0
  %insertval205 = insertvalue %complex_128bit %insertval, double %add203, 1
  %val_fetch207 = load ptr, ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", align 1
  %"val$[]210" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"val$[]210_fetch" = load i64, ptr %"val$[]210", align 1
  %"complex_alloc_array_reduction_$IR_fetch212" = load i32, ptr %"complex_alloc_array_reduction_$IR", align 1
  %int_sext214 = sext i32 %"complex_alloc_array_reduction_$IR_fetch212" to i64
  %"val$[]217" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  %"val$[]217_fetch" = load i64, ptr %"val$[]217", align 1
  %"val$[]220" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  %"val$[]220_fetch" = load i64, ptr %"val$[]220", align 1
  %"val$[]223" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"val$[]223_fetch" = load i64, ptr %"val$[]223", align 1
  %mul225 = mul nsw i64 %"val$[]223_fetch", 16
  %"val$[]228" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 2), i32 1)
  %"val$[]228_fetch" = load i64, ptr %"val$[]228", align 1
  %"val$[]231" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%complex_128bit*$rank2$", ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", i32 0, i32 6, i32 0, i32 1), i32 1)
  %"val$[]231_fetch" = load i64, ptr %"val$[]231", align 1
  %mul233 = mul nsw i64 %"val$[]228_fetch", %"val$[]231_fetch"
  %add235 = add nsw i64 %mul225, %mul233
  %"val_fetch207[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"val$[]220_fetch", i64 %"val$[]217_fetch", ptr elementtype(%complex_128bit) %val_fetch207, i64 1)
  %"val_fetch207[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]210_fetch", i64 16, ptr elementtype(%complex_128bit) %"val_fetch207[]", i64 %int_sext214)
  store %complex_128bit %insertval205, ptr %"val_fetch207[][]", align 1
  %"complex_alloc_array_reduction_$JPOL_fetch" = load i32, ptr %"complex_alloc_array_reduction_$JPOL", align 1
  %add242 = add nsw i32 %"complex_alloc_array_reduction_$JPOL_fetch", 1
  store i32 %add242, ptr %"complex_alloc_array_reduction_$JPOL", align 1
  %"var$7_fetch244" = load i32, ptr %"var$7", align 1
  %"complex_alloc_array_reduction_$JPOL_fetch246" = load i32, ptr %"complex_alloc_array_reduction_$JPOL", align 1
  %rel248 = icmp sle i32 %"complex_alloc_array_reduction_$JPOL_fetch246", %"var$7_fetch244"
  br i1 %rel248, label %bb44, label %bb47

bb47:                                             ; preds = %bb44
  br label %bb45

bb45:                                             ; preds = %bb47, %bb39
  %temp127_fetch250 = load i32, ptr %temp127, align 1
  %temp131_fetch252 = load i32, ptr %temp131, align 1
  %add254 = add nsw i32 %temp131_fetch252, 1
  store i32 %add254, ptr %temp131, align 1
  %temp133_fetch = load i32, ptr %temp133, align 1
  %temp131_fetch256 = load i32, ptr %temp131, align 1
  %rel258 = icmp sle i32 %temp131_fetch256, %temp133_fetch
  br i1 %rel258, label %bb39, label %bb36

bb36:                                             ; preds = %bb45
  br label %bb37

bb33:                                             ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:CMPLX.F90_DV.TYPED"(ptr @"complex_alloc_array_reduction_$PSIC_ALL_NC", %"QNCA_a0$%complex_128bit*$rank2$" zeroinitializer, %complex_128bit zeroinitializer),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"complex_alloc_array_reduction_$JPOL", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"complex_alloc_array_reduction_$IR", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"complex_alloc_array_reduction_$NPOL", i32 0, i64 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %"complex_alloc_array_reduction_$NXXS", i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %temp131, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %temp133, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %temp129, i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"var$7", i32 0, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"var$6", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"var$2", i64 0, i64 8) ]

  %temp129_fetch = load i32, ptr %temp129, align 1
  store i32 %temp129_fetch, ptr %temp131, align 1
  %temp131_fetch259 = load i32, ptr %temp131, align 1
  %temp133_fetch260 = load i32, ptr %temp133, align 1
  %rel261 = icmp slt i32 %temp133_fetch260, %temp131_fetch259
  br i1 %rel261, label %bb37, label %bb35

bb37:                                             ; preds = %bb33, %bb36
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  store [4 x i8] c"8\04\01\00", ptr %"(&)val$262", align 1
  %BLKFIELD_265 = getelementptr inbounds { i64, ptr }, ptr %argblock263, i32 0, i32 0
  store i64 14, ptr %BLKFIELD_265, align 1
  %BLKFIELD_267 = getelementptr inbounds { i64, ptr }, ptr %argblock263, i32 0, i32 1
  store ptr @0, ptr %BLKFIELD_267, align 1
  %"(i8*)var$2$269" = bitcast ptr %"var$2" to ptr
  %"(i8*)(&)val$262$" = bitcast ptr %"(&)val$262" to ptr
  %"(i8*)argblock263$" = bitcast ptr %argblock263 to ptr
  %func_result271 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr %"(i8*)var$2$269", i32 -1, i64 1239157112576, ptr %"(i8*)(&)val$262$", ptr %"(i8*)argblock263$")
  ret void
}

declare i32 @for_set_reentrancy(ptr)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32)
declare i32 @for_check_mult_overflow64(ptr, i32, ...)
declare i32 @for_alloc_allocatable(i64, ptr, i32)
declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...)
declare token @llvm.directive.region.entry()
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION
