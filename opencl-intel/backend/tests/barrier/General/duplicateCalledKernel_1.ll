; [LLVM 3.6 UPGRADE] FIXME: Temprorary disable this test. Enable as soon as clang
;                           is able to produce compatible debug info.
; RUN_x: opt -B-DuplicateCalledKernels -verify -S < %s | FileCheck %s

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



; ModuleID = ''
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

; CHECK: define void @bar
; Function Attrs: nounwind
define void @bar(float addrspace(1)* %a, float addrspace(1)* %b) #0 {
entry:
  %a.addr = alloca float addrspace(1)*, align 8
  %b.addr = alloca float addrspace(1)*, align 8
  %x = alloca i32, align 4
  store float addrspace(1)* %a, float addrspace(1)** %a.addr, align 8
  call void @llvm.dbg.declare(!{float addrspace(1)** %a.addr}, !15), !dbg !16
  store float addrspace(1)* %b, float addrspace(1)** %b.addr, align 8
  call void @llvm.dbg.declare(!{float addrspace(1)** %b.addr}, !17), !dbg !16
  call void @llvm.dbg.declare(!{i32* %x}, !18), !dbg !20
  %call = call i64 @_Z12get_local_idj(i32 0) #1, !dbg !20
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #1

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) #2

; CHECK: define void @foo
define void @foo(float addrspace(1)* %a, float addrspace(1)* %b) #0 {
entry:
  %a.addr = alloca float addrspace(1)*, align 8
  %b.addr = alloca float addrspace(1)*, align 8
  store float addrspace(1)* %a, float addrspace(1)** %a.addr, align 8
  call void @llvm.dbg.declare(!{float addrspace(1)** %a.addr}, !23), !dbg !24
  store float addrspace(1)* %b, float addrspace(1)** %b.addr, align 8
  call void @llvm.dbg.declare(!{float addrspace(1)** %b.addr}, !25), !dbg !24
  %0 = load float addrspace(1)** %a.addr, align 8, !dbg !26
  %1 = load float addrspace(1)** %b.addr, align 8, !dbg !26
  call void @bar(float addrspace(1)* %0, float addrspace(1)* %1), !dbg !26
  ret void, !dbg !27
; CHECK-NOT: call void @bar
; CHECK: call void @__internal.bar
; CHECK-NOT: call void @bar
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

; CHECK: define void @__internal.bar

; CHECK: !llvm.dbg.cu = !{!0}

;;; Check that that debug info metadata for the old function was changed to the
;;; new function, but that there's a new debug metadata for the old function.
; CHECK: !3 = !{!4, !11, metadata [[NewMD:![0-9]+]]}
; CHECK: !4 = !{i32 786478, !5, !6, !"bar", !"bar", !"", i32 3, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @__internal.bar, null, null, !2, i32 3}
; CHECK: [[NewMD]] = !{i32 786478, !5, !6, !"bar", !"bar", !"", i32 3, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @bar, null, null, !2, i32 3}

;;; The following checks that exactly one metadata node was added and that @bar is still a kernel.
; CHECK-NOT: !29
; CHECK: !{void (float addrspace(1)*, float addrspace(1)*)* @bar}
; CHECK: !28

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!12, !13}
!opencl.compiler.options = !{!14}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{i32 786449, !1, i32 12, !"clang version 3.4 ", i1 false, !"", i32 0, !2, !2, !3, !2, !2, !""} ; [ DW_TAG_compile_unit ] [] [DW_LANG_C99]
!1 = !{!"OutputFileName", !"WorkingDir"}
!2 = !{i32 0}
!3 = !{!4, !11}
!4 = !{i32 786478, !5, !6, !"bar", !"bar", !"", i32 3, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @bar, null, null, !2, i32 3} ; [ DW_TAG_subprogram ] [line 3] [def] [bar]
!5 = !{!"InputFileName", !"WorkingDir"}
!6 = !{i32 786473, !5}          ; [ DW_TAG_file_type ] []
!7 = !{i32 786453, i32 0, i32 0, !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null, !9, !9}
!9 = !{i32 786447, null, null, !"", i32 0, i64 64, i64 64, i64 0, i32 0, !10} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from float]
!10 = !{i32 786468, null, null, !"float", i32 0, i64 32, i64 32, i64 0, i32 0, i32 4} ; [ DW_TAG_base_type ] [float] [line 0, size 32, align 32, offset 0, enc DW_ATE_float]
!11 = !{i32 786478, !5, !6, !"foo", !"foo", !"", i32 8, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (float addrspace(1)*, float addrspace(1)*)* @foo, null, null, !2, i32 8} ; [ DW_TAG_subprogram ] [line 8] [def] [foo]
!12 = !{void (float addrspace(1)*, float addrspace(1)*)* @bar}
!13 = !{void (float addrspace(1)*, float addrspace(1)*)* @foo}
!14 = !{!"-cl-std=CL1.2"}
!15 = !{i32 786689, !4, !"a", !6, i32 16777219, !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 3]
!16 = !{i32 3, i32 0, !4, null}
!17 = !{i32 786689, !4, !"b", !6, i32 33554435, !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [b] [line 3]
!18 = !{i32 786688, !4, !"x", !6, i32 4, !19, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [x] [line 4]
!19 = !{i32 786468, null, null, !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!20 = !{i32 4, i32 0, !4, null}
!21 = !{i32 5, i32 0, !4, null}
!22 = !{i32 6, i32 0, !4, null}
!23 = !{i32 786689, !11, !"a", !6, i32 16777224, !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 8]
!24 = !{i32 8, i32 0, !11, null} ; [ DW_TAG_imported_declaration ]
!25 = !{i32 786689, !11, !"b", !6, i32 33554440, !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [b] [line 8]
!26 = !{i32 9, i32 0, !11, null}
!27 = !{i32 10, i32 0, !11, null}
