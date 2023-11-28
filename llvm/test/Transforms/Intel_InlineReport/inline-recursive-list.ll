; Set callee (apply to all calls to f2,f4)
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-report=0xe807 -inline-recursive-list="f2;f4" < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-C,CHECK-C-IR

; Set caller,callee (all calls to f4 within main)
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-report=0xe807 -inline-recursive-list="f4;main,f4" < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CC,CHECK-CC-IR

; Set caller,callee,linenum (specific call to f3 within main)
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-report=0xe807 -inline-recursive-list="main,f3,23" < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-CCL,CHECK-CCL-IR

; Check metadata inlining report
; RUN: opt -passes='inlinereportsetup,inlinelists,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-recursive-list="f2;f4" -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-C,CHECK-C-META-IR
; RUN: opt -passes='inlinereportsetup,inlinelists,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-recursive-list="f4;main,f4" -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-CC,CHECK-CC-META-IR
; RUN: opt -passes='inlinereportsetup,inlinelists,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -inline-recursive-list="main,f3,23" -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-CCL,CHECK-CCL-META-IR

; Check list contradictions
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-recursive-list="main,f4" -inline-noinline-list="main,f4" -debug-only=inlinelists < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE
; RUN: opt -passes='inlinelists,cgscc(inline)' -inline-recursive-list="main,f4" -inline-inline-list="main,f4" -debug-only=inlinelists < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-IGNORE

; Test should force recursive inlining according to recursive-inline-list option.

; CHECK-C-IR: call i32 @f1()
; CHECK-C-IR: call i32 @f1()
; CHECK-C-IR: call i32 @f1()
; CHECK-C-IR: call i32 @f1()
; CHECK-C-IR: add nsw i32{{.*}}3
; CHECK-C-IR: add nsw i32{{.*}}7
; CHECK-C-IR: add nsw i32{{.*}}7
; CHECK-C-IR: call i32 @f5()

; CHECK-CC-IR: call i32 @f1()
; CHECK-CC-IR: call i32 @f1()
; CHECK-CC-IR: call i32 @f2()
; CHECK-CC-IR: call i32 @f2()
; CHECK-CC-IR: add nsw i32{{.*}}3
; CHECK-CC-IR: add nsw i32{{.*}}7
; CHECK-CC-IR: add nsw i32{{.*}}7
; CHECK-CC-IR: call i32 @f5()

; CHECK-CCL-IR: call i32 @f1()
; CHECK-CCL-IR: call i32 @f1()
; CHECK-CCL-IR: call i32 @f2()
; CHECK-CCL-IR: call i32 @f2()
; CHECK-CCL-IR: add nsw i32{{.*}}3
; CHECK-CCL-IR: call i32 @f4()
; CHECK-CCL-IR: call i32 @f4()
; CHECK-CCL-IR: call i32 @f5()

; CHECK-C: COMPILE FUNC: f5
; CHECK-C-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-C-NEXT: INLINE: f3{{.*}}Callee is single basic block

; CHECK-C: COMPILE FUNC: main
; CHECK-C-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-C-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-C-NEXT: INLINE: f2{{.*}}Callsite on inline-recursive list
; CHECK-C-NEXT:   f1{{.*}}Callee has noinline attribute
; CHECK-C-NEXT: INLINE: f2{{.*}}Callsite on inline-recursive list
; CHECK-C-NEXT:   f1{{.*}}Callee has noinline attribute
; CHECK-C-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-C-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-C-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-C-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-C-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-C-NEXT: f5{{.*}}Callee has noinline attribute

; CHECK-CC: COMPILE FUNC: f5
; CHECK-CC-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-CC-NEXT: INLINE: f3{{.*}}Callee is single basic block

; CHECK-CC: COMPILE FUNC: main
; CHECK-CC-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-CC-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-CC-NEXT: f2{{.*}}Callee has noinline attribute
; CHECK-CC-NEXT: f2{{.*}}Callee has noinline attribute
; CHECK-CC-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-CC-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-CC-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-CC-NEXT: INLINE: f4{{.*}}Callsite on inline-recursive list
; CHECK-CC-NEXT: INLINE: f3{{.*}}Callee is single basic block
; CHECK-CC-NEXT: f5{{.*}}Callee has noinline attribute

; CHECK-CCL: COMPILE FUNC: f5
; CHECK-CCL-NEXT: f4{{.*}}Callee has noinline attribute

; CHECK-CCL: COMPILE FUNC: main
; CHECK-CCL-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: f1{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: f2{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: f2{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: INLINE: f3{{.*}}Callsite on inline-recursive list
; CHECK-CCL-NEXT: f4{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: f4{{.*}}Callee has noinline attribute
; CHECK-CCL-NEXT: f5{{.*}}Callee has noinline attribute

; CHECK-C-META-IR: call i32 @f1()
; CHECK-C-META-IR: call i32 @f1()
; CHECK-C-META-IR: call i32 @f1()
; CHECK-C-META-IR: call i32 @f1()
; CHECK-C-META-IR: add nsw i32{{.*}}3
; CHECK-C-META-IR: add nsw i32{{.*}}7
; CHECK-C-META-IR: add nsw i32{{.*}}7
; CHECK-C-META-IR: call i32 @f5()

; CHECK-CC-META-IR: call i32 @f1()
; CHECK-CC-META-IR: call i32 @f1()
; CHECK-CC-META-IR: call i32 @f2()
; CHECK-CC-META-IR: call i32 @f2()
; CHECK-CC-META-IR: add nsw i32{{.*}}3
; CHECK-CC-META-IR: add nsw i32{{.*}}7
; CHECK-CC-META-IR: add nsw i32{{.*}}7
; CHECK-CC-META-IR: call i32 @f5()

; CHECK-CCL-META-IR: call i32 @f1()
; CHECK-CCL-META-IR: call i32 @f1()
; CHECK-CCL-META-IR: call i32 @f2()
; CHECK-CCL-META-IR: call i32 @f2()
; CHECK-CCL-META-IR: add nsw i32{{.*}}3
; CHECK-CCL-META-IR: call i32 @f4()
; CHECK-CCL-META-IR: call i32 @f4()
; CHECK-CCL-META-IR: call i32 @f5()


; CHECK-IGNORE: IPO warning: ignoring triple <main,f4> since it is in multiple lists

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32 @f1() #1 !dbg !8 {
entry:
  ret i32 1, !dbg !10
}

; Function Attrs: noinline nounwind uwtable
define i32 @f2() #1 !dbg !11 {
entry:
  %call = call i32 @f1(), !dbg !12
  %add = add nsw i32 %call, 2, !dbg !13
  ret i32 %add, !dbg !14
}

; Function Attrs: nounwind uwtable
define i32 @f3() !dbg !15 {
entry:
  ret i32 3, !dbg !16
}

; Function Attrs: noinline nounwind uwtable
define i32 @f4() #1 !dbg !17 {
entry:
  %call = call i32 @f3(), !dbg !18
  %add = add nsw i32 %call, 4, !dbg !19
  ret i32 %add, !dbg !20
}

define i32 @f5() #1 {
entry:
  %call = call i32 @f4()
  %add = add nsw i32 %call, 5
  ret i32 %add
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
  %call13 = call i32 @f5()
  %add14 = add nsw i32 %add12, %call13
  ret i32 %add14, !dbg !35
}


attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
