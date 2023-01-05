; RUN: opt -enable-new-pm=0 -wholeprogramanalysis -function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST1
; TODO: Include the run command for the new pass manager once we fix the
; whole program analysis in the new pass manager.
; RUN-TODO: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST1

; Check that the attribute "willreturn" is set for the libfunc since
; whole-program was achieved, and the libfunc is marked as "readonly"
; and "mustprogress"

; CHECK-TEST1: declare dso_local i8* @libfunc1(i8*, i8*, i8*, i64) #[[T1:[0-9]+]]
; CHECK-TEST1: attributes #[[T1]]
; CHECK-TEST1-SAME: willreturn

declare dso_local i8* @libfunc1(i8*, i8*, i8*, i64) #1

define i8* @test1(i8* %0, i8* %1, i8* %2, i64 %3) {
  %temp1 = call i8* @libfunc1(i8* %0, i8* %1, i8* %2, i64 %3)
  ret i8* %temp1
}

attributes #1 = { mustprogress nofree nounwind memory(read) }

; RUN: opt -enable-new-pm=0 -wholeprogramanalysis -function-attrs -S %s | FileCheck %s --check-prefix=CHECK-TEST2
; TODO: Include the run command for the new pass manager once we fix the
; whole program analysis in the new pass manager.
; RUN-TODO: opt -passes='require<wholeprogram>',function-attrs -S %s | FileCheck %s --check-prefix=CHECK-TEST2

; Check that the attribute "willreturn" is NOT set for the libfunc since
; whole-program was NOT achieved.

; CHECK-TEST2: declare dso_local i8* @libfunc2(i8*, i8*, i8*, i64)
; CHECK-TEST2-NOT: willreturn

declare dso_local i8* @libfunc2(i8*, i8*, i8*, i64) #2

define i8* @test2(i8* %0, i8* %1, i8* %2, i64 %3) {
  %temp1 = call i8* @libfunc2(i8* %0, i8* %1, i8* %2, i64 %3)
  ret i8* %temp1
}

attributes #2 = { mustprogress nofree nounwind memory(read) }

; RUN: opt -enable-new-pm=0 -wholeprogramanalysis -function-attrs -whole-program-assume  -S %s | FileCheck %s --check-prefix=CHECK-TEST3
; TODO: Include the run command for the new pass manager once we fix the
; whole program analysis in the new pass manager.
; RUN-TODO: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST3

; Check that the attribute "willreturn" is NOT set for the libfunc since
; "mustprogress" is not set.

; CHECK-TEST3: declare dso_local i8* @libfunc3(i8*, i8*, i8*, i64) #[[T3:[0-9]+]]
; CHECK-TEST3: attributes #[[T3]] = { nofree nounwind memory(read) }

declare dso_local i8* @libfunc3(i8*, i8*, i8*, i64) #3

define i8* @test3(i8* %0, i8* %1, i8* %2, i64 %3) {
  %temp1 = call i8* @libfunc3(i8* %0, i8* %1, i8* %2, i64 %3)
  ret i8* %temp1
}

attributes #3 = { nofree nounwind memory(read) }

; RUN: opt -enable-new-pm=0 -wholeprogramanalysis -function-attrs -whole-program-assume  -S %s | FileCheck %s --check-prefix=CHECK-TEST4
; TODO: Include the run command for the new pass manager once we fix the
; whole program analysis in the new pass manager.
; RUN-TODO: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST4

; Check that the attribute "willreturn" is NOT set for the libfunc since
; "readonly" is not set.

; CHECK-TEST4: declare dso_local i8* @libfunc4(i8*, i8*, i8*, i64) #[[T4:[0-9]+]]
; CHECK-TEST4: attributes #[[T4]] = { mustprogress nofree nounwind }

declare dso_local i8* @libfunc4(i8*, i8*, i8*, i64) #4

define i8* @test4(i8* %0, i8* %1, i8* %2, i64 %3) {
  %temp1 = call i8* @libfunc4(i8* %0, i8* %1, i8* %2, i64 %3)
  ret i8* %temp1
}

attributes #4 = { mustprogress nofree nounwind }

; RUN: opt -enable-new-pm=0 -wholeprogramanalysis -function-attrs -whole-program-assume  -S %s | FileCheck %s --check-prefix=CHECK-TEST5
; TODO: Include the run command for the new pass manager once we fix the
; whole program analysis in the new pass manager.
; RUN-TODO: opt -passes='require<wholeprogram>',function-attrs -whole-program-assume -S %s | FileCheck %s --check-prefix=CHECK-TEST5

; Check that the attribute "willreturn" is NOT set for the libfunc since
; the libfunc is used in an invoke instruction.

; CHECK-TEST5: declare dso_local i8* @libfunc5(i8*, i8*, i8*, i64) #[[T5:[0-9]+]]
; CHECK-TEST5: attributes #[[T5]] = { mustprogress nofree memory(read) }

declare dso_local i8* @libfunc5(i8*, i8*, i8*, i64) #5
declare i32 @__gxx_personality_v0(...)

define i8* @test5(i8* %0, i8* %1, i8* %2, i64 %3) personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {

  %temp1 = invoke i8* @libfunc5(i8* %0, i8* %1, i8* %2, i64 %3)
             to label %Pass unwind label %LPad
Pass:
  ret i8* %temp1

LPad:
  %val = landingpad { i8*, i32 }
           catch i8* null
  ret i8* null
}

attributes #5 = { mustprogress nofree memory(read) }
