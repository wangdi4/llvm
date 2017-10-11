; RUN: opt -inlinelists -inline -inline-report=7 -inline-inline-list="main,f1;main,f2,22;f3" -inline-noinline-list="main,f2,21;main,f4,26" < %s -S 2>&1 | FileCheck %s
; Test should force inlining and not inlining functions according to inline and noinline list options.

; CHECK: COMPILE FUNC: f2
; CHECK-NEXT: f1{{.*}}Callee has noinline attribute
 
; CHECK: COMPILE FUNC: f4
; CHECK-NEXT: INLINE: f3{{.*}}Callee is in inline list
 
; CHECK: COMPILE FUNC: main
; CHECK-NEXT: llvm.dbg.declare
; CHECK-NEXT: INLINE: f1{{.*}}Callee is in inline list
; CHECK-NEXT: INLINE: f1{{.*}}Callee is in inline list
; CHECK-NEXT: f2{{.*}}Callee is in noinline list
; CHECK-NEXT: INLINE: f2{{.*}}Callee is in inline list
; CHECK-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-NEXT: INLINE: f3{{.*}}Callee is in inline list
; CHECK-NEXT: INLINE: f3{{.*}}Callee is in inline list
; CHECK-NEXT: INLINE: f4{{.*}}Callee is always inline
; CHECK-NEXT: INLINE: f3{{.*}}Callee is in inline list
; CHECK-NEXT:  f4{{.*}}Callee is in noinline list




target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32 @f1() #0 !dbg !7 {
entry:
  ret i32 1, !dbg !11
}

; Function Attrs: noinline nounwind uwtable
define i32 @f2() #0 !dbg !12 {
entry:
  %call = call i32 @f1(), !dbg !13
  %add = add nsw i32 %call, 2, !dbg !14
  ret i32 %add, !dbg !15
}

; Function Attrs: noinline nounwind uwtable
define i32 @f3() #0 !dbg !16 {
entry:
  ret i32 3, !dbg !17
}

; Function Attrs: alwaysinline nounwind uwtable
define i32 @f4() #1 !dbg !18 {
entry:
  %call = call i32 @f3(), !dbg !19
  %add = add nsw i32 %call, 4, !dbg !20
  ret i32 %add, !dbg !21
}

; Function Attrs: noinline nounwind uwtable
define i32 @main() #0 !dbg !22 {
entry:
  %retval = alloca i32, align 4
  %total = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @llvm.dbg.declare(metadata i32* %total, metadata !23, metadata !24), !dbg !25
  store i32 0, i32* %total, align 4, !dbg !25
  %call = call i32 @f1(), !dbg !26
  %call1 = call i32 @f1(), !dbg !27
  %add = add nsw i32 %call, %call1, !dbg !28
  %call2 = call i32 @f2(), !dbg !29
  %add3 = add nsw i32 %add, %call2, !dbg !30
  %call4 = call i32 @f2(), !dbg !31
  %add5 = add nsw i32 %add3, %call4, !dbg !32
  %call6 = call i32 @f3(), !dbg !33
  %add7 = add nsw i32 %add5, %call6, !dbg !34
  %call8 = call i32 @f3(), !dbg !35
  %add9 = add nsw i32 %add7, %call8, !dbg !36
  %call10 = call i32 @f4(), !dbg !37
  %add11 = add nsw i32 %add9, %call10, !dbg !38
  %call12 = call i32 @f4(), !dbg !39
  %add13 = add nsw i32 %add11, %call12, !dbg !40
  %0 = load i32, i32* %total, align 4, !dbg !41
  %add14 = add nsw i32 %0, %add13, !dbg !41
  store i32 %add14, i32* %total, align 4, !dbg !41
  %1 = load i32, i32* %total, align 4, !dbg !42
  ret i32 %1, !dbg !43
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 5.0.0 (cfe/trunk)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test.c", directory: "/export/users/ochupina/workspaces/ws_xmain_ipo_inlinelist/llvm")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 5.0.0 (cfe/trunk)"}
!7 = distinct !DISubprogram(name: "f1", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocation(line: 2, column: 3, scope: !7)
!12 = distinct !DISubprogram(name: "f2", scope: !1, file: !1, line: 5, type: !8, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: false, unit: !0, variables: !2)
!13 = !DILocation(line: 6, column: 11, scope: !12)
!14 = !DILocation(line: 6, column: 16, scope: !12)
!15 = !DILocation(line: 6, column: 3, scope: !12)
!16 = distinct !DISubprogram(name: "f3", scope: !1, file: !1, line: 9, type: !8, isLocal: false, isDefinition: true, scopeLine: 9, isOptimized: false, unit: !0, variables: !2)
!17 = !DILocation(line: 10, column: 3, scope: !16)
!18 = distinct !DISubprogram(name: "f4", scope: !1, file: !1, line: 13, type: !8, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: false, unit: !0, variables: !2)
!19 = !DILocation(line: 14, column: 11, scope: !18)
!20 = !DILocation(line: 14, column: 15, scope: !18)
!21 = !DILocation(line: 14, column: 3, scope: !18)
!22 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !8, isLocal: false, isDefinition: true, scopeLine: 17, isOptimized: false, unit: !0, variables: !2)
!23 = !DILocalVariable(name: "total", scope: !22, file: !1, line: 18, type: !10)
!24 = !DIExpression()
!25 = !DILocation(line: 18, column: 7, scope: !22)
!26 = !DILocation(line: 19, column: 12, scope: !22)
!27 = !DILocation(line: 20, column: 12, scope: !22)
!28 = !DILocation(line: 20, column: 10, scope: !22)
!29 = !DILocation(line: 21, column: 12, scope: !22)
!30 = !DILocation(line: 21, column: 10, scope: !22)
!31 = !DILocation(line: 22, column: 12, scope: !22)
!32 = !DILocation(line: 22, column: 10, scope: !22)
!33 = !DILocation(line: 23, column: 12, scope: !22)
!34 = !DILocation(line: 23, column: 10, scope: !22)
!35 = !DILocation(line: 24, column: 12, scope: !22)
!36 = !DILocation(line: 24, column: 10, scope: !22)
!37 = !DILocation(line: 25, column: 12, scope: !22)
!38 = !DILocation(line: 25, column: 10, scope: !22)
!39 = !DILocation(line: 26, column: 12, scope: !22)
!40 = !DILocation(line: 26, column: 10, scope: !22)
!41 = !DILocation(line: 19, column: 9, scope: !22)
!42 = !DILocation(line: 27, column: 10, scope: !22)
!43 = !DILocation(line: 27, column: 3, scope: !22)
