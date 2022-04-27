; INTEL_CUSTOMIZATION
; RUN: opt -switch-to-offload -lower-subscript -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(lower-subscript,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test that we emit call to f90_dv_init inside the target region to handle
; the private clause on target. This is temporary until we have a better way
; to handle them.

; Test src:

;   program main
;       integer, allocatable::a(:)
;       allocate(a(3))
;       a(1) = 10
;       print *, a(1)
;           !$omp target private(a)
;           print *, a(1)
;           a(1) = 20
;           !$omp end target
;       print *, a(1)
;       deallocate(a)
;   end program

; Check for the deflaration of f90_dv_init function
; CHECK: declare spir_func i64 @_f90_dope_vector_init(i8 addrspace(4)*, i8 addrspace(4)*)

; Check for allocation and initialization of local DV inside outlined function
; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.+}}MAIN__{{.+}}(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* {{.+}})
; CHECK: [[MAIN_A_PRIV:%[^ ]+]] = alloca %"QNCA_a0$i32 addrspace(4)*$rank1$"
; CHECK: [[DV_SIZE:%[^ ]+]] = call spir_func i64 @_f90_dope_vector_init(i8 addrspace(4)* {{[^ ,]+}}, i8 addrspace(4)* %{{[^ ,]+}})
; CHECK: [[NUM_ELEMENTS:%[^ ]+]] = udiv i64 [[DV_SIZE]], 4
; CHECK: [[MAIN_A_PRIV_ADDR0:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$"* {{[^ ]+}}, i32 0, i32 0
; CHECK: [[MAIN_A_PRIV_DATA:%[^ ]+]] = alloca i32, i64 [[NUM_ELEMENTS]]
; CHECK: [[MAIN_A_PRIV_DATA_CAST:%[^ ]+]] = addrspacecast i32* [[MAIN_A_PRIV_DATA]] to i32 addrspace(4)*
; CHECK: store i32 addrspace(4)* [[MAIN_A_PRIV_DATA_CAST]], i32 addrspace(4)** [[MAIN_A_PRIV_ADDR0]]

source_filename = "target_map_dv_allocatable.f90"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%"QNCA_a0$i32 addrspace(4)*$rank1$" = type { i32 addrspace(4)*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"main_$A" = internal addrspace(1) global %"QNCA_a0$i32 addrspace(4)*$rank1$" { i32 addrspace(4)* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@0 = internal unnamed_addr addrspace(1) constant [6 x i8] c"%lld\0A\00"

define void @MAIN__() #0 {
alloca:
  %"var$2" = alloca [8 x i64], align 8
  %addressof = alloca [4 x i8]
  %ARGBLOCK_0 = alloca { i32 }
  %addressof118 = alloca [4 x i8]
  %ARGBLOCK_1 = alloca { i32 }
  %_fetch40 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and41 = and i64 %_fetch40, 256
  %lshr42 = lshr i64 %and41, 8
  %shl43 = shl i64 %lshr42, 8
  %or44 = or i64 133, %shl43
  %and45 = and i64 %_fetch40, 1030792151040
  %lshr46 = lshr i64 %and45, 36
  %and47 = and i64 %or44, -1030792151041
  %shl48 = shl i64 %lshr46, 36
  %or49 = or i64 %and47, %shl48
  store i64 %or49, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  store i64 4, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 1)
  store i64 1, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 4)
  store i64 0, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 2)
  %"[]" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, i64 addrspace(1)* %"[]"
  %"[]1" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 3, i64 addrspace(1)* %"[]1"
  %"[]2" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 4, i64 addrspace(1)* %"[]2"
  %_fetch = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %_fetch4 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and = and i64 %_fetch4, -68451041281
  %or = or i64 %and, 1073741824
  store i64 %or, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and6 = and i64 %_fetch, 1
  %shl = shl i64 %and6, 1
  %int_zext = trunc i64 %shl to i32
  %or8 = or i32 0, %int_zext
  %and10 = and i32 %or8, -17
  %and12 = and i64 %_fetch, 256
  %lshr = lshr i64 %and12, 8
  %and14 = and i32 %and10, -2097153
  %shl16 = shl i64 %lshr, 21
  %int_zext18 = trunc i64 %shl16 to i32
  %or20 = or i32 %and14, %int_zext18
  %and22 = and i64 %_fetch, 1030792151040
  %lshr24 = lshr i64 %and22, 36
  %and26 = and i32 %or20, -31457281
  %shl28 = shl i64 %lshr24, 21
  %int_zext30 = trunc i64 %shl28 to i32
  %or32 = or i32 %and26, %int_zext30
  %and34 = and i32 %or32, -2031617
  %or36 = or i32 %and34, 262144
  %func_result = call i32 @for_alloc_allocatable(i64 12, i8** addrspacecast (i8* addrspace(1)* bitcast (%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A" to i8* addrspace(1)*) to i8**), i32 %or36)
  %_fetch60 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %"[]52" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"[]52_fetch" = load i64, i64 addrspace(1)* %"[]52"
  %"_fetch[]" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 %"[]52_fetch", i64 4, i32 addrspace(4)* elementtype(i32) %_fetch60, i64 1)
  store i32 10, i32 addrspace(4)* %"_fetch[]"
  %_fetch78 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %"[]63" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"[]63_fetch" = load i64, i64 addrspace(1)* %"[]63"
  %"_fetch[]71" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 %"[]63_fetch", i64 4, i32 addrspace(4)* elementtype(i32) %_fetch78, i64 1)
  %"_fetch[]71_fetch" = load i32, i32 addrspace(4)* %"_fetch[]71"
  store [4 x i8] c"\09\01\01\00", [4 x i8]* %addressof
  %BLKFIELD_ = getelementptr inbounds { i32 }, { i32 }* %ARGBLOCK_0, i32 0, i32 0
  store i32 %"_fetch[]71_fetch", i32* %BLKFIELD_
  %ptr_cast = bitcast [8 x i64]* %"var$2" to i8*
  %ptr_cast73 = bitcast [4 x i8]* %addressof to i8*
  %ptr_cast75 = bitcast { i32 }* %ARGBLOCK_0 to i8*
  %func_result77 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast, i32 -1, i64 1239157112576, i8* %ptr_cast73, i8* %ptr_cast75)
  br label %bb21

bb21:                                             ; preds = %alloca
  %A.cast = addrspacecast %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A" to %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)*
  %addr0 = getelementptr inbounds %"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(4)* %A.cast, i32 0, i32 0
  %addr0.val = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %addr0, align 8
  %next = getelementptr i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %addr0, i64 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i64 72, i64 32), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 addrspace(4)* addrspace(4)* %next, i64 64, i64 281474976710657), "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 addrspace(4)* %addr0.val, i64 12, i64 281474976710673), "QUAL.OMP.PRIVATE:F90_DV"(%"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A") ]


  %_fetch94 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %"[]81" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"[]81_fetch" = load i64, i64 addrspace(1)* %"[]81"
  %"_fetch[]89" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 %"[]81_fetch", i64 4, i32 addrspace(4)* elementtype(i32) %_fetch94, i64 1)
  %"_fetch[]89_fetch" = load i32, i32 addrspace(4)* %"_fetch[]89"
  %int_sext = sext i32 %"_fetch[]89_fetch" to i64
  %func_result93 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(1)* @0, i32 0, i32 0) to i8 addrspace(4)*), i64 %int_sext)
  %_fetch106 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %"[]97" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"[]97_fetch" = load i64, i64 addrspace(1)* %"[]97"
  %"_fetch[]105" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 %"[]97_fetch", i64 4, i32 addrspace(4)* elementtype(i32) %_fetch106, i64 1)
  store i32 20, i32 addrspace(4)* %"_fetch[]105"

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]


  %_fetch129 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %"[]109" = call i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8 0, i64 0, i32 24, i64 addrspace(1)* elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 6, i32 0, i32 2), i32 0)
  %"[]109_fetch" = load i64, i64 addrspace(1)* %"[]109"
  %"_fetch[]117" = call i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8 0, i64 %"[]109_fetch", i64 4, i32 addrspace(4)* elementtype(i32) %_fetch129, i64 1)
  %"_fetch[]117_fetch" = load i32, i32 addrspace(4)* %"_fetch[]117"
  store [4 x i8] c"\09\01\01\00", [4 x i8]* %addressof118
  %BLKFIELD_120 = getelementptr inbounds { i32 }, { i32 }* %ARGBLOCK_1, i32 0, i32 0
  store i32 %"_fetch[]117_fetch", i32* %BLKFIELD_120
  %ptr_cast122 = bitcast [8 x i64]* %"var$2" to i8*
  %ptr_cast124 = bitcast [4 x i8]* %addressof118 to i8*
  %ptr_cast126 = bitcast { i32 }* %ARGBLOCK_1 to i8*
  %func_result128 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* %ptr_cast122, i32 -1, i64 1239157112576, i8* %ptr_cast124, i8* %ptr_cast126)
  %_fetch133 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %_fetch135 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and137 = and i64 %_fetch135, 2
  %lshr139 = lshr i64 %and137, 1
  %shl141 = shl i64 %lshr139, 2
  %int_zext143 = trunc i64 %shl141 to i32
  %or145 = or i32 0, %int_zext143
  %and147 = and i64 %_fetch135, 1
  %and149 = and i32 %or145, -3
  %shl151 = shl i64 %and147, 1
  %int_zext153 = trunc i64 %shl151 to i32
  %or155 = or i32 %and149, %int_zext153
  %and157 = and i64 %_fetch135, 2048
  %lshr159 = lshr i64 %and157, 11
  %and161 = and i32 %or155, -257
  %shl163 = shl i64 %lshr159, 8
  %int_zext165 = trunc i64 %shl163 to i32
  %or167 = or i32 %and161, %int_zext165
  %and169 = and i64 %_fetch135, 256
  %lshr171 = lshr i64 %and169, 8
  %and173 = and i32 %or167, -2097153
  %shl175 = shl i64 %lshr171, 21
  %int_zext177 = trunc i64 %shl175 to i32
  %or179 = or i32 %and173, %int_zext177
  %and181 = and i64 %_fetch135, 1030792151040
  %lshr183 = lshr i64 %and181, 36
  %and185 = and i32 %or179, -31457281
  %shl187 = shl i64 %lshr183, 21
  %int_zext189 = trunc i64 %shl187 to i32
  %or191 = or i32 %and185, %int_zext189
  %and193 = and i32 %or191, -2031617
  %or195 = or i32 %and193, 262144
  %1 = addrspacecast i32 addrspace(4)* %_fetch133 to i32*
  %ptr_cast197 = bitcast i32* %1 to i8*
  %func_result199 = call i32 @for_dealloc_allocatable(i8* %ptr_cast197, i32 %or195)
  store i32 addrspace(4)* null, i32 addrspace(4)* addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 0)
  %_fetch201 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and203 = and i64 %_fetch201, -2
  store i64 %and203, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %_fetch205 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and207 = and i64 %_fetch205, -2049
  store i64 %and207, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %_fetch209 = load i64, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  %and211 = and i64 %_fetch209, -1030792151041
  store i64 %and211, i64 addrspace(1)* getelementptr inbounds (%"QNCA_a0$i32 addrspace(4)*$rank1$", %"QNCA_a0$i32 addrspace(4)*$rank1$" addrspace(1)* @"main_$A", i32 0, i32 3)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64 addrspace(1)* @llvm.intel.subscript.p1i64.i64.i32.p1i64.i32(i8, i64, i32, i64 addrspace(1)*, i32) #1

declare i32 @for_alloc_allocatable(i64, i8**, i32)

; Function Attrs: nounwind readnone speculatable
declare i32 addrspace(4)* @llvm.intel.subscript.p4i32.i64.i64.p4i32.i64(i8, i64, i64, i32 addrspace(4)*, i64) #1

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

declare spir_func i32 @printf(i8 addrspace(4)*, ...)

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @for_dealloc_allocatable(i8*, i32)

attributes #0 = { "contains-openmp-target"="true" "min-legal-vector-width"="0" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2055, i32 152712445, !"MAIN__", i32 6, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
