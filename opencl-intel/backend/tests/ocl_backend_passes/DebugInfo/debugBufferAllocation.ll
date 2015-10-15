; RUN: opt -debug-info -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -x cl -emit-llvm -triple=spir64-unknown-unknown -g -O0 -o -
;;
;;void f1(__local char* a) {
;;}
;;
;;__kernel void mykernel() {
;;  __local char x[100];
;;  f1(x);
;;}

; The test checks that debug info (specifically calls to dbg_declare_global) do not cause extra local memory to be allocated.
; The request size is 100 bytes. Together with 256 bytes of padding needed for vectorizer, it is 356, which we expect to be rounded to 384 (128*3).
; In the problematic scenario, the call to dbg_declare global in @f1 causes double memory allocation, which wil result in alloca [512 .
; CHECK-NOT: alloca [
; CHECK: alloca [384
; CHECK-NOT: alloca [

; ModuleID = 'ker.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

@mykernel.x = internal addrspace(3) global [100 x i8] undef, align 1

; Function Attrs: nounwind
define spir_func void @f1(i8 addrspace(3)* %a) #0 {
  %1 = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(3)* %a, i8 addrspace(3)** %1, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(3)** %1, metadata !29, metadata !30), !dbg !31
  ret void, !dbg !32
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
define spir_kernel void @mykernel() #0 {
  call spir_func void @f1(i8 addrspace(3)* getelementptr inbounds ([100 x i8] addrspace(3)* @mykernel.x, i32 0, i32 0)), !dbg !33
  ret void, !dbg !34
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!19}
!llvm.module.flags = !{!25, !26}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!27}
!opencl.ocl.version = !{!27}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!28}

!0 = !{!"0x11\0012\00clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)\000\00\000\00\001", !1, !2, !2, !3, !14, !2} ; [ DW_TAG_compile_unit ] [/tmp/<stdin>] [DW_LANG_C99]
!1 = !{!"<stdin>", !"/tmp"}
!2 = !{}
!3 = !{!4, !11}
!4 = !{!"0x2e\00f1\00f1\00\001\000\001\000\000\00256\000\001", !5, !6, !7, null, void (i8 addrspace(3)*)* @f1, null, null, !2} ; [ DW_TAG_subprogram ] [line 1] [def] [f1]
!5 = !{!"ker.cl", !"/tmp"}
!6 = !{!"0x29", !5}                               ; [ DW_TAG_file_type ] [/tmp/ker.cl]
!7 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !8, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null, !9}
!9 = !{!"0xf\00\000\0064\0064\000\000", null, null, !10} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from char]
!10 = !{!"0x24\00char\000\008\008\000\000\006", null, null} ; [ DW_TAG_base_type ] [char] [line 0, size 8, align 8, offset 0, enc DW_ATE_signed_char]
!11 = !{!"0x2e\00mykernel\00mykernel\00\004\000\001\000\000\000\000\004", !5, !6, !12, null, void ()* @mykernel, null, null, !2} ; [ DW_TAG_subprogram ] [line 4] [def] [mykernel]
!12 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !13, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = !{null}
!14 = !{!15}
!15 = !{!"0x34\00x\00x\00\005\001\001", !11, !6, !16, [100 x i8] addrspace(3)* @mykernel.x, null} ; [ DW_TAG_variable ] [x] [line 5] [local] [def]
!16 = !{!"0x1\00\000\00800\008\000\000\000", null, null, !10, !17, null, null, null} ; [ DW_TAG_array_type ] [line 0, size 800, align 8, offset 0] [from char]
!17 = !{!18}
!18 = !{!"0x21\000\00100"}                        ; [ DW_TAG_subrange_type ] [0, 99]
!19 = !{void ()* @mykernel, !20, !21, !22, !23, !24}
!20 = !{!"kernel_arg_addr_space"}
!21 = !{!"kernel_arg_access_qual"}
!22 = !{!"kernel_arg_type"}
!23 = !{!"kernel_arg_base_type"}
!24 = !{!"kernel_arg_type_qual"}
!25 = !{i32 2, !"Dwarf Version", i32 4}
!26 = !{i32 2, !"Debug Info Version", i32 2}
!27 = !{i32 1, i32 2}
!28 = !{!"clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)"}
!29 = !{!"0x101\00a\0016777217\000", !4, !6, !9}  ; [ DW_TAG_arg_variable ] [a] [line 1]
!30 = !{!"0x102"}                                 ; [ DW_TAG_expression ]
!31 = !MDLocation(line: 1, scope: !4)
!32 = !MDLocation(line: 2, scope: !4)
!33 = !MDLocation(line: 6, scope: !11)
!34 = !MDLocation(line: 7, scope: !11)
