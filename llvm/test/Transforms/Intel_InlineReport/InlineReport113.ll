; RUN: opt < %s -disable-output -passes='cgscc(inline),simplifycfg' -inline-report=0xe807 2>&1 | FileCheck %s 
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline),simplifycfg' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that when an invoke is converted to a call (as is the case for
; _ZN3ArrIPiEC2EiP3Mem and _ZN3ArrIPvEC2EiP3Mem below, the call appears
; in the inline report and is not marked as DELETE.

; CHECK: COMPILE FUNC: _ZN1FC2Ev
; CHECK: EXTERN: _Znwm
; CHECK:_ZN3ArrIPiEC2EiP3Mem {{.*}}Callee has noinline attribute
; CHECK-NOT: DELETE: _ZN3ArrIPiEC2EiP3Mem
; CHECK: EXTERN: _Znwm
; CHECK: _ZN3ArrIPvEC2EiP3Mem {{.*}}Callee has noinline attribute
; CHECK-NOT: DELETE: _ZN3ArrIPvEC2EiP3Mem
; CHECK: DELETE: _ZdlPv
; CHECK: DELETE: _ZdlPv

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
%class.F = type { %struct.Mem*, %struct.Arr*, %struct.Arr.0*, %struct.Arr1* }
%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Arr1 = type { %struct.Arr.base.3, [4 x i8] }
%struct.Arr.base.3 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32 }>
%struct.Arr.2 = type <{ %struct.Mem*, i32, [4 x i8], float**, i32, [4 x i8] }>

@_ZN3ArrIPiEC1EiP3Mem = dso_local unnamed_addr alias void (%struct.Arr*, i32, %struct.Mem*), void (%struct.Arr*, i32, %struct.Mem*)* @_ZN3ArrIPiEC2EiP3Mem

; Function Attrs: nofree
declare noalias i8* @_Znwm(i64) local_unnamed_addr #0

; Function Attrs: noinline
define void @_ZN1FC2Ev(%class.F* nocapture %this) local_unnamed_addr #1 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call dereferenceable_or_null(32) i8* @_Znwm(i64 32)
  %i = bitcast i8* %call to %struct.Arr*
  invoke void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %i, i32 10, %struct.Mem* null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, %class.F* %this, i64 0, i32 1
  %i1 = bitcast %struct.Arr** %f1 to i8**
  store i8* %call, i8** %i1, align 8
  %call2 = call dereferenceable_or_null(32) i8* @_Znwm(i64 32)
  %i2 = bitcast i8* %call2 to %struct.Arr.0*
  invoke void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* %i2, i32 10, %struct.Mem* null)
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  ret void

lpad:                                             ; preds = %entry
  %i8 = landingpad { i8*, i32 }
          cleanup
  call void @_ZdlPv(i8* %call)
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %i9 = landingpad { i8*, i32 }
          cleanup
  call void @_ZdlPv(i8* %call2)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad6, %lpad3, %lpad
  %.pn = phi { i8*, i32 } [ %i9, %lpad3 ], [ %i8, %lpad ]
  resume { i8*, i32 } %.pn
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*) local_unnamed_addr

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define internal void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* nocapture %this, i32 %c, %struct.Mem* %mem) #2 align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 3
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i64 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define internal void @_ZN3ArrIPvEC2EiP3Mem(%struct.Arr.0* nocapture %this, i32 %c, %struct.Mem* %mem) local_unnamed_addr #2 align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 3
  store i8** null, i8*** %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i64 0, i32 4
  store i32 0, i32* %size, align 8
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { nofree }
attributes #1 = { noinline }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
