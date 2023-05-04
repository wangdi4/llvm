; RUN: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST1

; Check that the attribute "willreturn" is set for the libfunc since
; whole-program was achieved, and the libfunc is marked as "readonly"
; and "mustprogress"

; CHECK-TEST1: declare dso_local ptr @libfunc1(ptr, ptr, ptr, i64) #[[T1:[0-9]+]]
; CHECK-TEST1: attributes #[[T1]]
; CHECK-TEST1-SAME: willreturn

declare dso_local ptr @libfunc1(ptr, ptr, ptr, i64) #1

define ptr @test1(ptr %0, ptr %1, ptr %2, i64 %3) {
  %temp1 = call ptr @libfunc1(ptr %0, ptr %1, ptr %2, i64 %3)
  ret ptr %temp1
}

attributes #1 = { mustprogress nofree nounwind memory(read) }

; RUN: opt -passes='require<wholeprogram>',function-attrs -S %s | FileCheck %s --check-prefix=CHECK-TEST2

; Check that the attribute "willreturn" is NOT set for the libfunc since
; whole-program was NOT achieved.

; CHECK-TEST2: declare dso_local ptr @libfunc2(ptr, ptr, ptr, i64)
; CHECK-TEST2-NOT: willreturn

declare dso_local ptr @libfunc2(ptr, ptr, ptr, i64) #2

define ptr @test2(ptr %0, ptr %1, ptr %2, i64 %3) {
  %temp1 = call ptr @libfunc2(ptr %0, ptr %1, ptr %2, i64 %3)
  ret ptr %temp1
}

attributes #2 = { mustprogress nofree nounwind memory(read) }

; RUN: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST3

; Check that the attribute "willreturn" is NOT set for the libfunc since
; "mustprogress" is not set.

; CHECK-TEST3: declare dso_local ptr @libfunc3(ptr, ptr, ptr, i64) #[[T3:[0-9]+]]
; CHECK-TEST3: attributes #[[T3]] = { nofree nounwind memory(read) }

declare dso_local ptr @libfunc3(ptr, ptr, ptr, i64) #3

define ptr @test3(ptr %0, ptr %1, ptr %2, i64 %3) {
  %temp1 = call ptr @libfunc3(ptr %0, ptr %1, ptr %2, i64 %3)
  ret ptr %temp1
}

attributes #3 = { nofree nounwind memory(read) }

; RUN: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST4

; Check that the attribute "willreturn" is NOT set for the libfunc since
; "readonly" is not set.

; CHECK-TEST4: declare dso_local ptr @libfunc4(ptr, ptr, ptr, i64) #[[T4:[0-9]+]]
; CHECK-TEST4: attributes #[[T4]] = { mustprogress nofree nounwind }

declare dso_local ptr @libfunc4(ptr, ptr, ptr, i64) #4

define ptr @test4(ptr %0, ptr %1, ptr %2, i64 %3) {
  %temp1 = call ptr @libfunc4(ptr %0, ptr %1, ptr %2, i64 %3)
  ret ptr %temp1
}

attributes #4 = { mustprogress nofree nounwind }

; RUN: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST5

; Check that the attribute "willreturn" is NOT set for the libfunc since
; the libfunc is used in an invoke instruction.

; CHECK-TEST5: declare dso_local ptr @libfunc5(ptr, ptr, ptr, i64) #[[T5:[0-9]+]]
; CHECK-TEST5: attributes #[[T5]] = { mustprogress nofree memory(read) }

declare dso_local ptr @libfunc5(ptr, ptr, ptr, i64) #5
declare i32 @__gxx_personality_v0(...)

define ptr @test5(ptr %0, ptr %1, ptr %2, i64 %3) personality ptr bitcast (ptr @__gxx_personality_v0 to ptr) {

  %temp1 = invoke ptr @libfunc5(ptr %0, ptr %1, ptr %2, i64 %3)
             to label %Pass unwind label %LPad
Pass:
  ret ptr %temp1

LPad:
  %val = landingpad { ptr, i32 }
           catch ptr null
  ret ptr null
}

attributes #5 = { mustprogress nofree memory(read) }
