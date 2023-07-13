; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -S < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -dtrans-print-types 2>&1 | FileCheck %s

; See explanation in C code.

; CHECK-LABEL: LLVMType: %struct.inner1 = type { i32 }
; CHECK: Call graph: top
; CHECK: Safety data:{{.*}} Local pointer {{.*}}
;
; CHECK-LABEL: LLVMType: %struct.inner2 = type { i32 }
; CHECK: Call graph: enclosing type: struct.outer
; CHECK: Safety data:{{.*}} Local pointer {{.*}}
;
; CHECK-LABEL: LLVMType: %struct.outer = type { ptr, ptr }
; CHECK: Call graph: top

; inner2 is accessed only from outer's method foo and its own method get,
;  so 'outer' is reported.
; inner1 is accessed from unsupported get_outer function,
;  so 'top' is reported due to access from get_outer.
; outer is accessed from main and its method,
;  so 'top' is reported due to access from main.
;
; struct inner1 {
;   int i;
;   int get() { return i; }
; };
;
; int get_outer(int a, inner1 *i) {
;   return i->i;
; }
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
;     return i1->get() + i2->get() + i2->i + get_outer(-1, i1);
;   }
; };
;
; int main() {
;   outer *o = new outer();
;   int i = o->foo();
;   delete o;
;   return i;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.inner1 = type { i32 }
%struct.outer = type { ptr, ptr }
%struct.inner2 = type { i32 }

$_ZN5outerC2Ev = comdat any

$_ZN5outer3fooEv = comdat any

$_ZN6inner13getEv = comdat any

$_ZN6inner23getEv = comdat any

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @_Z9get_outeriP6inner1(i32 %a, ptr "intel_dtrans_func_index"="1" %i) #0 !intel.dtrans.func.type !10 {
entry:
  %a.addr = alloca i32, align 4
  %i.addr = alloca ptr, align 8, !intel_dtrans_type !6
  store i32 %a, ptr %a.addr, align 4
  store ptr %i, ptr %i.addr, align 8
  %0 = load ptr, ptr %i.addr, align 8
  %i1 = getelementptr inbounds %struct.inner1, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %i1, align 4
  ret i32 %1
}

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #1 personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) {
entry:
  %retval = alloca i32, align 4
  %o = alloca ptr, align 8, !intel_dtrans_type !11
  %exn.slot = alloca ptr
  %ehselector.slot = alloca i32
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %call = call ptr @_Znwm(i64 16) #5
  %0 = bitcast ptr %call to ptr
  invoke void @_ZN5outerC2Ev(ptr %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  store ptr %0, ptr %o, align 8
  %1 = load ptr, ptr %o, align 8
  %call1 = call i32 @_ZN5outer3fooEv(ptr %1)
  store i32 %call1, ptr %i, align 4
  %2 = load ptr, ptr %o, align 8
  %isnull = icmp eq ptr %2, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont
  %3 = bitcast ptr %2 to ptr
  call void @_ZdlPv(ptr %3) #6
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %invoke.cont
  %4 = load i32, ptr %i, align 4
  ret i32 %4

lpad:                                             ; preds = %entry
  %5 = landingpad { ptr, i32 }
          cleanup
  %6 = extractvalue { ptr, i32 } %5, 0
  store ptr %6, ptr %exn.slot, align 8
  %7 = extractvalue { ptr, i32 } %5, 1
  store i32 %7, ptr %ehselector.slot, align 4
  call void @_ZdlPv(ptr %call) #6
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %sel = load i32, ptr %ehselector.slot, align 4
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %exn, 0
  %lpad.val2 = insertvalue { ptr, i32 } %lpad.val, i32 %sel, 1
  resume { ptr, i32 } %lpad.val2
}

declare noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64) #2

define void @_ZN5outerC2Ev(ptr "intel_dtrans_func_index"="1" %this) unnamed_addr #3 comdat align 2 !intel.dtrans.func.type !14 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !11
  store ptr %this, ptr %this.addr, align 8, !intel_dtrans_type !11
  %this1 = load ptr, ptr %this.addr, align 8
  %call = call ptr @_Znwm(i64 4) #5
  %0 = bitcast ptr %call to ptr
  %i1 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 0
  store ptr %0, ptr %i1, align 8
  %i12 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 0
  %1 = load ptr, ptr %i12, align 8
  %i = getelementptr inbounds %struct.inner1, ptr %1, i32 0, i32 0
  store i32 0, ptr %i, align 4
  %call3 = call ptr @_Znwm(i64 4) #5
  %2 = bitcast ptr %call3 to ptr
  %i2 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 1
  store ptr %2, ptr %i2, align 8
  %i24 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 1
  %3 = load ptr, ptr %i24, align 8
  %i5 = getelementptr inbounds %struct.inner2, ptr %3, i32 0, i32 0
  store i32 0, ptr %i5, align 4
  ret void
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !15 void @_ZdlPv(ptr "intel_dtrans_func_index"="1") #4

define i32 @_ZN5outer3fooEv(ptr "intel_dtrans_func_index"="1" %this) #3 comdat align 2 !intel.dtrans.func.type !16 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !11
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %i1 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 0
  %0 = load ptr, ptr %i1, align 8
  %call = call i32 @_ZN6inner13getEv(ptr %0)
  %i2 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 1
  %1 = load ptr, ptr %i2, align 8
  %call2 = call i32 @_ZN6inner23getEv(ptr %1)
  %add = add nsw i32 %call, %call2
  %i23 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 1
  %2 = load ptr, ptr %i23, align 8
  %i = getelementptr inbounds %struct.inner2, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %i, align 4
  %add4 = add nsw i32 %add, %3
  %i15 = getelementptr inbounds %struct.outer, ptr %this1, i32 0, i32 0
  %4 = load ptr, ptr %i15, align 8
  %call6 = call i32 @_Z9get_outeriP6inner1(i32 -1, ptr %4)
  %add7 = add nsw i32 %add4, %call6
  ret i32 %add7
}

define i32 @_ZN6inner13getEv(ptr "intel_dtrans_func_index"="1" %this) #0 comdat align 2 !intel.dtrans.func.type !17  {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !6
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %i = getelementptr inbounds %struct.inner1, ptr %this1, i32 0, i32 0
  %0 = load i32, ptr %i, align 4
  ret i32 %0
}

define i32 @_ZN6inner23getEv(ptr "intel_dtrans_func_index"="1" %this) #0 comdat align 2 !intel.dtrans.func.type !18 {
entry:
  %this.addr = alloca ptr, align 8, !intel_dtrans_type !7
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %i = getelementptr inbounds %struct.inner2, ptr %this1, i32 0, i32 0
  %0 = load i32, ptr %i, align 4
  ret i32 %0
}

attributes #0 = { noinline nounwind uwtable }
attributes #1 = { noinline norecurse uwtable }
attributes #3 = { noinline uwtable }
attributes #4 = { nobuiltin nounwind }
attributes #5 = { builtin }
attributes #6 = { builtin nounwind }

!intel.dtrans.types = !{!3, !5, !8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"S", %struct.inner1 zeroinitializer, i32 1, !4}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %struct.outer zeroinitializer, i32 2, !6, !7}
!6 = !{%struct.inner1 zeroinitializer, i32 1}
!7 = !{%struct.inner2 zeroinitializer, i32 1}
!8 = !{!"S", %struct.inner2 zeroinitializer, i32 1, !4}
!9 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!10 = distinct !{!6}
!11 = !{%struct.outer zeroinitializer, i32 1}
!12 = distinct !{!13}
!13 = !{i8 0, i32 1}
!14 = distinct !{!11}
!15 = distinct !{!13}
!16 = distinct !{!11}
!17 = distinct !{!6}
!18 = distinct !{!7}
