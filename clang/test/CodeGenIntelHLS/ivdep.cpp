//RUN: %clang_cc1 -fhls -O0 -triple x86_64-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s

#define SIZE 10
unsigned char ucharVar[SIZE];
struct A { int i; int j; char tmp[SIZE]; } structVar[SIZE];
struct A structVar2;

// "plain" ivdep
// CHECK: define{{.*}}foo0
void foo0()
{
//CHECK: br{{.*}}!llvm.loop [[MD2:![0-9]+]]
#pragma ivdep
for (int i = 0; i < 10; ++i) {}
}

// ivdep with safelen specified
// CHECK: define{{.*}}foo1
void foo1()
{
//CHECK: br{{.*}}!llvm.loop [[MD4:![0-9]+]]
#pragma ivdep safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// ivdep with array specified
// CHECK: define{{.*}}foo2
void foo2()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD6:![0-9]+]]
#pragma ivdep array(ucharVar)
for (int i = 0; i < 10; ++i) {}
}

// ivdep with array and safelen specified
// CHECK: define{{.*}}foo2a
void foo2a()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD9:![0-9]+]]
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified same safelen
// CHECK: define{{.*}}foo3
void foo3()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD12:![0-9]+]]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar) safelen(8)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified and different safelen specified
// CHECK: define{{.*}}foo4
void foo4()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD16:![0-9]+]]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified, not all safelen specified
// CHECK: define{{.*}}foo5
void foo5()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD21:![0-9]+]]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo6
void foo6()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD26:![0-9]+]]
#pragma ivdep array(ucharVar) safelen(4)
#pragma ivdep
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo7
void foo7()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD27:![0-9]+]]
#pragma ivdep array(structVar2.tmp) safelen(8)
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// An array with or without a safelen combined with a safelen with no array
// CHECK: define{{.*}}foo8
void foo8()
{
//CHECK: alloca i32, align 4
//CHECK: store i32 0, ptr %i, align 4
//CHECK: load i32, ptr %i, align 4
//CHECK: br{{.*}}!llvm.loop [[MD32:![0-9]+]]
#pragma ivdep array(ucharVar) safelen(8)
#pragma ivdep safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// An array with no safelen on while(1) loop
// CHECK: define{{.*}}foo9
void foo9(long* buffer1)
{
//CHECK: buffer1.addr = alloca ptr, align 8
//CHECK: br label %while.body, !llvm.loop [[MD35:![0-9]+]]
#pragma ivdep array(buffer1)
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
  //CHECK: store i32 %N, ptr %N.addr, align 4
  //CHECK: store i32 0, ptr %temp, align 4
  //CHECK: store i32 0, ptr %i, align 4
  //CHECK: br{{.*}}!llvm.loop [[MD38:![0-9]+]]
  #pragma ivdep safelen(LEN)
  for (int i = 0; i < N; ++i) {
    temp += i;
  }
  return temp;
}

int dut() {
  return do_stuff<5>(10);
}

//CHECK: [[MD2]] = distinct !{[[MD2]], ![[LOOP_MUSTPROGRESS:[0-9]+]], [[MD3:![0-9]+]]}
//CHECK: [[MD3]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[MD4]] = distinct !{[[MD4]], ![[LOOP_MUSTPROGRESS]], [[MD5:![0-9]+]]}
//CHECK: [[MD5]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[MD6]] = distinct !{[[MD6]], ![[LOOP_MUSTPROGRESS]], [[MD7:![0-9]+]]}
//CHECK: [[MD7]] = !{!"llvm.loop.parallel_access_indices", [[MD8:![0-9]+]]}
//CHECK: [[MD8]] = distinct !{}
//CHECK: [[MD9]] = distinct !{[[MD9]], ![[LOOP_MUSTPROGRESS]], [[MD10:![0-9]+]]}
//CHECK: [[MD10]] = !{!"llvm.loop.parallel_access_indices", [[MD11:![0-9]+]], i32 4}
//CHECK: [[MD11]] = distinct !{}
//CHECK: [[MD12]] = distinct !{[[MD12]], ![[LOOP_MUSTPROGRESS]], [[MD13:![0-9]+]]}
//CHECK: [[MD13]] = !{!"llvm.loop.parallel_access_indices", [[MD14:![0-9]+]], [[MD15:![0-9]+]], i32 8}
//CHECK: [[MD14]] = distinct !{}
//CHECK: [[MD15]] = distinct !{}
//CHECK: [[MD16]] = distinct !{[[MD16]], ![[LOOP_MUSTPROGRESS]], [[MD17:![0-9]+]], [[MD19:![0-9]+]]}
//CHECK: [[MD17]] = !{!"llvm.loop.parallel_access_indices", [[MD18:![0-9]+]], i32 8}
//CHECK: [[MD18]] = distinct !{}
//CHECK: [[MD19]] = !{!"llvm.loop.parallel_access_indices", [[MD20:![0-9]+]], i32 4}
//CHECK: [[MD20]] = distinct !{}
//CHECK: [[MD21]] = distinct !{[[MD21]], ![[LOOP_MUSTPROGRESS]], [[MD22:![0-9]+]], [[MD24:![0-9]+]]}
//CHECK: [[MD22]] = !{!"llvm.loop.parallel_access_indices", [[MD23:![0-9]+]], i32 8}
//CHECK: [[MD23]] = distinct !{}
//CHECK: [[MD24]] = !{!"llvm.loop.parallel_access_indices", [[MD25:![0-9]+]]}
//CHECK: [[MD25]] = distinct !{}
//CHECK: [[MD26]] = distinct !{[[MD26]], ![[LOOP_MUSTPROGRESS]], [[MD3]]}
//CHECK: [[MD27]] = distinct !{[[MD27]], ![[LOOP_MUSTPROGRESS]], [[MD28:![0-9]+]], [[MD30:![0-9]+]]}
//CHECK: [[MD28]] = !{!"llvm.loop.parallel_access_indices", [[MD29:![0-9]+]], i32 8}
//CHECK: [[MD29]] = distinct !{}
//CHECK: [[MD30]] = !{!"llvm.loop.parallel_access_indices", [[MD31:![0-9]+]], i32 4}
//CHECK: [[MD31]] = distinct !{}
//CHECK: [[MD32]] = distinct !{[[MD32]], ![[LOOP_MUSTPROGRESS]], [[MD5]], [[MD33:![0-9]+]]}
//CHECK: [[MD33]] = !{!"llvm.loop.parallel_access_indices", [[MD34:![0-9]+]], i32 8}
//CHECK: [[MD34]] = distinct !{}
//CHECK: [[MD35]] = distinct !{[[MD35]], ![[LOOP_MUSTPROGRESS]], [[MD36:![0-9]+]]}
//CHECK: [[MD36]] = !{!"llvm.loop.parallel_access_indices", [[MD37:![0-9]+]]}
//CHECK: [[MD37]]  = distinct !{}
//CHECK: [[MD38]] = distinct !{[[MD38]], ![[LOOP_MUSTPROGRESS]], [[MD39:![0-9]+]]}
//CHECK: [[MD39]] = !{!"llvm.loop.ivdep.safelen", i32 5}
