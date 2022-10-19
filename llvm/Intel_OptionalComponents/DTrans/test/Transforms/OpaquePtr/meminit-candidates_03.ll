; This testcase verifies that MemInitTrimDown transformation shouldn't
; detect class.F as possible candidate because "%struct.Arr.0" type is
; not in expected form of vector class. This test is exactly same as
; meminit-candidates_01.ll except "%struct.Arr.0" and "%struct.Arr" have
; additional "i8" type field.

; RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-meminittrimdownop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-meminittrimdownop -disable-output 2>&1 | FileCheck %s

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
;   bool flag1;
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

%class._ZTS1F.F = type { %struct._ZTS3Mem.Mem*, %struct._ZTS3ArrIPiE.Arr*, %struct._ZTS4Arr1IPfE.Arr1* }
%struct._ZTS3Mem.Mem = type { i32 (...)** }
%struct._ZTS3ArrIPiE.Arr = type { i8, i8, i32, i32, i32***, %struct._ZTS3Mem.Mem* }
%struct._ZTS4Arr1IPfE.Arr1 = type { %struct._ZTS3ArrIPfE.Arr }
%struct._ZTS3ArrIPfE.Arr = type { i8, i8, i32, i32, float***, %struct._ZTS3Mem.Mem* }


define dso_local i32 @main() {
entry:
  %call = call i8* @_Znwm(i64 24)
  %0 = bitcast i8* %call to %class._ZTS1F.F*
  call void @_ZN1FC2Ev(%class._ZTS1F.F* %0)
  ret i32 0
}

define void @_ZN1FC2Ev(%class._ZTS1F.F* "intel_dtrans_func_index"="1" %this)  !intel.dtrans.func.type !22 {
entry:
  %call = tail call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct._ZTS3ArrIPiE.Arr*
  tail call void @_ZN3ArrIPiEC2EiP3Mem(%struct._ZTS3ArrIPiE.Arr* nonnull %0, i32 10, %struct._ZTS3Mem.Mem* null)
  %f1 = getelementptr inbounds %class._ZTS1F.F, %class._ZTS1F.F* %this, i64 0, i32 1
  store %struct._ZTS3ArrIPiE.Arr* %0, %struct._ZTS3ArrIPiE.Arr** %f1, align 8
  %call2 = tail call i8* @_Znwm(i64 32)
  %1 = bitcast i8* %call2 to %struct._ZTS4Arr1IPfE.Arr1*
  call void @_ZN4Arr1IPfEC2Ev(%struct._ZTS4Arr1IPfE.Arr1* nonnull %1)
  %f2 = getelementptr inbounds %class._ZTS1F.F, %class._ZTS1F.F* %this, i64 0, i32 2
  store %struct._ZTS4Arr1IPfE.Arr1* %1, %struct._ZTS4Arr1IPfE.Arr1** %f2, align 8
  %2 = load %struct._ZTS3ArrIPiE.Arr*, %struct._ZTS3ArrIPiE.Arr** %f1, align 8
  tail call void @_ZN3ArrIPiE3setEiPS0_(%struct._ZTS3ArrIPiE.Arr* %2, i32 0, i32** null)
  %3 = bitcast %struct._ZTS4Arr1IPfE.Arr1** %f2 to %struct._ZTS3ArrIPfE.Arr**
  %4 = load %struct._ZTS3ArrIPfE.Arr*, %struct._ZTS3ArrIPfE.Arr** %3, align 8
  tail call void @_ZN3ArrIPfE3setEiPS0_(%struct._ZTS3ArrIPfE.Arr* %4, i32 0, float** null)
  %5 = load %struct._ZTS3ArrIPfE.Arr*, %struct._ZTS3ArrIPfE.Arr** %3, align 8
  tail call void @_ZN3ArrIPfE3addEPS0_(%struct._ZTS3ArrIPfE.Arr* %5, float** null)
  %6 = load %struct._ZTS3ArrIPiE.Arr*, %struct._ZTS3ArrIPiE.Arr** %f1, align 8
  tail call void @_ZN3ArrIPiE3addEPS0_(%struct._ZTS3ArrIPiE.Arr* %6, i32** null)
  ret void
}

define void @_ZN3ArrIPiEC2EiP3Mem(%struct._ZTS3ArrIPiE.Arr*  "intel_dtrans_func_index"="1" %this, i32 %c, %struct._ZTS3Mem.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !26 {
entry:
  ret void
}

define void @_ZN4Arr1IPfEC2Ev(%struct._ZTS4Arr1IPfE.Arr1* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !27 {
entry:
  %0 = getelementptr inbounds %struct._ZTS4Arr1IPfE.Arr1, %struct._ZTS4Arr1IPfE.Arr1* %this, i64 0, i32 0
  tail call void @_ZN3ArrIPfEC2EiP3Mem(%struct._ZTS3ArrIPfE.Arr* %0, i32 1, %struct._ZTS3Mem.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiPS0_(%struct._ZTS3ArrIPiE.Arr* "intel_dtrans_func_index"="1" %this, i32 %i, i32**  "intel_dtrans_func_index"="2" %val)  !intel.dtrans.func.type !28 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3setEiPS0_(%struct._ZTS3ArrIPfE.Arr* "intel_dtrans_func_index"="1" %this, i32 %i, float** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !30 {
entry:
  ret void
}

define void @_ZN3ArrIPfE3addEPS0_(%struct._ZTS3ArrIPfE.Arr* "intel_dtrans_func_index"="1" %this, float** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !33 {
entry:
  tail call void @_ZN3ArrIPfE6resizeEv(%struct._ZTS3ArrIPfE.Arr* %this)
  ret void
}

define void @_ZN3ArrIPiE3addEPS0_(%struct._ZTS3ArrIPiE.Arr* "intel_dtrans_func_index"="1" %this, i32** "intel_dtrans_func_index"="2" %val) !intel.dtrans.func.type !34 {
entry:
  tail call void @_ZN3ArrIPiE6resizeEv(%struct._ZTS3ArrIPiE.Arr* %this)
  ret void
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct._ZTS3ArrIPfE.Arr* "intel_dtrans_func_index"="1" %this, i32 %c, %struct._ZTS3Mem.Mem* "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !35 {
entry:
  ret void
}

define void @_ZN3ArrIPfE6resizeEv(%struct._ZTS3ArrIPfE.Arr* "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !36 {
entry:
  ret void
}

define void @_ZN3ArrIPiE6resizeEv(%struct._ZTS3ArrIPiE.Arr* "intel_dtrans_func_index"="1"  %this) !intel.dtrans.func.type !37 {
entry:
  ret void
}

declare !intel.dtrans.func.type !20 dso_local noalias "intel_dtrans_func_index"="1" i8* @_Znwm(i64)
declare !intel.dtrans.func.type !27 void @Unused_declare_ZN4Arr1IPfEC2Ev(%struct._ZTS4Arr1IPfE.Arr1* "intel_dtrans_func_index"="1" )

!intel.dtrans.types = !{!3, !7, !11, !14, !16}

!3 = !{!"S", %class._ZTS1F.F zeroinitializer, i32 3, !4, !5, !6}
!4 = !{%struct._ZTS3Mem.Mem zeroinitializer, i32 1}
!5 = !{%struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 1}
!6 = !{%struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 1}
!7 = !{!"S", %struct._ZTS3Mem.Mem zeroinitializer, i32 1, !8}
!8 = !{!9, i32 2}
!9 = !{!"F", i1 true, i32 0, !10}
!10 = !{i32 0, i32 0}
!11 = !{!"S", %struct._ZTS3ArrIPiE.Arr zeroinitializer, i32 6, !12, !12, !10, !10, !13, !4}
!12 = !{i8 0, i32 0}
!13 = !{i32 0, i32 3}
!14 = !{!"S", %struct._ZTS4Arr1IPfE.Arr1 zeroinitializer, i32 1, !15}
!15 = !{%struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 0}
!16 = !{!"S", %struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 6, !12, !12, !10, !10, !17, !4}
!17 = !{float 0.000000e+00, i32 3}
!18 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!19 = !{%class._ZTS1F.F zeroinitializer, i32 1}
!20 = distinct !{!21}
!21 = !{i8 0, i32 1}
!22 = distinct !{!19}
!23 = !{!"F", i1 false, i32 0, !24}
!24 = !{!"void", i32 0}
!25 = distinct !{!21}
!26 = distinct !{!5, !4}
!27 = distinct !{!6}
!28 = distinct !{!5, !29}
!29 = !{i32 0, i32 2}
!30 = distinct !{!31, !32}
!31 = !{%struct._ZTS3ArrIPfE.Arr zeroinitializer, i32 1}
!32 = !{float 0.000000e+00, i32 2}
!33 = distinct !{!31, !32}
!34 = distinct !{!5, !29}
!35 = distinct !{!31, !4}
!36 = distinct !{!31}
!37 = distinct !{!5}
