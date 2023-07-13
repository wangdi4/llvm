; RUN: opt < %s -passes=dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

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

%class.F = type { ptr, ptr, ptr, ptr }
%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.2 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ ptr, i32, [4 x i8], ptr, i32 }>
%class.A = type { ptr, ptr, ptr, ptr }

define i32 @main() personality ptr @__gxx_personality_v0 {
entry:
  %call = call ptr @malloc(i64 32)
; Implicit bitcast is causing safety violation on F
;  %i = bitcast ptr %call to ptr
;  %bad_bc = bitcast ptr %i to ptr
  store i8 2, ptr %call, align 2
  invoke void @_ZN1FC2Ev(ptr %call)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i32 0

lpad:                                             ; preds = %entry
  %i1 = landingpad { ptr, i32 }
          cleanup
  call void @free(ptr %call)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  resume { ptr, i32 } undef
}

declare !intel.dtrans.func.type !22 noalias "intel_dtrans_func_index"="1" ptr @malloc(i64)

define void @_ZN1FC2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !23 {
entry:
  %call = call ptr @malloc(i64 32)
  invoke void @_ZN3ArrIPiEC2EiP3Mem(ptr %call, i32 10, ptr null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = call ptr @malloc(i64 32)
  invoke void @_ZN3ArrIPvEC2EiP3Mem(ptr %call2, i32 10, ptr null)
          to label %invoke.cont4 unwind label %lpad

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 2
  store ptr %call2, ptr %f2, align 8
  %call5 = call ptr @malloc(i64 32)
  call void @llvm.memset.p0.i64(ptr align 16 %call5, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(ptr %call5)
          to label %invoke.cont7 unwind label %lpad

invoke.cont7:                                     ; preds = %invoke.cont4
  %f3 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 3
  store ptr %call5, ptr %f3, align 8
  %f18 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 1
  %i4 = load ptr, ptr %f18, align 8
  call void @_ZN3ArrIPiE3setEiS0_(ptr %i4, i32 0, ptr null)
  %f29 = getelementptr inbounds %class.F, ptr %this, i32 0, i32 2
  %i5 = load ptr, ptr %f29, align 8
  %call10 = call ptr @_ZN3ArrIPvE3getEi(ptr %i5, i32 0)
  ret void

lpad:                                             ; preds = %invoke.cont4, %invoke.cont, %entry
  %i6 = landingpad { ptr, i32 }
          cleanup
  ret void
}

declare i32 @__gxx_personality_v0(...)

declare !intel.dtrans.func.type !25 void @free(ptr "intel_dtrans_func_index"="1")

define void @_ZN3ArrIPiEC2EiP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !26 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  store i32 %c, ptr %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  store i32 0, ptr %size, align 8
  ret void
}

define void @_ZN3ArrIPvEC2EiP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !27 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 1
  store i32 %c, ptr %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 4
  store i32 0, ptr %size, align 8
  ret void
}

define void @_ZN4Arr1IPfEC2Ev(ptr nocapture "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !28 {
entry:
  call void @_ZN3ArrIPfEC2EiP3Mem(ptr %this, i32 1, ptr null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(ptr nocapture readonly "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %val) align 2 !intel.dtrans.func.type !29 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %i1 = load ptr, ptr %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  store ptr %val, ptr %arrayidx, align 8
  ret void
}

define "intel_dtrans_func_index"="1" ptr @_ZN3ArrIPvE3getEi(ptr nocapture readonly "intel_dtrans_func_index"="2" %this, i32 %i) align 2 !intel.dtrans.func.type !31 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i32 0, i32 3
  %i1 = load ptr, ptr %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx, align 8
  ret ptr %i2
}

define void @_ZN3ArrIPfEC2EiP3Mem(ptr nocapture "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !32 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.2, ptr %this, i32 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.2, ptr %this, i32 0, i32 1
  store i32 %c, ptr %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.2, ptr %this, i32 0, i32 3
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.2, ptr %this, i32 0, i32 4
  store i32 0, ptr %size, align 8
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: write) }

!intel.dtrans.types = !{!0, !5, !9, !13, !15, !17, !19, !20}

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
!20 = !{!"S", %class.A zeroinitializer, i32 4, !21, !21, !21, !21}
!21 = !{i8 0, i32 1}
!22 = distinct !{!21}
!23 = distinct !{!24}
!24 = !{%class.F zeroinitializer, i32 1}
!25 = distinct !{!21}
!26 = distinct !{!2, !1}
!27 = distinct !{!3, !1}
!28 = distinct !{!4}
!29 = distinct !{!2, !30}
!30 = !{i32 0, i32 1}
!31 = distinct !{!21, !3}
!32 = distinct !{!33, !1}
!33 = !{%struct.Arr.2 zeroinitializer, i32 1}
