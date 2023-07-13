; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -dtrans-print-types 2>&1 | FileCheck %s

; See explanation in C code.

; CHECK-LABEL: LLVMType: %struct.inner1 = type { i32 }
; CHECK: Call graph: top
; CHECK: Safety data:{{.*}} Local instance {{.*}}
;
; CHECK-LABEL: LLVMType: %struct.inner2 = type { i32 }
; CHECK: Call graph: enclosing type: struct.outer
; CHECK: Safety data:{{.*}}
;
; CHECK-LABEL: LLVMType: %struct.outer = type { ptr, ptr }
; CHECK: Call graph: top
; CHECK: Safety data:{{.*}}

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

%struct.outer = type { ptr, ptr }
%struct.inner1 = type { i32 }
%struct.inner2 = type { i32 }

$_ZN5outerC2Ev = comdat any

$_ZN5outer3fooEv = comdat any

$_ZN6inner13getEv = comdat any

$_ZN6inner23getEv = comdat any

define i32 @main() #0 personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) {
entry:
  %i1 = alloca %struct.inner1, align 4
  %call = call ptr @_Znwm(i64 16) #5
  %tmp = bitcast ptr %call to ptr
  invoke void @_ZN5outerC2Ev(ptr %tmp)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %i = getelementptr inbounds %struct.inner1, ptr %i1, i32 0, i32 0
  store i32 0, ptr %i, align 4
  %call3 = call i32 @_ZN5outer3fooEv(ptr %tmp)
  %isnull = icmp eq ptr %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont
  %tmp1 = bitcast ptr %tmp to ptr
  call void @_ZdlPv(ptr %tmp1) #6
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %invoke.cont
  %i4 = getelementptr inbounds %struct.inner1, ptr %i1, i32 0, i32 0
  %tmp2 = load i32, ptr %i4, align 4
  %add = add nsw i32 %call3, %tmp2
  ret i32 %add

lpad:                                             ; preds = %entry
  %tmp3 = landingpad { ptr, i32 }
          cleanup
  %tmp4 = extractvalue { ptr, i32 } %tmp3, 0
  %tmp5 = extractvalue { ptr, i32 } %tmp3, 1
  call void @_ZdlPv(ptr %call) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %tmp4, 0
  %lpad.val5 = insertvalue { ptr, i32 } %lpad.val, i32 %tmp5, 1
  resume { ptr, i32 } %lpad.val5
}

declare noalias ptr @_Znwm(i64) #1

define void @_ZN5outerC2Ev(ptr "intel_dtrans_func_index"="1" %this) unnamed_addr #2 comdat align 2 !intel.dtrans.func.type !13 {
entry:
  %call = call ptr @_Znwm(i64 4) #5
  %tmp = bitcast ptr %call to ptr
  %i1 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 0
  store ptr %tmp, ptr %i1, align 8
  %i12 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 0
  %tmp1 = load ptr, ptr %i12, align 8
  %i = getelementptr inbounds %struct.inner1, ptr %tmp1, i32 0, i32 0
  store i32 0, ptr %i, align 4
  %call3 = call ptr @_Znwm(i64 4) #5
  %tmp2 = bitcast ptr %call3 to ptr
  %i2 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 1
  store ptr %tmp2, ptr %i2, align 8
  %i24 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 1
  %tmp3 = load ptr, ptr %i24, align 8
  %i5 = getelementptr inbounds %struct.inner2, ptr %tmp3, i32 0, i32 0
  store i32 0, ptr %i5, align 4
  ret void
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !14 void @_ZdlPv(ptr "intel_dtrans_func_index"="1") #3

define i32 @_ZN5outer3fooEv(ptr "intel_dtrans_func_index"="1" %this) #2 comdat align 2 !intel.dtrans.func.type !15 {
entry:
  %i1 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 0
  %tmp = load ptr, ptr %i1, align 8
  %call = call i32 @_ZN6inner13getEv(ptr %tmp)
  %i12 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 0
  %tmp1 = load ptr, ptr %i12, align 8
  %i = getelementptr inbounds %struct.inner1, ptr %tmp1, i32 0, i32 0
  %tmp2 = load i32, ptr %i, align 4
  %add = add nsw i32 %call, %tmp2
  %i2 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 1
  %tmp3 = load ptr, ptr %i2, align 8
  %call3 = call i32 @_ZN6inner23getEv(ptr %tmp3)
  %add4 = add nsw i32 %add, %call3
  %i25 = getelementptr inbounds %struct.outer, ptr %this, i32 0, i32 1
  %tmp4 = load ptr, ptr %i25, align 8
  %i6 = getelementptr inbounds %struct.inner2, ptr %tmp4, i32 0, i32 0
  %tmp5 = load i32, ptr %i6, align 4
  %add7 = add nsw i32 %add4, %tmp5
  ret i32 %add7
}

define i32 @_ZN6inner13getEv(ptr "intel_dtrans_func_index"="1" %this) #4 comdat align 2 !intel.dtrans.func.type !16 {
entry:
  %i = getelementptr inbounds %struct.inner1, ptr %this, i32 0, i32 0
  %tmp = load i32, ptr %i, align 4
  ret i32 %tmp
}

define i32 @_ZN6inner23getEv(ptr "intel_dtrans_func_index"="1" %this) #4 comdat align 2 !intel.dtrans.func.type !17 {
entry:
  %i = getelementptr inbounds %struct.inner2, ptr %this, i32 0, i32 0
  %tmp = load i32, ptr %i, align 4
  ret i32 %tmp
}

attributes #0 = { noinline norecurse uwtable }
attributes #1 = { nobuiltin }
attributes #2 = { noinline uwtable }
attributes #3 = { nobuiltin nounwind }
attributes #4 = { noinline nounwind uwtable }
attributes #5 = { builtin }
attributes #6 = { builtin nounwind }

!intel.dtrans.types = !{!3, !6, !8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"S", %struct.outer zeroinitializer, i32 2, !4, !5}
!4 = !{%struct.inner1 zeroinitializer, i32 1}
!5 = !{%struct.inner2 zeroinitializer, i32 1}
!6 = !{!"S", %struct.inner1 zeroinitializer, i32 1, !7}
!7 = !{i32 0, i32 0}
!8 = !{!"S", %struct.inner2 zeroinitializer, i32 1, !7}
!9 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!10 = !{%struct.outer zeroinitializer, i32 1}
!11 = distinct !{!12}
!12 = !{i8 0, i32 1}
!13 = distinct !{!10}
!14 = distinct !{!12}
!15 = distinct !{!10}
!16 = distinct !{!4}
!17 = distinct !{!5}

