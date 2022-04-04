; RUN: opt -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
; RUN: opt -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

; Verify that fields are not reordered when any invoke instruction is involved
;   (_Z11doSomethingP4testi).
; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

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
  %exception = call i8* @__cxa_allocate_exception(i64 1) #2
  %tmp = bitcast i8* %exception to %struct.exc*
  call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8* }* @_ZTI3exc to i8*), i8* null) #4
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

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
  %i12 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp1 = load i32, i32* %i12, align 8
  %add3 = add nsw i32 %tmp1, 30
  %i3 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  store i32 %add3, i32* %i3, align 8
  call void @_Z8tryThrowi(i32 %cond)
  %i14 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp2 = load i32, i32* %i14, align 8
  %add5 = add nsw i32 %tmp2, 40
  %i4 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 3
  store i32 %add5, i32* %i4, align 4
  %i16 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp3 = load i32, i32* %i16, align 8
  %add7 = add nsw i32 %tmp3, 50
  %conv8 = trunc i32 %add7 to i16
  %i5 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 4
  store i16 %conv8, i16* %i5, align 8
  %i19 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp4 = load i32, i32* %i19, align 8
  %add10 = add nsw i32 %tmp4, 60
  %conv11 = sext i32 %add10 to i64
  %i6 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 5
  store i64 %conv11, i64* %i6, align 8
  %i112 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 0
  %tmp5 = load i32, i32* %i112, align 8
  %add13 = add nsw i32 %tmp5, 70
  %conv14 = sext i32 %add13 to i64
  %i7 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 6
  store i64 %conv14, i64* %i7, align 8
  %i315 = getelementptr inbounds %struct.test, %struct.test* %p, i32 0, i32 2
  %tmp6 = load i32, i32* %i315, align 8
  ret i32 %tmp6
}

define i32 @main(i32 %argc, i8** %argv) #1 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %call = call noalias i8* @calloc(i64 1, i64 48) #2
  %tmp = bitcast i8* %call to %struct.test*
  %call1 = invoke i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %tmp1 = bitcast %struct.test* %tmp to i8*
  call void @free(i8* %tmp1) #2
  br label %try.cont

lpad:                                             ; preds = %entry
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
  %tmp8 = bitcast %struct.test* %tmp to i8*
  call void @free(i8* %tmp8) #2
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %invoke.cont
  %retval.0 = phi i32 [ %call1, %invoke.cont ], [ 0, %catch ]
  ret i32 %retval.0

eh.resume:                                        ; preds = %catch.dispatch
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %tmp3, 0
  %lpad.val4 = insertvalue { i8*, i32 } %lpad.val, i32 %tmp4, 1
  resume { i8*, i32 } %lpad.val4
}

declare noalias i8* @calloc(i64, i64) #2

declare i32 @__gxx_personality_v0(...)

declare void @free(i8*) #2

declare i32 @llvm.eh.typeid.for(i8*) #3

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

attributes #0 = { noinline uwtable }
attributes #1 = { noinline norecurse uwtable }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone }
attributes #4 = { noreturn }
