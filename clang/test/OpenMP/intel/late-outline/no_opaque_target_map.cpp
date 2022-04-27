// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S1 {
  int y;
  double d[50];
  struct S1 *next;
};
extern void zoo();
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
  // CHECK-SAME: MAP.TOFROM{{.*}}(i32* [[A]], i32* [[A]], i64 4, i64 35,
  // CHECK-SAME: MAP.TOFROM{{.*}}(double* [[B]], double* [[B]], i64 8, i64 35
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(a,b)
  {
    a = 1;
    b = 2;
  }
  zoo();
  // CHECK: call {{.*}}zoo
  // CHECK: [[L0:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[L1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[L2:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[N2:%[y.*]]] = getelementptr inbounds %struct.S1, %struct.S1* [[L2]], i32 0, i32 0
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.S1* [[L1]], i32* [[N2]], i64 4, i64 35
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.S1** [[PS1_MAP]])
  // CHECK: store %struct.S1* [[L1]], %struct.S1** [[PS1_MAP]], align 8
  // CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->y)
  {
    ps1->y = 3;
  }

  // CHECK: [[L6_0:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L6:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[L6_1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N1:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L6_1]], i32 0, i32 2
  // CHECK: [[L7:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK: [[N2:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L7]], i32 0, i32 2
  // CHECK: [[L8:%[0-9]+]] = load %struct.S1*, %struct.S1** [[N2]],
  // CHECK: [[Y3:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L8]], i32 0, i32 0
  // CHECK: [[L17:%.+]] = sdiv
  // CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.S1* [[L6]], %struct.S1** %next, i64 [[L17]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S1** %next, i32* %y3, i64 4, i64 281474976710675
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.S1** [[PS1_MAP4]])
  // CHECK: store %struct.S1* [[L6]], %struct.S1** [[PS1_MAP4]]
  #pragma omp target map(ps1->next->y)
  {
    ps1->next->y = 4;
  }
  // CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
  // CHECK-NEXT: [[L13_0:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[L13_1:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[L13:%[0-9]+]] = load %struct.S1*, %struct.S1** [[PS1_ADDR]],
  // CHECK-NEXT: [[N6:%.+]] = getelementptr inbounds %struct.S1, %struct.S1* [[L13]], i32 0, i32 2
  // CHECK: [[L33:%.+]] = getelementptr %struct.S1*, %struct.S1**  [[N6]], i32 1
  // CHECK-NEXT: [[L34:%.+]] = bitcast %struct.S1** [[N6]] to i8*
  // CHECK-NEXT: [[L35:%.+]] = bitcast %struct.S1** [[L33]] to i8*
  // CHECK-NEXT: [[L36:%.+]] = ptrtoint i8* [[L35]] to i64
  // CHECK-NEXT: [[L37:%.+]] = ptrtoint i8* [[L34]] to i64
  // CHECK-NEXT: [[L38:%.+]] = sub i64 [[L36]], [[L37]]
  // CHECK-NEXT: [[L39:%.+]] = sdiv
  // CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME:  "QUAL.OMP.MAP.TOFROM"(%struct.S1* [[L13_1]], %struct.S1** [[N6]], i64 [[L39]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.S1** [[N6]], double* %arrayidx, i64 200, i64 281474976710675
  // CHECK-SAME:  "QUAL.OMP.PRIVATE"(%struct.S1** %ps1.map.ptr.tmp9)
  // CHECK: store %struct.S1* [[L13_1]], %struct.S1** %ps1.map.ptr.tmp9, align 8
  // CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->d[17:25])
  {
    ps1->next->d[17] = 5;
    ps1->next->d[40] = 6;
  }

  // CHECK: [[AI14:%.+]] = getelementptr inbounds [99 x i32], [99 x i32]* [[ARRS]], i64 0, i64 42
  // CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM"([99 x i32]* [[ARRS]], i32* [[AI14]], i64 80, i64 35
  // CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrS[42:20])
  {
    arrS[50] = 3;
  }

  // CHECK: [[AI16:%.+]] = getelementptr inbounds [49 x [29 x i32]], [49 x [29 x i32]]* [[ARRD]], i64 0, i64 9
  // CHECK: [[TV6:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM"([49 x [29 x i32]]* [[ARRD]], [29 x i32]* [[AI16]], i64 1392, i64 35
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
  // CHECK: [[MAPPTR:%.+]] = alloca %struct.A*, align 8
  // CHECK: [[L0:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[L00:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8

  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.A* [[L00]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[3][2][n:3])
  {
    ap->f_two[1][2][1] = 1;
  }
  // CHECK: [[L1:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[L11:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.A* [[L11]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_one[n:3])
  {
    ap->f_one[1]=1;
  }
  // CHECK: [[L2:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[L22:%.+]] = load %struct.A*, %struct.A** %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.A* [[L22]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[n])
  {
    ap->f_two[1][1][1] = 1;
  }
  // CHECK: [[L3:%.+]] = load i32*, i32** %ip.addr, align 8
  // CHECK: [[L33:%.+]] = load i32*, i32** %ip.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* [[L33]],
  // CHECK-NEXT: store i32* [[L33]], i32** %ip.map.ptr.tmp, align 8
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ip[n])
  {
    ip[1]=0;
  }
}

// CHECK-LABEL: foo_three
double foo_three(double *x) {
  int i;
  double s_foo = 0.0;
  double *sp_foo = &s_foo;

  // CHECK: [[SFOO:%s_foo.*]] = alloca double,
  // CHECK: [[SPFOO:%sp_foo.*]] = alloca double*,
  // CHECK: %x.map.ptr.tmp = alloca double*, align 8
  // CHECK: [[L0:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[L1:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[L2:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(double* [[L1]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double** %x.map.ptr.tmp),
  // CHECK-NEXT: store double* [[L1]], double** %x.map.ptr.tmp, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(double* [[SFOO]]
  // CHECK: load double*, double** %x.map.ptr.tmp, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for reduction(+:s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[L14:%[0-9]+]] = load double*, double** %x.addr
  // CHECK: [[L15:%[0-9]+]] = load double*, double** %x.addr
  // CHECK: [[L16:%[0-9]+]] = load double*, double** %x.addr
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]]
  // CHECK-NEXT: store double* [[L15]], double** %x.map.ptr.tmp6, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.SHARED"(double** %x.map.ptr.tmp6)
  // CHECK: load double*, double** %x.map.ptr.tmp6, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for lastprivate(s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[L28:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[L30:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[L31:%[0-9]+]] = load double*, double** %x.addr,
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double** [[SPFOO]]
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(double* [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double** %x.map.ptr.tmp26),
  // CHECK-NEXT: store double* [[L30]], double** %x.map.ptr.tmp26, align 8
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
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]],
  // CHECK-NOT: "QUAL.OMP.MAP.TOFROM"(double* [[SFOO]],
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
  // CHECK: [[P_MAP9:%.+]] = alloca %struct.SP*,
  // CHECK: [[L0:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L1:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L2:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F1:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L2]], i32 0, i32 0
  // CHECK: [[L3:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L4:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F2:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L4]], i32 0, i32 1
  // CHECK: [[L11:%[0-9]+]] = sdiv
  // CHECK: [[TV22:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.SP* [[L1]], i32* [[F1]], i64 [[L11]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L1]], i32* [[F1]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L3]], i32* [[F2]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP]])
  // CHECK: store %struct.SP* [[L1]], %struct.SP** [[P_MAP]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP]]
  // CHECK: region.exit(token [[TV22]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1, p->f2)
  {
    p->f1 = 0;
  }
  // CHECK: [[L14:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L15:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L16:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F12:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L16]], i32 0, i32 0
  // CHECK: [[L17:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L18:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F23:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L18]], i32 0, i32 1
  // CHECK: [[L25:%[0-9]+]] = sdiv
  // CHECK: [[TV42:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.SP* [[L15]], i32* [[F12]], i64 [[L25]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L15]], i32* [[F12]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L17]], i32* [[F23]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP4]])
  // CHECK: store %struct.SP* [[L15]], %struct.SP** [[P_MAP4]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP4]]
  // CHECK: region.exit(token [[TV42]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1) map(p->f2)
  {
    p->f2 = 0;
    p->f1 = 0;
  }

  // CHECK: [[L29:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L30:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L31:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F17:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L31]], i32 0, i32 0
  // CHECK: [[L32:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L33:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F28:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L33]], i32 0, i32 1
  // CHECK: [[L34:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[L35:%[0-9]+]] = load %struct.SP*, %struct.SP** %p.addr,
  // CHECK: [[F3:%.+]] =  getelementptr inbounds %struct.SP, %struct.SP* [[L35]], i32 0, i32 2
  // CHECK: [[L42:%[0-9]+]] = sdiv
  // CHECK: [[TV76:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.SP* [[L30]], i32* [[F17]], i64 [[L42]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L30]], i32* [[F17]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SP* [[L32]], i32* [[F28]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:ALWAYS.CHAIN"(%struct.SP* [[L34]], i32* [[F3]], i64 4, i64 281474976710663
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.SP** [[P_MAP9]])
  // CHECK: store %struct.SP* %30, %struct.SP** [[P_MAP9]]
  // CHECK: load %struct.SP*, %struct.SP** [[P_MAP9]]
  // CHECK: region.exit(token [[TV76]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1, p->f2) map(always tofrom: p->f3)
  {
    p->f1 = 0;
  }
}

class BOO {
public:
// CHECK-LABEL: BOOC2
  BOO() {
  // CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(%class.BOO* %this1, %class.BOO* %this1, i64 8, i64 1
#pragma omp target enter data map(to:this[0:1])
      {
       zoo[1] = 1.0;
      }
  }
  double* zoo;
};
BOO * obj = new BOO();

class synced_ptr {
public:
    float* qqq;
    void sync() {
        float* ppp;
  // CHECK: [[T2:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%class.synced_ptr* %this1, float** %qqq
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(float** %ppp, float** %ppp
        #pragma omp target  map(qqq, ppp)
        {
            qqq = ppp;
        }
  // CHECK: [[T3:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(%class.synced_ptr* %this1, float** %qqq
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(float** %ppp, float** %ppp
        #pragma omp target enter data map(to: qqq, ppp)
        {
            qqq = ppp;
        }
    }
    synced_ptr()  { }
};
void foo()
{
  synced_ptr sync;
  sync.sync();
}
// end INTEL_COLLAB
