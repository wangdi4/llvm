; RUN: opt --cloneblockinvokefunctokernel -verify -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; Ticket ID : CSSD100020567
;; This test was generated using the following cl code with this command:
;;  clang -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -include opencl_.h -I <Path-TO>/clang_headers/ -g -O0 -D__OPENCL_C_VERSION__=200 -o debugInfoCloneBlockToKernel.ll
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__block_descriptor = type { i64, i64 }
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr addrspace(2) constant [6 x i8] c"v8@?0\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } { i64 0, i64 32, i8 addrspace(2)* getelementptr inbounds ([6 x i8] addrspace(2)* @.str, i32 0, i32 0), i8 addrspace(2)* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (void (i8*)* @__ker_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8 addrspace(2)*, i8 addrspace(2)* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8

; Function Attrs: nounwind
define spir_kernel void @ker() #0 {
  %1 = alloca %struct.ndrange_t, align 8
  %2 = call spir_func %opencl.queue_t* @get_default_queue() #3, !dbg !24
  call spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret %1, i64 4) #4, !dbg !25
  %3 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t* %2, i32 0, %struct.ndrange_t* byval %1, void ()* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to void ()*)), !dbg !24
  ret void, !dbg !26
}

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvvE(%opencl.queue_t*, i32, %struct.ndrange_t* byval, void ()*) #1

; Function Attrs: nounwind readnone
declare spir_func %opencl.queue_t* @get_default_queue() #2

; Function Attrs: nounwind
declare spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret, i64) #0

; Function Attrs: nounwind
define internal spir_func void @__ker_block_invoke(i8* %.block_descriptor) #0 {
  %1 = alloca i8*, align 8
  %2 = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, align 8
  %id = alloca i64, align 8
  store i8* %.block_descriptor, i8** %1, align 8
  %3 = load i8** %1
  call void @llvm.dbg.value(metadata i8* %3, i64 0, metadata !27, metadata !44), !dbg !45
  call void @llvm.dbg.declare(metadata i8* %.block_descriptor, metadata !27, metadata !44), !dbg !45
  %4 = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, !dbg !45
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>* %4, <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>** %2, align 8
  call void @llvm.dbg.declare(metadata i64* %id, metadata !46, metadata !44), !dbg !52
  store i64 4, i64* %id, align 8, !dbg !52
  ret void, !dbg !53
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #3

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #3

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!13}
!llvm.module.flags = !{!19, !20}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!21}
!opencl.ocl.version = !{!22}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!23}

!0 = !{!"0x11\0012\00clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)\000\00\000\00\001", !1, !2, !2, !3, !2, !2} ; [ DW_TAG_compile_unit ] [//tmp/<stdin>] [DW_LANG_C99]
!1 = !{!"/tmp/<stdin>", !"/tmp"}
!2 = !{}
!3 = !{!4, !9}
!4 = !{!"0x2e\00ker\00ker\00\001\000\001\000\000\000\000\002", !5, !6, !7, null, void ()* @ker, null, null, !2} ; [ DW_TAG_subprogram ] [line 1] [def] [scope 2] [ker]
!5 = !{!"/tmp/ker.cl", !"/tmp"}
!6 = !{!"0x29", !5}                               ; [ DW_TAG_file_type ] [/tmp/ker.cl]
!7 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !8, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!8 = !{null}
!9 = !{!"0x2e\00__ker_block_invoke\00__ker_block_invoke\00\006\001\001\000\000\00256\000\006", !5, !6, !10, null, void (i8*)* @__ker_block_invoke, null, null, !2} ; [ DW_TAG_subprogram ] [line 6] [local] [def] [__ker_block_invoke]
!10 = !{!"0x15\00\000\000\000\000\000\000", null, null, null, !11, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!11 = !{null, !12}
!12 = !{!"0xf\00\000\0064\0064\000\000", null, null, null} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!13 = !{void ()* @ker, !14, !15, !16, !17, !18}
!14 = !{!"kernel_arg_addr_space"}
!15 = !{!"kernel_arg_access_qual"}
!16 = !{!"kernel_arg_type"}
!17 = !{!"kernel_arg_base_type"}
!18 = !{!"kernel_arg_type_qual"}
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 2}
!21 = !{i32 1, i32 2}
!22 = !{i32 2, i32 0}
!23 = !{!"clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)"}
!24 = !MDLocation(line: 3, scope: !4)
!25 = !MDLocation(line: 5, scope: !4)
!26 = !MDLocation(line: 9, scope: !4)
!27 = !{!"0x101\00.block_descriptor\0016777222\0064", !9, !6, !28} ; [ DW_TAG_arg_variable ] [.block_descriptor] [line 6]
!28 = !{!"0xf\00\000\0064\000\000\000", null, null, !29} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 0, offset 0] [from __block_literal_1]
!29 = !{!"0x13\00__block_literal_1\006\00256\0064\000\000\000", !5, !6, null, !30, null, null, null} ; [ DW_TAG_structure_type ] [__block_literal_1] [line 6, size 256, align 64, offset 0] [def] [from ]
!30 = !{!31, !32, !34, !35, !37}
!31 = !{!"0xd\00__isa\006\0064\0064\000\003", !5, !6, !12} ; [ DW_TAG_member ] [__isa] [line 6, size 64, align 64, offset 0] [public] [from ]
!32 = !{!"0xd\00__flags\006\0032\0032\0064\003", !5, !6, !33} ; [ DW_TAG_member ] [__flags] [line 6, size 32, align 32, offset 64] [public] [from int]
!33 = !{!"0x24\00int\000\0032\0032\000\000\005", null, null} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!34 = !{!"0xd\00__reserved\006\0032\0032\0096\003", !5, !6, !33} ; [ DW_TAG_member ] [__reserved] [line 6, size 32, align 32, offset 96] [public] [from int]
!35 = !{!"0xd\00__FuncPtr\006\0064\0064\00128\003", !5, !6, !36} ; [ DW_TAG_member ] [__FuncPtr] [line 6, size 64, align 64, offset 128] [public] [from ]
!36 = !{!"0xf\00\000\0064\0064\000\000", null, null, !7} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from ]
!37 = !{!"0xd\00__descriptor\006\0064\0064\00192\003", !5, !6, !38} ; [ DW_TAG_member ] [__descriptor] [line 6, size 64, align 64, offset 192] [public] [from ]
!38 = !{!"0xf\00\000\0064\0064\000\000", null, null, !39} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from __block_descriptor]
!39 = !{!"0x13\00__block_descriptor\006\00128\0064\000\000\000", !1, null, null, !40, null, null, null} ; [ DW_TAG_structure_type ] [__block_descriptor] [line 6, size 128, align 64, offset 0] [def] [from ]
!40 = !{!41, !43}
!41 = !{!"0xd\00reserved\006\0064\0064\000\000", !1, !39, !42} ; [ DW_TAG_member ] [reserved] [line 6, size 64, align 64, offset 0] [from unsigned long]
!42 = !{!"0x24\00unsigned long\000\0064\0064\000\000\007", null, null} ; [ DW_TAG_base_type ] [unsigned long] [line 0, size 64, align 64, offset 0, enc DW_ATE_unsigned]
!43 = !{!"0xd\00Size\006\0064\0064\0064\000", !1, !39, !42} ; [ DW_TAG_member ] [Size] [line 6, size 64, align 64, offset 64] [from unsigned long]
!44 = !{!"0x102"}                                 ; [ DW_TAG_expression ]
!45 = !MDLocation(line: 6, scope: !9)
!46 = !{!"0x100\00id\007\000", !47, !6, !48}      ; [ DW_TAG_auto_variable ] [id] [line 7]
!47 = !{!"0xb\006\000\000", !5, !9}               ; [ DW_TAG_lexical_block ] [/tmp/ker.cl]
!48 = !{!"0x16\00size_t\0011\000\000\000\000", !49, null, !50} ; [ DW_TAG_typedef ] [size_t] [line 11, size 0, align 0, offset 0] [from ulong]
!49 = !{!"/include/cclang/cl_headers/cpu/cpu_opencl_plat_spec.h", !"/tmp"}
!50 = !{!"0x16\00ulong\0021\000\000\000\000", !51, null, !42} ; [ DW_TAG_typedef ] [ulong] [line 21, size 0, align 0, offset 0] [from unsigned long]
!51 = !{!"/include/cclang/cl_headers/opencl_common.h", !"/tmp"}
!52 = !MDLocation(line: 7, scope: !47)
!53 = !MDLocation(line: 8, scope: !9)
