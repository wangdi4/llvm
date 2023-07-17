; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-sg-emu-value-widen -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: %dbg.param.c = alloca ptr addrspace(4), align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %dbg.param.c, metadata !{{.*}}, metadata !DIExpression(DW_OP_deref))
; CHECK: %[[#PARAM:]] = extractelement <16 x ptr addrspace(4)> %c, i32 %sg.lid.{{.*}}
; CHECK-NEXT: store ptr addrspace(4) %[[#PARAM]], ptr %dbg.param.c

; Function Attrs: convergent noinline norecurse nounwind
define void @foo(i32 %b, ptr addrspace(4) %c) #0 !dbg !15 !kernel_arg_base_type !51 !arg_type_null_val !52 {
entry:
  call void @dummy_sg_barrier()
  %b.addr = alloca i32, align 4
  %c.addr = alloca ptr addrspace(4), align 8
  store i32 %b, ptr %b.addr, align 4
  call void @llvm.dbg.declare(metadata ptr %b.addr, metadata !21, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.declare(metadata ptr addrspace(4) %c, metadata !21, metadata !DIExpression()), !dbg !22
  store ptr addrspace(4) %c, ptr %c.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %c.addr, metadata !23, metadata !DIExpression()), !dbg !24
  %0 = load i32, ptr %b.addr, align 4, !dbg !25
  br label %sg.barrier.bb.1

sg.barrier.bb.1:                                  ; preds = %entry
  call void @_Z17sub_group_barrierj(i32 1)
  %call = call i32 @_Z13sub_group_alli(i32 %0) #6, !dbg !26
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.1
  call void @dummy_sg_barrier()
  %1 = load ptr addrspace(4), ptr %c.addr, align 8, !dbg !27
  store i32 %call, ptr addrspace(4) %1, align 4, !dbg !28
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  ret void, !dbg !29
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent
declare i32 @_Z13sub_group_alli(i32) #2

; Function Attrs: convergent noinline norecurse nounwind
define void @test(ptr addrspace(1) noalias %a) #3 !dbg !30 !kernel_has_sub_groups !39 !sg_emu_size !41 !kernel_arg_base_type !53 !arg_type_null_val !54 {
entry:
  call void @dummybarrier.()
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %entry
  call void @dummy_sg_barrier()
  %a.addr = alloca ptr addrspace(1), align 8
  %b = alloca i32, align 4
  store ptr addrspace(1) %a, ptr %a.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !42, metadata !DIExpression()), !dbg !43
  call void @llvm.dbg.declare(metadata ptr %b, metadata !44, metadata !DIExpression()), !dbg !45
  %call = call i64 @_Z12get_local_idj(i32 0) #7, !dbg !46
  %conv = trunc i64 %call to i32, !dbg !46
  store i32 %conv, ptr %b, align 4, !dbg !45
  %0 = load i32, ptr %b, align 4, !dbg !47
  %1 = load ptr addrspace(1), ptr %a.addr, align 8, !dbg !48
  %2 = addrspacecast ptr addrspace(1) %1 to ptr addrspace(4)
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.1
  call void @_Z17sub_group_barrierj(i32 1)
  call void @foo(i32 %0, ptr addrspace(4) %2) #5, !dbg !49
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.2

sg.dummy.bb.2:                                    ; preds = %sg.dummy.bb.
  call void @dummy_sg_barrier()
  ret void, !dbg !50
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #4

declare void @dummybarrier.()

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #5

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j)" }
attributes #3 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { convergent }
attributes #6 = { convergent "has-vplan-mask" }
attributes #7 = { convergent nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!opencl.stat.type = !{!8}
!opencl.stat.exec_time = !{!9}
!opencl.stat.run_time_version = !{!10}
!opencl.stat.workload_name = !{!11}
!opencl.stat.module_name = !{!12}
!sycl.kernels = !{!13}
!opencl.stats.InstCounter.CanVect = !{!14}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "1", directory: "/")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{!""}
!9 = !{!"2020-12-07 11:27:35"}
!10 = !{!"2020.11.12.0"}
!11 = !{!"test.dbg"}
!12 = !{!"test.dbg1"}
!13 = !{ptr @test}
!14 = !{!"Code is vectorizable"}
!15 = distinct !DISubprogram(name: "foo", scope: !16, file: !16, line: 1, type: !17, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!16 = !DIFile(filename: "1", directory: "/")
!17 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !18)
!18 = !{null, !19, !20}
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!21 = !DILocalVariable(name: "b", arg: 1, scope: !15, file: !16, line: 1, type: !19)
!22 = !DILocation(line: 1, column: 14, scope: !15)
!23 = !DILocalVariable(name: "c", arg: 2, scope: !15, file: !16, line: 1, type: !20)
!24 = !DILocation(line: 1, column: 22, scope: !15)
!25 = !DILocation(line: 2, column: 22, scope: !15)
!26 = !DILocation(line: 2, column: 8, scope: !15)
!27 = !DILocation(line: 2, column: 4, scope: !15)
!28 = !DILocation(line: 2, column: 6, scope: !15)
!29 = !DILocation(line: 3, column: 1, scope: !15)
!30 = distinct !DISubprogram(name: "test", scope: !16, file: !16, line: 4, type: !31, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!31 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !32)
!32 = !{null, !20}
!33 = !{i32 1}
!34 = !{!"none"}
!35 = !{!"int*"}
!36 = !{!"a"}
!37 = !{i1 false}
!38 = !{i32 0}
!39 = !{i1 true}
!40 = !{i32 13}
!41 = !{i32 16}
!42 = !DILocalVariable(name: "a", arg: 1, scope: !30, file: !16, line: 4, type: !20)
!43 = !DILocation(line: 4, column: 32, scope: !30)
!44 = !DILocalVariable(name: "b", scope: !30, file: !16, line: 5, type: !19)
!45 = !DILocation(line: 5, column: 5, scope: !30)
!46 = !DILocation(line: 5, column: 9, scope: !30)
!47 = !DILocation(line: 6, column: 5, scope: !30)
!48 = !DILocation(line: 6, column: 8, scope: !30)
!49 = !DILocation(line: 6, column: 1, scope: !30)
!50 = !DILocation(line: 7, column: 1, scope: !30)
!51 = !{!"int", !"int*"}
!52 = !{i32 0, ptr addrspace(4) null}
!53 = !{!"int*"}
!54 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
