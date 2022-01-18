; RUN: opt < %s -dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test is same as soatoaos_cand_01.ll except additional "Unexpected*"
; argument is passed to "set" function.
; This test verifies that %class.F is not considered as candidate due to
; "Unexpected*" argument of "set" function.

; struct Mem {
;   virtual void *allocate() = 0;
;   virtual void *deallocate() = 0;
; };
;
; struct Unexpected {
;  int *i;
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
;   void set(int i, S val, Unexpected *p) { base[i] = val; }

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
;     f1->set(0, nullptr, nullptr);
;     f2->get(0);
;   }
; };
;
; int main() { F *f = new F(); }
;
; CHECK: dtrans-soatoaos-layout: array method has unsupported parameter.
; CHECK: Rejecting %class.F because it does not look like a candidate from CFG analysis.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Unexpected = type { i32* }
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

define i32 @main() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %class.F*
  invoke void @_ZN1FC2Ev(%class.F* %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i32 0

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 }
          cleanup
  %2 = extractvalue { i8*, i32 } %1, 0
  %3 = extractvalue { i8*, i32 } %1, 1
  call void @_ZdlPv(i8* %call)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %2, 0
  %lpad.val1 = insertvalue { i8*, i32 } %lpad.val, i32 %3, 1
  resume { i8*, i32 } %lpad.val1
}

declare !intel.dtrans.func.type !25 dso_local nonnull "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

define void @_ZN1FC2Ev(%class.F* nocapture "intel_dtrans_func_index"="1" %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !27 {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %0, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 1
  store %struct.Arr* %0, %struct.Arr** %f1, align 8
  %call2 = call i8* @_Znwm(i64 32)
  %1 = bitcast i8* %call2 to %struct.Arr.0*
  invoke void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* %1, i32 10, %struct.Mem* null)
          to label %invoke.cont4 unwind label %lpad

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  store %struct.Arr.0* %1, %struct.Arr.0** %f2, align 8
  %call5 = call i8* @_Znwm(i64 32)
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
  call void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %4, i32 0, i32* null, %struct.Unexpected* null)
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

declare !intel.dtrans.func.type !39 void @_ZdlPv(i8*  "intel_dtrans_func_index"="1")

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture  "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !40 {
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

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !48 {
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

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture align 8 dereferenceable(32)  "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !55 {
entry:
  %0 = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %this, i32 %i, i32* "intel_dtrans_func_index"="2" %val, %struct.Unexpected* nocapture "intel_dtrans_func_index"="3" %p) align 2 !intel.dtrans.func.type !56 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %0 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %0, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define "intel_dtrans_func_index"="1" i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %this, i32 %i) align 2  !intel.dtrans.func.type !60 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %0 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %0, i64 %idxprom
  %1 = load i8*, i8** %arrayidx, align 8
  ret i8* %1
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture  "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2  !intel.dtrans.func.type !63 {
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

!llvm.module.flags = !{!0, !1}
!intel.dtrans.types = !{!2, !7, !11, !15, !17, !19, !21, !22}
!llvm.ident = !{!24}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"S", %class.F zeroinitializer, i32 4, !3, !4, !5, !6}
!3 = !{%struct.Mem zeroinitializer, i32 1}
!4 = !{%struct.Arr zeroinitializer, i32 1}
!5 = !{%struct.Arr.0 zeroinitializer, i32 1}
!6 = !{%struct.Arr1 zeroinitializer, i32 1}
!7 = !{!"S", %struct.Mem zeroinitializer, i32 1, !8}
!8 = !{!9, i32 2}
!9 = !{!"F", i1 true, i32 0, !10}
!10 = !{i32 0, i32 0}
!11 = !{!"S", %struct.Arr zeroinitializer, i32 6, !3, !10, !12, !14, !10, !12}
!12 = !{!"A", i32 4, !13}
!13 = !{i8 0, i32 0}
!14 = !{i32 0, i32 2}
!15 = !{!"S", %struct.Arr.0 zeroinitializer, i32 6, !3, !10, !12, !16, !10, !12}
!16 = !{i8 0, i32 2}
!17 = !{!"S", %struct.Arr1 zeroinitializer, i32 2, !18, !12}
!18 = !{%struct.Arr.base.3 zeroinitializer, i32 0}
!19 = !{!"S", %struct.Arr.2 zeroinitializer, i32 6, !3, !10, !12, !20, !10, !12}
!20 = !{float 0.000000e+00, i32 2}
!21 = !{!"S", %struct.Arr.base.3 zeroinitializer, i32 5, !3, !10, !12, !20, !10}
!22 = !{!"S", %struct.Unexpected zeroinitializer, i32 1, !23}
!23 = !{i32 0, i32 1}
!24 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!25 = distinct !{!26}
!26 = !{i8 0, i32 1}
!27 = distinct !{!28}
!28 = !{%class.F zeroinitializer, i32 1}
!29 = !{!30, !34, i64 8}
!30 = !{!"struct@_ZTS1F", !31, i64 0, !34, i64 8, !35, i64 16, !36, i64 24}
!31 = !{!"pointer@_ZTSP3Mem", !32, i64 0}
!32 = !{!"omnipotent char", !33, i64 0}
!33 = !{!"Simple C++ TBAA"}
!34 = !{!"pointer@_ZTSP3ArrIPiE", !32, i64 0}
!35 = !{!"pointer@_ZTSP3ArrIPvE", !32, i64 0}
!36 = !{!"pointer@_ZTSP4Arr1IPfE", !32, i64 0}
!37 = !{!30, !35, i64 16}
!38 = !{!30, !36, i64 24}
!39 = distinct !{!26}
!40 = distinct !{!4, !3}
!41 = !{!42, !31, i64 0}
!42 = !{!"struct@_ZTS3ArrIPiE", !31, i64 0, !43, i64 8, !44, i64 16, !43, i64 24}
!43 = !{!"int", !32, i64 0}
!44 = !{!"pointer@_ZTSPPi", !32, i64 0}
!45 = !{!42, !43, i64 8}
!46 = !{!42, !44, i64 16}
!47 = !{!42, !43, i64 24}
!48 = distinct !{!5, !3}
!49 = !{!50, !31, i64 0}
!50 = !{!"struct@_ZTS3ArrIPvE", !31, i64 0, !43, i64 8, !51, i64 16, !43, i64 24}
!51 = !{!"pointer@_ZTSPPv", !32, i64 0}
!52 = !{!50, !43, i64 8}
!53 = !{!50, !51, i64 16}
!54 = !{!50, !43, i64 24}
!55 = distinct !{!6}
!56 = distinct !{!4, !23, !57}
!57 = !{%struct.Unexpected zeroinitializer, i32 1}
!58 = !{!59, !59, i64 0}
!59 = !{!"pointer@_ZTSPi", !32, i64 0}
!60 = distinct !{!26, !5}
!61 = !{!62, !62, i64 0}
!62 = !{!"pointer@_ZTSPv", !32, i64 0}
!63 = distinct !{!64, !3}
!64 = !{%struct.Arr.2 zeroinitializer, i32 1}
!65 = !{!66, !31, i64 0}
!66 = !{!"struct@_ZTS3ArrIPfE", !31, i64 0, !43, i64 8, !67, i64 16, !43, i64 24}
!67 = !{!"pointer@_ZTSPPf", !32, i64 0}
!68 = !{!66, !43, i64 8}
!69 = !{!66, !67, i64 16}
!70 = !{!66, !43, i64 24}
