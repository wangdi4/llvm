; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; Verify that 2 fields are deleted with invoke instruction involved
;   not directly related to type, see checks.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32 }

$_ZTS3exc = comdat any

$_ZTI3exc = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS3exc = constant [5 x i8] c"3exc\00", comdat
@_ZTI3exc = constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @_ZTS3exc, i32 0, i32 0) }, comdat

define void @_Z8tryThrowi(i32 %cond) #0 {
entry:
  %tobool = icmp ne i32 %cond, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = call i8* @__cxa_allocate_exception(i64 1) #3
  %tmp = bitcast i8* %exception to %struct.exc*
  call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*), i8* null) #4
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

define i32 @_Z11doSomethingP4testi(%struct.test* %p, i32 %cond) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %i1 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  store i32 1, i32* %i1, align 8
  %i11 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp = load i32, i32* %i11, align 8
  invoke void @_Z8tryThrowi(i32 %cond)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %i3 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  store i32 2, i32* %i3, align 8
  br label %try.cont

lpad:                                             ; preds = %entry
  %tmp1 = landingpad { i8*, i32 }
          catch i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*)
  %tmp2 = extractvalue { i8*, i32 } %tmp1, 0
  %tmp3 = extractvalue { i8*, i32 } %tmp1, 1
  br label %catch.dispatch

catch.dispatch:                                   ; preds = %lpad
  %tmp4 = call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*)) #3
  %matches = icmp eq i32 %tmp3, %tmp4
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %catch.dispatch
  %tmp5 = call i8* @__cxa_begin_catch(i8* %tmp2) #3
  %tmp6 = bitcast i8* %tmp5 to %struct.exc*
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %invoke.cont
  %retval.0 = phi i32 [ %tmp, %invoke.cont ], [ 0, %catch ]
  ret i32 %retval.0

eh.resume:                                        ; preds = %catch.dispatch
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %tmp2, 0
  %lpad.val4 = insertvalue { i8*, i32 } %lpad.val, i32 %tmp3, 1
  resume { i8*, i32 } %lpad.val4
}

declare i32 @__gxx_personality_v0(...)

declare i32 @llvm.eh.typeid.for(i8*) #1

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

define i32 @main(i32 %argc, i8** %argv) #2 {
entry:
  %call = call noalias i8* @malloc(i64 24) #3
  %tmp = bitcast i8* %call to %struct.test*
  %call1 = call i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
  %tmp1 = bitcast %struct.test* %tmp to i8*
  call void @free(i8* %tmp1) #3
  ret i32 %call1
}

; CHECK: %__DFT_struct.test = type { i32 }

; 1st and 2nd fields are deleted, 0th is left.
; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %call = call noalias i8* @malloc(i64 4)
; CHECK: %tmp = bitcast i8* %call to %__DFT_struct.test*
; CHECK: %call1 = call i32 @_Z11doSomethingP4testi.1(%__DFT_struct.test* %tmp, i32 %argc)


; CHECK-LABEL: define internal i32 @_Z11doSomethingP4testi.1(%__DFT_struct.test* %p, i32 %cond)
; CHECK-SAME:  #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
; CHECK  %i1 = getelementptr inbounds %__DFT_struct.test, %__DFT_struct.test* %p, i32 0, i32 0
; CHECK  store i32 1, i32* %i1, align 8
; CHECK  %i11 = getelementptr inbounds %__DFT_struct.test, %__DFT_struct.test* %p, i32 0, i32 0

declare noalias i8* @malloc(i64) #3

declare void @free(i8*) #3

attributes #0 = { noinline uwtable }
attributes #1 = { nounwind readnone }
attributes #2 = { noinline norecurse uwtable }
attributes #3 = { nounwind }
attributes #4 = { noreturn }
