// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
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
  // CHECK: [[PS1_ADDR:%.+]] = alloca ptr,
  // CHECK: [[A:%.+]] = alloca i32,
  // CHECK: [[B:%.+]] = alloca double,
  // CHECK: [[ARRS:%.+]] = alloca [99 x i32],
  // CHECK: [[ARRD:%.+]] = alloca [49 x [29 x i32]],
  // CHECK: [[PS1_MAP:%.+]] = alloca ptr,
  // CHECK: [[PS1_MAP4:%.+]] = alloca ptr,
  // CHECK: [[PS1_MAP9:%.+]] = alloca ptr,
  int a; double b;
  int arrS[99];
  int arrD[49][29];

  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM{{.*}}(ptr [[A]], ptr [[A]], i64 4, i64 35,
  // CHECK-SAME: MAP.TOFROM{{.*}}(ptr [[B]], ptr [[B]], i64 8, i64 35
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(a,b)
  {
    a = 1;
    b = 2;
  }
  zoo();
  // CHECK: call {{.*}}zoo
  // CHECK: [[L0:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[L1:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[L2:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[N2:%[y.*]]] = getelementptr inbounds %struct.S1, ptr [[L2]], i32 0, i32 0
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]], ptr [[N2]], i64 4, i64 35
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[PS1_MAP]])
  // CHECK: store ptr [[L1]], ptr [[PS1_MAP]], align 8
  // CHECK: region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->y)
  {
    ps1->y = 3;
  }

  // CHECK: [[L6_0:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK: [[L6:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK: [[L6_1:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK: [[N1:%.+]] = getelementptr inbounds %struct.S1, ptr [[L6_1]], i32 0, i32 2
  // CHECK: [[L7:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK: [[N2:%.+]] = getelementptr inbounds %struct.S1, ptr [[L7]], i32 0, i32 2
  // CHECK: [[L8:%[0-9]+]] = load ptr, ptr [[N2]],
  // CHECK: [[Y3:%.+]] = getelementptr inbounds %struct.S1, ptr [[L8]], i32 0, i32 0
  // CHECK: [[L17:%.+]] = sdiv
  // CHECK: [[TV3:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L6]], ptr %next, i64 [[L17]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %next, ptr %y3, i64 4, i64 281474976710675
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[PS1_MAP4]])
  // CHECK: store ptr [[L6]], ptr [[PS1_MAP4]]
  #pragma omp target map(ps1->next->y)
  {
    ps1->next->y = 4;
  }
  // CHECK: region.exit(token [[TV3]]) [ "DIR.OMP.END.TARGET"() ]
  // CHECK-NEXT: [[L13_0:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[L13_1:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[L13:%[0-9]+]] = load ptr, ptr [[PS1_ADDR]],
  // CHECK-NEXT: [[N6:%.+]] = getelementptr inbounds %struct.S1, ptr [[L13]], i32 0, i32 2
  // CHECK: [[L33:%.+]] = getelementptr ptr, ptr [[N6]], i32 1
  // CHECK-NEXT: [[L36:%.+]] = ptrtoint ptr [[L33]] to i64
  // CHECK-NEXT: [[L37:%.+]] = ptrtoint ptr [[N6]] to i64
  // CHECK-NEXT: [[L38:%.+]] = sub i64 [[L36]], [[L37]]
  // CHECK-NEXT: [[L39:%.+]] = sdiv
  // CHECK: [[TV4:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME:  "QUAL.OMP.MAP.TOFROM"(ptr [[L13_1]], ptr [[N6]], i64 [[L39]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[N6]], ptr %arrayidx, i64 200, i64 281474976710675
  // CHECK-SAME:  "QUAL.OMP.PRIVATE"(ptr %ps1.map.ptr.tmp9)
  // CHECK: store ptr [[L13_1]], ptr %ps1.map.ptr.tmp9, align 8
  // CHECK: region.exit(token [[TV4]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(ps1->next->d[17:25])
  {
    ps1->next->d[17] = 5;
    ps1->next->d[40] = 6;
  }

  // CHECK: [[AI14:%.+]] = getelementptr inbounds [99 x i32], ptr [[ARRS]], i64 0, i64 42
  // CHECK: [[TV5:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM"(ptr [[ARRS]], ptr [[AI14]], i64 80, i64 35
  // CHECK: region.exit(token [[TV5]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(arrS[42:20])
  {
    arrS[50] = 3;
  }

  // CHECK: [[AI16:%.+]] = getelementptr inbounds [49 x [29 x i32]], ptr [[ARRD]], i64 0, i64 9
  // CHECK: [[TV6:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: MAP.TOFROM"(ptr [[ARRD]], ptr [[AI16]], i64 1392, i64 35
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
  // CHECK: [[IP_ADDR:%.+]] = alloca ptr,
  // CHECK: [[AP_ADDR:%.+]] = alloca ptr,
  // CHECK: [[MAPPTR:%.+]] = alloca ptr, align 8
  // CHECK: [[L0:%.+]] = load ptr, ptr %ap.addr, align 8
  // CHECK: [[L00:%.+]] = load ptr, ptr %ap.addr, align 8

  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L00]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[3][2][n:3])
  {
    ap->f_two[1][2][1] = 1;
  }
  // CHECK: [[L1:%.+]] = load ptr, ptr %ap.addr, align 8
  // CHECK: [[L11:%.+]] = load ptr, ptr %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L11]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_one[n:3])
  {
    ap->f_one[1]=1;
  }
  // CHECK: [[L2:%.+]] = load ptr, ptr %ap.addr, align 8
  // CHECK: [[L22:%.+]] = load ptr, ptr %ap.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L22]],
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : ap->f_two[n])
  {
    ap->f_two[1][1][1] = 1;
  }
  // CHECK: [[L3:%.+]] = load ptr, ptr %ip.addr, align 8
  // CHECK: [[L33:%.+]] = load ptr, ptr %ip.addr, align 8
  // CHECK: [[T:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L33]],
  // CHECK-NEXT: store ptr [[L33]], ptr %ip.map.ptr.tmp, align 8
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
  // CHECK: [[SPFOO:%sp_foo.*]] = alloca ptr,
  // CHECK: %x.map.ptr.tmp = alloca ptr, align 8
  // CHECK: [[L0:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[L1:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[L2:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[SFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[L1]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr %x.map.ptr.tmp),
  // CHECK-NEXT: store ptr [[L1]], ptr %x.map.ptr.tmp, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr [[SFOO]]
  // CHECK: load ptr, ptr %x.map.ptr.tmp, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for reduction(+:s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[L14:%[0-9]+]] = load ptr, ptr %x.addr
  // CHECK: [[L15:%[0-9]+]] = load ptr, ptr %x.addr
  // CHECK: [[L16:%[0-9]+]] = load ptr, ptr %x.addr
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[SFOO]]
  // CHECK-NEXT: store ptr [[L15]], ptr %x.map.ptr.tmp6, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.SHARED"(ptr %x.map.ptr.tmp6)
  // CHECK: load ptr, ptr %x.map.ptr.tmp6, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for lastprivate(s_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    s_foo += x[i];

  // CHECK: [[L28:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[L30:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[L31:%[0-9]+]] = load ptr, ptr %x.addr,
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(ptr [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[SPFOO]]
  // CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(ptr [[SPFOO]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr %x.map.ptr.tmp26),
  // CHECK-NEXT: store ptr [[L30]], ptr %x.map.ptr.tmp26, align 8
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.SHARED"(ptr %x.map.ptr.tmp26)
  // CHECK: load ptr, ptr %x.map.ptr.tmp26, align 8
  // CHECK: region.exit(token [[L]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target parallel for linear(sp_foo) map(to: x[:100])
  for (int i = 0; i < 100; ++i)
    x[i+(int)(*sp_foo)]++;

  // Check that no implicit map is added if there is an explicit map.
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[SFOO]],
  // CHECK-NOT: "QUAL.OMP.MAP.TOFROM"(ptr [[SFOO]],
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

  // CHECK: [[P:%.+]] = alloca ptr,
  // CHECK: [[P_MAP:%.+]] = alloca ptr,
  // CHECK: [[P_MAP4:%.+]] = alloca ptr,
  // CHECK: [[P_MAP9:%.+]] = alloca ptr,
  // CHECK: [[L0:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L1:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L2:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F1:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L2]], i32 0, i32 0
  // CHECK: [[L3:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L4:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F2:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L4]], i32 0, i32 1
  // CHECK: [[L11:%[0-9]+]] = sdiv
  // CHECK: [[TV22:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]], ptr [[F1]], i64 [[L11]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L1]], ptr [[F1]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L3]], ptr [[F2]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[P_MAP]])
  // CHECK: store ptr [[L1]], ptr [[P_MAP]]
  // CHECK: load ptr, ptr [[P_MAP]]
  // CHECK: region.exit(token [[TV22]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1, p->f2)
  {
    p->f1 = 0;
  }
  // CHECK: [[L14:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L15:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L16:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F12:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L16]], i32 0, i32 0
  // CHECK: [[L17:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L18:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F23:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L18]], i32 0, i32 1
  // CHECK: [[L25:%[0-9]+]] = sdiv
  // CHECK: [[TV42:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L15]], ptr [[F12]], i64 [[L25]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L15]], ptr [[F12]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L17]], ptr [[F23]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[P_MAP4]])
  // CHECK: store ptr [[L15]], ptr [[P_MAP4]]
  // CHECK: load ptr, ptr [[P_MAP4]]
  // CHECK: region.exit(token [[TV42]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(p->f1) map(p->f2)
  {
    p->f2 = 0;
    p->f1 = 0;
  }

  // CHECK: [[L29:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L30:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L31:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F17:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L31]], i32 0, i32 0
  // CHECK: [[L32:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L33:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F28:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L33]], i32 0, i32 1
  // CHECK: [[L34:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[L35:%[0-9]+]] = load ptr, ptr %p.addr,
  // CHECK: [[F3:%.+]] =  getelementptr inbounds %struct.SP, ptr [[L35]], i32 0, i32 2
  // CHECK: [[L42:%[0-9]+]] = sdiv
  // CHECK: [[TV76:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L30]], ptr [[F17]], i64 [[L42]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L30]], ptr [[F17]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L32]], ptr [[F28]], i64 4, i64 281474976710659
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:ALWAYS.CHAIN"(ptr [[L34]], ptr [[F3]], i64 4, i64 281474976710663
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[P_MAP9]])
  // CHECK: store ptr [[L30]], ptr [[P_MAP9]]
  // CHECK: load ptr, ptr [[P_MAP9]]
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
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %this1, ptr %this1, i64 8, i64 1
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
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %this1, ptr %qqq
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %ppp, ptr %ppp
        #pragma omp target  map(qqq, ppp)
        {
            qqq = ppp;
        }
  // CHECK: [[T3:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %this1, ptr %qqq
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %ppp, ptr %ppp
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
