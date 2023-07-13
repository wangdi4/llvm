; This testcase verifies that MemInitTrimDown transformation shouldn't
; detect class.F as possible candidate because "class.F" type has
; non-pointer field. This test is exactly same as meminit-candidates_01.ll
; except "%class.F" has additional "i32" type field.

; RUN: opt < %s -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop -disable-output 2>&1 | FileCheck %s

; REQUIRES: asserts

; Here is C++ version of the testcase. "F" will be detected as candidate
; struct. "f1" and "f2" will be considered as candidate vector fields.
; Definitions of most of the member functions are empty or doesn't make
; sense.

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
;template <typename S> struct Arr {
;   bool flag;
;   int capacity;
;   int size;
;   S **base;
;   Mem *mem;
;   void resize(void) {
;   }
;   void add(S* val) {
;     resize();
;   }
;  void set(int i, S* val) { }
;  Arr(int c = 1, Mem *mem = 0)
;      : mem(mem), capacity(c), size(0), base(0) {}
;  ~Arr() {}
; };
;
; template <typename S>
; struct Arr1 : public Arr<S> {
; };
;
; class F {
;   Mem *mem;
; public:
;   Arr<int*>* f1;
;   Arr1<float*>* f2;
;   int dummy;
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->set(0, nullptr);
;     f2->add(nullptr);
;     f1->add(nullptr);
;   }
;
; };
; int main() {
;   F *f = new F();
; }
;

;CHECK: MemInitTrimDown transformation:
;CHECK:  Failed: No candidates found.
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS1F.F = type { ptr, ptr, ptr, i32 }
%struct._ZTS4Arr1IPfE.Arr1 = type { %struct._ZTS3ArrIPfE.Arr }
%struct._ZTS3ArrIPfE.Arr = type { i8, i32, i32, ptr, ptr }
%struct._ZTS3Mem.Mem = type { ptr }
%struct._ZTS3ArrIPiE.Arr = type { i8, i32, i32, ptr, ptr }

define dso_local i32 @main() {
entry:
  %call = call ptr @_Znwm(i64 24)
  call void @_ZN1FC2Ev(ptr %call)
  ret i32 0
}

define void @_ZN1FC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !15 {
entry:
  %call = tail call ptr @_Znwm(i64 32)
  tail call void @_ZN3ArrIPiEC2EiP3Mem(ptr nonnull %call, i32 10, ptr null)
  %f1 = getelementptr inbounds %class._ZTS1F.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = tail call ptr @_Znwm(i64 32)
  call void @_ZN4Arr1IPfEC2Ev(ptr nonnull %call2)
  %f2 = getelementptr inbounds %class._ZTS1F.F, ptr %this, i64 0, i32 2
  store ptr %call2, ptr %f2, align 8
  %i2 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3setEiPS0_(ptr %i2, i32 0, ptr null)
  %i4 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3setEiPS0_(ptr %i4, i32 0, ptr null)
  %i5 = load ptr, ptr %f2, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(ptr %i5, ptr null)
  %i6 = load ptr, ptr %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(ptr %i6, ptr null)
  ret void
}

define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !17 {
entry:
  ret void
}

define void @_ZN4Arr1IPfEC2Ev(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !18 {
entry:
  %0 = getelementptr inbounds %struct._ZTS4Arr1IPfE.Arr1, ptr %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(ptr %0, i32 1, ptr null)
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !19 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !21 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !24 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEv(ptr %this)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !25 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEv(ptr %this)
  ret void
}

define void @_ZN3ArrIPfEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !26 {
entry:
  ret void
}

define void @_ZN3ArrIPfE6resizeEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !27 {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEv(ptr "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !28 {
entry:
  ret void
}

declare !intel.dtrans.func.type !29 dso_local noalias "intel_dtrans_func_index"="1" ptr @_Znwm(i64)

declare !intel.dtrans.func.type !18 void @Unused_declare_ZN4Arr1IPfEC2Ev(ptr "intel_dtrans_func_index"="1")

!intel.dtrans.types = !{!0, !5, !8, !11, !13}

!0 = !{!"S", %class._ZTS1F.F zeroinitializer, i32 4, !1, !2, !3, !4}
!1 = !{%struct._ZTS3Mem.Mem zeroinitializer, i32 1}
!2 = !{%struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 1}
!3 = !{%struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 1}
!4 = !{i32 0, i32 0}
!5 = !{!"S", %struct._ZTS3Mem.Mem zeroinitializer, i32 1, !6}
!6 = !{!7, i32 2}
!7 = !{!"F", i1 true, i32 0, !4}
!8 = !{!"S", %struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 5, !9, !4, !4, !10, !1}
!9 = !{i8 0, i32 0}
!10 = !{i32 0, i32 3}
!11 = !{!"S", %struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 1, !12}
!12 = !{%struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 0}
!13 = !{!"S", %struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 5, !9, !4, !4, !14, !1}
!14 = !{float 0.000000e+00, i32 3}
!15 = distinct !{!16}
!16 = !{%class._ZTS1F.F zeroinitializer, i32 1}
!17 = distinct !{!2, !1}
!18 = distinct !{!3}
!19 = distinct !{!2, !20}
!20 = !{i32 0, i32 2}
!21 = distinct !{!22, !23}
!22 = !{%struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 1}
!23 = !{float 0.000000e+00, i32 2}
!24 = distinct !{!22, !23}
!25 = distinct !{!2, !20}
!26 = distinct !{!22, !1}
!27 = distinct !{!22}
!28 = distinct !{!2}
!29 = distinct !{!30}
!30 = !{i8 0, i32 1}
