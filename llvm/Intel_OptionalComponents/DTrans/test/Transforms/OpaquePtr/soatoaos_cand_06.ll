; RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -passes=dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-soatoaosop -whole-program-assume -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; This test is same as soatoaos_cand_01.ll except %struct.Arr.0 is
; referenced in "main". This violates SubGraph's check.
; This test verifies that class.F is NOT considered as candidate by checking
; layout and basic CFG properties.
; This test checks Sub-CallGraph properties.

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
; int main() { F *f = new F(); f->f3 = null;}
;
; CHECK: Struct's class.F methods:
; CHECK: array type has unsupported CFG.
; CHECK: Rejecting %class.F because it does not look like a candidate from CFG analysis

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

define i32 @main() personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call i8* @_Znwm(i64 32)
  %0 = bitcast i8* %call to %class.F*
  %f3 = getelementptr inbounds %class.F, %class.F* %0, i32 0, i32 2
  store %struct.Arr.0* null, %struct.Arr.0** %f3, align 8
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

declare !intel.dtrans.func.type !23 dso_local nonnull "intel_dtrans_func_index"="1" i8* @_Znwm(i64)

define void @_ZN1FC2Ev(%class.F* nocapture "intel_dtrans_func_index"="1" %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !intel.dtrans.func.type !25 {
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

declare !intel.dtrans.func.type !37 void @_ZdlPv(i8*  "intel_dtrans_func_index"="1")

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture  "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !38 {
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

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2 !intel.dtrans.func.type !46 {
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

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture align 8 dereferenceable(32)  "intel_dtrans_func_index"="1" %this) align 2 !intel.dtrans.func.type !53 {
entry:
  %0 = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="1" %this, i32 %i, i32* "intel_dtrans_func_index"="2" %val) align 2 !intel.dtrans.func.type !54 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %0 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %0, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define "intel_dtrans_func_index"="1" i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly align 8 dereferenceable(32) "intel_dtrans_func_index"="2" %this, i32 %i) align 2  !intel.dtrans.func.type !58 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %0 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %0, i64 %idxprom
  %1 = load i8*, i8** %arrayidx, align 8
  ret i8* %1
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture  "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem) align 2  !intel.dtrans.func.type !61 {
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
!intel.dtrans.types = !{!2, !7, !11, !15, !17, !19, !21}
!llvm.ident = !{!22}

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
!22 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!23 = distinct !{!24}
!24 = !{i8 0, i32 1}
!25 = distinct !{!26}
!26 = !{%class.F zeroinitializer, i32 1}
!27 = !{!28, !32, i64 8}
!28 = !{!"struct@_ZTS1F", !29, i64 0, !32, i64 8, !33, i64 16, !34, i64 24}
!29 = !{!"pointer@_ZTSP3Mem", !30, i64 0}
!30 = !{!"omnipotent char", !31, i64 0}
!31 = !{!"Simple C++ TBAA"}
!32 = !{!"pointer@_ZTSP3ArrIPiE", !30, i64 0}
!33 = !{!"pointer@_ZTSP3ArrIPvE", !30, i64 0}
!34 = !{!"pointer@_ZTSP4Arr1IPfE", !30, i64 0}
!35 = !{!28, !33, i64 16}
!36 = !{!28, !34, i64 24}
!37 = distinct !{!24}
!38 = distinct !{!4, !3}
!39 = !{!40, !29, i64 0}
!40 = !{!"struct@_ZTS3ArrIPiE", !29, i64 0, !41, i64 8, !42, i64 16, !41, i64 24}
!41 = !{!"int", !30, i64 0}
!42 = !{!"pointer@_ZTSPPi", !30, i64 0}
!43 = !{!40, !41, i64 8}
!44 = !{!40, !42, i64 16}
!45 = !{!40, !41, i64 24}
!46 = distinct !{!5, !3}
!47 = !{!48, !29, i64 0}
!48 = !{!"struct@_ZTS3ArrIPvE", !29, i64 0, !41, i64 8, !49, i64 16, !41, i64 24}
!49 = !{!"pointer@_ZTSPPv", !30, i64 0}
!50 = !{!48, !41, i64 8}
!51 = !{!48, !49, i64 16}
!52 = !{!48, !41, i64 24}
!53 = distinct !{!6}
!54 = distinct !{!4, !55}
!55 = !{i32 0, i32 1}
!56 = !{!57, !57, i64 0}
!57 = !{!"pointer@_ZTSPi", !30, i64 0}
!58 = distinct !{!24, !5}
!59 = !{!60, !60, i64 0}
!60 = !{!"pointer@_ZTSPv", !30, i64 0}
!61 = distinct !{!62, !3}
!62 = !{%struct.Arr.2 zeroinitializer, i32 1}
!63 = !{!64, !29, i64 0}
!64 = !{!"struct@_ZTS3ArrIPfE", !29, i64 0, !41, i64 8, !65, i64 16, !41, i64 24}
!65 = !{!"pointer@_ZTSPPf", !30, i64 0}
!66 = !{!64, !41, i64 8}
!67 = !{!64, !65, i64 16}
!68 = !{!64, !41, i64 24}
