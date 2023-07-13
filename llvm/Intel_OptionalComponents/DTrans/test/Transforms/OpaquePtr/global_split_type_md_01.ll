; This test verifies that intel_dtrans_type metadata is propagated
; to global variables that are created by GlobalSplit.
;
; RUN: opt < %s -S -passes=globalsplit 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @_ZTVN11xercesc_2_725XMLPlatformUtilsExceptionE.0 = private constant [6 x ptr] [ptr null, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef], !intel_dtrans_type  ![[DT0:[0-9]+]]

; CHECK: ![[DT0]] = !{!"A", i32 6, ![[DT1:[0-9]+]]}
; CHECK: ![[DT1]] = !{i8 0, i32 1}


$_ZTVN11xercesc_2_725XMLPlatformUtilsExceptionE = comdat any

@_ZTVN11xercesc_2_725XMLPlatformUtilsExceptionE = internal constant { [6 x ptr] } { [6 x ptr] [ptr null, ptr undef, ptr undef, ptr undef, ptr undef, ptr undef] }, comdat, !intel_dtrans_type !0

define internal void @_ZN11xercesc_2_716XMLPlatformUtils5panicENS_12PanicHandler12PanicReasonsE() {
  %1 = tail call i1 @llvm.type.test(ptr null, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata)


!0 = !{!"L", i32 1, !1}
!1 = !{!"A", i32 6, !2}
!2 = !{i8 0, i32 1}
