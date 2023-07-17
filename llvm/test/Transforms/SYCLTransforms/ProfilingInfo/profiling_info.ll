; RUN: opt -S -passes=sycl-kernel-profiling-info %s | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @test(ptr noundef %p) #0 !dbg !8 !kernel_arg_base_type !43 !arg_type_null_val !44 {
; CHECK-NOT: call void @llvm.dbg.declare
; CHECK-NOT: call void @llvm.dbg.value
entry:
  %retval = alloca i32, align 4
  %p.addr = alloca ptr, align 8
  %res = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %p, ptr %p.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %p.addr, metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.declare(metadata ptr %res, metadata !16, metadata !DIExpression()), !dbg !17
  store i32 0, ptr %res, align 4, !dbg !17
  %0 = load ptr, ptr %p.addr, align 8, !dbg !18
  %tobool = icmp ne ptr %0, null, !dbg !18
  br i1 %tobool, label %if.end, label %if.then, !dbg !20

if.then:                                          ; preds = %entry
  store i32 0, ptr %retval, align 4, !dbg !21
  br label %return, !dbg !21

if.end:                                           ; preds = %entry
  call void @llvm.dbg.declare(metadata ptr %i, metadata !22, metadata !DIExpression()), !dbg !24
  store i32 0, ptr %i, align 4, !dbg !24
  br label %for.cond, !dbg !25

for.cond:                                         ; preds = %for.inc, %if.end
  %1 = load i32, ptr %i, align 4, !dbg !26
  %2 = load ptr, ptr %p.addr, align 8, !dbg !28
  %3 = load i32, ptr %2, align 4, !dbg !29
  %cmp = icmp slt i32 %1, %3, !dbg !30
  br i1 %cmp, label %for.body, label %for.end, !dbg !31

for.body:                                         ; preds = %for.cond
  %4 = load i32, ptr %i, align 4, !dbg !32
  %5 = load i32, ptr %res, align 4, !dbg !33
  %add = add nsw i32 %5, %4, !dbg !33
  store i32 %add, ptr %res, align 4, !dbg !33
  br label %for.inc, !dbg !34

for.inc:                                          ; preds = %for.body
  %6 = load i32, ptr %i, align 4, !dbg !35
  %inc = add nsw i32 %6, 1, !dbg !35
  store i32 %inc, ptr %i, align 4, !dbg !35
  br label %for.cond, !dbg !36, !llvm.loop !37

for.end:                                          ; preds = %for.cond
  %7 = load i32, ptr %res, align 4, !dbg !40
  store i32 %7, ptr %retval, align 4, !dbg !41
  br label %return, !dbg !41

return:                                           ; preds = %for.end, %if.then
  %8 = load i32, ptr %retval, align 4, !dbg !42
  ret i32 %8, !dbg !42
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, flags: " -g -O0 -c test.c -S -emit-llvm", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 2}
!6 = !{i32 7, !"frame-pointer", i32 2}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!13 = !{}
!14 = !DILocalVariable(name: "p", arg: 1, scope: !8, file: !1, line: 1, type: !12)
!15 = !DILocation(line: 1, column: 15, scope: !8)
!16 = !DILocalVariable(name: "res", scope: !8, file: !1, line: 2, type: !11)
!17 = !DILocation(line: 2, column: 7, scope: !8)
!18 = !DILocation(line: 3, column: 8, scope: !19)
!19 = distinct !DILexicalBlock(scope: !8, file: !1, line: 3, column: 7)
!20 = !DILocation(line: 3, column: 7, scope: !8)
!21 = !DILocation(line: 3, column: 11, scope: !19)
!22 = !DILocalVariable(name: "i", scope: !23, file: !1, line: 4, type: !11)
!23 = distinct !DILexicalBlock(scope: !8, file: !1, line: 4, column: 3)
!24 = !DILocation(line: 4, column: 11, scope: !23)
!25 = !DILocation(line: 4, column: 7, scope: !23)
!26 = !DILocation(line: 4, column: 18, scope: !27)
!27 = distinct !DILexicalBlock(scope: !23, file: !1, line: 4, column: 3)
!28 = !DILocation(line: 4, column: 23, scope: !27)
!29 = !DILocation(line: 4, column: 22, scope: !27)
!30 = !DILocation(line: 4, column: 20, scope: !27)
!31 = !DILocation(line: 4, column: 3, scope: !23)
!32 = !DILocation(line: 5, column: 12, scope: !27)
!33 = !DILocation(line: 5, column: 9, scope: !27)
!34 = !DILocation(line: 5, column: 5, scope: !27)
!35 = !DILocation(line: 4, column: 27, scope: !27)
!36 = !DILocation(line: 4, column: 3, scope: !27)
!37 = distinct !{!37, !31, !38, !39}
!38 = !DILocation(line: 5, column: 12, scope: !23)
!39 = !{!"llvm.loop.mustprogress"}
!40 = !DILocation(line: 6, column: 10, scope: !8)
!41 = !DILocation(line: 6, column: 3, scope: !8)
!42 = !DILocation(line: 7, column: 1, scope: !8)
!43 = !{!"int*"}
!44 = !{ptr null}
