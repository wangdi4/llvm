; REQUIRES: asserts
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -debug-only=dtrans-bca -disable-output 2>&1 | FileCheck %s

; Test that the bad casting analyzer does not do anything because it has
; no structure which qualifies as a candidate root type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.mynextcoder = type { i8*, i32, i32 }
%struct.mycoder1 = type { i32, i32* }
%struct.mycoder2 = type { i32*, i32 }

@myglobalint2 = internal global i32 100, align 4
@myglobalint1 = internal global i32 50, align 4
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

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #2

define internal i32 @myoperation(%struct.mynextcoder* nocapture readonly) {
  %2 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 1
  %3 = load i32, i32* %2, align 8
  %4 = getelementptr inbounds %struct.mynextcoder, %struct.mynextcoder* %0, i64 0, i32 2
  %5 = load i32, i32* %4, align 8
  %6 = add i32 %3, %5
  ret i32 %6
}

define dso_local i32 @main() {
  store i8* null, i8** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 0), align 8
  store i32 1, i32* getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 1), align 8
  store i32 2 , i32* getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder1, i64 0, i32 2), align 8
  store i8* null, i8** getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 0), align 8
  store i32 3 , i32* getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 1), align 8
  store i32 4, i32* getelementptr inbounds (%struct.mynextcoder, %struct.mynextcoder* @localnextcoder2, i64 0, i32 2), align 8
  %t1 = tail call i32 @myoperation(%struct.mynextcoder* nonnull @localnextcoder1)
  %t2 = tail call i32 @myoperation(%struct.mynextcoder* nonnull @localnextcoder2)
  %t3 = tail call i32 @myoperation(%struct.mynextcoder* nonnull @localnextcoder3)
  %t4 = tail call i32 @myoperation(%struct.mynextcoder* nonnull @localnextcoder4)
  %t5 = add i32 %t1, %t2
  %t6 = add i32 %t3, %t4
  %t7 = add i32 %t5, %t6
  ret i32 %t7
}

; CHECK: dtrans-bca: Begin bad casting analysis
; CHECK: dtrans-bca: Candidate Root Type: <<NONE>>
; CHECK: dtrans-bca: Found safety violation
; CHECK: dtrans-bca: End bad casting analysis: (NOT OK)

; CHECK: LLVMType: %struct.mycoder1
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.mycoder2
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.mynextcoder
; CHECK: Safety data: Global instance

