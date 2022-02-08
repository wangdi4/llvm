; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans

; This test verifies that instcombine can't convert indirect call to direct
; call by applying constant folding when two GEPs and GlobalAlias are
; involved.
; This test is same as intel-const-fold-geps-1.ll except index of outer
; GEP (i.e %vfn) is 5 instead of 4. It can't find the target since effective
; offset exceeds the size of initializer.

; RUN: opt -instcombine -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes=instcombine -S < %s 2>&1 | FileCheck %s

; InstCombine can't convert %i5 to @foo by applying constant folding.
;
; CHECK: define linkonce_odr dso_local void @test
; CHECK-NOT: invoke void @foo
; %i5 is OOB --- the invoke may be turned into an undef.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27044"

%rtti.CompleteObjectLocator = type { i32, i32, i32, i32, i32, i32 }
%RAB = type { %A, i8 }
%A = type { i32 (...)**, i16, %X }
%X = type { %M*, %Node*, %Node* }
%Node = type { %R*, %Node*, %Node* }
%R = type { %ABB, i16, i16 }
%ABB = type { %Alloc, i16, i16, %C* }
%Alloc = type { %M* }
%C = type { %B, %Path }
%B = type { %O, double, %P }
%O = type { %Ref, i32, %F* }
%Ref = type { i32 (...)**, i32 }
%F = type opaque
%P = type { %FPG, %PT }
%FPG = type { %XDF }
%XDF = type { %XN }
%XN = type { i32 (...)** }
%PT = type { %XT, %O*, %M* }
%XT = type { %CD }
%CD = type { %XN }
%Path = type { %EC*, %XDS* }
%EC = type { %ExeC, %F* }
%ExeC = type { i32 (...)**, %M* }
%XDS = type { %XV, i32 }
%XV = type { %M*, i64, i64, i16* }
%M = type { i32 (...)** }
%SCA = type { %RAB }

$GA = comdat largest

@GV = private unnamed_addr constant { [6 x i8*] } { [6 x i8*] [i8* bitcast (%rtti.CompleteObjectLocator* null to i8*), i8* bitcast (i8* (%A*, i32)* null to i8*), i8* bitcast (%C* (%A*)* null to i8*), i8* bitcast (void (%A*, %C*)* null to i8*), i8* bitcast (i1 (%A*, %C*)* null to i8*), i8* bitcast (void (%A*)* @foo to i8*)] }

@GA = unnamed_addr alias i8*, getelementptr inbounds ({ [6 x i8*] }, { [6 x i8*] }* @GV, i32 0, i32 0, i32 1)

define linkonce_odr dso_local void @test(%A* nonnull dereferenceable(40) %this) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  %i = bitcast %A* %this to i32 (...)***
  store i32 (...)** bitcast (i8** @GA to i32 (...)**), i32 (...)*** %i, align 8
  %i1 = bitcast %A* %this to void (%A*)***
  %vtable = load void (%A*)**, void (%A*)*** %i1, align 8
  %i2 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i3 = bitcast void (%A*)** %vtable to i8*
  %i4 = call i1 @llvm.type.test(i8* %i3, metadata !"baz")
  call void @llvm.assume(i1 %i4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%A*)*, void (%A*)** %vtable, i64 5
  %i5 = load void (%A*)*, void (%A*)** %vfn, align 8
  invoke void %i5(%A* nonnull dereferenceable(40) %this)
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %whpr.continue
  %m_blocks = getelementptr inbounds %A, %A* %this, i32 0, i32 2
  call void @bar(%X* nonnull dereferenceable(24) %m_blocks)
  ret void

ehcleanup:                                        ; preds = %whpr.continue
  %i6 = cleanuppad within none []
  %m_blocks2 = getelementptr inbounds %A, %A* %this, i32 0, i32 2
  call void @bar(%X* nonnull dereferenceable(24) %m_blocks2) [ "funclet"(token %i6) ]
  call void @__std_terminate() [ "funclet"(token %i6) ]
  unreachable
}

declare dso_local void @foo(%A*)

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe()

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(i8*, metadata)

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef)

declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @bar(%X*)

declare dso_local void @__std_terminate() local_unnamed_addr

!0 = !{i64 8, !"baz"}
!1 = !{i64 8, !"goo"}
!2 = !{i64 1}

; end INTEL_FEATURE_SW_DTRANS
