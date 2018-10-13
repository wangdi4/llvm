; REQUIRES: asserts
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -debug-only=dtrans-bca -disable-output 2>&1 | FileCheck %s

; This test case tests the bad cast analyzer on a case which removes bad
; casting and unsafe pointer stores, but requires the addition of tests
; in some functions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.mynextcoder = type { i8*, void (i8*)*, void (i8*)* }
%struct.mycoder1 = type { i32, i32* }
%struct.mycoder2 = type { i32*, i32 }

@myglobalint2 = internal global i32 100, align 4
@myglobalint1 = internal global i32 50, align 4
@mystupidint = internal global i32 75, align 4
@localnextcoder1 = internal global %struct.mynextcoder zeroinitializer, align 8
@localnextcoder2 = internal global %struct.mynextcoder zeroinitializer, align 8
@localnextcoder3 = internal global %struct.mynextcoder zeroinitializer, align 8
@localnextcoder4 = internal global %struct.mynextcoder zeroinitializer, align 8

define internal void @coder1_startup(i8* nocapture) {
  %2 = bitcast i8* %0 to %struct.mycoder1*
  %3 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %2, i64 0, i32 0
  store i32 150, i32* %3, align 8
  %4 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %2, i64 0, i32 1
  store i32* @myglobalint2, i32** %4, align 8
  ret void
}

define internal void @coder1_shutdown(i8* nocapture) {
  %2 = bitcast i8* %0 to %struct.mycoder1*
  %3 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %2, i64 0, i32 0
  store i32 0, i32* %3, align 8
  %4 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %2, i64 0, i32 1
  store i32* @myglobalint1, i32** %4, align 8
  ret void
}

define internal void @coder2_startup(i8* nocapture) {
  %2 = bitcast i8* %0 to %struct.mycoder2*
  %3 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %2, i64 0, i32 1
  store i32 200, i32* %3, align 8
  %4 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %2, i64 0, i32 0
  store i32* @myglobalint1, i32** %4, align 8
  ret void
}

define internal void @coder2_shutdown(i8* nocapture) {
  %2 = bitcast i8* %0 to %struct.mycoder2*
  %3 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %2, i64 0, i32 1
  store i32 0, i32* %3, align 8
  %4 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %2, i64 0, i32 0
  store i32* @myglobalint2, i32** %4, align 8
  ret void
}

define internal fastcc void @init_with_coder1(%struct.mynextcoder* nocapture) unnamed_addr {
  %t2 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 0
  %t6 = tail call noalias i8* @malloc(i64 16) #4
  %t13.1 = bitcast i8* %t6 to %struct.mycoder1*
  store i8* %t6, i8** %t2, align 8
  %t9 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 1
  store void (i8*)* @coder1_startup, void (i8*)** %t9, align 8
  %t10 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 2
  store void (i8*)* @coder1_shutdown, void (i8*)** %t10, align 8
  %t41 = load i32, i32* @mystupidint, align 4
  %t45 = icmp eq i32 %t41, 4
  br i1 %t45, label %t11, label %t50

t50:                                     ; preds = %1
  %t52 = load i32, i32* @mystupidint, align 4
  %t51 = icmp eq i32 %t52, 19
  br i1 %t51, label %t20, label %t30

t20:                                     ; preds = %t50
  %t13.20 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 0
  %t13.21 = load i8*, i8** %t13.20, align 8
  %t13.22 = bitcast i8* %t13.21 to %struct.mycoder1*
  br label %t40

t30:                                     ; preds = %t50
  %t13.30 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 0
  %t13.31 = load i8*, i8** %t13.30, align 8
  %t13.32 = bitcast i8* %t13.31 to %struct.mycoder1*
  br label %t40

t40:                                     ; preds = %t20, %t30
  %t13.40 = phi %struct.mycoder1* [ %t13.22, %t20 ], [ %t13.32, %t30 ]
  br label %t11

t11:                                     ; preds = %1, %t40
  %t12 = phi %struct.mycoder1* [ %t13.1, %1 ], [ %t13.40, %t40 ]
  %t13 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %t12, i64 0, i32 0
  store i32 15, i32* %t13, align 8
  %t14 = getelementptr inbounds %struct.mycoder1, %struct.mycoder1* %t12, i64 0, i32 1
  store i32* @myglobalint1, i32** %t14, align 8
  ret void
}

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #2

define internal fastcc void @init_with_coder2(%struct.mynextcoder* nocapture) unnamed_addr {
  %t2 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 0
  %t3 = load i8*, i8** %t2, align 8
  %t13.0 = bitcast i8* %t3 to %struct.mycoder2*
  %t4 = icmp eq i8* %t3, null
  br i1 %t4, label %t5, label %t11

t5:                                      ; preds = %t1
  %t6 = tail call noalias i8* @malloc(i64 16) #4
  %t13.1 = bitcast i8* %t6 to %struct.mycoder2*
  store i8* %t6, i8** %t2, align 8
  %t9 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 1
  store void (i8*)* @coder2_startup, void (i8*)** %t9, align 8
  %t10 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 2
  store void (i8*)* @coder2_shutdown, void (i8*)** %t10, align 8
  br label %t11

t11:                                     ; preds = %t1, %t5
  %t12 = phi %struct.mycoder2* [ %t13.0, %1 ], [ %t13.1, %t5 ]
  %t13 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %t12, i64 0, i32 1
  store i32 15, i32* %t13, align 8
  %t14 = getelementptr inbounds %struct.mycoder2, %struct.mycoder2* %t12, i64 0, i32 0
  store i32* @myglobalint2, i32** %t14, align 8
  ret void
}

define internal void @myoperation(%struct.mynextcoder* nocapture readonly) {
  %2 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 1
  %3 = load void (i8*)*, void (i8*)** %2, align 8
  %4 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 0
  %5 = load i8*, i8** %4, align 8
  tail call void %3(i8* %5) #4
  %6 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 2
  %7 = load void (i8*)*, void (i8*)** %6, align 8
  %8 = load i8*, i8** %4, align 8
  tail call void %7(i8* %8) #4
  ret void
}

define dso_local i32 @main() {
  store i8* null, i8** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 0), align 8
  store void (i8*)* null, void (i8*)** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 1), align 8
  store void (i8*)* null, void (i8*)** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 2), align 8
  store i8* null, i8** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 0), align 8
  store void (i8*)* null, void (i8*)** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 1), align 8
  store void (i8*)* null, void (i8*)** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 2), align 8
  tail call void @init_with_coder1(%struct.mynextcoder* nonnull @localnextcoder1)
  tail call void @init_with_coder2(%struct.mynextcoder* nonnull @localnextcoder2)
  tail call void @init_with_coder1(%struct.mynextcoder* nonnull @localnextcoder3)
  tail call void @init_with_coder2(%struct.mynextcoder* nonnull @localnextcoder4)
  tail call void @myoperation(%struct.mynextcoder* nonnull @localnextcoder1)
  tail call void @myoperation(%struct.mynextcoder* nonnull @localnextcoder2)
  tail call void @myoperation(%struct.mynextcoder* nonnull @localnextcoder3)
  tail call void @myoperation(%struct.mynextcoder* nonnull @localnextcoder4)
  ret i32 0
}

; CHECK: dtrans-bca: Begin bad casting analysis
; CHECK: dtrans-bca: Candidate Root Type: struct.mynextcoder
; CHECK: dtrans-bca: Unsafe pointer store --  (pending)   Unmatch store of aliased value:
; CHECK: dtrans-bca: (SV) [0] Improper incoming block for PHI node:  %t12 = phi %struct.mycoder1* [ %t13.1, %1 ], [ %t13.40, %t40 ]
; CHECK: dtrans-bca: Bad casting (pending) -- unknown pointer cast to type of interest:
; CHECK: %t13.22 = bitcast i8* %t13.21 to %struct.mycoder1*
; CHECK: dtrans-bca: Bad casting (pending) -- unknown pointer cast to type of interest:
; CHECK: %t13.32 = bitcast i8* %t13.31 to %struct.mycoder1*
; CHECK: dtrans-bca: Bad casting (pending) -- unknown pointer cast to type of interest:
; CHECK: %t13.0 = bitcast i8* %t3 to %struct.mycoder2*
; CHECK: dtrans-bca: Unsafe pointer store --  (pending)   Unmatch store of aliased value:
; CHECK: dtrans-bca: Found safety violation
; CHECK: dtrans-bca: End bad casting analysis: (NOT OK)

; CHECK: LLVMType: %struct.mycoder1
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.mycoder2
; CHECK: Bad casting | Unsafe pointer store
; CHECK: LLVMType: %struct.mynextcoder
; CHECK: Safety data: Unsafe pointer store | Global instance | Has function ptr

