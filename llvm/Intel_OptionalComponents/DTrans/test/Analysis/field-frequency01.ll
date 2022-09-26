; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; This test verifies that frequencies of field accesses are computed as
; expected when BlockFrequencyInfo is not used (i.e -dtrans-ignore-bfi=true).

; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-ignore-bfi=true -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-ignore-bfi=true -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks field frequencies of struct.test01.
;  1st field is accessed 7 times.
;  2nd field is accessed 2 times.
;  3rd field is never accessed.
;  Total Frequency of all fields: 9
; CHECK: Name: struct.test01
; CHECK: Frequency: 7
; CHECK: Frequency: 2
; CHECK: Frequency: 0
; CHECK: Total Frequency: 9
%struct.test01 = type { i32, i64, i32 }
@g_instance.test01 = internal unnamed_addr global %struct.test01 zeroinitializer

; Checks field frequencies of struct.test02.
;  1st field is accessed 1 time.
;  2nd field is accessed 3 times.
;  Total Frequency of all fields: 4
; CHECK: Name: struct.test02
; CHECK: Frequency: 1
; CHECK: Frequency: 3
; CHECK: Total Frequency: 4
%struct.test02 = type { %struct.test01*, i64 }

; Checks field frequencies of struct.test03.
;  1st field is accessed 5 times.
;  2nd field is accessed 3 times.
;  Total Frequency of all fields: 8
; CHECK: Name: struct.test03
; CHECK: Frequency: 5
; CHECK: Frequency: 3
; CHECK: Total Frequency: 8
%struct.test03 = type { %struct.test02*, %struct.test01* }

; Max Total Frequency of all three structs.
; CHECK: MaxTotalFrequency: 9

define void @foo(%struct.test01* %tp, %struct.test02* %tp2) {
entry:
  %i = getelementptr inbounds %struct.test01, %struct.test01* %tp, i64 0, i32 0
  %0 = load i32, i32* %i, align 8
  %1 = load i32, i32* %i, align 8
  %2 = load i32, i32* %i, align 8
  %3 = load i32, i32* %i, align 8
  %4 = load i32, i32* %i, align 8
  %j = getelementptr inbounds %struct.test02, %struct.test02* %tp2, i64 0, i32 1
  %5 = load i64, i64* %j, align 8
  %c = getelementptr inbounds %struct.test01, %struct.test01* %tp, i64 0, i32 1
  store i64 0, i64* %c, align 8
  %t = getelementptr inbounds %struct.test01, %struct.test01* %tp, i64 0, i32 0
  store i32 10, i32* %t, align 8
  %h = getelementptr inbounds %struct.test01, %struct.test01* %tp, i64 0, i32 0
  store i32 20, i32* %h, align 4
  store i64 30, i64* getelementptr (%struct.test01, %struct.test01* @g_instance.test01, i64 0, i32 1), align 4
  ret void
}

define void @bar(%struct.test01* %tp, %struct.test02* %tp2, %struct.test03* %tp3) {
entry:
  %i = getelementptr inbounds %struct.test02, %struct.test02* %tp2, i64 0, i32 1
  %0 = load i64, i64* %i, align 8
  %j = getelementptr inbounds %struct.test02, %struct.test02* %tp2, i64 0, i32 0
  store %struct.test01* null,  %struct.test01** %j, align 4
  %k = getelementptr inbounds %struct.test03, %struct.test03* %tp3, i64 0, i32 1
  store %struct.test01* null,  %struct.test01** %k, align 4
  store %struct.test01* null,  %struct.test01** %k, align 4
  store %struct.test01* null,  %struct.test01** %k, align 4
  ret void
}

define void @baz(%struct.test02* %tp2, %struct.test03* %tp3) {
entry:
  %i = getelementptr inbounds %struct.test02, %struct.test02* %tp2, i64 0, i32 1
  %0 = load i64, i64* %i, align 8
  %j = getelementptr inbounds %struct.test03, %struct.test03* %tp3, i64 0, i32 0
  store %struct.test02* null,  %struct.test02** %j, align 4
  %1 = load %struct.test02*, %struct.test02** %j, align 8
  %2 = load %struct.test02*, %struct.test02** %j, align 8
  %3 = load %struct.test02*, %struct.test02** %j, align 8
  %4 = load %struct.test02*, %struct.test02** %j, align 8
  ret void
}
