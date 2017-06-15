; RUN: opt -cloneblockinvokefunctokernel -verify -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; Ticket ID : CSSD100020567
;; This test was generated using the following cl code with this command:
;;  clang.exe -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited -dwarf-version=4 -O0 -D__OPENCL_C_VERSION__=200 -o - reproducer_CloneBlockToKernel.cl -I llvm\llvm\install\include\cclang -include opencl-c.h -include opencl-c-intel.h
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
; CHECK: [[SRCDIPL:.*]] = distinct !DISubprogram([[SRCDI:name: "__ker_block_invoke".*]])
; CHECK: [[CLONDIPL:.*]] = distinct !DISubprogram([[SRCDI]])
; CHECK: !DILocation({{.*}} scope: [[SRCDIPL]])
; CHECK: !DILocation({{.*}} scope: [[CLONDIPL]])

; ModuleID = 'reproducer_CloneBlockToKernel.cl'

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__block_descriptor = type { i64, i64 }
%opencl.queue_t = type opaque
%opencl.ndrange_t = type opaque

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr addrspace(2) constant [6 x i8] c"v8@?0\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } { i64 0, i64 32, i8 addrspace(2)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(2)* @.str, i32 0, i32 0), i8 addrspace(2)* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (void (i8*)* @__ker_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8 addrspace(2)*, i8 addrspace(2)* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8

; Function Attrs: nounwind
define spir_kernel void @ker() #0 !dbg !4 {
entry:
  %call = call spir_func %opencl.queue_t* @_Z17get_default_queuev(), !dbg !22
  %call1 = call spir_func %opencl.ndrange_t* @_Z10ndrange_1Dm(i64 4), !dbg !23
  %call2 = call spir_func i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangeU13block_pointerFvvE(%opencl.queue_t* %call, i32 1, %opencl.ndrange_t* %call1, void ()* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to void ()*)), !dbg !22
  ret void, !dbg !24
}

declare spir_func i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangeU13block_pointerFvvE(%opencl.queue_t*, i32, %opencl.ndrange_t*, void ()*) #1

declare spir_func %opencl.queue_t* @_Z17get_default_queuev() #1

declare spir_func %opencl.ndrange_t* @_Z10ndrange_1Dm(i64) #1

; Function Attrs: nounwind
define internal spir_func void @__ker_block_invoke(i8* %.block_descriptor) #0 !dbg !8 {
entry:
  %.block_descriptor.addr = alloca i8*, align 8
  %block.addr = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, align 8
  %id = alloca i64, align 8
  store i8* %.block_descriptor, i8** %.block_descriptor.addr, align 8
  %0 = load i8*, i8** %.block_descriptor.addr, align 8
  call void @llvm.dbg.value(metadata i8* %0, i64 0, metadata !25, metadata !42), !dbg !43
  call void @llvm.dbg.declare(metadata i8* %.block_descriptor, metadata !25, metadata !42), !dbg !43
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*, !dbg !43
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>* %block, <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>** %block.addr, align 8
  call void @llvm.dbg.declare(metadata i64* %id, metadata !44, metadata !42), !dbg !48
  store i64 4, i64* %id, align 8, !dbg !48
  ret void, !dbg !49
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!12}
!llvm.module.flags = !{!18, !19}
!opencl.enable.FP_CONTRACT = !{}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!20}
!opencl.spir.version = !{!21, !21, !21, !21}
!opencl.ocl.version = !{!21, !21, !21, !21}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.8.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 5e5f31ff4099022445ddc9519bdd4c03b14193bd) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 6fe444928badc9c60eafb8d750910014ec4a8ece)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2)
!1 = !DIFile(filename: "D:\5Ctemp\5C<stdin>", directory: "D:\5Cetyurin\5Copencl\5Csrc\5Cbackend\5Ctests\5Cocl_backend_passes\5CDebugInfo")
!2 = !{}
!3 = !{!4, !8}
!4 = distinct !DISubprogram(name: "ker", scope: !5, file: !5, line: 1, type: !6, isLocal: false, isDefinition: true, scopeLine: 2, isOptimized: false, unit: !0, variables: !2)
!5 = !DIFile(filename: "D:\5Ctemp\5Cenqueue_kernel.cl", directory: "D:\5Cetyurin\5Copencl\5Csrc\5Cbackend\5Ctests\5Cocl_backend_passes\5CDebugInfo")
!6 = !DISubroutineType(types: !7)
!7 = !{null}
!8 = distinct !DISubprogram(name: "__ker_block_invoke", scope: !5, file: !5, line: 6, type: !9, isLocal: true, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64, align: 64)
!12 = !{void ()* @ker, !13, !14, !15, !16, !17}
!13 = !{!"kernel_arg_addr_space"}
!14 = !{!"kernel_arg_access_qual"}
!15 = !{!"kernel_arg_type"}
!16 = !{!"kernel_arg_base_type"}
!17 = !{!"kernel_arg_type_qual"}
!18 = !{i32 2, !"Dwarf Version", i32 4}
!19 = !{i32 2, !"Debug Info Version", i32 3}
!20 = !{!"clang version 3.8.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 5e5f31ff4099022445ddc9519bdd4c03b14193bd) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 6fe444928badc9c60eafb8d750910014ec4a8ece)"}
!21 = !{i32 2, i32 0}
!22 = !DILocation(line: 3, scope: !4)
!23 = !DILocation(line: 5, scope: !4)
!24 = !DILocation(line: 9, scope: !4)
!25 = !DILocalVariable(name: ".block_descriptor", arg: 1, scope: !8, file: !5, line: 6, type: !26, flags: DIFlagArtificial)
!26 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !27, size: 64)
!27 = !DICompositeType(tag: DW_TAG_structure_type, name: "__block_literal_1", scope: !5, file: !5, line: 6, size: 256, align: 64, elements: !28)
!28 = !{!29, !30, !32, !33, !35}
!29 = !DIDerivedType(tag: DW_TAG_member, name: "__isa", scope: !5, file: !5, line: 6, baseType: !11, size: 64, align: 64, flags: DIFlagPublic)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "__flags", scope: !5, file: !5, line: 6, baseType: !31, size: 32, align: 32, offset: 64, flags: DIFlagPublic)
!31 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "__reserved", scope: !5, file: !5, line: 6, baseType: !31, size: 32, align: 32, offset: 96, flags: DIFlagPublic)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "__FuncPtr", scope: !5, file: !5, line: 6, baseType: !34, size: 64, align: 64, offset: 128, flags: DIFlagPublic)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64, align: 64)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "__descriptor", scope: !5, file: !5, line: 6, baseType: !36, size: 64, align: 64, offset: 192, flags: DIFlagPublic)
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !37, size: 64, align: 64)
!37 = !DICompositeType(tag: DW_TAG_structure_type, name: "__block_descriptor", file: !1, line: 6, size: 128, align: 64, elements: !38)
!38 = !{!39, !41}
!39 = !DIDerivedType(tag: DW_TAG_member, name: "reserved", scope: !37, file: !1, line: 6, baseType: !40, size: 64, align: 64)
!40 = !DIBasicType(name: "unsigned long", size: 64, align: 64, encoding: DW_ATE_unsigned)
!41 = !DIDerivedType(tag: DW_TAG_member, name: "Size", scope: !37, file: !1, line: 6, baseType: !40, size: 64, align: 64, offset: 64)
!42 = !DIExpression()
!43 = !DILocation(line: 6, scope: !8)
!44 = !DILocalVariable(name: "id", scope: !45, file: !5, line: 7, type: !46)
!45 = distinct !DILexicalBlock(scope: !8, file: !5, line: 6)
!46 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !47, line: 53, baseType: !40)
!47 = !DIFile(filename: "D:\5Cetyurin\5Cllvm\5Cllvm\5Cinstall\5Cinclude\5Ccclang\5Copencl-c.h", directory: "D:\5Cetyurin\5Copencl\5Csrc\5Cbackend\5Ctests\5Cocl_backend_passes\5CDebugInfo")
!48 = !DILocation(line: 7, scope: !45)
!49 = !DILocation(line: 8, scope: !8)
