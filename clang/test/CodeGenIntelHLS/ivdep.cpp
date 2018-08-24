//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s

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
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x i8]* {{.*}}ucharVar{{.*}}, i32 -1) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(ucharVar)
for (int i = 0; i < 10; ++i) {}
}

// ivdep with array and safelen specified
// CHECK: define{{.*}}foo2a
void foo2a()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x i8]* {{.*}}ucharVar{{.*}}, i32 4) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified same safelen
// CHECK: define{{.*}}foo3
void foo3()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x %struct.A]* {{.*}}structVar
//CHECK-SAME: , i32 8, [10 x i8]* {{.*}}ucharVar{{.*}}, i32 8) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar) safelen(8)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified and different safelen specified
// CHECK: define{{.*}}foo4
void foo4()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x %struct.A]* {{.*}}structVar
//CHECK-SAME: , i32 8, [10 x i8]* {{.*}}ucharVar{{.*}}, i32 4) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep with array specified, not all safelen specified
// CHECK: define{{.*}}foo5
void foo5()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x %struct.A]* {{.*}}structVar
//CHECK-SAME: , i32 8, [10 x i8]* {{.*}}ucharVar{{.*}}, i32 -1) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(structVar) safelen(8)
#pragma ivdep array(ucharVar)
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo6
void foo6()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x i8]* {{.*}}ucharVar{{.*}}, i32 4) ]
//CHECK: br{{.*}}!llvm.loop [[MD6:![0-9]+]]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(ucharVar) safelen(4)
#pragma ivdep
for (int i = 0; i < 10; ++i) {}
}

// multiple ivdep, not all with array specified
// CHECK: define{{.*}}foo7
void foo7()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x i8]* getelementptr
//CHECK-SAME: structVar2{{.*}}, i32 0, i32 2), i32 8,
//CHECK-SAME: [10 x i8]* {{.*}}ucharVar{{.*}}, i32 4) ]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(structVar2.tmp) safelen(8)
#pragma ivdep array(ucharVar) safelen(4)
for (int i = 0; i < 10; ++i) {}
}

// An array with or without a safelen combined with a safelen with no array
// CHECK: define{{.*}}foo8
void foo8()
{
//CHECK: [[TOK0:%[0-9]+]] = call token{{.*}}region.entry()
//CHECK-SAME: [ "DIR.PRAGMA.IVDEP"(),
//CHECK-SAME: "QUAL.PRAGMA.ARRAY"([10 x i8]* {{.*}}ucharVar{{.*}}, i32 8) ]
//CHECK: br{{.*}}!llvm.loop [[MD7:![0-9]+]]
//CHECK: region.exit(token [[TOK0]]) [ "DIR.PRAGMA.END.IVDEP"() ]
#pragma ivdep array(ucharVar) safelen(8)
#pragma ivdep safelen(4)
for (int i = 0; i < 10; ++i) {}
}

//CHECK: [[MD2]] = distinct !{[[MD2]], [[MD3:![0-9]+]]}
//CHECK: [[MD3]] = !{!"llvm.loop.ivdep.enable"}
//CHECK: [[MD4]] = distinct !{[[MD4]], [[MD5:![0-9]+]]}
//CHECK: [[MD5]] = !{!"llvm.loop.ivdep.safelen", i32 4}
//CHECK: [[MD6]] = distinct !{[[MD6]], [[MD3:![0-9]+]]}
//CHECK: [[MD7]] = distinct !{[[MD7]], [[MD5]]}
