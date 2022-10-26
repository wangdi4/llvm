; RUN: opt < %s -dtransop-allow-typed-pointers -passes=dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; This test checks safety data violations on structure of interest (F).

; CHECK: ; Rejecting %class.F because safety checks were violated

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; struct S1;
; struct S2;
;
; template <typename S> struct Arr {
;   Mem *mem;
;   int capacilty;
;   S *base;
;   int size;
;   Parameter types are restricted.
;   S get(int i) { return base[i]; }
;   Parameter types are restricted.
;   void set(int i, S val) { base[i] = val; }

;   Only Mem is captured.

;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {}
;   ~Arr() {}
; };
;
; template <typename S> struct Arr1 : public Arr<S> {};
;
; class F {
;   Mem *mem;
;
; public:
;   Arr<int *> *f1;
;   Arr<void *> *f2;
;   Arr1<float *> *f3;
;   f1 and f2 are only accessed from F's methods.
;   F() {
;     f1 = new Arr<int *>(10, nullptr);
;     f2 = new Arr<void *>(10, nullptr);
;     f3 = new Arr1<float *>();
;     f1->set(0, nullptr);
;     f2->get(0);
;   }
; };
;
; int main() { F *f = new F(); <bad bitcast causing safety violation on F> }
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.A = type { i8*, i8*, i8*, i8* }
%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

define i32 @main() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @malloc(i64 32)
  %0 = bitcast i8* %call to %class.F*
  %bad_bc = bitcast %class.F* %0 to i8*
  store i8 2, i8* %bad_bc, align 2
  invoke void @_ZN1FC2Ev(%class.F* %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i32 0

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 }
          cleanup
  call void @free(i8* %call)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  resume { i8*, i32 } undef
}

declare !intel.dtrans.func.type !20 noalias "intel_dtrans_func_index"="1" i8* @malloc(i64)

define void @_ZN1FC2Ev(%class.F* nocapture "intel_dtrans_func_index"="1" %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !22 {
entry:
  %call = call i8* @malloc(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %0, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  store %struct.Arr* %0, %struct.Arr** %f1, align 8
  %call2 = call i8* @malloc(i64 32)
  %1 = bitcast i8* %call2 to %struct.Arr.0*
  invoke void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* %1, i32 10, %struct.Mem* null)
          to label %invoke.cont4 unwind label %lpad

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  store %struct.Arr.0* %1, %struct.Arr.0** %f2, align 8
  %call5 = call i8* @malloc(i64 32)
  %2 = bitcast i8* %call5 to %struct.Arr1*
  %3 = bitcast %struct.Arr1* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %3, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* %2)
          to label %invoke.cont7 unwind label %lpad

invoke.cont7:                                     ; preds = %invoke.cont4
  %f3 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 3
  store %struct.Arr1* %2, %struct.Arr1** %f3, align 8
  %f18 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  %4 = load %struct.Arr*, %struct.Arr** %f18, align 8
  call void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %4, i32 0, i32* null)
  %f29 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  %5 = load %struct.Arr.0*, %struct.Arr.0** %f29, align 8
  %call10 = call i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* %5, i32 0)
  ret void

lpad:                                             ; preds = %entry
  %6 = landingpad { i8*, i32 }
          cleanup
  ret void
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !33 void @free(i8* "intel_dtrans_func_index"="1" )

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !24 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !25 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  store i8** null, i8*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !26 {
entry:
  %0 = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, i32* "intel_dtrans_func_index"="2" %val) align 2 !intel.dtrans.func.type !27 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %0 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %0, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define "intel_dtrans_func_index"="1" i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly "intel_dtrans_func_index"="2" %this, i32 %i) align 2 !intel.dtrans.func.type !29 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %0 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %0, i64 %idxprom
  %1 = load i8*, i8** %arrayidx, align 8
  ret i8* %1
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !30 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 3
  store float** null, float*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.2, %struct.Arr.2* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

attributes #0 = { argmemonly nofree nounwind willreturn writeonly }

!intel.dtrans.types = !{!0, !5, !9, !13, !15, !17, !19, !32}

!0 = !{!"S", %class.F zeroinitializer, i32 4, !1, !2, !3, !4}
!1 = !{%struct.Mem zeroinitializer, i32 1}
!2 = !{%struct.Arr zeroinitializer, i32 1}
!3 = !{%struct.Arr.0 zeroinitializer, i32 1}
!4 = !{%struct.Arr1 zeroinitializer, i32 1}
!5 = !{!"S", %struct.Mem zeroinitializer, i32 1, !6}
!6 = !{!7, i32 2}
!7 = !{!"F", i1 true, i32 0, !8}
!8 = !{i32 0, i32 0}
!9 = !{!"S", %struct.Arr zeroinitializer, i32 6, !1, !8, !10, !12, !8, !10}
!10 = !{!"A", i32 4, !11}
!11 = !{i8 0, i32 0}
!12 = !{i32 0, i32 2}
!13 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !1, !8, !10, !14, !8, !10}
!14 = !{i8 0, i32 2}
!15 = !{!"S", %struct.Arr1 zeroinitializer, i32 2, !16, !10}
!16 = !{%struct.Arr.base.3 zeroinitializer, i32 0}
!17 = !{!"S", %struct.Arr.2 zeroinitializer, i32 6, !1, !8, !10, !18, !8, !10}
!18 = !{float 0.000000e+00, i32 2}
!19 = !{!"S", %struct.Arr.base.3 zeroinitializer, i32 5, !1, !8, !10, !18, !8}
!20 = distinct !{!21}
!21 = !{i8 0, i32 1}
!22 = distinct !{!23}
!23 = !{%class.F zeroinitializer, i32 1}
!24 = distinct !{!2, !1}
!25 = distinct !{!3, !1}
!26 = distinct !{!4}
!27 = distinct !{!2, !28}
!28 = !{i32 0, i32 1}
!29 = distinct !{!21, !3}
!30 = distinct !{!31, !1}
!31 = !{%struct.Arr.2 zeroinitializer, i32 1}
!32 = !{!"S", %class.A zeroinitializer, i32 4, !21, !21, !21, !21}
!33 = distinct !{!21}
!34 = !{%class.A zeroinitializer, i32 1}
!35 = distinct !{!34}
