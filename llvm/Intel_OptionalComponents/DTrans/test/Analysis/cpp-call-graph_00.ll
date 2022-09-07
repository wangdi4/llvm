; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -S < %s -whole-program-assume -dtransanalysis -disable-output -dtrans-print-types 2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume -passes='require<dtransanalysis>' -disable-output -dtrans-print-types 2>&1 | FileCheck %s

; See explanation in C code.

; CHECK-LABEL: LLVMType: %struct.inner1 = type { i32 }
; CHECK: Call graph: top
; CHECK: Safety data:{{.*}} Local instance {{.*}} Has C++ handling
;
; CHECK-LABEL: LLVMType: %struct.inner2 = type { i32 }
; CHECK: Call graph: enclosing type: struct.outer
; CHECK: Safety data:{{.*}} Has C++ handling
;
; CHECK-LABEL: LLVMType: %struct.outer = type { %struct.inner1*, %struct.inner2* }
; CHECK: Call graph: top

; inner2 is accessed only from outer's method foo and its own method get,
;  so 'outer' is reported.
; inner1 is accessed from main as well,
;  so 'top' is reported due to access from main.
; outer is accessed from main and its method,
;  so 'top' is reported due to access from main.
;
; struct inner1 {
;   int i;
;   int get() { return i; }
; };
;
; struct inner2 {
;   int i;
;   int get() { return i; }
; };
;
; struct outer {
;     inner1*  i1;
;     inner2*  i2;
;   outer() {
;     i1 = new inner1;
;     i1->i = 0;
;     i2 = new inner2;
;     i2->i = 0;
;   }
;   int foo() {
;     return i1->get() + i1->i + i2->get() + i2->i;
;   }
; };
;
; int main() {
;   outer *o = new outer();
;   inner1 i1;
;   i1.i = 0;
;   int i = o->foo();
;   delete o;
;   return i + i1.i;
; }
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.outer = type { %struct.inner1*, %struct.inner2* }
%struct.inner1 = type { i32 }
%struct.inner2 = type { i32 }

$_ZN5outerC2Ev = comdat any

$_ZN5outer3fooEv = comdat any

$_ZN6inner13getEv = comdat any

$_ZN6inner23getEv = comdat any

define i32 @main() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %i1 = alloca %struct.inner1, align 4
  %call = call i8* @_Znwm(i64 16) #5
  %tmp = bitcast i8* %call to %struct.outer*
  invoke void @_ZN5outerC2Ev(%struct.outer* %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %i = getelementptr inbounds %struct.inner1, %struct.inner1* %i1, i32 0, i32 0
  store i32 0, i32* %i, align 4
  %call3 = call i32 @_ZN5outer3fooEv(%struct.outer* %tmp)
  %isnull = icmp eq %struct.outer* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont
  %tmp1 = bitcast %struct.outer* %tmp to i8*
  call void @_ZdlPv(i8* %tmp1) #6
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %invoke.cont
  %i4 = getelementptr inbounds %struct.inner1, %struct.inner1* %i1, i32 0, i32 0
  %tmp2 = load i32, i32* %i4, align 4
  %add = add nsw i32 %call3, %tmp2
  ret i32 %add

lpad:                                             ; preds = %entry
  %tmp3 = landingpad { i8*, i32 }
          cleanup
  %tmp4 = extractvalue { i8*, i32 } %tmp3, 0
  %tmp5 = extractvalue { i8*, i32 } %tmp3, 1
  call void @_ZdlPv(i8* %call) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %tmp4, 0
  %lpad.val5 = insertvalue { i8*, i32 } %lpad.val, i32 %tmp5, 1
  resume { i8*, i32 } %lpad.val5
}

declare noalias i8* @_Znwm(i64) #1

define void @_ZN5outerC2Ev(%struct.outer* %this) unnamed_addr #2 comdat align 2 {
entry:
  %call = call i8* @_Znwm(i64 4) #5
  %tmp = bitcast i8* %call to %struct.inner1*
  %i1 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 0
  store %struct.inner1* %tmp, %struct.inner1** %i1, align 8
  %i12 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 0
  %tmp1 = load %struct.inner1*, %struct.inner1** %i12, align 8
  %i = getelementptr inbounds %struct.inner1, %struct.inner1* %tmp1, i32 0, i32 0
  store i32 0, i32* %i, align 4
  %call3 = call i8* @_Znwm(i64 4) #5
  %tmp2 = bitcast i8* %call3 to %struct.inner2*
  %i2 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 1
  store %struct.inner2* %tmp2, %struct.inner2** %i2, align 8
  %i24 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 1
  %tmp3 = load %struct.inner2*, %struct.inner2** %i24, align 8
  %i5 = getelementptr inbounds %struct.inner2, %struct.inner2* %tmp3, i32 0, i32 0
  store i32 0, i32* %i5, align 4
  ret void
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*) #3

define i32 @_ZN5outer3fooEv(%struct.outer* %this) #2 comdat align 2 {
entry:
  %i1 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 0
  %tmp = load %struct.inner1*, %struct.inner1** %i1, align 8
  %call = call i32 @_ZN6inner13getEv(%struct.inner1* %tmp)
  %i12 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 0
  %tmp1 = load %struct.inner1*, %struct.inner1** %i12, align 8
  %i = getelementptr inbounds %struct.inner1, %struct.inner1* %tmp1, i32 0, i32 0
  %tmp2 = load i32, i32* %i, align 4
  %add = add nsw i32 %call, %tmp2
  %i2 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 1
  %tmp3 = load %struct.inner2*, %struct.inner2** %i2, align 8
  %call3 = call i32 @_ZN6inner23getEv(%struct.inner2* %tmp3)
  %add4 = add nsw i32 %add, %call3
  %i25 = getelementptr inbounds %struct.outer, %struct.outer* %this, i32 0, i32 1
  %tmp4 = load %struct.inner2*, %struct.inner2** %i25, align 8
  %i6 = getelementptr inbounds %struct.inner2, %struct.inner2* %tmp4, i32 0, i32 0
  %tmp5 = load i32, i32* %i6, align 4
  %add7 = add nsw i32 %add4, %tmp5
  ret i32 %add7
}

define i32 @_ZN6inner13getEv(%struct.inner1* %this) #4 comdat align 2 {
entry:
  %i = getelementptr inbounds %struct.inner1, %struct.inner1* %this, i32 0, i32 0
  %tmp = load i32, i32* %i, align 4
  ret i32 %tmp
}

define i32 @_ZN6inner23getEv(%struct.inner2* %this) #4 comdat align 2 {
entry:
  %i = getelementptr inbounds %struct.inner2, %struct.inner2* %this, i32 0, i32 0
  %tmp = load i32, i32* %i, align 4
  ret i32 %tmp
}

attributes #0 = { noinline norecurse uwtable }
attributes #1 = { nobuiltin }
attributes #2 = { noinline uwtable }
attributes #3 = { nobuiltin nounwind }
attributes #4 = { noinline nounwind uwtable }
attributes #5 = { builtin }
attributes #6 = { builtin nounwind }
