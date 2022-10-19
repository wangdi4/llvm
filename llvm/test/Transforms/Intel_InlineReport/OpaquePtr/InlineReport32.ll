; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 < %s -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 %s -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -S | opt -inlinereportemitter -inline-report=0xe886 -inline-threshold=10 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-threshold=10 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -inline-threshold=10 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the functions _ZN12cMessageHeap12removeFirst*Ev
; and _ZN12cMessageHeap7shiftupEi do not inline because they do not pass
; the "structure test" heuristic.

; CHECK-MD: COMPILE FUNC: _ZN11cSimulation17selectNextModule1Ev
; CHECK-MD: _ZN12cMessageHeap12removeFirst1Ev{{.*}}Inlining is not profitable
; CHECK-MD: _ZN12cMessageHeap12removeFirst1Ev{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN11cSimulation17selectNextModule2Ev
; CHECK-MD: _ZN12cMessageHeap12removeFirst2Ev{{.*}}Inlining is not profitable
; CHECK-MD: _ZN12cMessageHeap12removeFirst2Ev{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN11cSimulation17selectNextModule3Ev
; CHECK-MD: _ZN12cMessageHeap12removeFirst3Ev{{.*}}Inlining is not profitable
; CHECK-MD: _ZN12cMessageHeap12removeFirst3Ev{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN11cSimulation17selectNextModule4Ev
; CHECK-MD: _ZN12cMessageHeap12removeFirst4Ev{{.*}}Inlining is not profitable
; CHECK-MD: _ZN12cMessageHeap12removeFirst4Ev{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap12removeFirst1Ev
; CHECK-MD: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap12removeFirst2Ev
; CHECK-MD: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap12removeFirst3Ev
; CHECK-MD: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap12removeFirst4Ev
; CHECK-MD: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi

; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst1Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst1Ev
; CHECK-LABEL: define{{.*}}_ZN11cSimulation17selectNextModule2Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst2Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst2Ev
; CHECK-LABEL: define{{.*}}_ZN11cSimulation17selectNextModule3Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst3Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst3Ev
; CHECK-LABEL: define{{.*}}_ZN11cSimulation17selectNextModule4Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst4Ev
; CHECK: call{{.*}}_ZN12cMessageHeap12removeFirst4Ev
; CHECK-LABEL: define{{.*}}_ZN12cMessageHeap12removeFirst1Ev
; CHECK: call{{.*}}_ZN12cMessageHeap7shiftupEi
; CHECK-LABEL: define{{.*}}_ZN12cMessageHeap12removeFirst2Ev
; CHECK: call{{.*}}_ZN12cMessageHeap7shiftupEi
; CHECK-LABEL: define{{.*}}_ZN12cMessageHeap12removeFirst3Ev
; CHECK: call{{.*}}_ZN12cMessageHeap7shiftupEi
; CHECK-LABEL: define{{.*}}_ZN12cMessageHeap12removeFirst4Ev
; CHECK: call{{.*}}_ZN12cMessageHeap7shiftupEi

; CHECK-CL-DAG: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-CL-DAG: COMPILE FUNC: _ZN12cMessageHeap12removeFirst4Ev
; CHECK-CL-DAG: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN11cSimulation17selectNextModule4Ev
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst4Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst4Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN12cMessageHeap12removeFirst3Ev
; CHECK-CL-DAG: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN11cSimulation17selectNextModule3Ev
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst3Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst3Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN12cMessageHeap12removeFirst2Ev
; CHECK-CL-DAG: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN11cSimulation17selectNextModule2Ev
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst2Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst2Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN12cMessageHeap12removeFirst1Ev
; CHECK-CL-DAG: _ZN12cMessageHeap7shiftupEi{{.*}}Inlining is not profitable
; CHECK-CL-DAG: COMPILE FUNC: _ZN11cSimulation17selectNextModule1Ev
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst1Ev{{.*}}Inlining is not profitable
; CHECK-CL-DAG: _ZN12cMessageHeap12removeFirst1Ev{{.*}}Inlining is not profitable

%class.cSimulation = type { %class.cNoncopyableOwnedObject.base, i32, i32, ptr, i32, ptr, ptr, ptr, ptr, i32, ptr, ptr, %class.SimTime, i64, ptr, ptr, ptr, %class.cMessageHeap }
%class.cNoncopyableOwnedObject.base = type { %class.cOwnedObject.base }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], ptr, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, ptr, i16, i16 }>
%class.cObject = type { ptr }
%class.SimTime = type { i64 }
%class.cMessageHeap = type { %class.cOwnedObject.base, ptr, i32, i32, i64 }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, ptr, ptr, ptr, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.cNamedObject = type <{ %class.cObject, ptr, i16, i16, [4 x i8] }>

declare dso_local ptr @_ZN18cRealTimeScheduler12getNextEventEv(ptr nocapture readonly) unnamed_addr align 2

declare dso_local ptr @_ZN20cSequentialScheduler12getNextEventEv(ptr nocapture readonly) unnamed_addr align 2

declare dso_local i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160), ptr dereferenceable(160))

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #0

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #2

define dso_local ptr @_ZN11cSimulation17selectNextModule1Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 11
  %i1 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 3
  %i2 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 17
  br label %bb3

bb3:                                              ; preds = %bb44, %bb
  %i4 = load ptr, ptr %i, align 8
  %i6 = load ptr, ptr %i4, align 8
  %i7 = getelementptr inbounds ptr, ptr %i6, i64 23
  %i8 = load ptr, ptr %i7, align 8
  %i11 = icmp eq ptr %i8, @_ZN18cRealTimeScheduler12getNextEventEv
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb3
  %i13 = tail call ptr @_ZN18cRealTimeScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb14:                                             ; preds = %bb3
  %i15 = tail call ptr @_ZN20cSequentialScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb16:                                             ; preds = %bb14, %bb12
  %i17 = phi ptr [ %i13, %bb12 ], [ %i15, %bb14 ]
  br label %bb18

bb18:                                             ; preds = %bb16
  %i19 = icmp eq ptr %i17, null
  br i1 %i19, label %bb51, label %bb20

bb20:                                             ; preds = %bb18
  %i21 = load ptr, ptr %i1, align 8
  %i22 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 9
  %i23 = load i32, ptr %i22, align 8
  %i24 = sext i32 %i23 to i64
  %i25 = getelementptr inbounds ptr, ptr %i21, i64 %i24
  %i27 = load ptr, ptr %i25, align 8
  %i28 = icmp eq ptr %i27, null
  br i1 %i28, label %bb35, label %bb29

bb29:                                             ; preds = %bb20
  %i31 = getelementptr inbounds %class.cNamedObject, ptr %i27, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 8
  %i33 = and i16 %i32, 1024
  %i34 = icmp eq i16 %i33, 0
  br i1 %i34, label %bb44, label %bb35

bb35:                                             ; preds = %bb29, %bb20
  %i36 = tail call ptr @_ZN12cMessageHeap12removeFirst1Ev(ptr nonnull %i2)
  %i37 = icmp eq ptr %i36, null
  br i1 %i37, label %bb44, label %bb38

bb38:                                             ; preds = %bb35
  %t38 = tail call ptr @_ZN12cMessageHeap12removeFirst1Ev(ptr nonnull %i2)
  %t39 = icmp eq ptr %t38, null
  br i1 %i37, label %bb44, label %bb39

bb39:                                             ; preds = %bb38
  %i41 = load ptr, ptr %i36, align 8
  %i42 = getelementptr inbounds ptr, ptr %i41, i64 4
  %i43 = load ptr, ptr %i42, align 8
  tail call void %i43(ptr nonnull %i36)
  br label %bb44

bb44:                                             ; preds = %bb39, %bb38, %bb35, %bb29
  br label %bb3

bb45:                                             ; No predecessors!
  %i46 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 13
  %i47 = getelementptr inbounds %class.SimTime, ptr %i46, i64 0, i32 0
  %i48 = load i64, ptr %i47, align 8
  %i49 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 12
  %i50 = getelementptr inbounds %class.SimTime, ptr %i49, i64 0, i32 0
  store i64 %i48, ptr %i50, align 8
  br label %bb51

bb51:                                             ; preds = %bb45, %bb18
  %i52 = phi ptr [ %i27, %bb45 ], [ null, %bb18 ]
  ret ptr %i52
}

define dso_local ptr @_ZN11cSimulation17selectNextModule2Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 11
  %i1 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 3
  %i2 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 17
  br label %bb3

bb3:                                              ; preds = %bb44, %bb
  %i4 = load ptr, ptr %i, align 8
  %i6 = load ptr, ptr %i4, align 8
  %i7 = getelementptr inbounds ptr, ptr %i6, i64 23
  %i8 = load ptr, ptr %i7, align 8
  %i11 = icmp eq ptr %i8, @_ZN18cRealTimeScheduler12getNextEventEv
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb3
  %i13 = tail call ptr @_ZN18cRealTimeScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb14:                                             ; preds = %bb3
  %i15 = tail call ptr @_ZN20cSequentialScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb16:                                             ; preds = %bb14, %bb12
  %i17 = phi ptr [ %i13, %bb12 ], [ %i15, %bb14 ]
  br label %bb18

bb18:                                             ; preds = %bb16
  %i19 = icmp eq ptr %i17, null
  br i1 %i19, label %bb51, label %bb20

bb20:                                             ; preds = %bb18
  %i21 = load ptr, ptr %i1, align 8
  %i22 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 9
  %i23 = load i32, ptr %i22, align 8
  %i24 = sext i32 %i23 to i64
  %i25 = getelementptr inbounds ptr, ptr %i21, i64 %i24
  %i27 = load ptr, ptr %i25, align 8
  %i28 = icmp eq ptr %i27, null
  br i1 %i28, label %bb35, label %bb29

bb29:                                             ; preds = %bb20
  %i31 = getelementptr inbounds %class.cNamedObject, ptr %i27, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 8
  %i33 = and i16 %i32, 1024
  %i34 = icmp eq i16 %i33, 0
  br i1 %i34, label %bb44, label %bb35

bb35:                                             ; preds = %bb29, %bb20
  %i36 = tail call ptr @_ZN12cMessageHeap12removeFirst2Ev(ptr nonnull %i2)
  %i37 = icmp eq ptr %i36, null
  br i1 %i37, label %bb44, label %bb38

bb38:                                             ; preds = %bb35
  %t38 = tail call ptr @_ZN12cMessageHeap12removeFirst2Ev(ptr nonnull %i2)
  %t39 = icmp eq ptr %t38, null
  br i1 %i37, label %bb44, label %bb39

bb39:                                             ; preds = %bb38
  %i41 = load ptr, ptr %i36, align 8
  %i42 = getelementptr inbounds ptr, ptr %i41, i64 4
  %i43 = load ptr, ptr %i42, align 8
  tail call void %i43(ptr nonnull %i36)
  br label %bb44

bb44:                                             ; preds = %bb39, %bb38, %bb35, %bb29
  br label %bb3

bb45:                                             ; No predecessors!
  %i46 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 13
  %i47 = getelementptr inbounds %class.SimTime, ptr %i46, i64 0, i32 0
  %i48 = load i64, ptr %i47, align 8
  %i49 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 12
  %i50 = getelementptr inbounds %class.SimTime, ptr %i49, i64 0, i32 0
  store i64 %i48, ptr %i50, align 8
  br label %bb51

bb51:                                             ; preds = %bb45, %bb18
  %i52 = phi ptr [ %i27, %bb45 ], [ null, %bb18 ]
  ret ptr %i52
}

define dso_local void @_ZN11cSimulation17selectNextModule3Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 11
  %i1 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 3
  %i2 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 17
  br label %bb3

bb3:                                              ; preds = %bb
  %i4 = load ptr, ptr %i, align 8
  %i6 = load ptr, ptr %i4, align 8
  %i7 = getelementptr inbounds ptr, ptr %i6, i64 23
  %i8 = load ptr, ptr %i7, align 8
  %i11 = icmp eq ptr %i8, @_ZN18cRealTimeScheduler12getNextEventEv
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb3
  %i13 = tail call ptr @_ZN18cRealTimeScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb14:                                             ; preds = %bb3
  %i15 = tail call ptr @_ZN20cSequentialScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb16:                                             ; preds = %bb14, %bb12
  %i17 = phi ptr [ %i13, %bb12 ], [ %i15, %bb14 ]
  br label %bb18

bb18:                                             ; preds = %bb16
  %i19 = icmp eq ptr %i17, null
  br i1 %i19, label %bb36, label %bb20

bb20:                                             ; preds = %bb18
  %i21 = load ptr, ptr %i1, align 8
  %i22 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 9
  %i23 = load i32, ptr %i22, align 8
  %i24 = sext i32 %i23 to i64
  %i25 = getelementptr inbounds ptr, ptr %i21, i64 %i24
  %i27 = load ptr, ptr %i25, align 8
  %i28 = icmp eq ptr %i27, null
  br i1 %i28, label %bb35, label %bb29

bb29:                                             ; preds = %bb20
  %i31 = getelementptr inbounds %class.cNamedObject, ptr %i27, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 8
  %i33 = and i16 %i32, 1024
  %i34 = icmp eq i16 %i33, 0
  br i1 %i34, label %bb35, label %bb36

bb35:                                             ; preds = %bb29, %bb20
  tail call void @_ZN12cMessageHeap12removeFirst3Ev(ptr nonnull %i2)
  ret void

bb36:                                             ; preds = %bb29, %bb18
  tail call void @_ZN12cMessageHeap12removeFirst3Ev(ptr nonnull %i2)
  ret void
}

define dso_local ptr @_ZN11cSimulation17selectNextModule4Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 11
  %i1 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 3
  %i2 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 17
  br label %bb3

bb3:                                              ; preds = %bb44, %bb
  %i4 = load ptr, ptr %i, align 8
  %i6 = load ptr, ptr %i4, align 8
  %i7 = getelementptr inbounds ptr, ptr %i6, i64 23
  %i8 = load ptr, ptr %i7, align 8
  %i11 = icmp eq ptr %i8, @_ZN18cRealTimeScheduler12getNextEventEv
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb3
  %i13 = tail call ptr @_ZN18cRealTimeScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb14:                                             ; preds = %bb3
  %i15 = tail call ptr @_ZN20cSequentialScheduler12getNextEventEv(ptr %i4)
  br label %bb16

bb16:                                             ; preds = %bb14, %bb12
  %i17 = phi ptr [ %i13, %bb12 ], [ %i15, %bb14 ]
  br label %bb18

bb18:                                             ; preds = %bb16
  %i19 = icmp eq ptr %i17, null
  br i1 %i19, label %bb51, label %bb20

bb20:                                             ; preds = %bb18
  %i21 = load ptr, ptr %i1, align 8
  %i22 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 9
  %i23 = load i32, ptr %i22, align 8
  %i24 = sext i32 %i23 to i64
  %i25 = getelementptr inbounds ptr, ptr %i21, i64 %i24
  %i27 = load ptr, ptr %i25, align 8
  %i28 = icmp eq ptr %i27, null
  br i1 %i28, label %bb35, label %bb29

bb29:                                             ; preds = %bb20
  %i31 = getelementptr inbounds %class.cNamedObject, ptr %i27, i64 0, i32 2
  %i32 = load i16, ptr %i31, align 8
  %i33 = and i16 %i32, 1024
  %i34 = icmp eq i16 %i33, 0
  br i1 %i34, label %bb44, label %bb35

bb35:                                             ; preds = %bb29, %bb20
  %i36 = tail call ptr @_ZN12cMessageHeap12removeFirst4Ev(ptr nonnull %i2)
  %i37 = icmp eq ptr %i36, null
  br i1 %i37, label %bb44, label %bb38

bb38:                                             ; preds = %bb35
  %t38 = tail call ptr @_ZN12cMessageHeap12removeFirst4Ev(ptr nonnull %i2)
  %t39 = icmp eq ptr %t38, null
  br i1 %i37, label %bb44, label %bb39

bb39:                                             ; preds = %bb38
  %i41 = load ptr, ptr %i36, align 8
  %i42 = getelementptr inbounds ptr, ptr %i41, i64 4
  %i43 = load ptr, ptr %i42, align 8
  tail call void %i43(ptr nonnull %i36)
  br label %bb44

bb44:                                             ; preds = %bb39, %bb38, %bb35, %bb29
  br label %bb3

bb45:                                             ; No predecessors!
  %i46 = getelementptr inbounds %class.cMessage, ptr %i17, i64 0, i32 13
  %i47 = getelementptr inbounds %class.SimTime, ptr %i46, i64 0, i32 0
  %i48 = load i64, ptr %i47, align 8
  %i49 = getelementptr inbounds %class.cSimulation, ptr %arg, i64 0, i32 12
  %i50 = getelementptr inbounds %class.SimTime, ptr %i49, i64 0, i32 0
  store i64 %i48, ptr %i50, align 8
  br label %bb51

bb51:                                             ; preds = %bb45, %bb18
  %i52 = phi ptr [ %i27, %bb45 ], [ null, %bb18 ]
  ret ptr %i52
}

define internal ptr @_ZN12cMessageHeap12removeFirst1Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 2
  %i1 = load i32, ptr %i, align 8
  %i2 = icmp sgt i32 %i1, 0
  br i1 %i2, label %bb3, label %bb20

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  %i6 = getelementptr inbounds ptr, ptr %i5, i64 1
  %i7 = load ptr, ptr %i6, align 8
  %i8 = add nsw i32 %i1, -1
  store i32 %i8, ptr %i, align 8
  %i9 = sext i32 %i1 to i64
  %i10 = getelementptr inbounds ptr, ptr %i5, i64 %i9
  %i11 = load ptr, ptr %i10, align 8
  store ptr %i11, ptr %i6, align 8
  %i12 = getelementptr inbounds %class.cMessage, ptr %i11, i64 0, i32 15
  store i32 1, ptr %i12, align 8
  tail call void @_ZN12cMessageHeap7shiftupEi(ptr nonnull %arg, i32 1)
  %i13 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 0, i32 0, i32 0
  %i16 = load ptr, ptr %arg, align 8
  %i17 = getelementptr inbounds ptr, ptr %i16, i64 13
  %i18 = load ptr, ptr %i17, align 8
  tail call void %i18(ptr %i13, ptr %i7)
  %i19 = getelementptr inbounds %class.cMessage, ptr %i7, i64 0, i32 15
  store i32 -1, ptr %i19, align 8
  br label %bb20

bb20:                                             ; preds = %bb3, %bb
  %i21 = phi ptr [ %i7, %bb3 ], [ null, %bb ]
  br label %bb22

bb22:                                             ; preds = %bb20
  ret ptr %i21
}

define internal ptr @_ZN12cMessageHeap12removeFirst2Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 2
  %i1 = load i32, ptr %i, align 8
  %i2 = icmp sgt i32 %i1, 23
  br i1 %i2, label %bb3, label %bb20

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  %i6 = getelementptr inbounds ptr, ptr %i5, i64 1
  %i7 = load ptr, ptr %i6, align 8
  %i8 = add nsw i32 %i1, -1
  store i32 %i8, ptr %i, align 8
  %i9 = sext i32 %i1 to i64
  %i10 = getelementptr inbounds ptr, ptr %i5, i64 %i9
  %i11 = load ptr, ptr %i10, align 8
  store ptr %i11, ptr %i6, align 8
  %i12 = getelementptr inbounds %class.cMessage, ptr %i11, i64 0, i32 15
  store i32 1, ptr %i12, align 8
  tail call void @_ZN12cMessageHeap7shiftupEi(ptr nonnull %arg, i32 1)
  %i13 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 0, i32 0, i32 0
  %i16 = load ptr, ptr %arg, align 8
  %i17 = getelementptr inbounds ptr, ptr %i16, i64 13
  %i18 = load ptr, ptr %i17, align 8
  tail call void %i18(ptr %i13, ptr %i7)
  %i19 = getelementptr inbounds %class.cMessage, ptr %i7, i64 0, i32 15
  store i32 -1, ptr %i19, align 8
  br label %bb20

bb20:                                             ; preds = %bb3, %bb
  %i21 = phi ptr [ %i7, %bb3 ], [ null, %bb ]
  ret ptr %i21
}

define internal void @_ZN12cMessageHeap12removeFirst3Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 2
  %i1 = load i32, ptr %i, align 8
  %i2 = icmp sgt i32 %i1, 0
  br i1 %i2, label %bb3, label %bb20

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  %i6 = getelementptr inbounds ptr, ptr %i5, i64 1
  %i7 = load ptr, ptr %i6, align 8
  %i8 = add nsw i32 %i1, -1
  store i32 %i8, ptr %i, align 8
  %i9 = sext i32 %i1 to i64
  %i10 = getelementptr inbounds ptr, ptr %i5, i64 %i9
  %i11 = load ptr, ptr %i10, align 8
  store ptr %i11, ptr %i6, align 8
  %i12 = getelementptr inbounds %class.cMessage, ptr %i11, i64 0, i32 15
  store i32 1, ptr %i12, align 8
  tail call void @_ZN12cMessageHeap7shiftupEi(ptr nonnull %arg, i32 1)
  %i13 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 0, i32 0, i32 0
  %i16 = load ptr, ptr %arg, align 8
  %i17 = getelementptr inbounds ptr, ptr %i16, i64 13
  %i18 = load ptr, ptr %i17, align 8
  tail call void %i18(ptr %i13, ptr %i7)
  %i19 = getelementptr inbounds %class.cMessage, ptr %i7, i64 0, i32 15
  store i32 -1, ptr %i19, align 8
  br label %bb20

bb20:                                             ; preds = %bb3, %bb
  %i21 = phi ptr [ %i7, %bb3 ], [ null, %bb ]
  ret void
}

define internal ptr @_ZN12cMessageHeap12removeFirst4Ev(ptr %arg) align 2 {
bb:
  %i = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 2
  %i1 = load i32, ptr %i, align 8
  %i2 = icmp sgt i32 %i1, 0
  br i1 %i2, label %bb3, label %bb20

bb3:                                              ; preds = %bb
  %i4 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 1
  %i5 = load ptr, ptr %i4, align 8
  %i6 = getelementptr inbounds ptr, ptr %i5, i64 1
  %i7 = load ptr, ptr %i6, align 8
  %i8 = add nsw i32 %i1, -1
  store i32 %i8, ptr %i, align 8
  %i9 = sext i32 %i1 to i64
  %i10 = getelementptr inbounds ptr, ptr %i5, i64 %i9
  %i11 = load ptr, ptr %i10, align 8
  store ptr %i11, ptr %i6, align 8
  %i12 = getelementptr inbounds %class.cMessage, ptr %i11, i64 0, i32 15
  store i32 1, ptr %i12, align 8
  tail call void @_ZN12cMessageHeap7shiftupEi(ptr nonnull %arg, i32 1)
  %i13 = getelementptr inbounds %class.cMessageHeap, ptr %arg, i64 0, i32 0, i32 0, i32 0
  %i16 = load ptr, ptr %arg, align 8
  %i17 = getelementptr inbounds ptr, ptr %i16, i64 13
  %i18 = load ptr, ptr %i17, align 8
  tail call void %i18(ptr %i13, ptr %i7)
  %i19 = getelementptr inbounds %class.cMessage, ptr %i7, i64 0, i32 15
  store i32 -1, ptr %i19, align 8
  br i1 %i2, label %bb20, label %bb20

bb20:                                             ; preds = %bb3, %bb3, %bb
  %i21 = phi ptr [ %i7, %bb3 ], [ null, %bb ], [ %i7, %bb3 ]
  br i1 false, label %bb22, label %bb22

bb22:                                             ; preds = %bb20, %bb20
  ret ptr %i21
}

define dso_local void @_ZN12cMessageHeap7shiftupEi(ptr %this, i32 %from) align 2 {
entry:
  br label %while.cond

while.cond:                                       ; preds = %if.then15, %entry
  %i.0 = phi i32 [ %from, %entry ], [ %j.0, %if.then15 ]
  %shl = shl i32 %i.0, 1
  %n = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 2
  %i = load i32, ptr %n, align 8
  %cmp = icmp sle i32 %shl, %i
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %cmp3 = icmp slt i32 %shl, %i
  br i1 %cmp3, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %while.body
  %h = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 1
  %i1 = load ptr, ptr %h, align 8
  %idxprom = sext i32 %shl to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx, align 8
  %add = add nsw i32 %shl, 1
  %idxprom5 = sext i32 %add to i64
  %arrayidx6 = getelementptr inbounds ptr, ptr %i1, i64 %idxprom5
  %i3 = load ptr, ptr %arrayidx6, align 8
  %call = call i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160) %i2, ptr dereferenceable(160) %i3)
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %while.body
  %j.0 = phi i32 [ %add, %if.then ], [ %shl, %land.lhs.true ], [ %shl, %while.body ]
  %h7 = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 1
  %i4 = load ptr, ptr %h7, align 8
  %idxprom8 = sext i32 %i.0 to i64
  %arrayidx9 = getelementptr inbounds ptr, ptr %i4, i64 %idxprom8
  %i5 = load ptr, ptr %arrayidx9, align 8
  %idxprom11 = sext i32 %j.0 to i64
  %arrayidx12 = getelementptr inbounds ptr, ptr %i4, i64 %idxprom11
  %i6 = load ptr, ptr %arrayidx12, align 8
  %call13 = call i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160) %i5, ptr dereferenceable(160) %i6)
  %tobool14 = icmp ne i32 %call13, 0
  br i1 %tobool14, label %if.then15, label %while.end

if.then15:                                        ; preds = %if.end
  %i7 = load ptr, ptr %h7, align 8
  %arrayidx18 = getelementptr inbounds ptr, ptr %i7, i64 %idxprom11
  %i8 = load ptr, ptr %arrayidx18, align 8
  %arrayidx21 = getelementptr inbounds ptr, ptr %i7, i64 %idxprom8
  %i9 = load ptr, ptr %arrayidx21, align 8
  store ptr %i9, ptr %arrayidx18, align 8
  %heapindex = getelementptr inbounds %class.cMessage, ptr %i9, i32 0, i32 15
  store i32 %j.0, ptr %heapindex, align 8
  %i10 = load ptr, ptr %h7, align 8
  %arrayidx27 = getelementptr inbounds ptr, ptr %i10, i64 %idxprom8
  store ptr %i8, ptr %arrayidx27, align 8
  %heapindex28 = getelementptr inbounds %class.cMessage, ptr %i8, i32 0, i32 15
  store i32 %i.0, ptr %heapindex28, align 8
  br label %while.cond

while.end:                                        ; preds = %if.end, %while.cond
  ret void
}

attributes #0 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
; end INTEL_FEATURE_SW_ADVANCED
