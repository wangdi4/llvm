; RUN: opt --cloneblockinvokefunctokernel -verify -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; Ticket ID : CSSD100020567
;; This test was generated using the following cl code with this command:
;;  clang -cc1 -cl-std=CL2.0 -x cl -emit-llvm -include opencl_.h -I <Path-TO>/clang_headers/ -g -O0 -D__OPENCL_C_VERSION__=200 -o result.ll
;;
;;==========================================
;; __kernel void ker()
;; {
;;     enqueue_kernel( get_default_queue(),
;;                     CLK_ENQUEUE_FLAGS_WAIT_KERNEL,
;;                     ndrange_1D(4),
;;                     ^{
;;                         size_t id = 4;
;;                     });
;; }
;;==========================================
;;
;; The test checks that pass "CloneBlockInvokeFuncToKernel" copy debug info when create new function.
;
; CHECK: {{.*@__.kernel__ker_block_invoke.*DW_TAG_subprogram.*}}
; ModuleID = 'reproducer_CloneBlockToKernel.cl'

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__block_descriptor = type { i64, i64 }
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque

@_NSConcreteGlobalBlock = external global i8*
@.str = private addrspace(2) unnamed_addr constant [6 x i8] c"v8@?0\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* bitcast ([6 x i8] addrspace(2)* @.str to i8*), i8* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (void (i8*)* @__ker_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8

; Function Attrs: nounwind
define void @ker() #0 {
  %1 = alloca %struct.ndrange_t, align 8
  %2 = call %opencl.queue_t* @get_default_queue() #3, !dbg !15
  call void @_Z10ndrange_1Dm(%struct.ndrange_t* sret %1, i64 4) #4, !dbg !16
  %3 = call i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t* %2, i32 0, %struct.ndrange_t* %1, void ()* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to void ()*)), !dbg !16
  ret void, !dbg !17
}

declare i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t*, void ()*) #1

; Function Attrs: nounwind readnone
declare %opencl.queue_t* @get_default_queue() #2

; Function Attrs: nounwind
declare void @_Z10ndrange_1Dm(%struct.ndrange_t* sret, i64) #0

; Function Attrs: nounwind
define internal void @__ker_block_invoke(i8* %.block_descriptor) #0 {
  %1 = alloca i8*, align 8
  %2 = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, align 8
  %id = alloca i64, align 8
  store i8* %.block_descriptor, i8** %1, align 8
  %3 = load i8** %1
  call void @llvm.dbg.value(!{i8* %3}, i64 0, !18), !dbg !30
  call void @llvm.dbg.declare(!{i8* %.block_descriptor}, !18), !dbg !30
  %4 = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, !dbg !30
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>* %4, <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>** %2, align 8
  call void @llvm.dbg.declare(!{i64* %id}, !31), !dbg !36
  store i64 4, i64* %id, align 8, !dbg !36
  ret void, !dbg !37
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata) #3

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata) #3

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!13}
!opencl.compiler.options = !{!14}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{i32 786449, !1, i32 12, !"clang version 3.4 ", i1 false, !"", i32 0, !2, !2, !3, !2, !2, !""} ; [ DW_TAG_compile_unit ] [/home/vromanov/OCL_SDK//tmp/<unknown>] [DW_LANG_C99]
!1 = !{!"/tmp/<unknown>", !"/home/vromanov/OCL_SDK"}
!2 = !{i32 0}
!3 = !{!4, !9}
!4 = !{i32 786478, !5, !6, !"ker", !"ker", !"", i32 1, !7, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void ()* @ker, null, null, !2, i32 2} ; [ DW_TAG_subprogram ] [line 1] [def] [scope 2] [ker]
!5 = !{!"/tmp/reproducer_CloneBlockToKernel.cl", !"/home/vromanov/OCL_SDK"}
!6 = !{i32 786473, !5}          ; [ DW_TAG_file_type ] [/home/vromanov/OCL_SDK//tmp/reproducer_CloneBlockToKernel.cl]
!7 = !{i32 786453, i32 0, i32 0, !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null}
!9 = !{i32 786478, !5, !6, !"__ker_block_invoke", !"__ker_block_invoke", !"", i32 6, !10, i1 true, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i8*)* @__ker_block_invoke, null, null, !2, i32 6} ; [ DW_TAG_subprogram ] [line 6] [local] [def] [__ker_block_invoke]
!10 = !{i32 786453, i32 0, i32 0, !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, !11, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!11 = !{null, !12}
!12 = !{i32 786447, null, null, !"", i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!13 = !{void ()* @ker}
!14 = !{!"-cl-std=CL2.0"}
!15 = !{i32 3, i32 0, !4, null}
!16 = !{i32 5, i32 0, !4, null}
!17 = !{i32 9, i32 0, !4, null}
!18 = !{i32 786689, !9, !".block_descriptor", !6, i32 16777222, !19, i32 64, i32 0} ; [ DW_TAG_arg_variable ] [.block_descriptor] [line 6]
!19 = !{i32 786447, null, null, !"", i32 0, i64 64, i64 0, i64 0, i32 0, !20} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 0, offset 0] [from __block_literal_1]
!20 = !{i32 786451, !5, !6, !"__block_literal_1", i32 6, i64 256, i64 64, i32 0, i32 0, null, !21, i32 0, null, null} ; [ DW_TAG_structure_type ] [__block_literal_1] [line 6, size 256, align 64, offset 0] [from ]
!21 = !{!22, !23, !25, !26, !27}
!22 = !{i32 786445, !5, !6, !"__isa", i32 6, i64 64, i64 64, i64 0, i32 0, !12} ; [ DW_TAG_member ] [__isa] [line 6, size 64, align 64, offset 0] [from ]
!23 = !{i32 786445, !5, !6, !"__flags", i32 6, i64 32, i64 32, i64 64, i32 0, !24} ; [ DW_TAG_member ] [__flags] [line 6, size 32, align 32, offset 64] [from int]
!24 = !{i32 786468, null, null, !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!25 = !{i32 786445, !5, !6, !"__reserved", i32 6, i64 32, i64 32, i64 96, i32 0, !24} ; [ DW_TAG_member ] [__reserved] [line 6, size 32, align 32, offset 96] [from int]
!26 = !{i32 786445, !5, !6, !"__FuncPtr", i32 6, i64 64, i64 64, i64 128, i32 0, !12} ; [ DW_TAG_member ] [__FuncPtr] [line 6, size 64, align 64, offset 128] [from ]
!27 = !{i32 786445, !5, !6, !"__descriptor", i32 6, i64 64, i64 64, i64 192, i32 0, !28} ; [ DW_TAG_member ] [__descriptor] [line 6, size 64, align 64, offset 192] [from ]
!28 = !{i32 786447, null, null, !"", i32 0, i64 64, i64 64, i64 0, i32 0, !29} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from __block_descriptor]
!29 = !{i32 786451, !1, null, !"__block_descriptor", i32 6, i64 0, i64 0, i32 0, i32 4, null, null, i32 0} ; [ DW_TAG_structure_type ] [__block_descriptor] [line 6, size 0, align 0, offset 0] [fwd] [from ]
!30 = !{i32 6, i32 0, !9, null}
!31 = !{i32 786688, !32, !"id", !6, i32 7, !33, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [id] [line 7]
!32 = !{i32 786443, !5, !9, i32 6, i32 0, i32 0} ; [ DW_TAG_lexical_block ] [/home/vromanov/OCL_SDK//tmp/reproducer_CloneBlockToKernel.cl]
!33 = !{i32 786454, !5, null, !"size_t", i32 69, i64 0, i64 0, i64 0, i32 0, !34} ; [ DW_TAG_typedef ] [size_t] [line 69, size 0, align 0, offset 0] [from ulong]
!34 = !{i32 786454, !5, null, !"ulong", i32 58, i64 0, i64 0, i64 0, i32 0, !35} ; [ DW_TAG_typedef ] [ulong] [line 58, size 0, align 0, offset 0] [from unsigned long]
!35 = !{i32 786468, null, null, !"unsigned long", i32 0, i64 64, i64 64, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ] [unsigned long] [line 0, size 64, align 64, offset 0, enc DW_ATE_unsigned]
!36 = !{i32 7, i32 0, !32, null}
!37 = !{i32 8, i32 0, !9, null} ; [ DW_TAG_imported_declaration ]
