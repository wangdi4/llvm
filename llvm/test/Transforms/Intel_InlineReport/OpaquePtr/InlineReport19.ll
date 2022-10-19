; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; Note that -inline-deferral is being passed explicitly for the new pass
; manager test lines because inline deferral is currently off in the new pass
; manager. The heuristic being tested here was designed specifically for
; certain benchmarks. Before enabling the new pass manager by default, we
; should determine how the heuristics in the new pass manager need to be
; tuned, including this one.
; CMPLRLLVM-33738: We need to add inline-deferral now for the legacy pass
; manager lines, because they can be used to drive the new pass manager.
; RUN: opt -opaque-pointers -inline -inline-deferral -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-deferral -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-deferral -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)'  -inline-deferral -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test for inlining heuristic for stack computations.  All calls to @mypushptr
; should be inlined and the inlining report should report "Inlining for stack
; computations" in all cases of inlining @mypushptr, excpet for the last where
; it should report "Callee has single callsite and local linkage".

%union.any = type { ptr }

@globalvalue = common dso_local global ptr null, align 8
@mysavestack = internal global ptr null, align 8
@mysavestackix = internal global i32 0, align 4
@mysavestackmax = internal global i32 0, align 4
@mynomemok = internal global i8 0, align 1

declare dso_local void @exit(i32)

declare dso_local void @free(ptr nocapture)

declare dso_local noalias ptr @malloc(i64)

declare dso_local noalias ptr @realloc(ptr nocapture, i64)

define internal void @mycroak() {
bb:
  tail call void @exit(i32 10)
  unreachable
}

define internal noalias ptr @myrealloc(ptr %arg, i64 %arg1) {
bb:
  %i = icmp eq i64 %arg1, 0
  %i2 = icmp eq ptr %arg, null
  br i1 %i, label %bb3, label %bb5

bb3:                                              ; preds = %bb
  br i1 %i2, label %bb20, label %bb4

bb4:                                              ; preds = %bb3
  tail call void @free(ptr nonnull %arg)
  br label %bb20

bb5:                                              ; preds = %bb
  br i1 %i2, label %bb6, label %bb13

bb6:                                              ; preds = %bb5
  %i7 = tail call noalias ptr @malloc(i64 %arg1)
  %i8 = icmp eq ptr %i7, null
  %i9 = load i8, ptr @mynomemok, align 1
  %i10 = icmp eq i8 %i9, 0
  %i11 = and i1 %i8, %i10
  br i1 %i11, label %bb12, label %bb20

bb12:                                             ; preds = %bb6
  tail call void @mycroak()
  br label %bb20

bb13:                                             ; preds = %bb5
  %i14 = tail call ptr @realloc(ptr nonnull %arg, i64 %arg1)
  %i15 = icmp eq ptr %i14, null
  br i1 %i15, label %bb16, label %bb20

bb16:                                             ; preds = %bb13
  %i17 = load i8, ptr @mynomemok, align 1
  %i18 = icmp eq i8 %i17, 0
  br i1 %i18, label %bb19, label %bb20

bb19:                                             ; preds = %bb16
  tail call void @mycroak()
  br label %bb20

bb20:                                             ; preds = %bb19, %bb16, %bb13, %bb12, %bb6, %bb4, %bb3
  %i21 = phi ptr [ null, %bb19 ], [ %i14, %bb13 ], [ null, %bb16 ], [ null, %bb3 ], [ null, %bb4 ], [ %i7, %bb6 ], [ null, %bb12 ]
  ret ptr %i21
}

define internal void @mypushptr(ptr %arg, i32 %arg1) {
bb:
  %i = load i32, ptr @mysavestackix, align 4
  %i2 = load ptr, ptr @mysavestack, align 8
  %i3 = sext i32 %i to i64
  %i4 = getelementptr inbounds %union.any, ptr %i2, i64 %i3
  %i5 = getelementptr inbounds %union.any, ptr %i4, i64 1
  %i6 = getelementptr inbounds %union.any, ptr %i4, i64 0, i32 0
  store ptr %arg, ptr %i6, align 8
  %i7 = sext i32 %arg1 to i64
  store i64 %i7, ptr %i5, align 8
  %i9 = add nsw i32 %i, 2
  store i32 %i9, ptr @mysavestackix, align 4
  %i10 = add nsw i32 %i, 6
  %i11 = load i32, ptr @mysavestackmax, align 4
  %i12 = icmp sgt i32 %i10, %i11
  br i1 %i12, label %bb13, label %bb22

bb13:                                             ; preds = %bb
  %i14 = mul nsw i32 %i11, 3
  %i15 = sdiv i32 %i14, 2
  %i16 = add nsw i32 %i15, 4
  store i32 %i16, ptr @mysavestackmax, align 4
  %i17 = load ptr, ptr @mysavestack, align 8
  %i18 = sext i32 %i16 to i64
  %i19 = shl nsw i64 %i18, 3
  %i20 = tail call ptr @myrealloc(ptr %i17, i64 %i19)
  store ptr %i20, ptr @mysavestack, align 8
  br label %bb22

bb22:                                             ; preds = %bb13, %bb
  ret void
}

define dso_local i32 @main() {
bb:
  %t0 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t0, i32 14)
  %t1 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t1, i32 14)
  %t2 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t2, i32 14)
  %t3 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t3, i32 14)
  %t10 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t10, i32 14)
  %t11 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t11, i32 14)
  %t12 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t12, i32 14)
  %t13 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t13, i32 14)
  %t20 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t20, i32 14)
  %t21 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t21, i32 14)
  %t22 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t22, i32 14)
  %t23 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t23, i32 14)
  %t30 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t30, i32 14)
  %t31 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t31, i32 14)
  %t32 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t32, i32 14)
  %t33 = load ptr, ptr @globalvalue, align 8
  call void @mypushptr(ptr %t33, i32 14)
  ret i32 0
}

; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has key stack computations
; CHECK: INLINE: mypushptr{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: call void @mypushptr
; end INTEL_FEATURE_SW_ADVANCED
