; RUN: opt -S -passes="vec-clone" < %s | FileCheck %s
;
; Validate parameter debug information following the -passes=vec-clone pass.
; Specifically,
;   * Currently, leave the llvm.dbg.declare intrinsics the way they are.
;   * Add llvm.dbg.value intrinsics for parameters into the entry block.
;

; ModuleID = 'test.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @llvm.dbg.declare(metadata, metadata, metadata) #2
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %x, i32 %y) #0 !dbg !5 {
entry:
  %x.addr = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %x.addr, metadata !10, metadata !DIExpression()), !dbg !12
  %y.addr = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %y.addr, metadata !11, metadata !DIExpression()), !dbg !13
  store i32 %x, ptr %x.addr, align 4
  store i32 %y, ptr %y.addr, align 4
  %0 = load i32, ptr %y.addr, align 4
  %1 = load i32, ptr %x.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

; CHECK:      define <4 x i32> @_ZGVbN4lu_foo(i32 %x, i32 %y)
; CHECK-SAME:   !dbg [[VECCLONE_FOO:![0-9]+]]
; CHECK-SAME:   {
; CHECK:      entry:
; CHECK:        %alloca.x = alloca i32
; CHECK:        store i32 %x, ptr %alloca.x
; CHECK:        %alloca.y = alloca i32
; CHECK:        store i32 %y, ptr %alloca.y
; CHECK:        %x.addr = alloca i32
; CHECK:        %y.addr = alloca i32
; CHECK:        call void @llvm.dbg.declare(metadata ptr %x.addr
; CHECK-SAME:                               metadata [[X:![0-9]+]]
; CHECK-SAME:                               metadata !DIExpression()
; CHECK:        call void @llvm.dbg.declare(metadata ptr %y.addr
; CHECK-SAME:                               metadata [[Y:![0-9]+]]
; CHECK-SAME:                               metadata !DIExpression()
; CHECK:      }

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @bar(ptr %a, i32 %b) #1 !dbg !15 {
entry:
  call void @llvm.dbg.value(metadata ptr %a, metadata !17, metadata !DIExpression(DW_OP_deref)), !dbg !19
  call void @llvm.dbg.value(metadata i32 %b, metadata !18, metadata !DIExpression()), !dbg !20
  %idxprom = sext i32 %b to i64
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 20
  call void @llvm.dbg.value(metadata ptr %a, metadata !17, metadata !DIExpression(DW_OP_deref)), !dbg !19
  store i32 %add, ptr %arrayidx, align 4
  ret i32 undef
}

; CHECK:      define {{.*}} @_ZGVbN4ul_bar(ptr %a, i32 %b)
; CHECK-SAME:   !dbg [[VECCLONE_BAR:![0-9]+]]
; CHECK-SAME:   {
; CHECK:      entry:
; CHECK:        call void @llvm.dbg.value(metadata i32 %b
; CHECK-SAME:                             metadata [[B:![0-9]+]]
; CHECK-SAME:                             metadata !DIExpression()
; CHECK:        %alloca.b = alloca i32, align 4
; CHECK:        store i32 %b, ptr %alloca.b, align 4
; CHECK:        call void @llvm.dbg.value(metadata ptr %a
; CHECK-SAME:                             metadata [[A:![0-9]+]]
; CHECK-SAME:                             metadata !DIExpression(DW_OP_deref)
; CHECK-NOT:    call void @llvm.dbg.value(metadata ptr %a
; CHECK:        %alloca.a = alloca ptr, align 8
; CHECK:        store ptr %a, ptr %alloca.a, align 8
; CHECK:      simd.loop.header:
; CHECK:        call void @llvm.dbg.value(metadata ptr %load.a
; CHECK-SAME:                             metadata [[A]]
; CHECK-SAME:                             metadata !DIExpression(DW_OP_deref)
; CHECK:        call void @llvm.dbg.value(metadata i32 %load.b
; CHECK-SAME:                             metadata [[B]]
; CHECK-SAME:                             metadata !DIExpression()
; CHECK:        call void @llvm.dbg.value(metadata ptr %load.a
; CHECK-SAME:                             metadata [[A]]
; CHECK-SAME:                             metadata !DIExpression(DW_OP_deref)
; CHECK:      }

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4lu_foo" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "vector-variants"="_ZGVbN4ul_bar" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !4, producer: "icx", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{!"icx"}
!4 = !DIFile(filename: "test.cpp", directory: "/path/to")
!5 = distinct !DISubprogram(name: "foo", scope: !4, file: !4, line: 1, type: !6, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !9)
!6 = !DISubroutineType(types: !7)
!7 = !{!8}
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !{!10, !11}
!10 = !DILocalVariable(name: "x", line: 3, arg: 1, scope: !5, file: !4, type: !8)
!11 = !DILocalVariable(name: "y", line: 4, arg: 2, scope: !5, file: !4, type: !8)
!12 = !DILocation(line: 3, scope: !5)
!13 = !DILocation(line: 4, scope: !5)

!15 = distinct !DISubprogram(name: "bar", scope: !4, file: !4, line: 11, type: !6, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !16)
!16 = !{!17, !18}
!17 = !DILocalVariable(name: "a", line: 13, arg: 1, scope: !15, file: !4, type: !8)
!18 = !DILocalVariable(name: "b", line: 14, arg: 2, scope: !15, file: !4, type: !8)
!19 = !DILocation(line: 13, scope: !15)
!20 = !DILocation(line: 14, scope: !15)

; CHECK:       [[INT:![0-9]+]] = !DIBasicType(name: "int"
; CHECK:       [[VECCLONE_FOO]] = distinct !DISubprogram(name: "foo"
; CHECK:       [[X]] = !DILocalVariable(name: "x"
; CHECK-SAME:  arg: 1
; CHECK-SAME:  scope: [[VECCLONE_FOO]]
; CHECK-SAME:  type: [[INT]]
; CHECK:       [[Y]] = !DILocalVariable(name: "y"
; CHECK-SAME:  arg: 2
; CHECK-SAME:  scope: [[VECCLONE_FOO]]
; CHECK-SAME:  type: [[INT]]
; CHECK:       [[VECCLONE_BAR]] = distinct !DISubprogram(name: "bar"
; CHECK:       [[A]] = !DILocalVariable(name: "a"
; CHECK-SAME:                           arg: 1
; CHECK-SAME:                           scope: [[VECCLONE_BAR]]
; CHECK-SAME:                           type: [[INT]])
; CHECK:       [[B]] = !DILocalVariable(name: "b"
; CHECK-SAME:                           arg: 2
; CHECK-SAME:                           scope: [[VECCLONE_BAR]]
; CHECK-SAME:                           type: [[INT]])

