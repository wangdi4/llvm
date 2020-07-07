//RUN: %clang_cc1 -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -O0 -fsycl-is-device  -emit-llvm -o - %s | FileCheck %s

#define SIZE 10
unsigned char ucharVar[SIZE];
struct A { int i; int j; char tmp[SIZE]; } structVar[SIZE];
struct A structVar2;

// "plain" ivdep
// CHECK: define{{.*}}foo0
void foo0()
{
//CHECK: br{{.*}}!llvm.loop [[MD3:![0-9]+]]
[[intelfpga::ivdep()]]
for (int i = 0; i < 10; ++i) {}
}

// ivdep with safelen specified
// CHECK: define{{.*}}foo1
void foo1()
{
//CHECK: br{{.*}}!llvm.loop [[MD5:![0-9]+]]
[[intelfpga::ivdep(4)]]
for (int i = 0; i < 10; ++i) {}
}

// ivdep with array specified
// CHECK: define{{.*}}foo2
void foo2()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD7:![0-9]+]]
[[intelfpga::ivdep(ucharVar)]]
for (int i = 0; i < 10; ++i) {}
}

// ivdep with array and safelen specified
// CHECK: define{{.*}}foo2a
void foo2a()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD10:![0-9]+]]
[[intelfpga::ivdep(ucharVar, 4)]]
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified same safelen
// CHECK: define{{.*}}foo3
void foo3()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD13:![0-9]+]]
[[intelfpga::ivdep(structVar, 8)]]
[[intelfpga::ivdep(ucharVar, 8)]]
for (int i = 0; i < 10; ++i) {}
}
// multiple ivdep with array specified and different safelen specified
// CHECK: define{{.*}}foo4
void foo4()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD17:![0-9]+]]
[[intelfpga::ivdep(structVar, 8)]]
[[intelfpga::ivdep(ucharVar, 4)]]
for (int i = 0; i < 10; ++i) {}
}
// multiple ivdep with array specified, not all safelen specified
// CHECK: define{{.*}}foo5
void foo5()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD22:![0-9]+]]
[[intelfpga::ivdep(structVar, 8)]]
[[intelfpga::ivdep(ucharVar)]]
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo6
void foo6()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD27:![0-9]+]]
[[intelfpga::ivdep(ucharVar, 4)]]
[[intelfpga::ivdep()]]
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo7
void foo7()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD28:![0-9]+]]
[[intelfpga::ivdep(structVar2.tmp, 8)]]
[[intelfpga::ivdep(ucharVar, 4)]]
for (int i = 0; i < 10; ++i) {}
}

// An array with or without a safelen combined with a safelen with no array
// CHECK: define{{.*}}foo8
void foo8()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, i32* %i, align 4
//CHECK: load i32, i32* %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD33:![0-9]+]]
[[intelfpga::ivdep(ucharVar, 8)]]
[[intelfpga::ivdep(4)]]
for (int i = 0; i < 10; ++i) {}
}

// An array with no safelen on while(1) loop
// CHECK: define{{.*}}foo9
void foo9(long* buffer1)
{
//CHECK: buffer1.addr = alloca i64 addrspace(4)*, align 8
//CHECK: store i64 addrspace(4)* %buffer1, i64 addrspace(4)** %buffer1.addr, align 8
//CHECK: br label %while.body, !llvm.loop [[MD36:![0-9]+]]
[[intelfpga::ivdep(buffer1)]]
while (1) { }
}

// Pass template parameter on pragma ivdep safelen
// CHECK: define{{.*}}do_stuff
template<int LEN>
int do_stuff(int N) {
  int temp = 0;
  //CHECK: %N.addr = alloca i32, align 4
  //CHECK: %temp = alloca i32, align 4
  //CHECK: alloca i32, align 4
  //CHECK: store i32 %N, i32* %N.addr, align 4
  //CHECK: store i32 0, i32* %temp, align 4
  //CHECK: store i32 0, i32* %i, align 4
  //CHECK: br{{.*}}!llvm.loop [[MD39:![0-9]+]]
  [[intelfpga::ivdep(LEN)]]
  for (int i = 0; i < N; ++i) {
    temp += i;
  }
  return temp;
}

int dut() {
  return do_stuff<5>(10);
}

//CHECK: [[MD3]] = distinct !{[[MD3]], [[MD4:![0-9]+]]}
//CHECK: [[MD4]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[MD5]] = distinct !{[[MD5]], [[MD6:![0-9]+]]}
//CHECK: [[MD6]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[MD7]] = distinct !{[[MD7]], [[MD8:![0-9]+]]}
//CHECK: [[MD8]] = !{!"llvm.loop.parallel_access_indices", [[MD9:![0-9]+]]}
//CHECK: [[MD9]] = distinct !{}
//CHECK: [[MD10]] = distinct !{[[MD10]], [[MD11:![0-9]+]]}
//CHECK: [[MD11]] = !{!"llvm.loop.parallel_access_indices", [[MD12:![0-9]+]], i32 4}
//CHECK: [[MD12]] = distinct !{}
//CHECK: [[MD13]] = distinct !{[[MD13]], [[MD14:![0-9]+]]}
//CHECK: [[MD14]] = !{!"llvm.loop.parallel_access_indices", [[MD15:![0-9]+]], [[MD16:![0-9]+]], i32 8}
//CHECK: [[MD15]] = distinct !{}
//CHECK: [[MD16]] = distinct !{}
//CHECK: [[MD17]] = distinct !{[[MD17]], [[MD18:![0-9]+]], [[MD20:![0-9]+]]}
//CHECK: [[MD18]] = !{!"llvm.loop.parallel_access_indices", [[MD19:![0-9]+]], i32 8}
//CHECK: [[MD19]] = distinct !{}
//CHECK: [[MD20]] = !{!"llvm.loop.parallel_access_indices", [[MD21:![0-9]+]], i32 4}
//CHECK: [[MD21]] = distinct !{}
//CHECK: [[MD22]] = distinct !{[[MD22]], [[MD23:![0-9]+]], [[MD25:![0-9]+]]}
//CHECK: [[MD23]] = !{!"llvm.loop.parallel_access_indices", [[MD24:![0-9]+]], i32 8}
//CHECK: [[MD24]] = distinct !{}
//CHECK: [[MD25]] = !{!"llvm.loop.parallel_access_indices", [[MD26:![0-9]+]]}
//CHECK: [[MD26]] = distinct !{}
//CHECK: [[MD27]] = distinct !{[[MD27]], [[MD4]]}
//CHECK: [[MD28]] = distinct !{[[MD28]], [[MD29:![0-9]+]], [[MD31:![0-9]+]]}
//CHECK: [[MD29]] = !{!"llvm.loop.parallel_access_indices", [[MD30:![0-9]+]], i32 8}
//CHECK: [[MD30]] = distinct !{}
//CHECK: [[MD31]] = !{!"llvm.loop.parallel_access_indices", [[MD32:![0-9]+]], i32 4}
//CHECK: [[MD32]] = distinct !{}
//CHECK: [[MD33]] = distinct !{[[MD33]], [[MD6]], [[MD34:![0-9]+]]}
//CHECK: [[MD34]] = !{!"llvm.loop.parallel_access_indices", [[MD35:![0-9]+]], i32 8}
//CHECK: [[MD35]] = distinct !{}
//CHECK: [[MD36]] = distinct !{[[MD36]], [[MD37:![0-9]+]]}
//CHECK: [[MD37]] = !{!"llvm.loop.parallel_access_indices", [[MD38:![0-9]+]]}
//CHECK: [[MD38]]  = distinct !{}
//CHECK: [[MD39]] = distinct !{[[MD39]], [[MD40:![0-9]+]]}
//CHECK: [[MD40]] = !{!"llvm.loop.ivdep.safelen", i32 5}
