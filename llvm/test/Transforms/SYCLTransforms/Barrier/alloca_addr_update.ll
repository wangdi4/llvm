; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

; This test checks new alloca addr value is updated in the BBs where original
; alloca is not used but its addrspacecast value is used when original alloca
; is used in debugger intrinsic functions.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::group" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::range" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }
%"class.sycl::_V1::nd_item" = type { %"class.sycl::_V1::item", %"class.sycl::_V1::item.0", %"class.sycl::_V1::group" }
%"class.sycl::_V1::item" = type { %"struct.sycl::_V1::detail::ItemBase" }
%"struct.sycl::_V1::detail::ItemBase" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::item.0" = type { %"struct.sycl::_V1::detail::ItemBase.1" }
%"struct.sycl::_V1::detail::ItemBase.1" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range" }

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata)

declare i64 @_ZNK4sycl3_V16detail8ItemBaseILi1ELb1EE13get_linear_idEv(ptr addrspace(4) align 8 %this)

; Function Attrs: convergent noinline nounwind optnone
define internal void @_ZZZ4mainENKUlRT_E_clIN4sycl3_V17handlerEEEDaS0_ENKUlNS4_7nd_itemILi1EEEE_clES7_(ptr addrspace(4) align 1 %this, ptr %it) !dbg !5 {
entry:
  call void @dummy_barrier.()
  %this.addr.i = alloca ptr addrspace(4), align 8
  %Id.i = alloca i64, align 8
  %this.addr = alloca ptr addrspace(4), align 8
  %glid = alloca i32, align 4
  %my_i = alloca i32, align 4
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  %glid.ascast = addrspacecast ptr %glid to ptr addrspace(4)
  %my_i.ascast = addrspacecast ptr %my_i to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %it.ascast = addrspacecast ptr %it to ptr addrspace(4)
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata ptr %glid, metadata !22, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !25
  %this.addr.ascast.i = addrspacecast ptr %this.addr.i to ptr addrspace(4)
  %Id.ascast.i = addrspacecast ptr %Id.i to ptr addrspace(4)
  store ptr addrspace(4) %it.ascast, ptr addrspace(4) %this.addr.ascast.i, align 8
  %this1.i = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast.i, align 8
  %globalItem.i = getelementptr inbounds %"class.sycl::_V1::nd_item", ptr addrspace(4) %this1.i, i32 0, i32 0
  store ptr addrspace(4) %globalItem.i, ptr addrspace(4) %this.addr.ascast.i, align 8
  %this1.i8 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast.i, align 8
  %MImpl.i = getelementptr inbounds %"class.sycl::_V1::item", ptr addrspace(4) %this1.i8, i32 0, i32 0
  %call.i9 = call i64 @_ZNK4sycl3_V16detail8ItemBaseILi1ELb1EE13get_linear_idEv(ptr addrspace(4) align 8 %MImpl.i)
  store i64 %call.i9, ptr addrspace(4) %Id.ascast.i, align 8
  %0 = load i64, ptr addrspace(4) %Id.ascast.i, align 8
  %conv = trunc i64 %0 to i32
  store i32 %conv, ptr addrspace(4) %glid.ascast, align 4, !dbg !25
  br label %Split.Barrier.BB10

Split.Barrier.BB10:                               ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  br label %Split.Barrier.BB11

Split.Barrier.BB11:                               ; preds = %Split.Barrier.BB10
; Check new alloca addr is updated.
; CHECK-LABEL: SyncBB3:
; CEHCK-NEXT: [[SBIndex:%SBIndex[0-9]*]] = load i64, ptr %pCurrSBIndex, align 8
; CEHCK-NEXT: [[SB_LocalId_Offset1:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBIndex]],
; CEHCK-NEXT: [[PSB_LocalId1:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset1]]
; CEHCK-NEXT: store ptr [[PSB_LocalId1]], ptr %my_i.addr, align 8
; CEHCK-NEXT: [[SB_LocalId_Offset2:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBIndex]]
; CEHCK-NEXT: [[PSB_LocalId2:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset2]]
; CEHCK-NEXT: store ptr [[PSB_LocalId2]], ptr %glid.addr, align 8
  call void @dummy_barrier.()
  call void @llvm.dbg.declare(metadata ptr %my_i, metadata !24, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !26
; CEHCK-NEXT: call void @llvm.dbg.declare(metadata ptr %my_i.addr, metadata !{{[0-9]*}}, metadata !DIExpression(DW_OP_deref, DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg
; CEHCK-NEXT: [[SB_LocalId_Offset3:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBIndex]],
; CEHCK-NEXT: [[pSB_LocalId3:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset3]]
; CEHCK-NEXT: [[loadedValue0:%loadedValue[0-9]*]] = load ptr addrspace(4), ptr [[pSB_LocalId3]], align 8
; CEHCK-NEXT: [[glid:%[0-9]*]] = load i32, ptr addrspace(4) [[loadedValue0]], align 4
; CEHCK-NEXT: [[SB_LocalId_Offset4:%SB_LocalId_Offset[0-9]*]] = add nuw i64 [[SBIndex]],
; CEHCK-NEXT: [[pSB_LocalId4:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[SB_LocalId_Offset4]]
; CEHCK-NEXT: [[my_i:%loadedValue[0-9]*]] = load ptr addrspace(4), ptr [[pSB_LocalId4]], align 8
; CEHCK-NEXT: store i32 [[glid]], ptr addrspace(4) [[my_i]], align 4
  %1 = load i32, ptr addrspace(4) %glid.ascast, align 4
  store i32 %1, ptr addrspace(4) %my_i.ascast, align 4, !dbg !26
  br label %Split.Barrier.BB

Split.Barrier.BB:                                 ; preds = %Split.Barrier.BB11
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

declare void @dummy_barrier.()

declare void @_Z18work_group_barrierj(i32)

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}

!0 = !{i32 7, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4)
!3 = !DIFile(filename: "test.cpp", directory: "")
!4 = !{}
!5 = distinct !DISubprogram(name: "operator()", linkageName: "_ZZZ4mainENKUlRT_E_clIN4sycl3_V17handlerEEEDaS0_ENKUlNS4_7nd_itemILi1EEEE_clES7_", scope: !6, file: !3, line: 8, type: !7, scopeLine: 8, flags: DIFlagPrivate | DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !2, templateParams: !4, declaration: !18, retainedNodes: !19)
!6 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "_ZTSZZ4mainENKUlRT_E_clIN4sycl3_V17handlerEEEDaS0_EUlNS4_7nd_itemILi1EEEE_", file: !3, line: 8, size: 8, flags: DIFlagTypePassByValue, elements: !4)
!7 = !DISubroutineType(types: !8)
!8 = !{null, !9, !11}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer, dwarfAddressSpace: 4)
!10 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !6)
!11 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "nd_item<1>", scope: !13, file: !12, line: 40, size: 576, flags: DIFlagTypePassByValue, elements: !4, templateParams: !15, identifier: "_ZTSN4sycl3_V17nd_itemILi1EEE")
!12 = !DIFile(filename: "nd_item.hpp", directory: "")
!13 = !DINamespace(name: "_V1", scope: !14)
!14 = !DINamespace(name: "sycl", scope: null)
!15 = !{!16}
!16 = !DITemplateValueParameter(name: "Dimensions", type: !17, value: i32 1)
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !DISubprogram(name: "operator()", scope: !6, file: !3, line: 8, type: !7, scopeLine: 8, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagLocalToUnit, templateParams: !4)
!19 = !{!20, !22, !24}
!20 = !DILocalVariable(name: "this", arg: 1, scope: !5, type: !21, flags: DIFlagArtificial | DIFlagObjectPointer)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64, dwarfAddressSpace: 4)
!22 = !DILocalVariable(name: "glid", scope: !5, file: !3, line: 9, type: !23)
!23 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!24 = !DILocalVariable(name: "my_i", scope: !5, file: !3, line: 11, type: !17)
!25 = !DILocation(line: 9, column: 17, scope: !5)
!26 = !DILocation(line: 11, column: 8, scope: !5)
