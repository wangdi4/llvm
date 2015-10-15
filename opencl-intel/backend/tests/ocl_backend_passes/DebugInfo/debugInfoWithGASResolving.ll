; RUN: opt -generic-addr-static-resolution -verify -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -x cl -emit-llvm -g -O0 -cl-std=CL2.0
;;
;;==========================================
;;int func(__generic int* pInt)
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
; The test checks that GenericAddressStaticResolution pass copies DW_TAG_subprogram along with resolved func. 
;
; CHECK: {{0x2e\\00func\\00func\\00\\001\\000\\001\\000\\000\\00256\\000\\002.*@_Z4funcPi}}

; ModuleID = '/tmp/ker.cl'
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"

; Function Attrs: nounwind
define spir_func i32 @func(i32 addrspace(4)* %pInt) #0 {
  %1 = alloca i32 addrspace(4)*, align 4
  store i32 addrspace(4)* %pInt, i32 addrspace(4)** %1, align 4
  call void @llvm.dbg.declare(metadata i32 addrspace(4)** %1, metadata !25, metadata !26), !dbg !27
  %2 = load i32 addrspace(4)** %1, align 4, !dbg !28
  %3 = load i32 addrspace(4)* %2, align 4, !dbg !28
  ret i32 %3, !dbg !28
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
define spir_kernel void @invoke_func() #0 {
  %integer = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %integer, metadata !29, metadata !26), !dbg !30
  store i32 42, i32* %integer, align 4, !dbg !30
  %1 = addrspacecast i32* %integer to i32 addrspace(4)*, !dbg !31
  %2 = call spir_func i32 @func(i32 addrspace(4)* %1), !dbg !31
  store i32 %2, i32* %integer, align 4, !dbg !31
  ret void, !dbg !32
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!14}
!llvm.module.flags = !{!20, !21}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!22}
!opencl.ocl.version = !{!23}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!24}

!0 = !{!"0x11\0012\00clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)\000\00\000\00\001", !1, !2, !2, !3, !2, !2} ; [ DW_TAG_compile_unit ] [/tmp//tmp/<stdin>] [DW_LANG_C99]
!1 = !{!"/tmp/<stdin>", !"/tmp"}
!2 = !{}
!3 = !{!4, !11}
!4 = !{!"0x2e\00func\00func\00\001\000\001\000\000\00256\000\002", !5, !6, !7, null, i32 (i32 addrspace(4)*)* @func, null, null, !2} ; [ DW_TAG_subprogram ] [line 1] [def] [scope 2] [func]
!5 = !{!"/tmp/ker.cl", !"/tmp"}
!6 = !{!"0x29", !5}                               ; [ DW_TAG_file_type ] [/tmp//tmp/ker.cl]
!7 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !8, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{!9, !10}
!9 = !{!"0x24\00int\000\0032\0032\000\000\005", null, null} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!10 = !{!"0xf\00\000\0032\0032\000\000", null, null, !9} ; [ DW_TAG_pointer_type ] [line 0, size 32, align 32, offset 0] [from int]
!11 = !{!"0x2e\00invoke_func\00invoke_func\00\005\000\001\000\000\000\000\006", !5, !6, !12, null, void ()* @invoke_func, null, null, !2} ; [ DW_TAG_subprogram ] [line 5] [def] [scope 6] [invoke_func]
!12 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !13, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = !{null}
!14 = !{void ()* @invoke_func, !15, !16, !17, !18, !19}
!15 = !{!"kernel_arg_addr_space"}
!16 = !{!"kernel_arg_access_qual"}
!17 = !{!"kernel_arg_type"}
!18 = !{!"kernel_arg_base_type"}
!19 = !{!"kernel_arg_type_qual"}
!20 = !{i32 2, !"Dwarf Version", i32 4}
!21 = !{i32 2, !"Debug Info Version", i32 2}
!22 = !{i32 1, i32 2}
!23 = !{i32 2, i32 0}
!24 = !{!"clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)"}
!25 = !{!"0x101\00pInt\0016777217\000", !4, !6, !10} ; [ DW_TAG_arg_variable ] [pInt] [line 1]
!26 = !{!"0x102"}                                 ; [ DW_TAG_expression ]
!27 = !MDLocation(line: 1, scope: !4)
!28 = !MDLocation(line: 3, scope: !4)
!29 = !{!"0x100\00integer\007\000", !11, !6, !9}  ; [ DW_TAG_auto_variable ] [integer] [line 7]
!30 = !MDLocation(line: 7, scope: !11)
!31 = !MDLocation(line: 8, scope: !11)
!32 = !MDLocation(line: 9, scope: !11)
