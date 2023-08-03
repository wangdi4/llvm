; This test is same as devirt_single_after_filtering_unreachable_function.ll
; except "call void @exit()" call instead of "call void @llvm.trap()"
; in @_ZN4BaseD0Ev function.
; Test that @_ZN4BaseD0Ev function is not detected as unreachable function
; due to "call void @exit()" call.

; RUN: opt -S -passes=wholeprogramdevirt -whole-program-visibility -pass-remarks=wholeprogramdevirt %s 2>&1 | FileCheck %s

; CHECK: tail call void %i3
; CHECK-NOT: remark: tmp.cc:21:3: single-impl: devirtualized a call to _ZN7DerivedD0Ev
; CHECK-NOT: remark: <unknown>:0:0: devirtualized _ZN7DerivedD0Ev

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZTV7Derived = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN7DerivedD0Ev] }, !type !0, !type !1, !type !2, !type !3
@_ZTV4Base = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr null, ptr @_ZN4BaseD0Ev] }, !type !0, !type !1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #1

define i32 @func(ptr %b) {
entry:
  %i = bitcast ptr %b to ptr, !dbg !7
  %vtable = load ptr, ptr %i, align 8, !dbg !7
  %i1 = bitcast ptr %vtable to ptr, !dbg !7
  %i2 = tail call i1 @llvm.type.test(ptr %i1, metadata !"_ZTS4Base"), !dbg !7
  tail call void @llvm.assume(i1 %i2), !dbg !7
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 0, !dbg !7
  %i3 = load ptr, ptr %vfn, align 8, !dbg !7
  tail call void %i3(ptr %b), !dbg !7
  ret i32 0
}

define void @_ZN7DerivedD0Ev(ptr %this) {
entry:
  ret void
}

define void @_ZN4BaseD0Ev(ptr %this) {
entry:
  tail call void @exit(i32 1)
  unreachable
}

declare void @exit(i32)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }

!llvm.dbg.cu = !{!4}
!llvm.module.flags = !{!6}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 32, !"_ZTSM4BaseFvvE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 32, !"_ZTSM7DerivedFvvE.virtual"}
!4 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !5, isOptimized: false, runtimeVersion: 0, emissionKind: NoDebug)
!5 = !DIFile(filename: "tmp.cc", directory: "")
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !DILocation(line: 21, column: 3, scope: !8)
!8 = distinct !DISubprogram(name: "func", scope: !5, file: !5, spFlags: DISPFlagDefinition, unit: !4)
