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
; CHECK-DAG: [[SRCDIPL:.*]] = distinct !DISubprogram([[SRCDI:name: "__ker_block_invoke".*]])
; CHECK-DAG: [[CLONDIPL:.*]] = distinct !DISubprogram([[SRCDI]])
; CHECK-DAG: !DILocation({{.*}} scope: [[SRCDIPL]])
; CHECK-DAG: !DILocation({{.*}} scope: [[CLONDIPL]])

; ModuleID = 'reproducer_CloneBlockToKernel.cl'

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__block_descriptor = type { i64, i64 }
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr addrspace(2) constant [6 x i8] c"v8@?0\00", align 1
@__block_descriptor_tmp = internal addrspace(2) constant { i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } { i64 0, i64 32, i8 addrspace(2)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(2)* @.str, i32 0, i32 0), i8 addrspace(2)* null }, align 8
@__block_literal_global = internal addrspace(1) constant { i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } { i8** null, i32 1342177280, i32 0, i8* bitcast (void (i8 addrspace(4)*)* @__ker_block_invoke to i8*), %struct.__block_descriptor addrspace(2)* bitcast ({ i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } addrspace(2)* @__block_descriptor_tmp to %struct.__block_descriptor addrspace(2)*) }, align 8

; Function Attrs: nounwind
define spir_kernel void @ker() #0 !dbg !7 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 {
entry:
  %tmp = alloca %struct.ndrange_t, align 8
  %call = call spir_func %opencl.queue_t* @_Z17get_default_queuev(), !dbg !11
  call spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret %tmp, i64 4), !dbg !12
  %0 = call i32 @__enqueue_kernel_basic(%opencl.queue_t* %call, i32 1, %struct.ndrange_t* byval %tmp, i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*)), !dbg !13
  ret void, !dbg !15
}

declare spir_func %opencl.queue_t* @_Z17get_default_queuev() #1

declare spir_func void @_Z10ndrange_1Dm(%struct.ndrange_t* sret, i64) #1

; Function Attrs: nounwind
define internal spir_func void @__ker_block_invoke(i8 addrspace(4)* %.block_descriptor) #0 !dbg !16 {
entry:
  %.block_descriptor.addr = alloca i8 addrspace(4)*, align 8
  %block.addr = alloca <{ i8*, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }> addrspace(4)*, align 8
  %id = alloca i64, align 8
  store i8 addrspace(4)* %.block_descriptor, i8 addrspace(4)** %.block_descriptor.addr, align 8
  %0 = load i8 addrspace(4)*, i8 addrspace(4)** %.block_descriptor.addr, align 8
  call void @llvm.dbg.value(metadata i8 addrspace(4)* %0, i64 0, metadata !20, metadata !37), !dbg !38
  call void @llvm.dbg.declare(metadata i8 addrspace(4)* %.block_descriptor, metadata !20, metadata !37), !dbg !38
  %block = bitcast i8 addrspace(4)* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }> addrspace(4)*, !dbg !38
  store <{ i8*, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }> addrspace(4)* %block, <{ i8*, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }> addrspace(4)** %block.addr, align 8
  call void @llvm.dbg.declare(metadata i64* %id, metadata !39, metadata !37), !dbg !43
  store i64 4, i64* %id, align 8, !dbg !43
  ret void, !dbg !44
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

declare i32 @__enqueue_kernel_basic(%opencl.queue_t*, i32, %struct.ndrange_t*, i8 addrspace(4)*)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "../<stdin>", directory: "/data/kbessono/dev/opencl")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"clang version 4.0.1"}
!7 = distinct !DISubprogram(name: "ker", scope: !8, file: !8, line: 2, type: !9, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, variables: !2)
!8 = !DIFile(filename: "../debugInfoCloneBlockToKernel.cl", directory: "/data/dev/opencl")
!9 = !DISubroutineType(types: !10)
!10 = !{null}
!11 = !DILocation(line: 4, scope: !7)
!12 = !DILocation(line: 6, scope: !7)
!13 = !DILocation(line: 4, scope: !14)
!14 = !DILexicalBlockFile(scope: !7, file: !8, discriminator: 1)
!15 = !DILocation(line: 10, scope: !7)
!16 = distinct !DISubprogram(name: "__ker_block_invoke", scope: !8, file: !8, line: 7, type: !17, isLocal: true, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!17 = !DISubroutineType(types: !18)
!18 = !{null, !19}
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!20 = !DILocalVariable(name: ".block_descriptor", arg: 1, scope: !16, file: !8, line: 7, type: !21, flags: DIFlagArtificial)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!22 = !DICompositeType(tag: DW_TAG_structure_type, name: "__block_literal_1", scope: !8, file: !8, line: 7, size: 256, elements: !23)
!23 = !{!24, !25, !27, !28, !30}
!24 = !DIDerivedType(tag: DW_TAG_member, name: "__isa", scope: !8, file: !8, line: 7, baseType: !19, size: 64, flags: DIFlagPublic)
!25 = !DIDerivedType(tag: DW_TAG_member, name: "__flags", scope: !8, file: !8, line: 7, baseType: !26, size: 32, offset: 64, flags: DIFlagPublic)
!26 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "__reserved", scope: !8, file: !8, line: 7, baseType: !26, size: 32, offset: 96, flags: DIFlagPublic)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "__FuncPtr", scope: !8, file: !8, line: 7, baseType: !29, size: 64, offset: 128, flags: DIFlagPublic)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "__descriptor", scope: !8, file: !8, line: 7, baseType: !31, size: 64, offset: 192, flags: DIFlagPublic)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !32, size: 64)
!32 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__block_descriptor", file: !1, line: 7, size: 128, elements: !33)
!33 = !{!34, !36}
!34 = !DIDerivedType(tag: DW_TAG_member, name: "reserved", scope: !32, file: !1, line: 7, baseType: !35, size: 64)
!35 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!36 = !DIDerivedType(tag: DW_TAG_member, name: "Size", scope: !32, file: !1, line: 7, baseType: !35, size: 64, offset: 64)
!37 = !DIExpression()
!38 = !DILocation(line: 7, scope: !16)
!39 = !DILocalVariable(name: "id", scope: !40, file: !8, line: 8, type: !41)
!40 = distinct !DILexicalBlock(scope: !16, file: !8, line: 7)
!41 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !42, line: 19, baseType: !35)
!42 = !DIFile(filename: "opencl-c-platform.h", directory: "/data/dev/opencl")
!43 = !DILocation(line: 8, scope: !40)
!44 = !DILocation(line: 9, scope: !16)
