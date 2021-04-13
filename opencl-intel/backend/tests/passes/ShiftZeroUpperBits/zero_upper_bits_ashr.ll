; RUN: %oclopt -shift-ignore-upper-bits -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -shift-ignore-upper-bits -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<4 x i8> %x8bits, <4 x i16> %x16bits, <4 x i32> %x32bits, <4 x i64> %x64bits, <4 x i8> %y8bits, <4 x i16> %y16bits, <4 x i32> %y32bits, <4 x i64> %y64bits) {
entry:
    %temp8 = ashr <4 x i8> %x8bits, %y8bits
    %temp16 = ashr <4 x i16> %x16bits, %y16bits
    %temp32 = ashr <4 x i32> %x32bits, %y32bits
    %temp64 = ashr <4 x i64> %x64bits, %y64bits
    
    %result8 = ashr <4 x i8> %x8bits, <i8 0, i8 8, i8 33, i8 32>
    %result16 = ashr <4 x i16> %x16bits, <i16 0, i16 17, i16 116, i16 8>
    %result32 = ashr <4 x i32> %x32bits, <i32 4, i32 31, i32 32, i32 33>
    %result64 = ashr <4 x i64> %x64bits, <i64 4, i64 65, i64 32, i64 33>

    %res = ashr i32 526, 33
    ret void
}

; CHECK:        entry:
; CHECK-NEXT:   [[VARIABLE0:%[a-zA-Z0-9_]+]] = and <4 x i8> %y8bits, <i8 7, i8 7, i8 7, i8 7>
; CHECK-NEXT:   %temp8 = ashr <4 x i8> %x8bits, [[VARIABLE0]]
; CHECK-NEXT:   [[VARIABLE1:%[a-zA-Z0-9_]+]] = and <4 x i16> %y16bits, <i16 15, i16 15, i16 15, i16 15>
; CHECK-NEXT:   %temp16 = ashr <4 x i16> %x16bits, [[VARIABLE1]]
; CHECK-NEXT:   [[VARIABLE2:%[a-zA-Z0-9_]+]] = and <4 x i32> %y32bits, <i32 31, i32 31, i32 31, i32 31>
; CHECK-NEXT:   %temp32 = ashr <4 x i32> %x32bits, [[VARIABLE2]]
; CHECK-NEXT:   [[VARIABLE3:%[a-zA-Z0-9_]+]] = and <4 x i64> %y64bits, <i64 63, i64 63, i64 63, i64 63>
; CHECK-NEXT:   %temp64 = ashr <4 x i64> %x64bits, [[VARIABLE3]]

; CHECK-NEXT:   [[VARIABLE4:%[a-zA-Z0-9_]+]] = and <4 x i8> <i8 0, i8 8, i8 33, i8 32>, <i8 7, i8 7, i8 7, i8 7>
; CHECK-NEXT:   %result8 = ashr <4 x i8> %x8bits, [[VARIABLE4]]
; CHECK-NEXT:   [[VARIABLE5:%[a-zA-Z0-9_]+]] = and <4 x i16> <i16 0, i16 17, i16 116, i16 8>, <i16 15, i16 15, i16 15, i16 15>
; CHECK-NEXT:   %result16 = ashr <4 x i16> %x16bits, [[VARIABLE5]]
; CHECK-NEXT:   [[VARIABLE6:%[a-zA-Z0-9_]+]] = and <4 x i32> <i32 4, i32 31, i32 32, i32 33>, <i32 31, i32 31, i32 31, i32 31>
; CHECK-NEXT:   %result32 = ashr <4 x i32> %x32bits, [[VARIABLE6]]
; CHECK-NEXT:   [[VARIABLE7:%[a-zA-Z0-9_]+]] = and <4 x i64> <i64 4, i64 65, i64 32, i64 33>, <i64 63, i64 63, i64 63, i64 63>
; CHECK-NEXT:   %result64 = ashr <4 x i64> %x64bits, [[VARIABLE7]]

;               this line should not change
; CHECK-NEXT:   %res = ashr i32 526, 33
; CHECK-NEXT:   ret void


; DEBUGIFY-NOT: WARNING
