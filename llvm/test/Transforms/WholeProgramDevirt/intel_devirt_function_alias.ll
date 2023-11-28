; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -passes=wholeprogramdevirt -whole-program-assume %s | FileCheck %s

; Ensure we correctly devirtualization when vtable contains an alias.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-grtev4-linux-gnu"

%struct.D = type { ptr }
%struct.B = type { %struct.D }

@_ZTV1D = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr undef, ptr @_ZN1D1mEiAlias] }, !type !0
@_ZTV1B = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr undef, ptr @_ZN1B1mEi] }, !type !0, !type !1


define i32 @_ZN1D1mEi(ptr %this, i32 %a) {
   ret i32 0;
}

@_ZN1D1mEiAlias = unnamed_addr alias i32 (ptr, i32), ptr @_ZN1D1mEi

define i32 @_ZN1B1mEi(ptr %this, i32 %a) {
   ret i32 0;
}

; CHECK-LABEL: define i32 @test
define i32 @test(ptr %obj2, i32 %a) {
entry:
  %vtable2 = load ptr, ptr %obj2
  %p2 = call i1 @llvm.type.test(ptr %vtable2, metadata !"_ZTS1D")
  call void @llvm.assume(i1 %p2)

  %fptr33 = load ptr, ptr %vtable2, align 8

; Confirm the call was devirtualized.
; CHECK:  %[[I0:[0-9]+]] = bitcast ptr %fptr33 to ptr
; CHECK:  %[[I1:[0-9]+]] = bitcast ptr @_ZN1D1mEi to ptr
; CHECK:  %[[I2:[0-9]+]] = icmp eq ptr %[[I0]], %[[I1]]
; CHECK:  br i1 %[[I2]], label %BBDevirt__ZN1D1mEi, label %ElseDevirt__ZN1D1mEi

; CHECK:BBDevirt__ZN1D1mEi:                               ; preds = %entry
; CHECK:  %[[I3:[0-9]+]] = tail call i32 @_ZN1D1mEi(ptr nonnull %obj2, i32 %a), !_Intel.Devirt.Call
; CHECK:  br label %MergeBB

; CHECK:ElseDevirt__ZN1D1mEi:                             ; preds = %entry
; CHECK:  %[[I4:[0-9]+]] = bitcast ptr @_ZN1B1mEi to ptr
; CHECK:  %[[I5:[0-9]+]] = icmp eq ptr %[[I0]], %[[I4]]
; CHECK:  br i1 %[[I5]], label %BBDevirt__ZN1B1mEi, label %DefaultBB

; CHECK:BBDevirt__ZN1B1mEi:                               ; preds = %ElseDevirt__ZN1D1mEi
; CHECK:  %[[I6:[0-9]+]] = tail call i32 @_ZN1B1mEi(ptr nonnull %obj2, i32 %a), !_Intel.Devirt.Call
; CHECK:  br label %MergeBB

; CHECK:DefaultBB:                                        ; preds = %ElseDevirt__ZN1D1mEi
; CHECK:  %[[I7:[0-9]+]] = tail call i32 %fptr33(ptr nonnull %obj2, i32 %a)
; CHECK:  br label %MergeBB

; CHECK:MergeBB:                                          ; preds = %DefaultBB, %BBDevirt__ZN1B1mEi, %BBDevirt__ZN1D1mEi
; CHECK:  %[[I8:[0-9]+]] = phi i32 [ %[[I3]], %BBDevirt__ZN1D1mEi ], [ %[[I6]], %BBDevirt__ZN1B1mEi ], [ %[[I7]], %DefaultBB ]


  %call4 = tail call i32 %fptr33(ptr nonnull %obj2, i32 %a)
  ret i32 %call4
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)

!0 = !{i64 16, !"_ZTS1D"}
!1 = !{i64 16, !"_ZTS1B"}

; end INTEL_FEATURE_SW_DTRANS
