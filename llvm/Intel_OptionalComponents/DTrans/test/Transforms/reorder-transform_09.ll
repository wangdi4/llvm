; UNSUPPORTED: enable-opaque-pointers

; RUN: opt -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
; RUN: opt -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

; Field reordering should not be performed for new/delete.
; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
; CHECK:  %call = call i8* @_Znwm(i64 48)
; CHECK: call void @llvm.memset.p0i8.i64(i8* align 16 %{{.*}}, i8 0, i64 48, i1 false)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

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
  %call = call i8* @_Znwm(i64 48) #6
  %tmp = bitcast i8* %call to %struct.test*
  %tmp1 = bitcast %struct.test* %tmp to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %tmp1, i8 0, i64 48, i1 false)
  %call2 = call i32 @_Z11doSomethingP4testi(%struct.test* %tmp, i32 %argc)

  %isnull = icmp eq %struct.test* %tmp, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:
  %tmp2 = bitcast %struct.test* %tmp to i8*
  call void @_ZdlPv(i8* %tmp2) #7
  br label %delete.end

delete.end:
  br label %try.cont

try.cont:
  ret i32 %call2
}

declare noalias i8* @_Znwm(i64)

declare i32 @__gxx_personality_v0(...)

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #2

declare void @_ZdlPv(i8*) #3

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #4

declare i8* @__cxa_begin_catch(i8*)

declare void @__cxa_end_catch()

attributes #0 = { noinline uwtable }
attributes #1 = { noinline norecurse uwtable }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }
attributes #4 = { nounwind readnone }
attributes #5 = { noreturn }
attributes #6 = { builtin }
attributes #7 = { builtin nounwind }
