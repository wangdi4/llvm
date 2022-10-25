; Inline report
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-report=0xe807 -inline-inline-list="main,f1;main,f2,22;f3" -inline-noinline-list="main,f2,21;main,f4,25" < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='inlinelists,cgscc(inline)' -inline-report=0xe886 -inline-inline-list="main,f1;main,f2,22;f3" -inline-noinline-list="main,f2,21;main,f4,25" -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Test should force inlining and not inlining functions according to inline and noinline list options.

; CHECK: COMPILE FUNC: f2
; CHECK-NEXT: f1{{.*}}Callee is single basic block

; CHECK: COMPILE FUNC: f4
; CHECK-NEXT: INLINE: f3{{.*}}Callsite on inline list

; CHECK: COMPILE FUNC: main
; CHECK-NEXT: INLINE: f1{{.*}}Callsite on inline list
; CHECK-NEXT: INLINE: f1{{.*}}Callsite on inline list
; CHECK-NEXT: f2{{.*}}Callsite on noinline list
; CHECK-NEXT: INLINE: f2{{.*}}Callsite on inline list
; CHECK-NEXT: f1{{.*}}Callee is single basic block
; CHECK-NEXT: INLINE: f3{{.*}}Callsite on inline list
; CHECK-NEXT: INLINE: f4{{.*}}Callee is always inline
; CHECK-NEXT: INLINE: f3{{.*}}Callsite on inline list
; CHECK-NEXT: f4{{.*}}Callsite on noinline list


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @f1() #0 !dbg !8 {
entry:
  ret i32 1, !dbg !10
}

; Function Attrs: nounwind uwtable
define i32 @f2() #0 !dbg !11 {
entry:
  %call = call i32 @f1(), !dbg !12
  %add = add nsw i32 %call, 2, !dbg !13
  ret i32 %add, !dbg !14
}

; Function Attrs: noinline nounwind uwtable
define i32 @f3() #1 !dbg !15 {
entry:
  ret i32 3, !dbg !16
}

; Function Attrs: alwaysinline nounwind uwtable
define i32 @f4() #2 !dbg !17 {
entry:
  %call = call i32 @f3(), !dbg !18
  %add = add nsw i32 %call, 4, !dbg !19
  ret i32 %add, !dbg !20
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 !dbg !21 {
entry:
  %call = call i32 @f1(), !dbg !22
  %call1 = call i32 @f1(), !dbg !23
  %add2 = add nsw i32 %call, %call1, !dbg !24
  %call3 = call i32 @f2(), !dbg !25
  %add4 = add nsw i32 %add2, %call3, !dbg !26
  %call5 = call i32 @f2(), !dbg !27
  %add6 = add nsw i32 %add4, %call5, !dbg !28
  %call7 = call i32 @f3(), !dbg !29
  %add8 = add nsw i32 %add6, %call7, !dbg !30
  %call9 = call i32 @f4(), !dbg !31
  %add10 = add nsw i32 %add8, %call9, !dbg !32
  %call11 = call i32 @f4(), !dbg !33
  %add12 = add nsw i32 %add10, %call11, !dbg !34
  ret i32 %add12, !dbg !35
}


attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 6.0.0")
!1 = !DIFile(filename: "test1.c", directory: "/export/users/ochupina/ipo_trackers/inllist")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 6.0.0"}
!8 = distinct !DISubprogram(name: "f1", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, isOptimized: true, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 2, column: 3, scope: !8)
!11 = distinct !DISubprogram(name: "f2", scope: !1, file: !1, line: 5, type: !9, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: true, unit: !0, retainedNodes: !2)
!12 = !DILocation(line: 6, column: 10, scope: !11)
!13 = !DILocation(line: 6, column: 14, scope: !11)
!14 = !DILocation(line: 6, column: 3, scope: !11)
!15 = distinct !DISubprogram(name: "f3", scope: !1, file: !1, line: 9, type: !9, isLocal: false, isDefinition: true, scopeLine: 9, isOptimized: true, unit: !0, retainedNodes: !2)
!16 = !DILocation(line: 10, column: 3, scope: !15)
!17 = distinct !DISubprogram(name: "f4", scope: !1, file: !1, line: 13, type: !9, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: true, unit: !0, retainedNodes: !2)
!18 = !DILocation(line: 14, column: 10, scope: !17)
!19 = !DILocation(line: 14, column: 14, scope: !17)
!20 = !DILocation(line: 14, column: 3, scope: !17)
!21 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !9, isLocal: false, isDefinition: true, scopeLine: 17, isOptimized: true, unit: !0, retainedNodes: !2)
!22 = !DILocation(line: 19, column: 8, scope: !21)
!23 = !DILocation(line: 20, column: 8, scope: !21)
!24 = !DILocation(line: 20, column: 6, scope: !21)
!25 = !DILocation(line: 21, column: 8, scope: !21)
!26 = !DILocation(line: 21, column: 6, scope: !21)
!27 = !DILocation(line: 22, column: 8, scope: !21)
!28 = !DILocation(line: 22, column: 6, scope: !21)
!29 = !DILocation(line: 23, column: 8, scope: !21)
!30 = !DILocation(line: 23, column: 6, scope: !21)
!31 = !DILocation(line: 24, column: 8, scope: !21)
!32 = !DILocation(line: 24, column: 6, scope: !21)
!33 = !DILocation(line: 25, column: 8, scope: !21)
!34 = !DILocation(line: 25, column: 6, scope: !21)
!35 = !DILocation(line: 26, column: 3, scope: !21)
