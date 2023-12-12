; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify,sycl-kernel-prepare-args' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers,sycl-kernel-prepare-args' -S < %s | FileCheck %s

; The test checks that global variables usages marked with the dbg_declare_inst metadata are ignored by the pass, and do not cause extra local memory allocation.
; The request size is 100 bytes. Together with 256 bytes of padding needed for vectorizer, it is 356. When using heap memory for local memory,
; it should be aligned with DEV_MAXIMUM_ALIGN, so it is 384.
; In the problematic scenario, the call to dbg_declare global in @f1 is considered a usage and causes double memory allocation, which wil result in alloca [512 .

; CHECK: select i1 [[TMP1:%.*]], i32 384, i32 0
; CHECK: %pLocalMemBase = 

; ModuleID = 'debugInfo.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

@mykernel.x = internal addrspace(3) global [100 x i8] zeroinitializer, align 1

declare void @__f1_original(ptr addrspace(3) noalias) nounwind

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare void @__mykernel_original() nounwind

declare i32 @_Z13get_global_idj(i32)

declare void @__opencl_dbg_declare_local(ptr, i64, i64, i64, i64)

declare void @__opencl_dbg_declare_global(ptr, i64, i64, i64, i64)

declare void @__opencl_dbg_enter_function(i64, i64, i64, i64)

declare void @__opencl_dbg_exit_function(i64, i64, i64, i64)

declare void @__opencl_dbg_stoppoint(i64, i64, i64, i64)

define void @f1(ptr addrspace(3) noalias %a) nounwind !kernel_arg_base_type !36 !arg_type_null_val !37 {
entry:
  %_Z13get_global_idj0 = call i32 @_Z13get_global_idj(i32 0)
  %gid0_i64 = zext i32 %_Z13get_global_idj0 to i64
  %_Z13get_global_idj1 = call i32 @_Z13get_global_idj(i32 1)
  %gid1_i64 = zext i32 %_Z13get_global_idj1 to i64
  %_Z13get_global_idj2 = call i32 @_Z13get_global_idj(i32 2)
  %gid2_i64 = zext i32 %_Z13get_global_idj2 to i64
  call void @__opencl_dbg_enter_function(i64 6683384, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %a.addr = alloca ptr addrspace(3), align 4
  store ptr addrspace(3) %a, ptr %a.addr, align 4
  call void @__opencl_dbg_stoppoint(i64 6722880, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_declare_local(ptr %a.addr, i64 6722672, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_exit_function(i64 6683384, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  ret void, !dbg !31
}

define void @mykernel(ptr addrspace(3) noalias %a) nounwind !kernel_arg_base_type !36 !arg_type_null_val !37 {
entry:
  %_Z13get_global_idj0 = call i32 @_Z13get_global_idj(i32 0)
  %gid0_i64 = zext i32 %_Z13get_global_idj0 to i64
  %_Z13get_global_idj1 = call i32 @_Z13get_global_idj(i32 1)
  %gid1_i64 = zext i32 %_Z13get_global_idj1 to i64
  %_Z13get_global_idj2 = call i32 @_Z13get_global_idj(i32 2)
  %gid2_i64 = zext i32 %_Z13get_global_idj2 to i64
  call void @__opencl_dbg_enter_function(i64 6704936, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  %var_addr = addrspacecast ptr addrspace(3) @mykernel.x to ptr, !dbg_declare_inst !30
  call void @__opencl_dbg_declare_global(ptr %var_addr, i64 6721800, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_stoppoint(i64 6705624, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @f1(ptr addrspace(3) getelementptr inbounds ([100 x i8], ptr addrspace(3) @mykernel.x, i32 0, i32 1)), !dbg !33
  call void @__opencl_dbg_stoppoint(i64 6705768, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  call void @__opencl_dbg_exit_function(i64 6704936, i64 %gid0_i64, i64 %gid1_i64, i64 %gid2_i64)
  ret void, !dbg !35
}

!llvm.dbg.cu = !{!0}
!sycl.kernels = !{!21}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!27}
!opencl.ocl.version = !{!28}
!opencl.used.extensions = !{!29}
!opencl.used.optional.core.features = !{!29}
!opencl.compiler.options = !{!29}
!llvm.functions_info = !{}

!0 = !{i32 786449, i32 0, i32 12, !"3", !"C:\5Cocl\5Ctmp", !"clang version 3.2 (tags/RELEASE_32/final 78244)", i1 true, i1 false, !"", i32 0, !1, !1, !3, !15} ; [ DW_TAG_compile_unit ] [C:\ocl\tmp/3] [DW_LANG_C99]
!1 = !{!2}
!2 = !{i32 0}
!3 = !{!4}
!4 = !{!5, !12}
!5 = !{i32 786478, i32 0, !6, !"f1", !"f1", !"", !6, i32 2, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, ptr @f1, null, null, !1, i32 2} ; [ DW_TAG_subprogram ] [line 2] [def] [f1]
!6 = !{i32 786473, !"2", !"C:\5Cocl\5Ctmp", null} ; [ DW_TAG_file_type ]
!7 = !{i32 786453, i32 0, !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null, !9}
!9 = !{i32 786447, null, !"", null, i32 0, i64 32, i64 32, i64 0, i32 0, !10} ; [ DW_TAG_pointer_type ] [line 0, size 32, align 32, offset 0] [from uchar]
!10 = !{i32 786454, null, !"uchar", !6, i32 33, i64 0, i64 0, i64 0, i32 0, !11} ; [ DW_TAG_typedef ] [uchar] [line 33, size 0, align 0, offset 0] [from unsigned char]
!11 = !{i32 786468, null, !"unsigned char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 8} ; [ DW_TAG_base_type ] [unsigned char] [line 0, size 8, align 8, offset 0, enc DW_ATE_unsigned_char]
!12 = !{i32 786478, i32 0, !6, !"mykernel", !"mykernel", !"", !6, i32 4, !13, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, ptr @mykernel, null, null, !1, i32 4} ; [ DW_TAG_subprogram ] [line 4] [def] [mykernel]
!13 = !{i32 786453, i32 0, !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, !14, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!14 = !{null}
!15 = !{!16}
!16 = !{!17}
!17 = !{i32 786484, i32 0, !12, !"x", !"x", !"", !6, i32 5, !18, i32 1, i32 1, ptr addrspace(3) @mykernel.x} ; [ DW_TAG_variable ] [x] [line 5] [local] [def]
!18 = !{i32 786433, null, !"", null, i32 0, i64 800, i64 8, i32 0, i32 0, !10, !19, i32 0, i32 0} ; [ DW_TAG_array_type ] [line 0, size 800, align 8, offset 0] [from uchar]
!19 = !{!20}
!20 = !{i32 786465, i64 0, i64 99}       ; [ DW_TAG_subrange_type ] [0, 99]
!21 = !{ptr @mykernel}
!27 = !{i32 1, i32 0}
!28 = !{i32 0, i32 0}
!29 = !{}
!30 = !{i1 true}
!31 = !{i32 2, i32 0, !32, null}
!32 = !{i32 786443, !5, i32 2, i32 0, !6, i32 0} ; [ DW_TAG_lexical_block ] [C:\ocl\tmp/2]
!33 = !{i32 6, i32 0, !34, null}
!34 = !{i32 786443, !12, i32 4, i32 0, !6, i32 1} ; [ DW_TAG_lexical_block ] [C:\ocl\tmp/2]
!35 = !{i32 7, i32 0, !34, null}
!36 = !{!"char*"}
!37 = !{ptr addrspace(3) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
