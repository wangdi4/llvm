// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S1 {
  int y;
  double d[50];
  struct S1 *next;
};

// CHECK-LABEL: foo
void foo(S1 *ps1)
{
  // CHECK: [[PS1_ADDR:%.+]] = alloca %struct.S1*,
  // CHECK: [[A:%.+]] = alloca i32,
  // CHECK: [[B:%.+]] = alloca double,
  // CHECK: [[ARRS:%.+]] = alloca [99 x i32],
  // CHECK: [[ARRD:%.+]] = alloca [49 x [29 x i32]],
  // CHECK: [[PS1_MAP:%.+]] = alloca %struct.S1*,
  // CHECK: [[PS1_MAP4:%.+]] = alloca %struct.S1*,
  // CHECK: [[PS1_MAP9:%.+]] = alloca %struct.S1*,
  int a; double b;
  int arrS[99];
  int arrD[49][29];

  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM{{.*}}(i32* [[A]]),
  // CHECK-SAME: MAP.TOFROM{{.*}}(double* [[B]])
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(a,b)
  {
    a = 1;
    b = 2;
  }

  // CHECK: [[L1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L2:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[P1:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L2]], i32 0, i32 0
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.S1* [[L1]], i32* %y, i64 %9)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.S1** [[PS1_MAP]])
  // CHECK: store %struct.S1* [[L1]], %struct.S1** [[PS1_MAP]], align 8
  // CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->y)
  {
    ps1->y = 3;
  }

  // CHECK: [[L6:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L6_1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N1:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L6_1]], i32 0, i32 2
  // CHECK: [[L7:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N2:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L7]], i32 0, i32 2
  // CHECK: [[L8:%[0-9]+]] = load %struct.S1*, %struct.S1** [[N2]],
  // CHECK: [[Y3:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L8]], i32 0, i32 0
  // CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.S1* [[L6]], %struct.S1** %next, i64 %22)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.S1** %next, i32* %y3, i64 4)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.S1** [[PS1_MAP4]])
  // CHECK: store %struct.S1* %12, %struct.S1** [[PS1_MAP4]]
  // CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->y)
  {
    ps1->next->y = 4;
  }

  // CHECK: [[L13_1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L13:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N6:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L13]], i32 0, i32 2
  // CHECK: [[L14:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N7:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L14]], i32 0, i32 2
  // CHECK: [[L15:%[0-9]+]] = load %struct.S1*, %struct.S1** [[N7]],
  // CHECK: [[D0:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L15]], i32 0, i32 1
  // CHECK: [[AI:%.+]] = getelementptr inbounds [50 x double], [50 x double]* [[D0]], i64 0, i64 17
  // CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME:  "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.S1* %26, %struct.S1** %next7, i64 %36)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.S1** %next7, double* %arrayidx, i64 200)
  // CHECK-SAME:  "QUAL.OMP.PRIVATE"(%struct.S1** %ps1.map.ptr.tmp9)
  // CHECK: store %struct.S1* %26, %struct.S1** %ps1.map.ptr.tmp9, align 8
  // CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->d[17:25])
  {
    ps1->next->d[17] = 5;
    ps1->next->d[40] = 6;
  }

  // CHECK: [[AI14:%.+]] = getelementptr inbounds [99 x i32], [99 x i32]* [[ARRS]], i64 0, i64 42
  // CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD"([99 x i32]* [[ARRS]], i32* [[AI14]], i64 80)
  // CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrS[42:20])
  {
    arrS[50] = 3;
  }

  // CHECK: [[AI16:%.+]] = getelementptr inbounds [49 x [29 x i32]], [49 x [29 x i32]]* [[ARRD]], i64 0, i64 9
  // CHECK: [[TV6:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM:AGGRHEAD"([49 x [29 x i32]]* [[ARRD]], [29 x i32]* [[AI16]], i64 1392)
  // CHECK: region.exit(token [[TV6]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrD[9:12][:])
  {
    arrD[11][14] = 4;
  }
}

struct A {
  int f_one[20];
  int f_two[20][10][10];
};

// CHECK-LABEL: foo_two
void foo_two(int *ip, A *ap, int n) {
  // CHECK: [[IP_ADDR:%.+]] = alloca i32*,
  // CHECK: [[AP_ADDR:%.+]] = alloca %struct.A*,
  // CHECK: [[MAPPTR:%.+]] = alloca i32*, align 8
  // CHECK: [[L0:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8

  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.A* [[L0]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[3][2][n:3])
  {}
  // CHECK: [[L1:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.A* [[L1]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_one[n:3])
  {}
  // CHECK: [[L2:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.A* [[L2]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[n])
  {}
  // CHECK: [[L3:%.+]] = load i32*, i32** %ip.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32* [[L3]],
  // CHECK-NEXT: store i32* %35, i32** %ip.map.ptr.tmp, align 8
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ip[n])
  {}
}

// CHECK-LABEL: foo_three
double foo_three(double *x) {
  int i;
  double s_foo = 0.0;
  double *sp_foo = &s_foo;

  // CHECK: [[SFOO:%s_foo.*]] = alloca double,
  // CHECK: [[SPFOO:%sp_foo.*]] = alloca double*,
  // CHECK: %x.map.ptr.tmp = alloca double*, align 8
  // CHECK: %x.map.ptr.tmp6 = alloca double*, align 8
  // CHECK: %x.map.ptr.tmp26 = alloca double*, align 8

  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double** %x.map.ptr.tmp),
  // CHECK-NEXT: store double* %0, double** %x.map.ptr.tmp, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(double* [[SFOO]]
  // CHECK-SAME: "QUAL.OMP.SHARED"(double** %x.map.ptr.tmp)
  // CHECK: load double*, double** %x.map.ptr.tmp, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for reduction(+:s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]])
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SFOO]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double** %x.map.ptr.tmp6),
  // CHECK-NEXT: store double* %13, double** %x.map.ptr.tmp6, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.SHARED"(double** %x.map.ptr.tmp6)
  // CHECK: load double*, double** %x.map.ptr.tmp6, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for lastprivate(s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double** [[SPFOO]])
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double** %x.map.ptr.tmp26),
  // CHECK-NEXT: store double* %26, double** %x.map.ptr.tmp26, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.SHARED"(double** %x.map.ptr.tmp26)
  // CHECK: load double*, double** %x.map.ptr.tmp26, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for linear(sp_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    x[i+(int)(*sp_foo)]++;

  // Check that no implicit map is added if there is an explicit map.
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]])
  // CHECK-NOT: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]])
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target teams distribute parallel for \
     map(tofrom: s_foo) reduction(+:s_foo)
  for (int i = 0; i < 100; ++i) {}
  return s_foo;
}

// Check that canonical variable is used for mapping.
#pragma omp declare target
extern int foo_four_x;
extern void foo4_call();
#pragma omp end declare target
#pragma omp declare target
int foo_four_x;
#pragma omp end declare target
// CHECK-LABEL: foo_four
void foo_four()
{
  //CHECK: DIR.OMP.TARGET
  //CHECK-SAME: MAP{{.*}}foo_four_x
  #pragma omp target map(foo_four_x)
  {
    foo4_call();
    foo_four_x = foo_four_x + 200;
  }
  //CHECK: DIR.OMP.END.TARGET
  return;
}
struct SP {
  int f1;
  int f2;
  int f3;
};
// CHECK-LABEL: foo_five
void foo_five(SP *p) {

  // CHECK: [[P:%.+]] = alloca %struct.SP*,
  // CHECK: [[P_MAP:%.+]] = alloca %struct.SP*,
  // CHECK: [[P_MAP4:%.+]] = alloca %struct.SP*,
  // CHECK: [[P_MAP5:%.+]] = alloca %struct.SP*,
  // CHECK: [[P_MAP10:%.+]] = alloca %struct.SP*,
  // CHECK: [[P_MAP11:%.+]] = alloca %struct.SP*,
  // CHECK: [[TV22:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SP* %0, i32* %f1, i64 %10)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %0, i32* %f1, i64 4),
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %2, i32* %f2, i64 4)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP]])
  // CHECK: store %struct.SP* %0, %struct.SP** [[P_MAP]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP]]
  // CHECK: region.exit(token [[TV22]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1, p->f2)
  {
    p->f1 = 0;
  }
  // CHECK: [[TV42:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SP* %13, i32* %f12, i64 %21)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %13, i32* %f12, i64 4)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SP* %22, i32* %f23, i64 %30)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %22, i32* %f23, i64 4)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP4]])
  // CHECK: store %struct.SP* %13, %struct.SP** [[P_MAP4]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP4]]
  // CHECK: region.exit(token [[TV42]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1) map(p->f2)
  {
    p->f2 = 0;
    p->f1 = 0;
  }
  // CHECK: [[TV76:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SP* %34, i32* %f18, i64 %44)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %34, i32* %f18, i64 4)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %36, i32* %f29, i64 4)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(%struct.SP* %45, i32* %f3, i64 %53)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(%struct.SP* %45, i32* %f3, i64 4)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP10]])
  // CHECK: store %struct.SP* %34, %struct.SP** [[P_MAP10]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP10]]
  // CHECK: region.exit(token [[TV76]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1, p->f2) map(p->f3)
  {
    p->f1 = 0;
  }
}

class BOO {
public:
// CHECK-LABEL: BOOC2
  BOO() {
  // CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO:AGGRHEAD"(%class.BOO* %arrayidx, %class.BOO* %arrayidx2, i64 8)
#pragma omp target enter data map(to:this[0:1])
      {
       zoo[1] = 1.0;
      }
  }
  double* zoo;
};
BOO * obj = new BOO();


// end INTEL_COLLAB
