; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)'  -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Test for inlining heuristic for stack computations.  All calls to @mypushptr
; should be inlined and the inlining report should report "Inlining for stack
; computations" in all cases of inlining @mypushptr, excpet for the last where
; it should report "Callee has single callsite and local linkage".

; This also tests that when -dtrans-inline-heuristics -intel-libirc-allowed is enabled, inline
; deferral is performed, even in the new pass manager. Inline deferral is
; required to get all calls @mypushptr inlined.

target triple = "x86_64-unknown-linux-gnu"

%union.any = type { i8* }

@globalvalue = common dso_local global i8* null, align 8

@mysavestack = internal global %union.any* null, align 8

@mysavestackix = internal global i32 0, align 4

@mysavestackmax = internal global i32 0, align 4

@mynomemok = internal global i8 0, align 1

declare dso_local void @exit(i32)

declare dso_local void @free(i8* nocapture)

declare dso_local noalias i8* @malloc(i64)

declare dso_local noalias i8* @realloc(i8* nocapture, i64)

define internal void @mycroak() {
  tail call void @exit(i32 10)
  unreachable
}

define internal noalias i8* @myrealloc(i8*, i64) {
  %3 = icmp eq i64 %1, 0
  %4 = icmp eq i8* %0, null
  br i1 %3, label %5, label %7
; <label>:5:                                      ; preds = %2
  br i1 %4, label %22, label %6
; <label>:6:                                      ; preds = %5
  tail call void @free(i8* nonnull %0) #10
  br label %22
; <label>:7:                                      ; preds = %2
  br i1 %4, label %8, label %15
; <label>:8:                                      ; preds = %7
  %9 = tail call noalias i8* @malloc(i64 %1) #10
  %10 = icmp eq i8* %9, null
  %11 = load i8, i8* @mynomemok, align 1
  %12 = icmp eq i8 %11, 0
  %13 = and i1 %10, %12
  br i1 %13, label %14, label %22
; <label>:14:                                     ; preds = %8
  tail call void @mycroak() #10
  br label %22
; <label>:15:                                     ; preds = %7
  %16 = tail call i8* @realloc(i8* nonnull %0, i64 %1) #10
  %17 = icmp eq i8* %16, null
  br i1 %17, label %18, label %22
; <label>:18:                                     ; preds = %15
  %19 = load i8, i8* @mynomemok, align 1
  %20 = icmp eq i8 %19, 0
  br i1 %20, label %21, label %22
; <label>:21:                                     ; preds = %18
  tail call void @mycroak()
  br label %22
; <label>:22:                                     ; preds = %21, %18, %15, %14, %8, %6, %5
  %23 = phi i8* [ null, %21 ], [ %16, %15 ], [ null, %18 ], [ null, %5 ], [ null, %6 ], [ %9, %8 ], [ null, %14 ]
  ret i8* %23
}

define internal void @mypushptr(i8*, i32) {
  %3 = load i32, i32* @mysavestackix, align 4
  %4 = load %union.any*, %union.any** @mysavestack, align 8
  %5 = sext i32 %3 to i64
  %6 = getelementptr inbounds %union.any, %union.any* %4, i64 %5
  %7 = getelementptr inbounds %union.any, %union.any* %6, i64 1
  %8 = getelementptr inbounds %union.any, %union.any* %6, i64 0, i32 0
  store i8* %0, i8** %8, align 8
  %9 = sext i32 %1 to i64
  %10 = bitcast %union.any* %7 to i64*
  store i64 %9, i64* %10, align 8
  %11 = add nsw i32 %3, 2
  store i32 %11, i32* @mysavestackix, align 4
  %12 = add nsw i32 %3, 6
  %13 = load i32, i32* @mysavestackmax, align 4
  %14 = icmp sgt i32 %12, %13
  br i1 %14, label %15, label %24
; <label>:15:                                     ; preds = %2
  %16 = mul nsw i32 %13, 3
  %17 = sdiv i32 %16, 2
  %18 = add nsw i32 %17, 4
  store i32 %18, i32* @mysavestackmax, align 4
  %19 = load i8*, i8** bitcast (%union.any** @mysavestack to i8**), align 8
  %20 = sext i32 %18 to i64
  %21 = shl nsw i64 %20, 3
  %22 = tail call i8* @myrealloc(i8* %19, i64 %21) #10
  %23 = bitcast i8* %22 to %union.any*
  store %union.any* %23, %union.any** @mysavestack, align 8
  br label %24
; <label>:24:                                     ; preds = %15, %2
  ret void
}

define dso_local i32 @main() {
  %t0 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t0, i32 14)
  %t1 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t1, i32 14)
  %t2 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t2, i32 14)
  %t3 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t3, i32 14)
  %t10 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t10, i32 14)
  %t11 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t11, i32 14)
  %t12 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t12, i32 14)
  %t13 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t13, i32 14)
  %t20 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t20, i32 14)
  %t21 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t21, i32 14)
  %t22 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t22, i32 14)
  %t23 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t23, i32 14)
  %t30 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t30, i32 14)
  %t31 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t31, i32 14)
  %t32 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t32, i32 14)
  %t33 = load i8*, i8** @globalvalue, align 8
  call void @mypushptr(i8* %t33, i32 14)
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
