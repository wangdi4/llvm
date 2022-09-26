; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-print-callinfo -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-print-callinfo -disable-output < %s 2>&1 | FileCheck %s

; Should not detect any calls to free. In particular, @test01 should not be
; recognized as a user free function.

%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 1)
  ret i32 0
}

define void @test01(i64 %idx1) {
  %base = load %struct.test01*, %struct.test01** @g_test01ptr
  %y_addr = getelementptr inbounds %struct.test01, %struct.test01* %base, i64 %idx1, i32 1
  br i1 undef, label %label_one, label %label_two
label_one:
  store i32 1, i32* %y_addr
  br label %label_end
label_two:
  store i32 2, i32* %y_addr
  br label %label_end

label_end:
  ret void
}

; CHECK-NOT: FreeCallInfo:
; CHECK-NOT: Kind: UserFree
