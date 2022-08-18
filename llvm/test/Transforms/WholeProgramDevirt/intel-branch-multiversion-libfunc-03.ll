; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that the indirect call is preserved after
; devirtualization when the caller is a LibFunc.

; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -whole-program-assume -passes=wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify %s 2>&1 | FileCheck %s

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

@vt1 = constant [1 x i8*] [i8* bitcast (void (i8*, i8*)* @vf to i8*)], !type !8

define void @vf(i8* %a, i8* %b) {
  ret void
}

define void @_ZNSt13runtime_errorC1EPKc(i8* %obj, i8* %alloc) #1 !dbg !5 {
  %vtableptr = bitcast i8* %obj to [1 x i8*]**
  %vtable = load [1 x i8*]*, [1 x i8*]** %vtableptr
  %vtablei8 = bitcast [1 x i8*]* %vtable to i8*
  %p = call i1 @llvm.type.test(i8* %vtablei8, metadata !"typeid")
  call void @llvm.assume(i1 %p)
  %fptrptr = getelementptr [1 x i8*], [1 x i8*]* %vtable, i32 0, i32 0
  %fptr = load i8*, i8** %fptrptr
  %fptr_casted = bitcast i8* %fptr to void (i8*, i8*)*
  call void %fptr_casted(i8* %obj, i8* %alloc), !dbg !6
  ret void
}

declare i1 @llvm.type.test(i8*, metadata)
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

; Check that the branching to vf is generated
; CHECK: [[T1:%[^ ]*]] = bitcast void (i8*, i8*)* %fptr_casted to i8*
; CHECK: [[T2:%[^ ]*]] = bitcast void (i8*, i8*)* @vf to i8*
; CHECK: [[T3:%[^ ]*]] = icmp eq i8* [[T1]], [[T2]]
; CHECK: br i1 [[T3]], label %BBDevirt_vf, label %DefaultBB

; Check that the call to vf is generated
; CHECK: call void @vf(i8* %obj, i8* %alloc)
; CHECK: br label %MergeBB

; Check that the default branch is generated
; CHECK: call void %fptr_casted(i8* %obj, i8* %alloc)
; CHECK: br label %MergeBB

; Check the merge
; CHECK: br label [[T4:%[^ ]*]]

; end INTEL_FEATURE_SW_DTRANS
