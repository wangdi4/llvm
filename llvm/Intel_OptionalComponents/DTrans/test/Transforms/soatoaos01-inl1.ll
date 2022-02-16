; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -S -passes='lto-pre-link<O3>' -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck --check-prefixes=CHECK-SUPP,CHECK-SUPP-CL %s
; RUN: opt < %s -S -passes='lto<O3>' -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck --check-prefixes=CHECK-NINL,CHECK-NINL-CL %s
; RUN: opt < %s -S -passes='lto-pre-link<O3>' -inline-report=0xe886 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck --check-prefixes=CHECK-SUPP,CHECK-SUPP-MD %s
; RUN: opt < %s -S -passes='lto<O3>' -inline-report=0xe886 -dtrans-inline-heuristics -inline-for-xmain 2>&1 | FileCheck --check-prefixes=CHECK-NINL,CHECK-NINL-MD %s

; This test checks that the inlining of the key five functions listed in the
; CHECK lines is suppressed during the compile step, but not suppressed during
; the link step in an LTO compilation in the new pass manager. This is a test
; that PrepareForLTO is passed down to DTrans Inlining analysis for SOA-to-AOS.

; CHECK-SUPP-CL: invoke {{.*}} @_ZN1FC2Ev
; CHECK-SUPP-CL: call {{.*}} @_ZN3ArrIPiEC2EiP3Mem
; CHECK-SUPP-CL: call {{.*}} @_ZN3ArrIPvEC2EiP3Mem
; CHECK-SUPP-CL: call {{.*}} @_ZN3ArrIPiE3setEiS0_

; CHECK-SUPP-MD: _ZN1FC2Ev{{.*}}Callsite preferred for SOA-to-AOS
; CHECK-SUPP: _ZN3ArrIPiEC2EiP3Mem {{.*}}Callsite preferred for SOA-to-AOS
; CHECK-SUPP: _ZN3ArrIPvEC2EiP3Mem {{.*}}Callsite preferred for SOA-to-AOS
; CHECK-SUPP: _ZN3ArrIPiE3setEiS0_ {{.*}}Callsite preferred for SOA-to-AOS
; CHECK-SUPP-CL: _ZN1FC2Ev{{.*}}Callsite preferred for SOA-to-AOS

; CHECK-SUPP-MD: invoke {{.*}} @_ZN1FC2Ev
; CHECK-SUPP-MD: call {{.*}} @_ZN3ArrIPiEC2EiP3Mem
; CHECK-SUPP-MD: call {{.*}} @_ZN3ArrIPvEC2EiP3Mem
; CHECK-SUPP-MD: call {{.*}} @_ZN3ArrIPiE3setEiS0_

; CHECK-NINL-CL-NOT: invoke {{.*}} @_ZN1FC2Ev
; CHECK-NINL-CL-NOT: call {{.*}} @_ZN3ArrIPiEC2EiP3Mem
; CHECK-NINL-CL-NOT: call {{.*}} @_ZN3ArrIPvEC2EiP3Mem
; CHECK-NINL-CL-NOT: call {{.*}} @_ZN3ArrIPiE3setEiS0_

; CHECK-NINL-MD: INLINE{{.*}}_ZN1FC2Ev
; CHECK-NINL: INLINE{{.*}}_ZN3ArrIPiEC2EiP3Mem
; CHECK-NINL: INLINE{{.*}}_ZN3ArrIPvEC2EiP3Mem
; CHECK-NINL: INLINE{{.*}}_ZN3ArrIPiE3setEiS0_
; CHECK-NINL-CL: INLINE{{.*}}_ZN1FC2Ev

; CHECK-NINL-MD-NOT: invoke {{.*}} @_ZN1FC2Ev
; CHECK-NINL-MD-NOT: call {{.*}} @_ZN3ArrIPiEC2EiP3Mem
; CHECK-NINL-MD-NOT: call {{.*}} @_ZN3ArrIPvEC2EiP3Mem
; CHECK-NINL-MD-NOT: call {{.*}} @_ZN3ArrIPiE3setEiS0_

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Mem = type { i32 (...)** }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

; Use of _ZN3ArrIPiEC2EiP3Mem is not CallInst.
@_ZN3ArrIPiEC1EiP3Mem = dso_local unnamed_addr alias void (%struct.Arr*, i32, %struct.Mem*), void (%struct.Arr*, i32, %struct.Mem*)* @_ZN3ArrIPiEC2EiP3Mem

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

declare noalias i8* @_Znwm(i64)

define void @_ZN1FC2Ev(%class.F* nocapture %this) align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
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
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  %f2 = getelementptr inbounds %class.F, %class.F* %this, i32 0, i32 2
  store %struct.Arr.0* %1, %struct.Arr.0** %f2, align 8
  %call5 = call i8* @_Znwm(i64 32)
  %2 = bitcast i8* %call5 to %struct.Arr1*
  %3 = bitcast %struct.Arr1* %2 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %3, i8 0, i64 32, i1 false)
  invoke void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* %2)
          to label %invoke.cont7 unwind label %lpad6

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
  %7 = extractvalue { i8*, i32 } %6, 0
  %8 = extractvalue { i8*, i32 } %6, 1
  call void @_ZdlPv(i8* %call)
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %9 = landingpad { i8*, i32 }
          cleanup
  %10 = extractvalue { i8*, i32 } %9, 0
  %11 = extractvalue { i8*, i32 } %9, 1
  call void @_ZdlPv(i8* %call2)
  br label %eh.resume

lpad6:                                            ; preds = %invoke.cont4
  %12 = landingpad { i8*, i32 }
          cleanup
  %13 = extractvalue { i8*, i32 } %12, 0
  %14 = extractvalue { i8*, i32 } %12, 1
  call void @_ZdlPv(i8* %call5)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad6, %lpad3, %lpad
  %exn.slot.0 = phi i8* [ %13, %lpad6 ], [ %10, %lpad3 ], [ %7, %lpad ]
  %ehselector.slot.0 = phi i32 [ %14, %lpad6 ], [ %11, %lpad3 ], [ %8, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val11 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val11
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*)

define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
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

define void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
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

define void @_ZN4Arr1IPfEC2Ev(%struct.Arr1* nocapture %this) align 2 {
entry:
  %0 = bitcast %struct.Arr1* %this to %struct.Arr.2*
  call void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* %0, i32 1, %struct.Mem* null)
  ret void
}

define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %this, i32 %i, i32* %val) align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %0 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %0, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

define i8* @_ZN3ArrIPvE3getEi(%struct.Arr.0* nocapture readonly %this, i32 %i) align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %0 = load i8**, i8*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i8*, i8** %0, i64 %idxprom
  %1 = load i8*, i8** %arrayidx, align 8
  ret i8* %1
}

define void @_ZN3ArrIPfEC2EiP3Mem(%struct.Arr.2* nocapture %this, i32 %c, %struct.Mem* %mem) align 2 {
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
; end INTEL_FEATURE_SW_ADVANCED
