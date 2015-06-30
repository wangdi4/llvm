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
  call void @llvm.dbg.declare(!{i8 addrspace(3)** %a.addr}, !21), !dbg !22
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

!0 = !{i32 786449, !1, i32 12, !"clang version 3.4 ", i1 false, !"", i32 0, !2, !2, !3, !14, !2, !""} ; [ DW_TAG_compile_unit ] [/home/bader/git/llvm/install/Debug/bin//tmp/<unknown>] [DW_LANG_C99]
!1 = !{!"/tmp/<unknown>", !"/home/bader/git/llvm/install/Debug/bin"}
!2 = !{i32 0}
!3 = !{!4, !11}
!4 = !{i32 786478, !5, !6, !"f1", !"f1", !"", i32 1, !7, i1 false, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i8 addrspace(3)*)* @f1, null, null, !2, i32 1} ; [ DW_TAG_subprogram ] [line 1] [def] [f1]
!5 = !{!"/tmp/test.cl", !"/home/bader/git/llvm/install/Debug/bin"}
!6 = !{i32 786473, !5}          ; [ DW_TAG_file_type ] [/home/bader/git/llvm/install/Debug/bin//tmp/test.cl]
!7 = !{i32 786453, i32 0, i32 0, !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null, !9}
!9 = !{i32 786447, null, null, !"", i32 0, i64 64, i64 64, i64 0, i32 0, !10} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from char]
!10 = !{i32 786468, null, null, !"char", i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ] [char] [line 0, size 8, align 8, offset 0, enc DW_ATE_signed_char]
!11 = !{i32 786478, !5, !6, !"mykernel", !"mykernel", !"", i32 4, !12, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void ()* @mykernel, null, null, !2, i32 4} ; [ DW_TAG_subprogram ] [line 4] [def] [mykernel]
!12 = !{i32 786453, i32 0, i32 0, !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, !13, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!13 = !{null}
!14 = !{!15}
!15 = !{i32 786484, i32 0, !11, !"x", !"x", !"", !6, i32 5, !16, i32 1, i32 1, [100 x i8] addrspace(3)* @mykernel.x, null} ; [ DW_TAG_variable ] [x] [line 5] [local] [def]
!16 = !{i32 786433, null, null, !"", i32 0, i64 800, i64 8, i32 0, i32 0, !10, !17, i32 0, i32 0} ; [ DW_TAG_array_type ] [line 0, size 800, align 8, offset 0] [from char]
!17 = !{!18}
!18 = !{i32 786465, i64 0, i64 100}      ; [ DW_TAG_subrange_type ] [0, 99]
!19 = !{void ()* @mykernel}
!20 = !{!"-cl-std=CL1.2"}
!21 = !{i32 786689, !4, !"a", !6, i32 16777217, !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ] [a] [line 1]
!22 = !{i32 1, i32 0, !4, null}
!23 = !{i32 2, i32 0, !4, null}
!24 = !{i32 6, i32 0, !11, null}
!25 = !{i32 7, i32 0, !11, null}
