; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

; This test checks that new alloca and llvm.dbg.declare are created for kernel
; byval argument %group_pid.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@__LocalIds = internal thread_local global [3 x i64] undef, align 16

%"class.cl::sycl::group" = type { %"class.cl::sycl::range", %"class.cl::sycl::range", %"class.cl::sycl::range", %"class.cl::sycl::range" }
%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

$_ZTSN29hierarchical_private_memory__6kernelILi1EEE = comdat any

define weak_odr void @_ZTSN29hierarchical_private_memory__6kernelILi1EEE() comdat {
entry:
  call void @dummy_barrier.()
  %agg.tmp = alloca %"class.cl::sycl::group", align 8
  br label %Split.Barrier.BB1

Split.Barrier.BB1:                                ; preds = %entry
  call void @_Z18work_group_barrierj()
  call void @_ZZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_ENKUlNS6_5groupILi1EEEE_clESB_(ptr byval(%"class.cl::sycl::group") align 8 %agg.tmp)
  ret void
}

define linkonce_odr void @_ZZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_ENKUlNS6_5groupILi1EEEE_clESB_(ptr byval(%"class.cl::sycl::group") align 8 %group_pid) !dbg !14 {
entry:
; CHECK-LABEL: @_ZZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_ENKUlNS6_5groupILi1EEEE_clESB_
; CHECK: %group_pid.addr = alloca ptr, align 8

  call void @dummy_barrier.()
  br label %leader

leader:                                           ; preds = %entry
  br label %wg_leader

wg_leader:                                        ; preds = %leader
; CHECK-LABEL: wg_leader:
; CHECK: [[LOAD:%.*]] = load ptr, ptr %group_pid.addr, align 8
; CHECK-NEXT: [[LOAD1:%.*]] = load ptr, ptr [[LOAD]], align 8
; CHECK-NEXT: addrspacecast ptr [[LOAD1]] to ptr addrspace(4)
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %group_pid.addr, metadata !{{.*}}, metadata !DIExpression(DW_OP_deref, DW_OP_deref)), !dbg

  %group_pid.ascast = addrspacecast ptr %group_pid to ptr addrspace(4)
  call void @llvm.dbg.declare(metadata ptr addrspace(4) %group_pid.ascast, metadata !13, metadata !DIExpression()), !dbg !21
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj()

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!9, !10, !11, !12}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, globals: !2, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "hierarchical_private_memory.cpp", directory: "")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "group<1>", scope: !7, file: !6, line: 108, size: 256, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !2, templateParams: !2, identifier: "_ZTSN2cl4sycl5groupILi1EEE")
!6 = !DIFile(filename: "ws/barrier-arg-dbg/deploy/linux_prod/include/sycl/CL/sycl/group.hpp", directory: "")
!7 = !DINamespace(name: "sycl", scope: !8)
!8 = !DINamespace(name: "cl", scope: null, exportSymbols: true)
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"frame-pointer", i32 2}
!13 = !DILocalVariable(name: "group_pid", arg: 2, scope: !14, file: !1, line: 44, type: !5)
!14 = distinct !DISubprogram(name: "operator()", linkageName: "_ZZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_ENKUlNS6_5groupILi1EEEE_clESB_", scope: !15, file: !1, line: 44, type: !16, scopeLine: 44, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !20, retainedNodes: !2)
!15 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "_ZTSZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_EUlNS6_5groupILi1EEEE_", file: !1, line: 44, size: 8, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !2, identifier: "_ZTSZZN29hierarchical_private_memory__9check_dimILi1EEEvRN8sycl_cts4util6loggerEENKUlRN2cl4sycl7handlerEE_clES8_EUlNS6_5groupILi1EEEE_")
!16 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !17)
!17 = !{null, !18, !5}
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!19 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !15)
!20 = !DISubprogram(name: "operator()", scope: !15, file: !1, line: 44, type: !16, scopeLine: 44, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!21 = !DILocation(line: 44, column: 58, scope: !14)
