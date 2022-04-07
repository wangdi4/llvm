; RUN: %oclopt -presucf=true -predicate -S %s | FileCheck %s

; %sext.ucf_allones should share the same DILocation with %sext
; CHECK: %sext = shl i64 %call, 32, !dbg [[SEXT_DBG:![0-9]+]]
; CHECK: %outside.user = add i32 %add2ucf_allones, 3, !dbg [[OUTSIDE_DBG:![0-9]+]]
; CHECK: call void @llvm.dbg.value(metadata i32 %add.ucf_allones, metadata {{![0-9]+}}, metadata !DIExpression()), !dbg {{![0-9]+}}
; CHECK: %sext.ucf_allones = shl i64 %call, 32, !dbg [[SEXT_DBG]]
; CHECK: call void @llvm.dbg.label(metadata {{![0-9]+}}), !dbg {{![0-9]+}}
; CHECK: [[SUBPROG_MD:![0-9]+]] = distinct !DISubprogram(name: "test",
; CHECK: [[SEXT_DBG]] = !DILocation(line: 1, column: 142,
; CHECK: [[OUTSIDE_DBG]] = !DILocation(line: 1, column: 163, scope: [[SUBPROG_MD]])

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata %0, metadata %1, metadata %2) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.label(metadata %0) #2

; Function Attrs: convergent nounwind
define void @__Vectorized_.test(i32 addrspace(1)* %out, i32 addrspace(1)* %in) local_unnamed_addr #0 !dbg !48 !kernel_arg_addr_space !24 !kernel_arg_access_qual !25 !kernel_arg_type !26 !kernel_arg_base_type !26 !kernel_arg_type_qual !27 !kernel_arg_host_accessible !28 !kernel_arg_pipe_depth !29 !kernel_arg_pipe_io !27 !kernel_arg_buffer_location !27 !kernel_arg_name !30 !vectorized_kernel !58 !recommended_vector_length !59 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %out, metadata !50, metadata !DIExpression()), !dbg !60
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %in, metadata !51, metadata !DIExpression()), !dbg !60
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3, !dbg !61
  %conv = trunc i64 %call to i32, !dbg !61
  call void @llvm.dbg.value(metadata i32 %conv, metadata !52, metadata !DIExpression()), !dbg !60
  %cmp = icmp sgt i32 %conv, 0, !dbg !62
  br i1 %cmp, label %if.then, label %if.end7, !dbg !63

if.then:                                          ; preds = %entry
  %0 = load i32, i32 addrspace(1)* %in, align 4, !dbg !64, !tbaa !36
  %cmp2 = icmp sgt i32 %0, 2, !dbg !65
  br i1 %cmp2, label %if.then4, label %phi-split-bb, !dbg !66

if.then4:                                         ; preds = %if.then
  %add = add nuw nsw i32 %0, 1, !dbg !67
  call void @llvm.dbg.value(metadata i32 %add, metadata !53, metadata !DIExpression()), !dbg !68
  %sext = shl i64 %call, 32, !dbg !69
  %idxprom = ashr exact i64 %sext, 32, !dbg !69
  %arrayidx6 = getelementptr inbounds i32, i32 addrspace(1)* %out, i64 %idxprom, !dbg !69
  call void @llvm.dbg.label(metadata !8), !dbg !69
  store i32 %add, i32 addrspace(1)* %arrayidx6, align 4, !dbg !70, !tbaa !36
  br label %phi-split-bb, !dbg !71

phi-split-bb:                                     ; preds = %if.then, %if.then4
  %add.res = phi i32 [ %add, %if.then4 ], [ 0, %if.then ]
  %add2 = add i32 %add.res, 1, !dbg !71
  br label %outside.ucf

outside.ucf:                                      ; preds = %phi-split-bb
  %outside.user = add i32 %add2, 3, !dbg !72
  br label %if.end7

if.end7:                                          ; preds = %outside.ucf, %entry
  ret void, !dbg !72
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable willreturn }
attributes #3 = { convergent nounwind readnone }

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

!0  = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "name", directory: "dir")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, i32 2}
!6 = !{!"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !DILabel(scope: !54, name: "label", file: !10, line: 1)
!9 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !15)
!10 = !DIFile(filename: "1", directory: "dir")
!11 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !12)
!12 = !{null, !13, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{!16, !17, !18, !19}
!16 = !DILocalVariable(name: "out", arg: 1, scope: !9, file: !10, line: 1, type: !13)
!17 = !DILocalVariable(name: "in", arg: 2, scope: !9, file: !10, line: 1, type: !13)
!18 = !DILocalVariable(name: "gid", scope: !9, file: !10, line: 1, type: !14)
!19 = !DILocalVariable(name: "g", scope: !20, file: !10, line: 1, type: !14)
!20 = distinct !DILexicalBlock(scope: !21, file: !10, line: 1, column: 116)
!21 = distinct !DILexicalBlock(scope: !22, file: !10, line: 1, column: 106)
!22 = distinct !DILexicalBlock(scope: !23, file: !10, line: 1, column: 99)
!23 = distinct !DILexicalBlock(scope: !9, file: !10, line: 1, column: 91)
!24 = !{i32 1, i32 1}
!25 = !{!"none", !"none"}
!26 = !{!"int*", !"int*"}
!27 = !{!"", !""}
!28 = !{i1 false, i1 false}
!29 = !{i32 0, i32 0}
!30 = !{!"out", !"in"}
!31 = !DILocation(line: 0, scope: !9)
!32 = !DILocation(line: 1, column: 68, scope: !9)
!33 = !DILocation(line: 1, column: 95, scope: !23)
!34 = !DILocation(line: 1, column: 91, scope: !9)
!35 = !DILocation(line: 1, column: 106, scope: !21)
!36 = !{!37, !37, i64 0}
!37 = !{!"int", !38, i64 0}
!38 = !{!"omnipotent char", !39, i64 0}
!39 = !{!"Simple C/C++ TBAA"}
!40 = !DILocation(line: 1, column: 112, scope: !21)
!41 = !DILocation(line: 1, column: 106, scope: !22)
!42 = !DILocation(line: 1, column: 135, scope: !20)
!43 = !DILocation(line: 0, scope: !20)
!44 = !DILocation(line: 1, column: 142, scope: !20)
!45 = !DILocation(line: 1, column: 151, scope: !20)
!46 = !DILocation(line: 1, column: 157, scope: !20)
!47 = !DILocation(line: 1, column: 163, scope: !9)
!48 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !49)
!49 = !{!50, !51, !52, !53}
!50 = !DILocalVariable(name: "out", arg: 1, scope: !48, file: !10, line: 1, type: !13)
!51 = !DILocalVariable(name: "in", arg: 2, scope: !48, file: !10, line: 1, type: !13)
!52 = !DILocalVariable(name: "gid", scope: !48, file: !10, line: 1, type: !14)
!53 = !DILocalVariable(name: "g", scope: !54, file: !10, line: 1, type: !14)
!54 = distinct !DILexicalBlock(scope: !55, file: !10, line: 1, column: 116)
!55 = distinct !DILexicalBlock(scope: !56, file: !10, line: 1, column: 106)
!56 = distinct !DILexicalBlock(scope: !57, file: !10, line: 1, column: 99)
!57 = distinct !DILexicalBlock(scope: !48, file: !10, line: 1, column: 91)
!58 = !{null}
!59 = !{i32 16}
!60 = !DILocation(line: 0, scope: !48)
!61 = !DILocation(line: 1, column: 68, scope: !48)
!62 = !DILocation(line: 1, column: 95, scope: !57)
!63 = !DILocation(line: 1, column: 91, scope: !48)
!64 = !DILocation(line: 1, column: 106, scope: !55)
!65 = !DILocation(line: 1, column: 112, scope: !55)
!66 = !DILocation(line: 1, column: 106, scope: !56)
!67 = !DILocation(line: 1, column: 135, scope: !54)
!68 = !DILocation(line: 0, scope: !54)
!69 = !DILocation(line: 1, column: 142, scope: !54)
!70 = !DILocation(line: 1, column: 151, scope: !54)
!71 = !DILocation(line: 1, column: 157, scope: !54)
!72 = !DILocation(line: 1, column: 163, scope: !48)


