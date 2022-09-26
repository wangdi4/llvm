; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that the indirect call is preserved and the PHI
; nodes are correct after devirtualization when there are multiple targets
; and at least one target is a LibFunc.

; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -whole-program-assume -passes=wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify %s 2>&1 | FileCheck %s

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

@vt1 = constant [1 x i8*] [i8* bitcast (i32 (i8*, i8*, i32)* @_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl to i8*)], !type !8
@vt2 = constant [1 x i8*] [i8* bitcast (i32 (i8*, i8*, i32)* @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl to i8*)], !type !8

declare i32 @_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl(i8*, i8*, i32)
declare i32 @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl(i8*, i8*, i32)


define i32 @call(i8* %obj, i8* %alloc, i32 %a) #1 !dbg !5 {
  %vtableptr = bitcast i8* %obj to [1 x i8*]**
  %vtable = load [1 x i8*]*, [1 x i8*]** %vtableptr
  %vtablei8 = bitcast [1 x i8*]* %vtable to i8*
  %p = call i1 @llvm.type.test(i8* %vtablei8, metadata !"typeid")
  call void @llvm.assume(i1 %p)
  %fptrptr = getelementptr [1 x i8*], [1 x i8*]* %vtable, i32 0, i32 0
  %fptr = load i8*, i8** %fptrptr
  %fptr_casted = bitcast i8* %fptr to i32 (i8*, i8*, i32)*
  %retval = call i32 %fptr_casted(i8* %obj, i8* %alloc, i32 %a), !dbg !6
  ret i32 %retval
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

; Check that the branching to _ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl is generated
; CHECK: [[T1:%[^ ]*]] = bitcast i32 (i8*, i8*, i32)* %fptr_casted to i8*
; CHECK: [[T2:%[^ ]*]] = bitcast i32 (i8*, i8*, i32)* @_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl to i8*
; CHECK: [[T3:%[^ ]*]] = icmp eq i8* [[T1]], [[T2]]
; CHECK: br i1 [[T3]], label %BBDevirt__ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl, label %ElseDevirt__ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPc

; Check that the call to @_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl is generated
; CHECK: [[T4:%[^ ]*]] = call i32 @_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl(i8* %obj, i8* %alloc, i32 %a)
; CHECK: br label %MergeBB

; Check that the branching to @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl
; was generated correctly
; CHECK: [[T5:%[^ ]*]] = bitcast i32 (i8*, i8*, i32)* @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl to i8*
; CHECK: [[T6:%[^ ]*]] = icmp eq i8* [[T1]], [[T5]]
; CHECK: br i1 [[T6]], label %BBDevirt__ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl, label %DefaultBB

; Check that the call to @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl is generated
; CHECK: [[T7:%[^ ]*]] = call i32 @_ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl(i8* %obj, i8* %alloc, i32 %a)
; CHECK: br label %MergeBB

; Check that the default branch is generated
; CHECK: [[T8:%[^ ]*]] = call i32 %fptr_casted(i8* %obj, i8* %alloc, i32 %a)
; CHECK: br label %MergeBB

; Check that the PHI node was generated correctly
; CHECK: [[T9:%[^ ]*]] = phi i32 [ [[T4]], %BBDevirt__ZNSt15basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl ], [ [[T7]], %BBDevirt__ZNSt15my_basic_streambufIcSt11char_traitsIcEE6xsgetnEPcl ], [ [[T8]], %DefaultBB ]
; CHECK: br label [[T10:%[^ ]*]]

; end INTEL_FEATURE_SW_DTRANS
