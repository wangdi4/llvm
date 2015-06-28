// CQ#371662
// RUN: %clang_cc1 -triple x86_64-unknown-linux -fintel-compatibility -DBITMANIP -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-BITMANIP-LINUX %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux -fintel-compatibility -DNO_BITMANIP -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-NO-BITMANIP-LINUX %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -DBITMANIP -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-BITMANIP-WIN %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -DNO_BITMANIP -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-NO-BITMANIP-WIN %s

extern unsigned char get_char();
extern unsigned short get_short();
extern unsigned int get_int();
extern unsigned long long get_long_long();
extern unsigned __int128 get_int128();

// CHECK-BITMANIP-LINUX-DAG: %struct.bf1 = type { i32**, i40 }
// CHECK-BITMANIP-LINUX-DAG: %struct.bf2 = type { i32, i8*, i32 }
// CHECK-BITMANIP-LINUX-DAG: %struct.bf3 = type { i8, i8 }
// CHECK-BITMANIP-LINUX-DAG: %struct.bf4 = type <{ i8, i16 }>
// CHECK-NO-BITMANIP-LINUX-DAG: %struct.bf5 = type { i64, i24 }
// CHECK-NO-BITMANIP-LINUX-DAG: %struct.bf6 = type { float, i32, i16, i16* }
// CHECK-NO-BITMANIP-LINUX-DAG: %struct.bf7 = type { i8*, i48 }
// CHECK-NO-BITMANIP-LINUX-DAG: %struct.bf8 = type { i64, i16, [6 x i8], [12 x i8], i8 }

// CHECK-BITMANIP-WIN-DAG: %struct.bf1 = type { i32**, i32, i64 }
// CHECK-BITMANIP-WIN-DAG: %struct.bf2 = type { i32, i8*, i32 }
// CHECK-BITMANIP-WIN-DAG: %struct.bf3 = type { i8, i16 }
// CHECK-BITMANIP-WIN-DAG: %struct.bf4 = type { i8, i8, i8 }
// CHECK-NO-BITMANIP-WIN-DAG: %struct.bf5 = type { i64, i32, i64 }
// CHECK-NO-BITMANIP-WIN-DAG: %struct.bf6 = type { float, i32, i32, i16* }
// CHECK-NO-BITMANIP-WIN-DAG: %struct.bf7 = type { i8*, i32, i32 }
// CHECK-NO-BITMANIP-WIN-DAG: %struct.bf8 = type { i64, i32, i128, i32, i8, [11 x i8] }

#ifdef BITMANIP

void check_bf1(void) {
  // offset = 3, size = 32
  struct bf1 {
    int **padding1;
    int offset_field : 3;
    long long value_field : 32;
  } s1;
  // CHECK-BITMANIP-LINUX: getelementptr inbounds %struct.bf1, %struct.bf1* %{{.+}}, i32 0, i32 1
  // CHECK-BITMANIP-LINUX-NEXT: bitcast i40* %{{.+}} to i64*
  // CHECK-BITMANIP-LINUX-NEXT: load i64, i64* %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: and i64 %{{.+}}, 4294967295
  // CHECK-BITMANIP-LINUX-NEXT: shl i64 %{{.+}}, 3
  // CHECK-BITMANIP-LINUX-NEXT: and i64 %{{.+}}, -34359738361
  // CHECK-BITMANIP-LINUX-NEXT: or i64 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: store i64 %{{.+}}, i64* %{{.+}}
  // CHECK-BITMANIP-WIN: getelementptr inbounds %struct.bf1, %struct.bf1* %{{.+}}, i32 0, i32 2
  // CHECK-BITMANIP-WIN_NEXT: bitcast i64* %{{.+}} to i32*
  // CHECK-BITMANIP-WIN_NEXT: trunc i64 %{{.+}} to i32
  // CHECK-BITMANIP-WIN_NEXT: store i32 %{{.+}}, i32* %{{.+}} 
  s1.value_field = get_long_long();
}

void check_bf2(void) {
  // offset = 16, size = 9
  struct bf2 {
    int padding1;
    char *padding2;
    unsigned offset_field1 : 3;
    int offset_field2 : 8;
    int offset_field3 : 5;
    int value_field : 9;
  } s2;
  // CHECK-BITMANIP-LINUX: getelementptr inbounds %struct.bf2, %struct.bf2* %{{.+}}, i32 0, i32 2
  // CHECK-BITMANIP-LINUX-NEXT: load i32, i32* %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: and i32 %c{{.+}}, 511
  // CHECK-BITMANIP-LINUX-NEXT: shl i32 %{{.+}}, 16
  // CHECK-BITMANIP-LINUX-NEXT: and i32 %{{.+}}, -33488897
  // CHECK-BITMANIP-LINUX-NEXT: or i32 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: store i32 %{{.+}}, i32* %{{.+}}
  // CHECK-BITMANIP-WIN: getelementptr inbounds %struct.bf2, %struct.bf2* %{{.+}}, i32 0, i32 2
  // CHECK-BITMANIP-WIN-NEXT: load i32, i32* %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: and i32 %{{.+}}, 511
  // CHECK-BITMANIP-WIN-NEXT: shl i32 %{{.+}}, 16
  // CHECK-BITMANIP-WIN-NEXT: and i32 %{{.+}}, -33488897
  // CHECK-BITMANIP-WIN-NEXT: or i32 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: store i32 %{{.+}}, i32* %{{.+}}
  s2.value_field = get_int();
}

void check_bf3(void) {
  // offset = 3, size = 1
  struct bf3 {
    unsigned char offset_field1 : 1;
    unsigned char offset_field2 : 1;
    unsigned char offset_field3 : 1;
    short value_field : 1;
  } s3;
  // CHECK-BITMANIP-LINUX: bitcast %struct.bf3* %{{.+}} to i8*
  // CHECK-BITMANIP-LINUX-NEXT: trunc i16 %{{.+}} to i8
  // CHECK-BITMANIP-LINUX-NEXT: load i8, i8* %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: and i8 %{{.+}}, 1
  // CHECK-BITMANIP-LINUX-NEXT: shl i8 %{{.+}}, 3
  // CHECK-BITMANIP-LINUX-NEXT: and i8 %{{.+}}, -9
  // CHECK-BITMANIP-LINUX-NEXT: or i8 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: store i8 %{{.+}}, i8* %{{.+}}
  // CHECK-BITMANIP-WIN: getelementptr inbounds %struct.bf3, %struct.bf3* %{{.+}}, i32 0, i32 1
  // CHECK-BITMANIP-WIN-NEXT: load i16, i16* %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: and i16 %{{.+}}, 1
  // CHECK-BITMANIP-WIN-NEXT: and i16 %{{.+}}, -2
  // CHECK-BITMANIP-WIN-NEXT: or i16 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: store i16 %{{.+}}, i16* %{{.+}}
  s3.value_field = get_short();
}

void check_bf4(void) {
  // offset = 0, size = 7
  struct bf4 {
    unsigned char padding1;
    char value_field : 7;
    unsigned char flag1 : 1;
    unsigned char flag2 : 1;
    unsigned char flag3 : 1;
  } s4;
  // CHECK-BITMANIP-LINUX: getelementptr inbounds %struct.bf4, %struct.bf4* %{{.+}}, i32 0, i32 1
  // CHECK-BITMANIP-LINUX-NEXT: zext i8 %{{.+}} to i16
  // CHECK-BITMANIP-LINUX-NEXT: load i16, i16* %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: and i16 %{{.+}}, 127
  // CHECK-BITMANIP-LINUX-NEXT: and i16 %{{.+}}, -128
  // CHECK-BITMANIP-LINUX-NEXT: or i16 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-LINUX-NEXT: store i16 %{{.+}}, i16* %{{.+}}
  // CHECK-BITMANIP-WIN: getelementptr inbounds %struct.bf4, %struct.bf4* %{{.+}}, i32 0, i32 1
  // CHECK-BITMANIP-WIN-NEXT: load i8, i8* %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: and i8 %{{.+}}, 127
  // CHECK-BITMANIP-WIN-NEXT: and i8 %{{.+}}, -128
  // CHECK-BITMANIP-WIN-NEXT: or i8 %{{.+}}, %{{.+}}
  // CHECK-BITMANIP-WIN-NEXT: store i8 %{{.+}}, i8* %{{.+}}
  s4.value_field = get_char();
}

#elif NO_BITMANIP

void check_bf5(void) {
  // offset = 0, size = 16
  struct bf5 {
    long long padding1;
    int offset_field: 8;
    long long value_field : 16;
  } s5;
  // CHECK-NO-BITMANIP-LINUX: getelementptr inbounds %struct.bf5, %struct.bf5* %{{.+}}, i32 0, i32 1
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i24* %{{.+}} to i32*
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i32* %{{.+}} to i8*
  // CHECK-NO-BITMANIP-LINUX-NEXT: getelementptr i8, i8* %{{.+}}, i64 1
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i8* %{{.+}} to i16*
  // CHECK-NO-BITMANIP-LINUX-NEXT: trunc i64 %{{.+}} to i16
  // CHECK-NO-BITMANIP-LINUX-NEXT: store i16 %{{.+}}, i16* %{{.+}}
  // CHECK-NO-BITMANIP-WIN: getelementptr inbounds %struct.bf5, %struct.bf5* %{{.+}}, i32 0, i32 2
  // CHECK-NO-BITMANIP-WIN-NEXT: bitcast i64* %{{.+}} to i16*
  // CHECK-NO-BITMANIP-WIN-NEXT: trunc i64 %{{.+}} to i16
  // CHECK-NO-BITMANIP-WIN-NEXT: 16 %{{.+}}, i16* %{{.+}}
  s5.value_field = get_long_long();
}

void check_bf6(void) {
  // offset = 8, size = 8
  struct bf6 {
    float padding1;
    int padding2;
    int offset_field : 8;
    int value_field : 8;
    short *padding3;
  } s6;
  // CHECK-NO-BITMANIP-LINUX: getelementptr inbounds %struct.bf6, %struct.bf6* %{{.+}}, i32 0, i32 2
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i16* %{{.+}} to i8*
  // CHECK-NO-BITMANIP-LINUX-NEXT: getelementptr i8, i8* %{{.+}}, i64 1
  // CHECK-NO-BITMANIP-LINUX-NEXT: trunc i32 %{{.+}} to i8
  // CHECK-NO-BITMANIP-LINUX-NEXT: store i8 %{{.+}}, i8* %{{.+}}
  // CHECK-NO-BITMANIP-WIN: getelementptr inbounds %struct.bf6, %struct.bf6* %{{.+}}, i32 0, i32 2
  // CHECK-NO-BITMANIP-WIN-NEXT: bitcast i32* %{{.+}} to i8*
  // CHECK-NO-BITMANIP-WIN-NEXT: getelementptr i8, i8* %{{.+}}, i64 1
  // CHECK-NO-BITMANIP-WIN-NEXT: trunc i32 %{{.+}} to i8
  // CHECK-NO-BITMANIP-WIN-NEXT: store i8 %{{.+}}, i8* %{{.+}}
  s6.value_field = get_char();
}

void check_bf7(void) {
  // offset = 32, size = 16
  struct bf7 {
    char *padding1;
    int offset_field1 : 1;
    int offset_field2 : 16;
    int offset_field3 : 15;
    int value_field : 16;
  } s7;
  // CHECK-NO-BITMANIP-LINUX: getelementptr inbounds %struct.bf7, %struct.bf7* %{{.+}}, i32 0, i32 1
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i48* %{{.+}} to i64*
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i64* %{{.+}} to i8*
  // CHECK-NO-BITMANIP-LINUX-NEXT: getelementptr i8, i8* %{{.+}}, i64 4
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i8* %{{.+}} to i16*
  // CHECK-NO-BITMANIP-LINUX-NEXT: trunc i32 %{{.+}} to i16
  // CHECK-NO-BITMANIP-LINUX-NEXT: store i16 %{{.+}}, i16* %{{.+}}
  // CHECK-NO-BITMANIP-WIN: getelementptr inbounds %struct.bf7, %struct.bf7* %{{.+}}, i32 0, i32 2
  // CHECK-NO-BITMANIP-WIN-NEXT: bitcast i32* %{{.+}} to i16*
  // CHECK-NO-BITMANIP-WIN-NEXT: trunc i32 %{{.+}} to i16
  // CHECK-NO-BITMANIP-WIN-NEXT: store i16 %{{.+}}, i16* %{{.+}}
  s7.value_field = get_int();
}

void check_bf8(void) {
  // offset = 16, size = 64
  struct bf8 {
    long long padding1;
    int offset_field1 : 6;
    int offset_field2 : 6;
    int offset_field3 : 4;
    __int128 value_field : 64;
    int offset_field : 32;
    char padding2;
  } s8;
  // CHECK-NO-BITMANIP-LINUX: getelementptr inbounds %struct.bf8, %struct.bf8* %{{.+}}, i32 0, i32 3
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast [12 x i8]* %{{.+}} to i96*
  // CHECK-NO-BITMANIP-LINUX-NEXT: bitcast i96* %{{.+}} to i64*
  // CHECK-NO-BITMANIP-LINUX-NEXT: trunc i128 %{{.+}} to i64
  // CHECK-NO-BITMANIP-LINUX-NEXT: store i64 %{{.+}}, i64* %{{.+}}
  // CHECK-NO-BITMANIP-WIN: getelementptr inbounds %struct.bf8, %struct.bf8* %{{.+}}, i32 0, i32 2
  // CHECK-NO-BITMANIP-WIN-NEXT: bitcast i128* %{{.+}} to i64*
  // CHECK-NO-BITMANIP-WIN-NEXT: trunc i128 %{{.+}} to i64
  // CHECK-NO-BITMANIP-WIN-NEXT: store i64 %{{.+}}, i64* %{{.+}}
  s8.value_field = get_int128();
}

#else

#error Unknown test mode

#endif
