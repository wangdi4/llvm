; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-DuplicateCalledKernels -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the the DuplicateCalledKernels pass clone a called kernel.
;; The case: kernel "bar" called from kernel "foo". Module contains debug info
;; The expected result:
;;      1. Kernel bar is cloned into a new function "__internal.bar"
;;      2. "__internal.bar" is called from "foo" instead of "bar"
;;      3. Metadata was not changed.
;;*****************************************************************************

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -x cl -emit-llvm -include opencl_.h -I <Path-TO>\clang_headers\ -g -O0 -o -
;;
;;__kernel void bar(__global float* a, __global float* b) {
;;  int x = get_local_id(0);
;;  a[x] = b[x];
;;}
;;
;;__kernel void foo(__global float* a, __global float* b) {
;;  bar(a, b);
;;}



; ModuleID = 'c:\temp\debugInfo.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; CHECK: define void @bar
define void @bar(float addrspace(1)* %a, float addrspace(1)* %b) nounwind {
entry:
  %a.addr = alloca float addrspace(1)*, align 8
  %b.addr = alloca float addrspace(1)*, align 8
  %x = alloca i32, align 4
  store float addrspace(1)* %a, float addrspace(1)** %a.addr, align 8
  call void @llvm.dbg.declare(metadata !{float addrspace(1)** %a.addr}, metadata !14), !dbg !15
  store float addrspace(1)* %b, float addrspace(1)** %b.addr, align 8
  call void @llvm.dbg.declare(metadata !{float addrspace(1)** %b.addr}, metadata !16), !dbg !15
  call void @llvm.dbg.declare(metadata !{i32* %x}, metadata !17), !dbg !20
  %call = call i64 @_Z12get_local_idj(i32 0) nounwind readnone, !dbg !20
  %conv = trunc i64 %call to i32, !dbg !20
  store i32 %conv, i32* %x, align 4, !dbg !20
  %0 = load i32* %x, align 4, !dbg !21
  %idxprom = sext i32 %0 to i64, !dbg !21
  %1 = load float addrspace(1)** %b.addr, align 8, !dbg !21
  %arrayidx = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom, !dbg !21
  %2 = load float addrspace(1)* %arrayidx, align 4, !dbg !21
  %3 = load i32* %x, align 4, !dbg !21
  %idxprom1 = sext i32 %3 to i64, !dbg !21
  %4 = load float addrspace(1)** %a.addr, align 8, !dbg !21
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom1, !dbg !21
  store float %2, float addrspace(1)* %arrayidx2, align 4, !dbg !21
  ret void, !dbg !22
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare i64 @_Z12get_local_idj(i32) nounwind readnone

; CHECK: define void @foo
define void @foo(float addrspace(1)* %a, float addrspace(1)* %b) nounwind {
entry:
  %a.addr = alloca float addrspace(1)*, align 8
  %b.addr = alloca float addrspace(1)*, align 8
  store float addrspace(1)* %a, float addrspace(1)** %a.addr, align 8
  call void @llvm.dbg.declare(metadata !{float addrspace(1)** %a.addr}, metadata !23), !dbg !24
  store float addrspace(1)* %b, float addrspace(1)** %b.addr, align 8
  call void @llvm.dbg.declare(metadata !{float addrspace(1)** %b.addr}, metadata !25), !dbg !24
  %0 = load float addrspace(1)** %a.addr, align 8, !dbg !26
  %1 = load float addrspace(1)** %b.addr, align 8, !dbg !26
  call void @bar(float addrspace(1)* %0, float addrspace(1)* %1), !dbg !26
  ret void, !dbg !28
; CHECK-NOT: call void @bar
; CHECK: call void @__internal.bar
; CHECK-NOT: call void @bar
}

; CHECK: define void @__internal.bar

; CHECK: !llvm.dbg.cu = !{!0}
;;; The following check is disabled till fixing the workaround in the DuplicateCallKernels pass.
; XCHECK-NOT: __internal.bar
; CHECK: !5 = metadata !{i32 786478, i32 0, metadata !6, metadata !"bar", metadata !"bar", metadata !"", metadata !6, i32 1, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @__internal.bar, null, null, metadata !1, i32 1}

;;; The following checks that no metadata nodes were added or removed and that @bar is still a kernel.
; CHECK-NOT: !29
; CHECK: !12 = metadata !{void (float addrspace(1)*, float addrspace(1)*)* @bar}
; CHECK: !28

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!12, !13}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{i32 786449, i32 0, i32 12, metadata !"c:\5Ctemp/<unknown>", metadata !"c:\5CVolcano\5CLLVM\5C78244\5CWin64\5CDebug\5Cbin", metadata !"clang version 3.2 (tags/RELEASE_32/final 78244)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ] [c:\Volcano\LLVM\78244\Win64\Debug\bin/c:\temp/<unknown>] [DW_LANG_C99]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !11}
!5 = metadata !{i32 786478, i32 0, metadata !6, metadata !"bar", metadata !"bar", metadata !"", metadata !6, i32 1, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @bar, null, null, metadata !1, i32 1} ; [ DW_TAG_subprogram ] [line 1] [def] [bar]
!6 = metadata !{i32 786473, metadata !"c:\5Ctemp\5CdebugInfo.cl", metadata !"c:\5CVolcano\5CLLVM\5C78244\5CWin64\5CDebug\5Cbin", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 786453, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{null, metadata !9, metadata !9}
!9 = metadata !{i32 786447, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !10} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from float]
!10 = metadata !{i32 786468, null, metadata !"float", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 4} ; [ DW_TAG_base_type ] [float] [line 0, size 32, align 32, offset 0, enc DW_ATE_float]
!11 = metadata !{i32 786478, i32 0, metadata !6, metadata !"foo", metadata !"foo", metadata !"", metadata !6, i32 6, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @foo, null, null, metadata !1, i32 6} ; [ DW_TAG_subprogram ] [line 6] [def] [foo]
!12 = metadata !{void (float addrspace(1)*, float addrspace(1)*)* @bar}
!13 = metadata !{void (float addrspace(1)*, float addrspace(1)*)* @foo}
!14 = metadata !{i32 786689, metadata !5, metadata !"a", metadata !6, i32 16777217, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 1]
!15 = metadata !{i32 1, i32 0, metadata !5, null}
!16 = metadata !{i32 786689, metadata !5, metadata !"b", metadata !6, i32 33554433, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [b] [line 1]
!17 = metadata !{i32 786688, metadata !18, metadata !"x", metadata !6, i32 2, metadata !19, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [x] [line 2]
!18 = metadata !{i32 786443, metadata !5, i32 1, i32 0, metadata !6, i32 0} ; [ DW_TAG_lexical_block ] [c:\Volcano\LLVM\78244\Win64\Debug\bin/c:\temp\debugInfo.cl]
!19 = metadata !{i32 786468, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!20 = metadata !{i32 2, i32 0, metadata !18, null}
!21 = metadata !{i32 3, i32 0, metadata !18, null}
!22 = metadata !{i32 4, i32 0, metadata !18, null}
!23 = metadata !{i32 786689, metadata !11, metadata !"a", metadata !6, i32 16777222, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 6]
!24 = metadata !{i32 6, i32 0, metadata !11, null}
!25 = metadata !{i32 786689, metadata !11, metadata !"b", metadata !6, i32 33554438, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [b] [line 6]
!26 = metadata !{i32 7, i32 0, metadata !27, null}
!27 = metadata !{i32 786443, metadata !11, i32 6, i32 0, metadata !6, i32 1} ; [ DW_TAG_lexical_block ] [c:\Volcano\LLVM\78244\Win64\Debug\bin/c:\temp\debugInfo.cl]
!28 = metadata !{i32 8, i32 0, metadata !27, null}
