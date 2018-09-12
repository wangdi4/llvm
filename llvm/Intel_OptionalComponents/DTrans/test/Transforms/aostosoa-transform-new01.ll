; RUN: sed -e s/^.malloc:// %s | \
; RUN:  opt -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-malloc %s
; RUN: sed -e s/^.malloc-exc:// %s | \
; RUN:  opt -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-malloc-exc %s
; RUN: sed -e s/^.new64:// %s | \
; RUN:  opt -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-new64 %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:  opt -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-new64nt %s

; RUN: sed -e s/^.malloc:// %s | \
; RUN:  opt -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-malloc %s
; RUN: sed -e s/^.malloc-exc:// %s | \
; RUN:  opt -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-malloc-exc %s
; RUN: sed -e s/^.new64:// %s | \
; RUN:  opt -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-new64 %s
; RUN: sed -e s/^.new64nt:// %s | \
; RUN:  opt -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck --check-prefix=CHECK-new64nt %s

; No C++ handling
; CHECK-malloc:  %__SOA_struct.test
; C++ handling in _Z11doSomethingP4testi invoke
; CHECK-malloc-exc-NOT:  %__SOA_struct.test
; Verify that AOS-to-SOA does not happen with new/delete.
; C++ handling in new-operator invocation.
; CHECK-new64-NOT:  %__SOA_struct.test
; C++ handling in new-operator invocation.
; CHECK-new64nt-NOT:  %__SOA_struct.test

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32 }
;new64nt: %"struct.std::nothrow_t" = type { i8 }
;new64nt: @nt = global %"struct.std::nothrow_t" zeroinitializer

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS3exc = constant [5 x i8] c"3exc\00"
@_ZTI3exc = constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @_ZTS3exc, i32 0, i32 0) }

define void @_Z8tryThrowi(i32 %cond) {
entry:
  %tobool = icmp ne i32 %cond, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = call i8* @__cxa_allocate_exception(i64 1)
  %tmp = bitcast i8* %exception to %struct.exc*
  call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*), i8* null)
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

define i32 @_Z11doSomethingP4testi(%struct.test* %p, i32 %cond) {
entry:
  %i1 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  store i32 10, i32* %i1
  %i11 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp = load i32, i32* %i11
  %add = add nsw i32 %tmp, 20
  %conv = sext i32 %add to i64
  %i2 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 1
  store i64 %conv, i64* %i2
  call void @_Z8tryThrowi(i32 %cond)
  %i12 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp1 = load i32, i32* %i12
  %add3 = add nsw i32 %tmp1, 30
  %i3 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  store i32 %add3, i32* %i3
  %i34 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  %tmp2 = load i32, i32* %i34
  ret i32 %tmp2
}

define i32 @main(i32 %argc, i8** %argv) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
;malloc: %call = call i8* @malloc(i64 120)
;malloc: br label %invoke.cont
;malloc-exc: %call = call i8* @malloc(i64 120)
;malloc-exc: br label %invoke.cont
;new64:  %call = invoke i8* @_Znam(i64 120)
;new64:          to label %invoke.cont unwind label %lpad
;new64nt: %call = call i8* @_ZnwmRKSt9nothrow_t(i64 120, %"struct.std::nothrow_t"* @nt)
;new64nt: br label %invoke.cont
; Verify the allocation size is not changed
; CHECK-new64:   %call = invoke i8* @_Znam(i64 120)
; CHECK-new64-NEXT:  to label %invoke.cont unwind label %lpad
; _ZnwmRKSt9nothrow_t is treated as malloc by MemoryBuiltin.cpp, becasue it may return null pointer.
; CHECK-new64nt: %call = call i8* @_ZnwmRKSt9nothrow_t(i64 120, %"struct.std::nothrow_t"* @nt)

invoke.cont:                                      ; preds = %entry

  %tmp = bitcast i8* %call to %struct.test*
;malloc:   %call2 = call i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
;malloc:   br label %invoke.cont1
;malloc-exc: %call2 = invoke i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
;malloc-exc: to label %invoke.cont1 unwind label %lpad
;new64:    %call2 = call i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
;new64:    br label %invoke.cont1
;new64nt:  %call2 = call i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
;new64nt:  br label %invoke.cont1

invoke.cont1:                                     ; preds = %invoke.cont
  %isnull = icmp eq %struct.test* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont1
  %tmp1 = bitcast %struct.test* %tmp to i8*
  ; Do not call delete to check only new-operator.
  ; call void @_ZdaPv(i8* %tmp1)
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %invoke.cont1
  br label %try.cont

lpad:                                             ; preds = %invoke.cont, %entry
  %tmp2 = landingpad { i8*, i32 }
          catch i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*)
  %tmp3 = extractvalue { i8*, i32 } %tmp2, 0
  %tmp4 = extractvalue { i8*, i32 } %tmp2, 1
  br label %catch.dispatch

catch.dispatch:                                   ; preds = %lpad
  %tmp5 = call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*))
  %matches = icmp eq i32 %tmp4, %tmp5
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %catch.dispatch
  %tmp6 = call i8* @__cxa_begin_catch(i8* %tmp3)
  %tmp7 = bitcast i8* %tmp6 to %struct.exc*
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %delete.end
  %retval.0 = phi i32 [ %call2, %delete.end ], [ 0, %catch ]
  ret i32 %retval.0

eh.resume:                                        ; preds = %catch.dispatch
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %tmp3, 0
  %lpad.val5 = insertvalue { i8*, i32 } %lpad.val, i32 %tmp4, 1
  resume { i8*, i32 } %lpad.val5
}

;malloc: declare i8* @malloc(i64)
;malloc-exc: declare i8* @malloc(i64)
;new64: declare i8* @_Znam(i64)
;new64nt: declare i8* @_ZnwmRKSt9nothrow_t(i64, %"struct.std::nothrow_t"*)

declare i32 @__gxx_personality_v0(...)

; declare void @_ZdaPv(i8*)

declare i32 @llvm.eh.typeid.for(i8*)

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()
