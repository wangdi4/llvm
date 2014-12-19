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
  call void @llvm.dbg.value(metadata !{i8* %3}, i64 0, metadata !18), !dbg !30
  call void @llvm.dbg.declare(metadata !{i8* %.block_descriptor}, metadata !18), !dbg !30
  %4 = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, !dbg !30
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>* %4, <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>** %2, align 8
  call void @llvm.dbg.declare(metadata !{i64* %id}, metadata !31), !dbg !36
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

!0 = metadata !{i32 786449, metadata !1, i32 12, metadata !"clang version 3.4 ", i1 false, metadata !"", i32 0, metadata !2, metadata !2, metadata !3, metadata !2, metadata !2, metadata !""} ; [ DW_TAG_compile_unit ] [/home/vromanov/OCL_SDK//tmp/<unknown>] [DW_LANG_C99]
!1 = metadata !{metadata !"/tmp/<unknown>", metadata !"/home/vromanov/OCL_SDK"}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4, metadata !9}
!4 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"ker", metadata !"ker", metadata !"", i32 1, metadata !7, i1 false, i1 true, i32 0, i32 0, null, i32 0, i1 false, void ()* @ker, null, null, metadata !2, i32 2} ; [ DW_TAG_subprogram ] [line 1] [def] [scope 2] [ker]
!5 = metadata !{metadata !"/tmp/reproducer_CloneBlockToKernel.cl", metadata !"/home/vromanov/OCL_SDK"}
!6 = metadata !{i32 786473, metadata !5}          ; [ DW_TAG_file_type ] [/home/vromanov/OCL_SDK//tmp/reproducer_CloneBlockToKernel.cl]
!7 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = metadata !{null}
!9 = metadata !{i32 786478, metadata !5, metadata !6, metadata !"__ker_block_invoke", metadata !"__ker_block_invoke", metadata !"", i32 6, metadata !10, i1 true, i1 true, i32 0, i32 0, null, i32 256, i1 false, void (i8*)* @__ker_block_invoke, null, null, metadata !2, i32 6} ; [ DW_TAG_subprogram ] [line 6] [local] [def] [__ker_block_invoke]
!10 = metadata !{i32 786453, i32 0, i32 0, metadata !"", i32 0, i64 0, i64 0, i64 0, i32 0, null, metadata !11, i32 0, i32 0} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!11 = metadata !{null, metadata !12}
!12 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!13 = metadata !{void ()* @ker}
!14 = metadata !{metadata !"-cl-std=CL2.0"}
!15 = metadata !{i32 3, i32 0, metadata !4, null}
!16 = metadata !{i32 5, i32 0, metadata !4, null}
!17 = metadata !{i32 9, i32 0, metadata !4, null}
!18 = metadata !{i32 786689, metadata !9, metadata !".block_descriptor", metadata !6, i32 16777222, metadata !19, i32 64, i32 0} ; [ DW_TAG_arg_variable ] [.block_descriptor] [line 6]
!19 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 0, i64 0, i32 0, metadata !20} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 0, offset 0] [from __block_literal_1]
!20 = metadata !{i32 786451, metadata !5, metadata !6, metadata !"__block_literal_1", i32 6, i64 256, i64 64, i32 0, i32 0, null, metadata !21, i32 0, null, null} ; [ DW_TAG_structure_type ] [__block_literal_1] [line 6, size 256, align 64, offset 0] [from ]
!21 = metadata !{metadata !22, metadata !23, metadata !25, metadata !26, metadata !27}
!22 = metadata !{i32 786445, metadata !5, metadata !6, metadata !"__isa", i32 6, i64 64, i64 64, i64 0, i32 0, metadata !12} ; [ DW_TAG_member ] [__isa] [line 6, size 64, align 64, offset 0] [from ]
!23 = metadata !{i32 786445, metadata !5, metadata !6, metadata !"__flags", i32 6, i64 32, i64 32, i64 64, i32 0, metadata !24} ; [ DW_TAG_member ] [__flags] [line 6, size 32, align 32, offset 64] [from int]
!24 = metadata !{i32 786468, null, null, metadata !"int", i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!25 = metadata !{i32 786445, metadata !5, metadata !6, metadata !"__reserved", i32 6, i64 32, i64 32, i64 96, i32 0, metadata !24} ; [ DW_TAG_member ] [__reserved] [line 6, size 32, align 32, offset 96] [from int]
!26 = metadata !{i32 786445, metadata !5, metadata !6, metadata !"__FuncPtr", i32 6, i64 64, i64 64, i64 128, i32 0, metadata !12} ; [ DW_TAG_member ] [__FuncPtr] [line 6, size 64, align 64, offset 128] [from ]
!27 = metadata !{i32 786445, metadata !5, metadata !6, metadata !"__descriptor", i32 6, i64 64, i64 64, i64 192, i32 0, metadata !28} ; [ DW_TAG_member ] [__descriptor] [line 6, size 64, align 64, offset 192] [from ]
!28 = metadata !{i32 786447, null, null, metadata !"", i32 0, i64 64, i64 64, i64 0, i32 0, metadata !29} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from __block_descriptor]
!29 = metadata !{i32 786451, metadata !1, null, metadata !"__block_descriptor", i32 6, i64 0, i64 0, i32 0, i32 4, null, null, i32 0} ; [ DW_TAG_structure_type ] [__block_descriptor] [line 6, size 0, align 0, offset 0] [fwd] [from ]
!30 = metadata !{i32 6, i32 0, metadata !9, null}
!31 = metadata !{i32 786688, metadata !32, metadata !"id", metadata !6, i32 7, metadata !33, i32 0, i32 0} ; [ DW_TAG_auto_variable ] [id] [line 7]
!32 = metadata !{i32 786443, metadata !5, metadata !9, i32 6, i32 0, i32 0} ; [ DW_TAG_lexical_block ] [/home/vromanov/OCL_SDK//tmp/reproducer_CloneBlockToKernel.cl]
!33 = metadata !{i32 786454, metadata !5, null, metadata !"size_t", i32 69, i64 0, i64 0, i64 0, i32 0, metadata !34} ; [ DW_TAG_typedef ] [size_t] [line 69, size 0, align 0, offset 0] [from ulong]
!34 = metadata !{i32 786454, metadata !5, null, metadata !"ulong", i32 58, i64 0, i64 0, i64 0, i32 0, metadata !35} ; [ DW_TAG_typedef ] [ulong] [line 58, size 0, align 0, offset 0] [from unsigned long]
!35 = metadata !{i32 786468, null, null, metadata !"unsigned long", i32 0, i64 64, i64 64, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ] [unsigned long] [line 0, size 64, align 64, offset 0, enc DW_ATE_unsigned]
!36 = metadata !{i32 7, i32 0, metadata !32, null}
!37 = metadata !{i32 8, i32 0, metadata !9, null} ; [ DW_TAG_imported_declaration ]
