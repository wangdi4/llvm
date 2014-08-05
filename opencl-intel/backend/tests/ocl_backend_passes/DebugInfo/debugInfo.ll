; RUN_x: opt -debug-info -S %s | FileCheck %s
; Temporary disable this test.
; TODO: enable it as soon as clang is produce compatible debug info.

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -x cl -emit-llvm -include opencl_.h -I <Path-TO>\clang_headers\ -g -O0 -o -
;;
;;void f1(__local char* a) {
;;}
;;
;;__kernel void mykernel() {
;;  __local char x[100];
;;  f1(x);
;;}

; The test checks that debugInfo pass adds metadata to variables passed to dbg_declare_global.
; We have 2 functions, so the call should appear twice.
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]
; CHECK: [[VAR:%[a-zA-Z_]+]] = {{.*}}!dbg_declare_inst
; CHECK: call void @__opencl_dbg_declare_global{{.*}}[[VAR]]

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@mykernel.x = internal addrspace(3) global [100 x i8] zeroinitializer, align 16

; Function Attrs: nounwind
define void @f1(i8 addrspace(3)* %a) #0 {
entry:
  %a.addr = alloca i8 addrspace(3)*, align 8
  store i8 addrspace(3)* %a, i8 addrspace(3)** %a.addr, align 8
  call void @llvm.dbg.declare(metadata !{i8 addrspace(3)** %a.addr}, metadata !21), !dbg !22
  ret void, !dbg !23
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #1

; Function Attrs: nounwind
define void @mykernel() #0 {
entry:
  call void @f1(i8 addrspace(3)* getelementptr inbounds ([100 x i8] addrspace(3)* @mykernel.x, i32 0, i32 0)), !dbg !24
  ret void, !dbg !25
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!19}
!opencl.compiler.options = !{!20}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.4 ", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !14, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [/home/bader/git/llvm/install/Debug/bin//tmp/<unknown>] [DW_LANG_C99]
!1 = metadata !{metadata !"/tmp/<unknown>", metadata !"/home/bader/git/llvm/install/Debug/bin"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !11}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"f1", metadata !"f1", metadata !"", i32 1, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i8 addrspace(3)*)* @f1, null, null, metadata !2, i32 1} ; [ DW_TAG_subprogram ] [line 1] [def] [f1]
!5 = metadata !{metadata !"/tmp/test.cl", metadata !"/home/bader/git/llvm/install/Debug/bin"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [/home/bader/git/llvm/install/Debug/bin//tmp/test.cl]
!7 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{null, metadata !9}
!9 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !10} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from char]
!10 = metadata !{i32 786468, null, null, metadata !"char", i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ] [char] [line 0, size 8, align 8, offset 0, enc DW_ATE_signed_char]
!11 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"mykernel", metadata !"mykernel", metadata !"", i32 4, metadata !12, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void ()* @mykernel, null, null, metadata !2, i32 4} ; [ DW_TAG_subprogram ] [line 4] [def] [mykernel]
!12 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !13, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = metadata !{null}
!14 = metadata !{metadata !15}
!15 = metadata !{i32 786484, i32 0, metadata !11, metadata !"x", metadata !"x", metadata !"", metadata !6, i32 5, metadata !16, i32 1, i32 1, [100 x i8] addrspace(3)* @mykernel.x, null} ; [ DW_TAG_variable ] [x] [line 5] [local] [def]
!16 = metadata !{i32 786433, null, null, metadata !"", i32 0, i64 800, i64 8, i32 0, i32 0, metadata !10, metadata !17, i32 0, i32 0} ; [ DW_TAG_array_type ] [line 0, size 800, align 8, offset 0] [from char]
!17 = metadata !{metadata !18}
!18 = metadata !{i32 786465, i64 0, i64 100}      ; [ DW_TAG_subrange_type ] [0, 99]
!19 = metadata !{void ()* @mykernel}
!20 = metadata !{metadata !"-cl-std=CL1.2"}
!21 = metadata !{i32 786689, metadata !4, metadata !"a", metadata !6, i32 16777217, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 1]
!22 = metadata !{i32 1, i32 0, metadata !4, null}
!23 = metadata !{i32 2, i32 0, metadata !4, null}
!24 = metadata !{i32 6, i32 0, metadata !11, null}
!25 = metadata !{i32 7, i32 0, metadata !11, null}
