; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that the indirect call is preserved after
; devirtualization in one caller function (@_ZNSt13runtime_errorC1EPKc),
; while it is removed in the other caller because is not a libfunc (@foo).
; Also, this test checks that the multiversioning happens in @foo since
; the virtual call has multiple targets.

; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -whole-program-assume -passes=wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify %s 2>&1 | FileCheck %s

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

@vt1 = constant [1 x ptr] [ptr bitcast (ptr @vf to ptr)], !type !8
@vt2 = constant [1 x ptr] [ptr bitcast (ptr @vf2 to ptr)], !type !8


define internal void @vf(ptr %a, ptr %b) {
  ret void
}

define internal void @vf2(ptr %a, ptr %b) {
  ret void
}


define void @_ZNSt13runtime_errorC1EPKc(ptr %obj, ptr %alloc) #1 !dbg !5 {
  %vtableptr = bitcast ptr %obj to ptr
  %vtable = load ptr, ptr %vtableptr
  %vtablei8 = bitcast ptr %vtable to ptr
  %p = call i1 @llvm.type.test(ptr %vtablei8, metadata !"typeid")
  call void @llvm.assume(i1 %p)
  %fptrptr = getelementptr [1 x ptr], ptr %vtable, i32 0, i32 0
  %fptr = load ptr, ptr %fptrptr
  %fptr_casted = bitcast ptr %fptr to ptr
  call void %fptr_casted(ptr %obj, ptr %alloc), !dbg !6
  ret void
}

define internal void @foo(ptr %obj, ptr %alloc) #1 !dbg !5 {
  %vtableptr = bitcast ptr %obj to ptr
  %vtable = load ptr, ptr %vtableptr
  %vtablei8 = bitcast ptr %vtable to ptr
  %p = call i1 @llvm.type.test(ptr %vtablei8, metadata !"typeid")
  call void @llvm.assume(i1 %p)
  %fptrptr = getelementptr [1 x ptr], ptr %vtable, i32 0, i32 0
  %fptr = load ptr, ptr %fptrptr
  %fptr_casted = bitcast ptr %fptr to ptr
  call void %fptr_casted(ptr %obj, ptr %alloc), !dbg !6
  ret void
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!llvm.ident = !{!4}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.0 (trunk 278098)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "devirt-single.cc", directory: ".")
!2 = !{i32 2, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"clang version 4.0.0 (trunk 278098)"}
!5 = distinct !DISubprogram(name: "call", linkageName: "_Z4callPv", scope: !1, file: !1, line: 29, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: false, unit: !0)
!6 = !DILocation(line: 30, column: 32, scope: !5)
!7 = distinct !DISubprogram(name: "vf", linkageName: "_ZN3vt12vfEv", scope: !1, file: !1, line: 13, isLocal: false, isDefinition: true, scopeLine: 13, flags: DIFlagPrototyped, isOptimized: false, unit: !0)
!8 = !{i32 0, !"typeid"}

; Check that the branching to vf was generated in @_ZNSt13runtime_errorC1EPKc
; CHECK: define void @_ZNSt13runtime_errorC1EPKc(ptr %obj, ptr %alloc)
; CHECK: [[T1:%[^ ]*]] = bitcast ptr %fptr_casted to ptr
; CHECK: [[T2:%[^ ]*]] = bitcast ptr @vf to ptr
; CHECK: [[T3:%[^ ]*]] = icmp eq ptr [[T1]], [[T2]]
; CHECK: br i1 [[T3]], label %BBDevirt_vf, label %ElseDevirt_vf

; Check that the call to vf is generated
; CHECK: call void @vf(ptr %obj, ptr %alloc)
; CHECK: br label %MergeBB

; Check that the branching to vf2 was generated correctly
; CHECK: [[T4:%[^ ]*]] = bitcast ptr @vf2 to ptr
; CHECK: [[T5:%[^ ]*]] = icmp eq ptr [[T1]], [[T4]]
; CHECK: br i1 [[T5]], label %BBDevirt_vf2, label %DefaultBB

; Check that the call to vf2 was generated
; CHECK: call void @vf2(ptr %obj, ptr %alloc)
; CHECK:  br label %MergeBB

; Check that the default branch is generated
; CHECK: call void %fptr_casted(ptr %obj, ptr %alloc)
; CHECK: br label %MergeBB

; Check the merge
; CHECK: br label [[T6:%[^ ]*]]

; Check that the multiverisoning in @foo was generated without the
; indirect call.
; CHECK: define internal void @foo(ptr %obj, ptr %alloc)
; CHECK: [[T1:%[^ ]*]] = bitcast ptr %fptr_casted to ptr
; CHECK: [[T2:%[^ ]*]] = bitcast ptr @vf to ptr
; CHECK: [[T3:%[^ ]*]] = icmp eq ptr [[T1]], [[T2]]
; CHECK: br i1 [[T3]], label %BBDevirt_vf, label %BBDevirt_vf2

; Check that the call to vf is generated
; CHECK: call void @vf(ptr %obj, ptr %alloc)
; CHECK: br label %MergeBB

; Check that the call to vf2 was generated
; CHECK: call void @vf2(ptr %obj, ptr %alloc)
; CHECK:  br label %MergeBB

; Check the merge
; CHECK: br label [[T4:%[^ ]*]]

; CHECK-NOT: call void %fptr_casted(ptr %obj, ptr %alloc)

; end INTEL_FEATURE_SW_DTRANS
