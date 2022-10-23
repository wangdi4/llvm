; RUN: opt -opaque-pointers < %s -disable-output -passes='cgscc(inline),simplifycfg' -inline-report=0xe807 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline),simplifycfg' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

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

%class.F = type { ptr, ptr, ptr, ptr }
%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Arr.0 = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>

@_ZN3ArrIPiEC1EiP3Mem = dso_local unnamed_addr alias void (ptr, i32, ptr), ptr @_ZN3ArrIPiEC2EiP3Mem

; Function Attrs: nofree
declare noalias ptr @_Znwm(i64) local_unnamed_addr #0

; Function Attrs: noinline
define void @_ZN1FC2Ev(ptr nocapture %this) local_unnamed_addr #1 align 2 personality ptr @__gxx_personality_v0 {
entry:
  %call = call dereferenceable_or_null(32) ptr @_Znwm(i64 32)
  invoke void @_ZN3ArrIPiEC2EiP3Mem(ptr %call, i32 10, ptr null)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %f1 = getelementptr inbounds %class.F, ptr %this, i64 0, i32 1
  store ptr %call, ptr %f1, align 8
  %call2 = call dereferenceable_or_null(32) ptr @_Znwm(i64 32)
  invoke void @_ZN3ArrIPvEC2EiP3Mem(ptr %call2, i32 10, ptr null)
          to label %invoke.cont4 unwind label %lpad3

invoke.cont4:                                     ; preds = %invoke.cont
  ret void

lpad:                                             ; preds = %entry
  %i8 = landingpad { ptr, i32 }
          cleanup
  call void @_ZdlPv(ptr %call)
  br label %eh.resume

lpad3:                                            ; preds = %invoke.cont
  %i9 = landingpad { ptr, i32 }
          cleanup
  call void @_ZdlPv(ptr %call2)
  br label %eh.resume

eh.resume:                                        ; preds = %lpad3, %lpad
  %.pn = phi { ptr, i32 } [ %i9, %lpad3 ], [ %i8, %lpad ]
  resume { ptr, i32 } %.pn
}

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(ptr) local_unnamed_addr

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define internal void @_ZN3ArrIPiEC2EiP3Mem(ptr nocapture %this, i32 %c, ptr %mem) #2 align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i64 0, i32 4
  store i32 0, ptr %size, align 8
  ret void
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly
define internal void @_ZN3ArrIPvEC2EiP3Mem(ptr nocapture %this, i32 %c, ptr %mem) local_unnamed_addr #2 align 2 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 1
  store i32 %c, ptr %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 3
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr.0, ptr %this, i64 0, i32 4
  store i32 0, ptr %size, align 8
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { nofree }
attributes #1 = { noinline }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
