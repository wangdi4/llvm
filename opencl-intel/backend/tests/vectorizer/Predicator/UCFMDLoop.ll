; RUN: %oclopt -presucf=true -predicate -S %s | FileCheck %s

; CHECK: , !llvm.loop [[PARAM:![0-9]+]]
; CHECK-DAG: [[PARAM]] = distinct !{{{![0-9]+}}, [[LINE1:![0-9]+]], [[LINE2:![0-9]+]]}
; CHECK-DAG: [[LINE1]] = !DILocation(line: 8, column: 3,
; CHECK-DAG: [[LINE2]] = !DILocation(line: 11, column: 3,

; ModuleID = 'main'
source_filename = "atomic.cl"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z10atomic_addPU3AS1Vjj(i32 addrspace(1)* %0, i32 %1) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata %0, metadata %1, metadata %2) #3

; Function Attrs: convergent nounwind
define void @__Vectorized_.test_single_wi(i32 addrspace(1)* %p, i32 addrspace(1)* %res) local_unnamed_addr #0 !dbg !52 !kernel_arg_addr_space !22 !kernel_arg_access_qual !23 !kernel_arg_type !24 !kernel_arg_base_type !24 !kernel_arg_type_qual !25 !kernel_arg_host_accessible !26 !kernel_arg_pipe_depth !27 !kernel_arg_pipe_io !25 !kernel_arg_buffer_location !25 !kernel_arg_name !28 !use_fpga_pipes !29 !vectorized_kernel !59 !recommended_vector_length !60 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %p, metadata !54, metadata !DIExpression()), !dbg !61
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %res, metadata !55, metadata !DIExpression()), !dbg !61
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4, !dbg !62
  %conv = trunc i64 %call to i32, !dbg !62
  call void @llvm.dbg.value(metadata i32 %conv, metadata !56, metadata !DIExpression()), !dbg !61
  %cmp = icmp eq i32 %conv, 0, !dbg !63
  br i1 %cmp, label %for.body.preheader, label %cleanup, !dbg !65

for.body.preheader:                               ; preds = %entry
  br label %for.body, !dbg !66

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !57, metadata !DIExpression()), !dbg !67
  %call4 = tail call i32 @_Z10atomic_addPU3AS1Vjj(i32 addrspace(1)* %p, i32 1) #5, !dbg !68
  %0 = load i32, i32 addrspace(1)* %p, align 4, !dbg !71, !tbaa !40
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %res, i64 %indvars.iv, !dbg !72
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4, !dbg !73, !tbaa !40
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !74
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !57, metadata !DIExpression()), !dbg !67
  %exitcond = icmp eq i64 %indvars.iv.next, 100, !dbg !75
  br i1 %exitcond, label %cleanup.loopexit, label %for.body, !dbg !66, !llvm.loop !76

cleanup.loopexit:                                 ; preds = %for.body
  br label %cleanup, !dbg !78

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret void, !dbg !78
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable willreturn }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind }

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

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "kernel", directory: "file")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, i32 2}
!6 = !{!"-cl-std=CL1.2", !"-g", !"-I."}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!9 = distinct !DISubprogram(name: "test_single_wi", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !16)
!10 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !11)
!11 = !{null, !12, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint", file: !14, line: 43, baseType: !15)
!14 = !DIFile(filename: "opencl-c-common.h", directory: "file")
!15 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!16 = !{!17, !18, !19, !20}
!17 = !DILocalVariable(name: "p", arg: 1, scope: !9, file: !1, line: 1, type: !12)
!18 = !DILocalVariable(name: "res", arg: 2, scope: !9, file: !1, line: 1, type: !12)
!19 = !DILocalVariable(name: "global_id", scope: !9, file: !1, line: 3, type: !13)
!20 = !DILocalVariable(name: "i", scope: !21, file: !1, line: 8, type: !15)
!21 = distinct !DILexicalBlock(scope: !9, file: !1, line: 8, column: 3)
!22 = !{i32 1, i32 1}
!23 = !{!"none", !"none"}
!24 = !{!"uint*", !"uint*"}
!25 = !{!"", !""}
!26 = !{i1 false, i1 false}
!27 = !{i32 0, i32 0}
!28 = !{!"p", !"res"}
!29 = !{i1 false}
!30 = !DILocation(line: 0, scope: !9)
!31 = !DILocation(line: 3, column: 20, scope: !9)
!32 = !DILocation(line: 6, column: 16, scope: !33)
!33 = distinct !DILexicalBlock(scope: !9, file: !1, line: 6, column: 6)
!34 = !DILocation(line: 6, column: 6, scope: !9)
!35 = !DILocation(line: 0, scope: !21)
!36 = !DILocation(line: 9, column: 5, scope: !37)
!37 = distinct !DILexicalBlock(scope: !38, file: !1, line: 8, column: 37)
!38 = distinct !DILexicalBlock(scope: !21, file: !1, line: 8, column: 3)
!39 = !DILocation(line: 10, column: 14, scope: !37)
!40 = !{!41, !41, i64 0}
!41 = !{!"int", !42, i64 0}
!42 = !{!"omnipotent char", !43, i64 0}
!43 = !{!"Simple C/C++ TBAA"}
!44 = !DILocation(line: 10, column: 5, scope: !37)
!45 = !DILocation(line: 10, column: 12, scope: !37)
!46 = !DILocation(line: 8, column: 33, scope: !38)
!47 = !DILocation(line: 8, column: 25, scope: !38)
!48 = !DILocation(line: 8, column: 3, scope: !21)
!49 = distinct !{!49, !48, !50}
!50 = !DILocation(line: 11, column: 3, scope: !21)
!51 = !DILocation(line: 12, column: 1, scope: !9)
!52 = distinct !DISubprogram(name: "test_single_wi", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !53)
!53 = !{!54, !55, !56, !57}
!54 = !DILocalVariable(name: "p", arg: 1, scope: !52, file: !1, line: 1, type: !12)
!55 = !DILocalVariable(name: "res", arg: 2, scope: !52, file: !1, line: 1, type: !12)
!56 = !DILocalVariable(name: "global_id", scope: !52, file: !1, line: 3, type: !13)
!57 = !DILocalVariable(name: "i", scope: !58, file: !1, line: 8, type: !15)
!58 = distinct !DILexicalBlock(scope: !52, file: !1, line: 8, column: 3)
!59 = !{null}
!60 = !{i32 16}
!61 = !DILocation(line: 0, scope: !52)
!62 = !DILocation(line: 3, column: 20, scope: !52)
!63 = !DILocation(line: 6, column: 16, scope: !64)
!64 = distinct !DILexicalBlock(scope: !52, file: !1, line: 6, column: 6)
!65 = !DILocation(line: 6, column: 6, scope: !52)
!66 = !DILocation(line: 8, column: 3, scope: !58)
!67 = !DILocation(line: 0, scope: !58)
!68 = !DILocation(line: 9, column: 5, scope: !69)
!69 = distinct !DILexicalBlock(scope: !70, file: !1, line: 8, column: 37)
!70 = distinct !DILexicalBlock(scope: !58, file: !1, line: 8, column: 3)
!71 = !DILocation(line: 10, column: 14, scope: !69)
!72 = !DILocation(line: 10, column: 5, scope: !69)
!73 = !DILocation(line: 10, column: 12, scope: !69)
!74 = !DILocation(line: 8, column: 33, scope: !70)
!75 = !DILocation(line: 8, column: 25, scope: !70)
!76 = distinct !{!76, !66, !77}
!77 = !DILocation(line: 11, column: 3, scope: !58)
!78 = !DILocation(line: 12, column: 1, scope: !52)
