; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test 2>&1 | FileCheck %s

; Verify that AOS-to-SOA does not happen with new/delete.
; CHECK-NOT:  %__SOA_struct.test

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32 }

$_ZTS3exc = comdat any

$_ZTI3exc = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS3exc = constant [5 x i8] c"3exc\00", comdat
@_ZTI3exc = constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @_ZTS3exc, i32 0, i32 0) }, comdat

; Function Attrs: noinline uwtable
define void @_Z8tryThrowi(i32 %cond) #0 {
entry:
  %tobool = icmp ne i32 %cond, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = call i8* @__cxa_allocate_exception(i64 1) #2
  %tmp = bitcast i8* %exception to %struct.exc*
  call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*), i8* null) #4
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

; Function Attrs: noinline uwtable
define i32 @_Z11doSomethingP4testi(%struct.test* %p, i32 %cond) #0 {
entry:
  %i1 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  store i32 10, i32* %i1, align 8
  %i11 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp = load i32, i32* %i11, align 8
  %add = add nsw i32 %tmp, 20
  %conv = sext i32 %add to i64
  %i2 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 1
  store i64 %conv, i64* %i2, align 8
  call void @_Z8tryThrowi(i32 %cond)
  %i12 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp1 = load i32, i32* %i12, align 8
  %add3 = add nsw i32 %tmp1, 30
  %i3 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  store i32 %add3, i32* %i3, align 8
  %i34 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  %tmp2 = load i32, i32* %i34, align 8
  ret i32 %tmp2
}

; Function Attrs: noinline norecurse uwtable
define i32 @main(i32 %argc, i8** %argv) #1 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = invoke i8* @_Znam(i64 120) #5
          to label %invoke.cont unwind label %lpad
; Verify the allocation size is not changed
; CHECK:   %call = invoke i8* @_Znam(i64 120)
; CHECK-NEXT:  to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry

  %tmp = bitcast i8* %call to %struct.test*
  %call2 = invoke i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %isnull = icmp eq %struct.test* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %invoke.cont1
  %tmp1 = bitcast %struct.test* %tmp to i8*
  call void @_ZdaPv(i8* %tmp1) #6
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
  %tmp5 = call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*)) #2
  %matches = icmp eq i32 %tmp4, %tmp5
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %catch.dispatch
  %tmp6 = call i8* @__cxa_begin_catch(i8* %tmp3) #2
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

declare noalias i8* @_Znam(i64)

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind
declare void @_ZdaPv(i8*) #2

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #3

declare i8* @__cxa_begin_catch(i8*)

declare dso_local void @__cxa_end_catch()

attributes #0 = { noinline uwtable }
attributes #1 = { noinline norecurse uwtable }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { noreturn }
attributes #5 = { builtin }
attributes #6 = { builtin nounwind }
