; RUN: opt -generic-addr-static-resolution -verify -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -x cl -emit-llvm -g -O0
;;
;;==========================================
;;int func(int* pInt)
;;{
;;    return *pInt;
;;}
;;__kernel void invoke_func()
;;{
;;    int integer = 42;
;;    integer = func(&integer);
;;}
;;==========================================
;;
; The test checks that GenericAddressStaticResolution pass copy debug metadata along with resolved func.
;
; CHECK: {{.*_Z4funcPi.*DW_TAG_subprogram.*}}
; CHECK: {{.*_Z4funcPi.*DW_TAG_subroutine_type.*}}
; CHECK: {{.*_Z4funcPi.*DW_TAG_pointer_type.*}}
; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

; Function Attrs: nounwind
define i32 @func(i32 addrspace(4)* %pInt) #0 {
entry:
  %pInt.addr = alloca i32 addrspace(4)*, align 4
  store i32 addrspace(4)* %pInt, i32 addrspace(4)** %pInt.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32 addrspace(4)** %pInt.addr}, metadata !16), !dbg !17
  %0 = load i32 addrspace(4)** %pInt.addr, align 4, !dbg !18
  %1 = load i32 addrspace(4)* %0, align 4, !dbg !18
  ret i32 %1, !dbg !18
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #1

; Function Attrs: nounwind
define void @invoke_func() #0 {
entry:
  %integer = alloca i32, align 4
  call void @llvm.dbg.declare(metadata !{i32* %integer}, metadata !19), !dbg !20
  store i32 42, i32* %integer, align 4, !dbg !20
  %0 = bitcast i32* %integer to i32 addrspace(4)*, !dbg !21
  %call = call i32 @func(i32 addrspace(4)* %0), !dbg !21
  store i32 %call, i32* %integer, align 4, !dbg !21
  ret void, !dbg !22
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!14}
!opencl.compiler.options = !{!15}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.4 ", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [D:\vromanov\ocl/D:\vromanov\cygwin\tmp/<unknown>] [DW_LANG_C99]
!1 = metadata !{metadata !"D:\5Cvromanov\5Ccygwin\5Ctmp/<unknown>", metadata !"D:\5Cvromanov\5Cocl"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !11}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"func", metadata !"func", metadata !"", i32 1, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, i32 (i32 addrspace(4)*)* @func, null, null, metadata !2, i32 2} ; [ DW_TAG_subprogram ] [line 1] [def] [scope 2] [func]
!5 = metadata !{metadata !"D:\5Cvromanov\5Ccygwin\5Ctmp\5Ctest_for_GAS_resolv.cl", metadata !"D:\5Cvromanov\5Cocl"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [D:\vromanov\ocl/D:\vromanov\cygwin\tmp\test_for_GAS_resolv.cl]
!7 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{metadata !9, metadata !10}
!9 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!10 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 32, i64 32, i64 0, i32 0, metadata !9} ; [ DW_TAG_pointer_type ] [line 0, size 32, align 32, offset 0] [from int]
!11 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"invoke_func", metadata !"invoke_func", metadata !"", i32 6, metadata !12, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void ()* @invoke_func, null, null, metadata !2, i32 7} ; [ DW_TAG_subprogram ] [line 6] [def] [scope 7] [invoke_func]
!12 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !13, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = metadata !{null}
!14 = metadata !{void ()* @invoke_func}
!15 = metadata !{metadata !"-cl-std=CL2.0"}
!16 = metadata !{i32 786689, metadata !4, metadata !"pInt", metadata !6, i32 16777217, metadata !10, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [pInt] [line 1]
!17 = metadata !{i32 1, i32 0, metadata !4, null}
!18 = metadata !{i32 3, i32 0, metadata !4, null}
!19 = metadata !{i32 786688, metadata !11, metadata !"integer", metadata !6, i32 8, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [integer] [line 8]
!20 = metadata !{i32 8, i32 0, metadata !11, null} ; [ DW_TAG_imported_declaration ]
!21 = metadata !{i32 9, i32 0, metadata !11, null}
!22 = metadata !{i32 10, i32 0, metadata !11, null}
