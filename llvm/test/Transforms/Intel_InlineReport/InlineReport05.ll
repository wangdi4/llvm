; RUN: opt -inline -inline-report=31 < %s -S 2>&1 | FileCheck %s

; Generated with clang -c -S -g -emit-llvm sm1.c

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
  %call = call i32 @foo(), !dbg !12
  ret i32 %call, !dbg !13
}

; Function Attrs: alwaysinline nounwind uwtable
define internal i32 @foo() #1 {
entry:
  ret i32 5, !dbg !14
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!9, !10}
!llvm.ident = !{!11}

!0 = !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.7.0 (trunk 967) (llvm/branches/ltoprof 992)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, retainedTypes: !2, subprograms: !3, globals: !2, imports: !2)
!1 = !DIFile(filename: "sm1.c", directory: "/users/rcox2/rgSmall")
!2 = !{}
!3 = !{!4, !8}
!4 = !DISubprogram(name: "fred", scope: !1, file: !1, line: 6, type: !5, isLocal: false, isDefinition: true, scopeLine: 7, isOptimized: false, function: i32 ()* @fred, variables: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!8 = !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !5, isLocal: true, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, function: i32 ()* @foo, variables: !2)
!9 = !{i32 2, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{!"clang version 3.7.0 (trunk 967) (llvm/branches/ltoprof 992)"}
!12 = !DILocation(line: 8, column: 11, scope: !4)
!13 = !DILocation(line: 8, column: 4, scope: !4)
!14 = !DILocation(line: 3, column: 4, scope: !8)
