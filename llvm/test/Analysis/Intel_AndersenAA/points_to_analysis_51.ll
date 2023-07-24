; This test verifies that "store ptr null, ptr %offset17" is not removed by
; InstCombine by using incorrect points-to info.
; Points-to info for %call11 shouldn't be empty as all target functions
; of %0 are not known in the current module.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK-LABEL: @create_timezone
; CHECK: store ptr null, ptr %offset17

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._typeobject = type { %struct.PyVarObject, ptr, i64, i64, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, ptr, ptr, ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, ptr, ptr, i8 }
%struct.PyVarObject = type { %struct._object, i64 }
%struct._object = type { %union.anon, ptr }
%union.anon = type { i64 }
%struct.PyMethodDef = type { ptr, ptr, i32, ptr }
%struct.PyDateTime_TimeZone = type { %struct._object, ptr, ptr }

@PyDateTime_TimeZoneType = internal global %struct._typeobject { %struct.PyVarObject { %struct._object { %union.anon { i64 4294967295 }, ptr null }, i64 0 }, ptr @.str.76, i64 32, i64 0, ptr null, i64 0, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, i64 0, ptr @timezone_doc, ptr null, ptr null, ptr @timezone_richcompare, i64 0, ptr null, ptr null, ptr @timezone_methods, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, i64 0, ptr null, ptr null, ptr @timezone_new, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, i32 0, ptr null, ptr null, i8 0 }
@.str.76 = external constant [18 x i8]
@timezone_doc = external constant [48 x i8]
@timezone_methods = external global [6 x %struct.PyMethodDef]

define internal fastcc ptr @create_timezone() {
cond.end:
  %0 = load ptr, ptr getelementptr inbounds (%struct._typeobject, ptr @PyDateTime_TimeZoneType, i64 0, i32 36), align 8
  %call11 = call ptr %0(ptr null, i64 0)
  %call12 = call ptr @foo(ptr null, ptr null)
  %offset17 = getelementptr %struct.PyDateTime_TimeZone, ptr %call11, i64 0, i32 1
  store ptr null, ptr %offset17, align 8
  ret ptr null
}

define internal ptr @timezone_richcompare(ptr %self) {
entry:
  br label %return

lor.rhs.i:                                        ; No predecessors!
  %call2.i = call i32 @PyType_IsSubtype(ptr null, ptr @PyDateTime_TimeZoneType)
  br label %return

return:                                           ; preds = %lor.rhs.i, %entry
  ret ptr null
}

declare ptr @timezone_new()
declare dso_local i32 @PyType_IsSubtype(ptr noundef, ptr noundef)
declare ptr @foo(ptr, ptr)
