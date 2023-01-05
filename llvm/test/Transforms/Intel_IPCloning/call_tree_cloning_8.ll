; Checks that the Call Tree Cloning transformation clones expected function with !dbg metadata

; RUN: opt < %s -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-do-mv=false -S | FileCheck %s

; CHECK:call fastcc void @"foo.1|7"(), !dbg !61
;                                      ^^^^^^^^

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"      foo(%d)\0A\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"        loop1:%d\0A\00", align 1
@.str.2 = private unnamed_addr constant [18 x i8] c"        loop2:%d\0A\00", align 1
@glob = common dso_local local_unnamed_addr global i32 0, align 4, !dbg !0

; Function Attrs: noinline nounwind uwtable
define internal fastcc void @foo(i32) unnamed_addr #0 !dbg !13 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !17, metadata !DIExpression()), !dbg !19
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32 %0), !dbg !20
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !21
  %3 = add i32 %0, 4, !dbg !22
  %4 = icmp sgt i32 %3, 0, !dbg !25
  br i1 %4, label %9, label %5, !dbg !26

; <label>:5:                                      ; preds = %9, %1
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !21
  %6 = shl i32 %0, 1, !dbg !27
  %7 = add i32 %6, 14, !dbg !30
  %8 = icmp sgt i32 %7, 0, !dbg !31
  br i1 %8, label %13, label %17, !dbg !32

; <label>:9:                                      ; preds = %9, %1
  %10 = phi i32 [ %11, %9 ], [ 0, %1 ]
  call void @llvm.dbg.value(metadata i32 %10, metadata !18, metadata !DIExpression()), !dbg !21
  tail call void @printf1(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i64 0, i64 0), i32 %10) #5, !dbg !33
  %11 = add nuw nsw i32 %10, 1, !dbg !35
  call void @llvm.dbg.value(metadata i32 %11, metadata !18, metadata !DIExpression()), !dbg !21
  %12 = icmp eq i32 %11, %3, !dbg !25
  br i1 %12, label %5, label %9, !dbg !26, !llvm.loop !36

; <label>:13:                                     ; preds = %13, %5
  %14 = phi i32 [ %15, %13 ], [ 0, %5 ]
  call void @llvm.dbg.value(metadata i32 %14, metadata !18, metadata !DIExpression()), !dbg !21
  tail call void @printf1(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.2, i64 0, i64 0), i32 %14) #5, !dbg !38
  %15 = add nuw nsw i32 %14, 1, !dbg !40
  call void @llvm.dbg.value(metadata i32 %15, metadata !18, metadata !DIExpression()), !dbg !21
  %16 = icmp eq i32 %15, %7, !dbg !31
  br i1 %16, label %17, label %13, !dbg !32, !llvm.loop !41

; <label>:17:                                     ; preds = %13, %5
  ret void, !dbg !43
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

declare dso_local void @printf1(i8*, i32) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #4 !dbg !44 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !51, metadata !DIExpression()), !dbg !53
  call void @llvm.dbg.value(metadata i8** %1, metadata !52, metadata !DIExpression()), !dbg !54
  store i32 %0, i32* @glob, align 4, !dbg !55, !tbaa !56
  tail call fastcc void @foo(i32 %0), !dbg !60
  tail call fastcc void @foo.1(i32 7), !dbg !61
  ret i32 0, !dbg !62
}

; Function Attrs: noinline nounwind uwtable
define internal fastcc void @foo.1(i32) unnamed_addr #0 !dbg !63 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !65, metadata !DIExpression()), !dbg !67
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32 %0), !dbg !68
  call void @llvm.dbg.value(metadata i32 0, metadata !66, metadata !DIExpression()), !dbg !69
  %3 = add i32 %0, 4, !dbg !70
  %4 = icmp sgt i32 %3, 0, !dbg !73
  br i1 %4, label %9, label %5, !dbg !74

; <label>:5:                                      ; preds = %9, %1
  call void @llvm.dbg.value(metadata i32 0, metadata !66, metadata !DIExpression()), !dbg !69
  %6 = shl i32 %0, 1, !dbg !75
  %7 = add i32 %6, 14, !dbg !78
  %8 = icmp sgt i32 %7, 0, !dbg !79
  br i1 %8, label %13, label %17, !dbg !80

; <label>:9:                                      ; preds = %9, %1
  %10 = phi i32 [ %11, %9 ], [ 0, %1 ]
  call void @llvm.dbg.value(metadata i32 %10, metadata !66, metadata !DIExpression()), !dbg !69
  tail call void @printf1(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i64 0, i64 0), i32 %10) #5, !dbg !81
  %11 = add nuw nsw i32 %10, 1, !dbg !83
  call void @llvm.dbg.value(metadata i32 %11, metadata !66, metadata !DIExpression()), !dbg !69
  %12 = icmp eq i32 %11, %3, !dbg !73
  br i1 %12, label %5, label %9, !dbg !74, !llvm.loop !84

; <label>:13:                                     ; preds = %13, %5
  %14 = phi i32 [ %15, %13 ], [ 0, %5 ]
  call void @llvm.dbg.value(metadata i32 %14, metadata !66, metadata !DIExpression()), !dbg !69
  tail call void @printf1(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.2, i64 0, i64 0), i32 %14) #5, !dbg !86
  %15 = add nuw nsw i32 %14, 1, !dbg !88
  call void @llvm.dbg.value(metadata i32 %15, metadata !66, metadata !DIExpression()), !dbg !69
  %16 = icmp eq i32 %15, %7, !dbg !79
  br i1 %16, label %17, label %13, !dbg !80, !llvm.loop !89

; <label>:17:                                     ; preds = %13, %5
  ret void, !dbg !91
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.dbg.intel.emit_class_debug_always = !{!7}
!llvm.ident = !{!8}
!llvm.module.flags = !{!9, !10, !11, !12}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "glob", scope: !2, file: !3, line: 17, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 8.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "test.c", directory: "/export/iusers/cczhao/Workspaces/xmainipo/llvm/test/Transforms/Intel_IPCloning")
!4 = !{}
!5 = !{!0}
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!"true"}
!8 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6b9bbf6f8fb943b10e475a754c407b0c06e3ca95) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 57f190924c00ec3ba4944ff14132dc2ade478278)"}
!9 = !{i32 2, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 1, !"ThinLTO", i32 0}
!13 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 21, type: !14, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{null, !6}
!16 = !{!17, !18}
!17 = !DILocalVariable(name: "p1", arg: 1, scope: !13, file: !3, line: 21, type: !6)
!18 = !DILocalVariable(name: "i", scope: !13, file: !3, line: 22, type: !6)
!19 = !DILocation(line: 21, column: 18, scope: !13)
!20 = !DILocation(line: 23, column: 3, scope: !13)
!21 = !DILocation(line: 22, column: 7, scope: !13)
!22 = !DILocation(line: 24, column: 20, scope: !23)
!23 = distinct !DILexicalBlock(scope: !24, file: !3, line: 24, column: 3)
!24 = distinct !DILexicalBlock(scope: !13, file: !3, line: 24, column: 3)
!25 = !DILocation(line: 24, column: 16, scope: !23)
!26 = !DILocation(line: 24, column: 3, scope: !24)
!27 = !DILocation(line: 25, column: 19, scope: !28)
!28 = distinct !DILexicalBlock(scope: !29, file: !3, line: 25, column: 3)
!29 = distinct !DILexicalBlock(scope: !13, file: !3, line: 25, column: 3)
!30 = !DILocation(line: 25, column: 22, scope: !28)
!31 = !DILocation(line: 25, column: 16, scope: !28)
!32 = !DILocation(line: 25, column: 3, scope: !29)
!33 = !DILocation(line: 24, column: 33, scope: !34)
!34 = distinct !DILexicalBlock(scope: !23, file: !3, line: 24, column: 31)
!35 = !DILocation(line: 24, column: 24, scope: !23)
!36 = distinct !{!36, !26, !37}
!37 = !DILocation(line: 24, column: 66, scope: !24)
!38 = !DILocation(line: 25, column: 33, scope: !39)
!39 = distinct !DILexicalBlock(scope: !28, file: !3, line: 25, column: 31)
!40 = !DILocation(line: 25, column: 27, scope: !28)
!41 = distinct !{!41, !32, !42}
!42 = !DILocation(line: 25, column: 66, scope: !29)
!43 = !DILocation(line: 26, column: 1, scope: !13)
!44 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 49, type: !45, scopeLine: 49, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !50)
!45 = !DISubroutineType(types: !46)
!46 = !{!6, !6, !47}
!47 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !48, size: 64)
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !49, size: 64)
!49 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!50 = !{!51, !52}
!51 = !DILocalVariable(name: "argc", arg: 1, scope: !44, file: !3, line: 49, type: !6)
!52 = !DILocalVariable(name: "argv", arg: 2, scope: !44, file: !3, line: 49, type: !47)
!53 = !DILocation(line: 49, column: 14, scope: !44)
!54 = !DILocation(line: 49, column: 27, scope: !44)
!55 = !DILocation(line: 50, column: 8, scope: !44)
!56 = !{!57, !57, i64 0}
!57 = !{!"int", !58, i64 0}
!58 = !{!"omnipotent char", !59, i64 0}
!59 = !{!"Simple C/C++ TBAA"}
!60 = !DILocation(line: 52, column: 3, scope: !44)
!61 = !DILocation(line: 53, column: 3, scope: !44)
!62 = !DILocation(line: 55, column: 3, scope: !44)
!63 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 21, type: !14, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !64)
!64 = !{!65, !66}
!65 = !DILocalVariable(name: "p1", arg: 1, scope: !63, file: !3, line: 21, type: !6)
!66 = !DILocalVariable(name: "i", scope: !63, file: !3, line: 22, type: !6)
!67 = !DILocation(line: 21, column: 18, scope: !63)
!68 = !DILocation(line: 23, column: 3, scope: !63)
!69 = !DILocation(line: 22, column: 7, scope: !63)
!70 = !DILocation(line: 24, column: 20, scope: !71)
!71 = distinct !DILexicalBlock(scope: !72, file: !3, line: 24, column: 3)
!72 = distinct !DILexicalBlock(scope: !63, file: !3, line: 24, column: 3)
!73 = !DILocation(line: 24, column: 16, scope: !71)
!74 = !DILocation(line: 24, column: 3, scope: !72)
!75 = !DILocation(line: 25, column: 19, scope: !76)
!76 = distinct !DILexicalBlock(scope: !77, file: !3, line: 25, column: 3)
!77 = distinct !DILexicalBlock(scope: !63, file: !3, line: 25, column: 3)
!78 = !DILocation(line: 25, column: 22, scope: !76)
!79 = !DILocation(line: 25, column: 16, scope: !76)
!80 = !DILocation(line: 25, column: 3, scope: !77)
!81 = !DILocation(line: 24, column: 33, scope: !82)
!82 = distinct !DILexicalBlock(scope: !71, file: !3, line: 24, column: 31)
!83 = !DILocation(line: 24, column: 24, scope: !71)
!84 = distinct !{!84, !74, !85}
!85 = !DILocation(line: 24, column: 66, scope: !72)
!86 = !DILocation(line: 25, column: 33, scope: !87)
!87 = distinct !DILexicalBlock(scope: !76, file: !3, line: 25, column: 31)
!88 = !DILocation(line: 25, column: 27, scope: !76)
!89 = distinct !{!89, !80, !90}
!90 = !DILocation(line: 25, column: 66, scope: !77)
!91 = !DILocation(line: 26, column: 1, scope: !63)

; Compile: icx -g -flto t.c
; Original source t.c is below.

; int glob;
; #define NOI __declspec(noinline)
;
;NOI void foo(int p1){
;  int i;
;  printf("      foo(%d)\n", p1);
;  for (i = 0;i < p1+4;i++)    { printf1("        loop1:%d\n", i);}
;  for (i = 0;i < 2*p1+14;i++) { printf1("        loop2:%d\n", i);}
;}
;
;int main(int argc, char** argv) {
;  glob = argc;
;  foo(glob);
;  foo(7);
;  return 0;
; }
;
