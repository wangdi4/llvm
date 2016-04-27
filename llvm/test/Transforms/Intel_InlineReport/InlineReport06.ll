; RUN: opt -inline -inline-report=31 < %s -S 2>&1 | FileCheck %s

; Generated with clang -c -gline-tables-only -S -emit-llvm sm1.c

; CHECK: Begin
; CHECK-NEXT: Option Values:
; CHECK-NEXT: inline-threshold:
; CHECK-NEXT: inlinehint-threshold:
; CHECK-NEXT: inlinecold-threshold:
; CHECK-NEXT: inlineoptsize-threshold:

; ModuleID = 'sm1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: DEAD STATIC FUNC: foo

; CHECK: COMPILE FUNC: fred
; CHECK-NEXT: INLINE: foo
; CHECK-SAME: <stdin>
; CHECK-SAME: (8,11)
; CHECK-SAME: <<Callee is always inline>>

; Function Attrs: nounwind uwtable
define i32 @fred() #0 {
entry:
  %call = call i32 @foo(), !dbg !10
  ret i32 %call, !dbg !11
}

; Function Attrs: alwaysinline nounwind uwtable
define internal i32 @foo() #1 {
entry:
  ret i32 5, !dbg !12
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1)
!1 = !DIFile(filename: "sm1.c", directory: "/users/rcox2/rgSmall")
!4 = distinct !DISubprogram(unit:!0, name: "fred", scope: !1, file: !1, line: 6)
!6 = distinct !DISubprogram(unit:!0, name: "foo", scope: !1, file: !1, line: 1)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!10 = !DILocation(line: 8, column: 11, scope: !4)
!11 = !DILocation(line: 8, column: 4, scope: !4)
!12 = !DILocation(line: 3, column: 4, scope: !6)
