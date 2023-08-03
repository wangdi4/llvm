; RUN: opt < %s -passes="print<hir-region-identification>"  2>&1 | FileCheck %s

; Verify that we are able to create two regions without compfailing.
; The backedge taken count of loops %bb65 and %bb30 is the AddRec of loops %bb21
; and %bb55 respectively.
; The logic which checks whether the loops can be fused was trying to compute
; the difference between the two AddRecs. This operation failed in
; ScalarEvolution as there is no domination relationship between loops %bb21 and
; %bb55 which was being expected by the code which sorts SCEV operands.

; CHECK: Region 1
; CHECK: EntryBB: %bb65

; CHECK: Region 2
; CHECK: EntryBB: %bb30


; ModuleID = 'red1.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local void @_Znwm() local_unnamed_addr #0

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #1

define hidden fastcc void @_ZNSt5stackIP10NEDElementSt5dequeIS1_SaIS1_EEEaSERKS5_.7377() unnamed_addr #0 align 2 personality ptr @__gxx_personality_v0 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  br label %bb13

bb13:                                             ; preds = %bb12, %bb9
  br i1 undef, label %bb14, label %bb48

bb14:                                             ; preds = %bb13
  br i1 undef, label %bb17, label %bb82

bb17:                                             ; preds = %bb15
  br i1 undef, label %bb19, label %bb19

bb19:                                             ; preds = %bb18, %bb17
  br i1 undef, label %bb82, label %bb20

bb20:                                             ; preds = %bb19
  br label %bb21

bb21:                                             ; preds = %bb22, %bb20
  %i = phi i64 [ %i23, %bb22 ], [ 1, %bb20 ]
  invoke void @_Znwm()
          to label %bb22 unwind label %bb24

bb22:                                             ; preds = %bb21
  %i23 = add nuw nsw i64 %i, 1
  br i1 undef, label %bb40, label %bb21

bb24:                                             ; preds = %bb21
  %i25 = phi i64 [ %i, %bb21 ]
  %i26 = landingpad { ptr, i32 }
          catch ptr null
  br i1 undef, label %bb27, label %bb29

bb27:                                             ; preds = %bb24
  br label %bb30

bb28:                                             ; preds = %bb30
  br label %bb29

bb29:                                             ; preds = %bb28, %bb24
  invoke void @__cxa_rethrow() #2
          to label %bb39 unwind label %bb34

bb30:                                             ; preds = %bb30, %bb27
  %i31 = phi i64 [ %i32, %bb30 ], [ 1, %bb27 ]
  %i32 = add nuw nsw i64 %i31, 1
  %i33 = icmp eq i64 %i32, %i25
  br i1 %i33, label %bb28, label %bb30

bb34:                                             ; preds = %bb29
  %i35 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb37

bb36:                                             ; preds = %bb69, %bb34
  resume { ptr, i32 } undef

bb37:                                             ; preds = %bb34
  %i38 = landingpad { ptr, i32 }
          catch ptr null
  unreachable

bb39:                                             ; preds = %bb29
  unreachable

bb40:                                             ; preds = %bb22
  br label %bb82

bb48:                                             ; preds = %bb13
  br i1 undef, label %bb51, label %bb77

bb51:                                             ; preds = %bb49
  br i1 undef, label %bb53, label %bb53

bb53:                                             ; preds = %bb52, %bb51
  br i1 undef, label %bb77, label %bb54

bb54:                                             ; preds = %bb53
  br label %bb55

bb55:                                             ; preds = %bb57, %bb54
  %i56 = phi i64 [ %i58, %bb57 ], [ 1, %bb54 ]
  invoke void @_Znwm()
          to label %bb57 unwind label %bb59

bb57:                                             ; preds = %bb55
  %i58 = add nuw nsw i64 %i56, 1
  br i1 undef, label %bb74, label %bb55

bb59:                                             ; preds = %bb55
  %i60 = phi i64 [ %i56, %bb55 ]
  %i61 = landingpad { ptr, i32 }
          catch ptr null
  br i1 undef, label %bb62, label %bb64

bb62:                                             ; preds = %bb59
  br label %bb65

bb63:                                             ; preds = %bb65
  br label %bb64

bb64:                                             ; preds = %bb63, %bb59
  invoke void @__cxa_rethrow() #2
          to label %bb73 unwind label %bb69

bb65:                                             ; preds = %bb65, %bb62
  %i66 = phi i64 [ %i67, %bb65 ], [ 1, %bb62 ]
  %i67 = add nuw nsw i64 %i66, 1
  %i68 = icmp eq i64 %i67, %i60
  br i1 %i68, label %bb63, label %bb65

bb69:                                             ; preds = %bb64
  %i70 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %bb36 unwind label %bb71

bb71:                                             ; preds = %bb69
  %i72 = landingpad { ptr, i32 }
          catch ptr null
  unreachable

bb73:                                             ; preds = %bb64
  unreachable

bb74:                                             ; preds = %bb57
  br label %bb77


bb77:                                             ; preds = %bb76
  br label %bb82

bb82:                                             ; preds = %bb81, %bb47, %bb6, %bb
  ret void
}

