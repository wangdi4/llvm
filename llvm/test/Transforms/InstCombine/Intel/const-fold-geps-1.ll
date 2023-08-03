; INTEL_FEATURE_SW_DTRANS

; REQUIRES: intel_feature_sw_dtrans

; This test verifies that instcombine can convert indirect call to direct
; call by applying constant folding when two GEPs and GlobalAlias are
; involved.

; RUN: opt -passes=instcombine -S < %s 2>&1 | FileCheck %s

; InstCombine can convert %i5 to @foo by applying constant folding.
;
; CHECK: define linkonce_odr dso_local void @test
; CHECK: invoke void @foo
; CHECK-NOT: invoke void %i5

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27044"

%rtti.CompleteObjectLocator = type { i32, i32, i32, i32, i32, i32 }
%RAB = type { %A, i8 }
%A = type { ptr, i16, %X }
%X = type { ptr, ptr, ptr }
%Node = type { ptr, ptr, ptr }
%R = type { %ABB, i16, i16 }
%ABB = type { %Alloc, i16, i16, ptr }
%Alloc = type { ptr }
%C = type { %B, %Path }
%B = type { %O, double, %P }
%O = type { %Ref, i32, ptr }
%Ref = type { ptr, i32 }
%F = type opaque
%P = type { %FPG, %PT }
%FPG = type { %XDF }
%XDF = type { %XN }
%XN = type { ptr }
%PT = type { %XT, ptr, ptr }
%XT = type { %CD }
%CD = type { %XN }
%Path = type { ptr, ptr }
%EC = type { %ExeC, ptr }
%ExeC = type { ptr, ptr }
%XDS = type { %XV, i32 }
%XV = type { ptr, i64, i64, ptr }
%M = type { ptr }
%SCA = type { %RAB }

$GA = comdat largest

@GV = private unnamed_addr constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr null, ptr @foo] }

@GA = unnamed_addr alias ptr, getelementptr inbounds ({ [6 x ptr] }, ptr @GV, i32 0, i32 0, i32 1)

define linkonce_odr dso_local void @test(ptr nonnull dereferenceable(40) %this) unnamed_addr align 2 personality ptr @__CxxFrameHandler3 {
entry:
  store ptr @GA, ptr %this, align 8
  %vtable = load ptr, ptr %this, align 8
  %i2 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i4 = call i1 @llvm.type.test(ptr %vtable, metadata !"baz")
  call void @llvm.assume(i1 %i4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 4
  %i5 = load ptr, ptr %vfn, align 8
  invoke void %i5(ptr nonnull dereferenceable(40) %this)
          to label %invoke.cont unwind label %ehcleanup

invoke.cont:                                      ; preds = %whpr.continue
  %m_blocks = getelementptr inbounds %A, ptr %this, i32 0, i32 2
  call void @bar(ptr nonnull dereferenceable(24) %m_blocks)
  ret void

ehcleanup:                                        ; preds = %whpr.continue
  %i6 = cleanuppad within none []
  %m_blocks2 = getelementptr inbounds %A, ptr %this, i32 0, i32 2
  call void @bar(ptr nonnull dereferenceable(24) %m_blocks2) [ "funclet"(token %i6) ]
  call void @__std_terminate() [ "funclet"(token %i6) ]
  unreachable
}

declare dso_local void @foo(ptr)

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe()

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata)

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef)

declare dso_local i32 @__CxxFrameHandler3(...)

declare dso_local void @bar(ptr)

declare dso_local void @__std_terminate() local_unnamed_addr

!0 = !{i64 8, !"baz"}
!1 = !{i64 8, !"goo"}
!2 = !{i64 1}

; end INTEL_FEATURE_SW_DTRANS