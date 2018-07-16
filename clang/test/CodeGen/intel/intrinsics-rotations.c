// RUN: %clang_cc1 -emit-llvm %s -o - -fintel-compatibility \
// RUN:            -triple i686-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm %s -o - -fintel-compatibility \
// RUN:            -triple x86_64-linux-gnu | FileCheck %s

// ==-- Rotate Left Intrinsics -----------------------------------------------==
//
unsigned char test_rotl8(unsigned char value, unsigned char shift) {
  return _rotl8(value, shift);
}
// CHECK: i8 @test_rotl8
// CHECK:   [[VALUE:%[0-9]+]] = load i8, i8* %value.addr, align 1
// CHECK:   [[SHIFT:%[0-9]+]] = load i8, i8* %shift.addr, align 1
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i8 [[SHIFT]], 7
// CHECK:   [[HIGH:%[0-9]+]] = shl i8 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i8 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i8 [[NEGSHIFT]], 7
// CHECK:   [[LOW:%[0-9]+]] = lshr i8 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i8 [[HIGH]], [[LOW]]
// CHECK:   ret i8 [[RESULT]]
// CHECK: }

unsigned short test_rotl16(unsigned short value, unsigned char shift) {
  return _rotl16(value, shift);
}
// CHECK: i16 @test_rotl16
// CHECK:   [[VALUE:%[0-9]+]] = load i16, i16* %value.addr, align 2
// CHECK:   [[SHIFT:%[0-9]+]] = load i8, i8* %shift.addr, align 1
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i16 [[SHIFT:%[0-9]+]], 15
// CHECK:   [[HIGH:%[0-9]+]] = shl i16 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i16 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i16 [[NEGSHIFT]], 15
// CHECK:   [[LOW:%[0-9]+]] = lshr i16 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i16 [[HIGH]], [[LOW]]
// CHECK:   ret i16 [[RESULT]]
// CHECK: }

unsigned int test_rotl(unsigned int value, int shift) {
  return _rotl(value, shift);
}
// CHECK: i32 @test_rotl
// CHECK:   [[VALUE:%[0-9]+]] = load i32, i32* %value.addr, align 4
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i32 [[SHIFT]], 31
// CHECK:   [[HIGH:%[0-9]+]] = shl i32 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i32 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i32 [[NEGSHIFT]], 31
// CHECK:   [[LOW:%[0-9]+]] = lshr i32 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i32 [[HIGH]], [[LOW]]
// CHECK:   ret i32 [[RESULT]]
// CHECK: }

unsigned long test_lrotl(unsigned long value, int shift) {
  return _lrotl(value, shift);
}
// CHECK: i{{32|64}} @test_lrotl
// CHECK:   [[VALUE:%[0-9]+]] = load i{{32|64}}, i{{32|64}}* %value.addr, align {{4|8}}
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i32 [[SHIFT]], 31
// CHECK:   [[HIGH:%[0-9]+]] = shl i32 %{{[0-9a-z]+}}, [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i32 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i32 [[NEGSHIFT]], 31
// CHECK:   [[LOW:%[0-9]+]] = lshr i32 %{{[0-9a-z]+}}, [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i32 [[HIGH]], [[LOW]]
// CHECK:   ret i{{32|64}} %{{[0-9a-z]+}}
// CHECK: }

unsigned __int64 test_rotl64(unsigned __int64 value, int shift) {
  return _rotl64(value, shift);
}
// CHECK: i64 @test_rotl64
// CHECK:   [[VALUE:%[0-9]+]] = load i64, i64* %value.addr, align 8
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFT64:%[0-9]+]] = zext i32 [[SHIFT]] to i64
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i64 [[SHIFT64]], 63
// CHECK:   [[HIGH:%[0-9]+]] = shl i64 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i64 0, [[SHIFT64]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i64 [[NEGSHIFT]], 63
// CHECK:   [[LOW:%[0-9]+]] = lshr i64 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i64 [[HIGH]], [[LOW]]
// CHECK:   ret i64 [[RESULT]]
// CHECK: }

// ==-- Rotate Right Intrinsics ----------------------------------------------==

unsigned char test_rotr8(unsigned char value, unsigned char shift) {
  return _rotr8(value, shift);
}
// CHECK: i8 @test_rotr8
// CHECK:   [[VALUE:%[0-9]+]] = load i8, i8* %value.addr, align 1
// CHECK:   [[SHIFT:%[0-9]+]] = load i8, i8* %shift.addr, align 1
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i8 [[SHIFT]], 7
// CHECK:   [[HIGH:%[0-9]+]] = lshr i8 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i8 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i8 [[NEGSHIFT]], 7
// CHECK:   [[LOW:%[0-9]+]] = shl i8 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i8 [[LOW]], [[HIGH]]
// CHECK:   ret i8 [[RESULT]]
// CHECK: }

unsigned short test_rotr16(unsigned short value, unsigned char shift) {
  return _rotr16(value, shift);
}
// CHECK: i16 @test_rotr16
// CHECK:   [[VALUE:%[0-9]+]] = load i16, i16* %value.addr, align 2
// CHECK:   [[SHIFT:%[0-9]+]] = load i8, i8* %shift.addr, align 1
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i16 [[SHIFT:%[0-9]+]], 15
// CHECK:   [[HIGH:%[0-9]+]] = lshr i16 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i16 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i16 [[NEGSHIFT]], 15
// CHECK:   [[LOW:%[0-9]+]] = shl i16 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i16 [[LOW]], [[HIGH]]
// CHECK:   ret i16 [[RESULT]]
// CHECK: }

unsigned int test_rotr(unsigned int value, int shift) {
  return _rotr(value, shift);
}
// CHECK: i32 @test_rotr
// CHECK:   [[VALUE:%[0-9]+]] = load i32, i32* %value.addr, align 4
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i32 [[SHIFT]], 31
// CHECK:   [[HIGH:%[0-9]+]] = lshr i32 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i32 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i32 [[NEGSHIFT]], 31
// CHECK:   [[LOW:%[0-9]+]] = shl i32 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i32 [[LOW]], [[HIGH]]
// CHECK:   ret i32 [[RESULT]]
// CHECK: }

unsigned long test_lrotr(unsigned long value, int shift) {
  return _lrotr(value, shift);
}
// CHECK: i{{32|64}} @test_lrotr
// CHECK:   [[VALUE:%[0-9]+]] = load i{{32|64}}, i{{32|64}}* %value.addr, align {{4|8}}
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i32 [[SHIFT]], 31
// CHECK:   [[HIGH:%[0-9]+]] = lshr i32 %{{[0-9a-z]+}}, [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i32 0, [[SHIFT]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i32 [[NEGSHIFT]], 31
// CHECK:   [[LOW:%[0-9]+]] = shl i32 %{{[0-9a-z]+}}, [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i32 [[LOW]], [[HIGH]]
// CHECK:   ret i{{32|64}} %{{[0-9a-z]+}}
// CHECK: }

unsigned __int64 test_rotr64(unsigned __int64 value, int shift) {
  return _rotr64(value, shift);
}
// CHECK: i64 @test_rotr64
// CHECK:   [[VALUE:%[0-9]+]] = load i64, i64* %value.addr, align 8
// CHECK:   [[SHIFT:%[0-9]+]] = load i32, i32* %shift.addr, align 4
// CHECK:   [[SHIFT64:%[0-9]+]] = zext i32 [[SHIFT]] to i64
// CHECK:   [[SHIFTHIGH:%[0-9]+]] = and i64 [[SHIFT64]], 63
// CHECK:   [[HIGH:%[0-9]+]] = lshr i64 [[VALUE]], [[SHIFTHIGH]]
// CHECK:   [[NEGSHIFT:%[0-9]+]] = sub i64 0, [[SHIFT64]]
// CHECK:   [[SHIFTLOW:%[0-9]+]] = and i64 [[NEGSHIFT]], 63
// CHECK:   [[LOW:%[0-9]+]] = shl i64 [[VALUE]], [[SHIFTLOW]]
// CHECK:   [[RESULT:%[0-9]+]] = or i64 [[LOW]], [[HIGH]]
// CHECK:   ret i64 [[RESULT]]
// CHECK: }
