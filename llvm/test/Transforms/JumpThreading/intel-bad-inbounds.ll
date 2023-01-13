; RUN: opt -S -passes=jump-threading %s | FileCheck %s
; CHECK: icmp{{.*}}tmp6, null

; The code computes the "negation" of a pointer, and adds it back using a GEP
; to result in null.
; This is incorrect IR, as the GEP guarantees that a non-null
; pointer cannot be changed to point outside the base object.
; The code source is 502.gcc and cannot be fixed, so it must be worked around
; in instcombine and jump-threading.
; We detect the subtraction pattern and prevent the optimization from assuming
; the result is non-null.

; The "useless" control flow such as the switch and empty blocks, are needed
; for jump threading to activate.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.33.31629"

define fastcc void @barney() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb1, %bb1, %bb1, %bb1, %bb1, %bb
  switch i8 0, label %bb13 [
    i8 0, label %bb1
    i8 63, label %bb4
    i8 62, label %bb4
    i8 52, label %bb1
    i8 1, label %bb1
    i8 38, label %bb1
    i8 60, label %bb1
    i8 61, label %bb4
    i8 64, label %bb4
    i8 65, label %bb4
    i8 53, label %bb13
    i8 54, label %bb13
    i8 70, label %bb13
  ]

bb2:                                              ; No predecessors!
  br label %bb4

bb3:                                              ; No predecessors!
  br label %bb4

bb4:                                              ; preds = %bb3, %bb2, %bb1, %bb1, %bb1, %bb1, %bb1
  %tmp = ptrtoint i8* null to i64
  %tmp5 = sub i64 0, %tmp
  %tmp6 = getelementptr inbounds i8, i8* null, i64 %tmp5
  %tmp7 = bitcast i8* null to i8**
  %tmp8 = load i8*, i8** %tmp7, align 8
  br label %bb10

bb9:                                              ; No predecessors!
  br label %bb10

bb10:                                             ; preds = %bb9, %bb4
  %tmp11 = icmp eq i8* %tmp6, null
  br i1 %tmp11, label %bb13, label %bb12

bb12:                                             ; preds = %bb10
  unreachable

bb13:                                             ; preds = %bb10, %bb1, %bb1, %bb1, %bb1
  ret void
}
