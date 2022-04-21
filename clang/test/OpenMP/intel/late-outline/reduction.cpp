// INTEL_COLLAB
//RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -fopenmp-version=50 %s | FileCheck %s

int bar(int i);
void cmplx(_Complex double x);
//CHECK: define{{.*}}foo1
//CHECK: [[N1_ADDR:%.+]] = alloca i64,
//CHECK: [[N2_ADDR:%.+]] = alloca i64,
//CHECK: [[N3_ADDR:%.+]] = alloca i64,
//CHECK: [[N4_ADDR:%.+]] = alloca i64,
//CHECK: [[N5_ADDR:%.+]] = alloca i64,
//CHECK: [[N6_ADDR:%.+]] = alloca i64,
//CHECK: [[N7_ADDR:%.+]] = alloca i64,
//CHECK: [[N8_ADDR:%.+]] = alloca i64,
//CHECK: [[N9_ADDR:%.+]] = alloca i64,
//CHECK: [[N10_ADDR:%.+]] = alloca i64,
//CHECK: [[X_ADDR:%.+]] = alloca { double, double },
void foo1(int *d) {
  long int n1=0, n2=1000, n3=1, n4=982, n5=0,
           n6=0, n7=0, n8=0, n9=0, n10=999;
  double _Complex x;

//CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKGROUP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr [[N1_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.SUB"(ptr [[N2_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(ptr [[N3_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BAND"(ptr [[N4_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BOR"(ptr [[N5_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BXOR"(ptr [[N6_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.AND"(ptr [[N7_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.OR"(ptr [[N8_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MAX"(ptr [[N9_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MIN"(ptr [[N10_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:CMPLX"(ptr [[X_ADDR]])

  #pragma omp taskgroup \
               task_reduction(+:n1)   \
               task_reduction(-:n2)   \
               task_reduction(*:n3)   \
               task_reduction(&:n4)   \
               task_reduction(|:n5)   \
               task_reduction(^:n6)   \
               task_reduction(&&:n7)  \
               task_reduction(||:n8)  \
               task_reduction(max:n9) \
               task_reduction(min:n10) \
               task_reduction(+:x)
  {
//CHECK: [[TOK1:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.PARALLEL"()
    #pragma omp parallel
    {
//CHECK: [[TOK2:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(ptr [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK2]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(+:n1)
      n1 += d[0];

//CHECK: [[TOK6:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(ptr [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK6]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(+:n1)
      n1 += d[n1];

//CHECK: [[TOK11:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.SUB"(ptr [[N2_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK11]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(-:n2)
      n2 -= d[n2];

//CHECK: [[TOK16:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MUL"(ptr [[N3_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK16]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(*:n3)
      n3 = bar(3);

//CHECK: [[TOK17:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BAND"(ptr [[N4_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK17]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(&:n4)
      n4 = bar(4);

//CHECK: [[TOK18:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BOR"(ptr [[N5_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK18]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(|:n5)
      n5 = bar(5);

//CHECK: [[TOK19:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BXOR"(ptr [[N6_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK19]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(^:n6)
      n6 = bar(6);

//CHECK: [[TOK20:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.AND"(ptr [[N7_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK20]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(&&:n7)
      n7 = bar(7);

//CHECK: [[TOK21:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.OR"(ptr [[N8_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK21]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(||:n8)
      n8 = bar(8);

//CHECK: [[TOK22:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MAX"(ptr [[N9_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK22]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(max:n9)
      n9 = bar(9);

//CHECK: [[TOK23:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MIN"(ptr [[N10_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK23]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(min:n10)
      n10 = bar(10);
//CHECK: [[TOK24:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD:CMPLX"(ptr [[X_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK24]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(+:x)
        cmplx(x);
    }
//CHECK: call void @llvm.directive.region.exit(token [[TOK1]])
//CHECK-SAME: [ "DIR.OMP.END.PARALLEL"() ]
  }
//CHECK: call void @llvm.directive.region.exit(token [[TOK0]])
//CHECK-SAME: [ "DIR.OMP.END.TASKGROUP"() ]
}

//CHECK: define{{.*}}foo2
//CHECK: [[N1_ADDR:%.+]] = alloca i64,
//CHECK: [[N2_ADDR:%.+]] = alloca i64,
//CHECK: [[N3_ADDR:%.+]] = alloca i64,
//CHECK: [[N4_ADDR:%.+]] = alloca i64,
//CHECK: [[N5_ADDR:%.+]] = alloca i64,
//CHECK: [[N6_ADDR:%.+]] = alloca i64,
//CHECK: [[N7_ADDR:%.+]] = alloca i64,
//CHECK: [[N8_ADDR:%.+]] = alloca i64,
//CHECK: [[N9_ADDR:%.+]] = alloca i64,
//CHECK: [[N10_ADDR:%.+]] = alloca i64,
//CHECK: [[X_ADDR:%.+]] = alloca { double, double },
void foo2(int *d) {
  long int n1=0, n2=1000, n3=1, n4=982, n5=0,
           n6=0, n7=0, n8=0, n9=0, n10=999;
  double _Complex x;
//CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKGROUP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr [[N1_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.SUB"(ptr [[N2_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(ptr [[N3_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BAND"(ptr [[N4_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BOR"(ptr [[N5_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BXOR"(ptr [[N6_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.AND"(ptr [[N7_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.OR"(ptr [[N8_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MAX"(ptr [[N9_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MIN"(ptr [[N10_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MUL:CMPLX"(ptr [[X_ADDR]])
  #pragma omp taskgroup \
               task_reduction(+:n1)   \
               task_reduction(-:n2)   \
               task_reduction(*:n3)   \
               task_reduction(&:n4)   \
               task_reduction(|:n5)   \
               task_reduction(^:n6)   \
               task_reduction(&&:n7)  \
               task_reduction(||:n8)  \
               task_reduction(max:n9) \
               task_reduction(min:n10) \
               task_reduction(*:x)
  {
//CHECK: [[TOK6:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(ptr [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK6]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(+:n1) nogroup
  for(int i=0;i<20;++i)
    n1 += bar(1);

//CHECK: [[TOK71:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.SUB"(ptr [[N2_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK71]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(-:n2) nogroup
  for(int i=0;i<20;++i)
    n2 += bar(2);

//CHECK: [[TOK81:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MUL"(ptr [[N3_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK81]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(*:n3) nogroup
  for(int i=0;i<20;++i)
    n3 += bar(3);

//CHECK: [[TOK91:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BAND"(ptr [[N4_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK91]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(&:n4) nogroup
  for(int i=0;i<20;++i)
    n4 += bar(4);

//CHECK: [[TOK101:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BOR"(ptr [[N5_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK101]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(|:n5) nogroup
  for(int i=0;i<20;++i)
    n5 += bar(5);

//CHECK: [[TOK111:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BXOR"(ptr [[N6_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK111]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(^:n6) nogroup
  for(int i=0;i<20;++i)
    n6 += bar(6);

//CHECK: [[TOK121:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.AND"(ptr [[N7_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK121]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(&&:n7) nogroup
  for(int i=0;i<20;++i)
    n7 += bar(7);

//CHECK: [[TOK131:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.OR"(ptr [[N8_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK131]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(||:n8) nogroup
  for(int i=0;i<20;++i)
    n8 += bar(8);

//CHECK: [[TOK141:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MAX"(ptr [[N9_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK141]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(max:n9) nogroup
  for(int i=0;i<20;++i)
    n9 += bar(9);

//CHECK: [[TOK151:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MIN"(ptr [[N10_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK151]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(min:n10) nogroup
  for(int i=0;i<20;++i)
    n10 += bar(10);
  }
//CHECK: call void @llvm.directive.region.exit(token [[TOK0]])
//CHECK-SAME: [ "DIR.OMP.END.TASKGROUP"() ]

//CHECK: [[TOK16:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr [[N9_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr [[N10_ADDR]])
  #pragma omp parallel for reduction(+ : n9, n10)
  for (int i = 0; i < 10; i++) {
    n9++;
    n10++;
  }
//CHECK: call void @llvm.directive.region.exit(token [[TOK16]])
//CHECK-SAME: [ "DIR.OMP.END.PARALLEL.LOOP"() ]

//CHECK: [[TOK171:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MUL:CMPLX"(ptr [[X_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK171]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(*:x) nogroup
  for(int i=0;i<20;++i)
    cmplx(x);
}

struct A {
  int m, a;
  A(int x=0, int y=0) :m(x),   a(y) {}
  A(const A& p)       :m(p.m), a(p.a) {}
  ~A() {m=-1;a=-1;}
};

struct B {
  int m, a;
  B(int x=0, int y=0) :m(x),   a(y) {}
  B(const A& p)       :m(p.m), a(p.a) {}
  ~B() {m=-1;a=-1;}
};

void init_an_A(B* ap, int m_init, int a_init) { ap->m=m_init,ap->a=a_init; }
#pragma omp declare reduction(myred1:A:omp_out.m*=omp_in.m) \
                              initializer(omp_priv = A(1,0))
#pragma omp declare reduction(myred2:B:omp_out.m*=omp_in.m) \
                              initializer(init_an_A(&omp_priv,1,0))
#pragma omp declare reduction(myred3:B:omp_out.m*=omp_in.m) \
                              initializer(omp_priv = B(1))


#pragma omp declare reduction(myred4:A:omp_out.m*=omp_in.m) \
                              initializer(omp_priv = omp_orig)
#pragma omp declare reduction(myred5:A:omp_out.m*=omp_in.m) \
                              initializer(omp_priv = A(omp_orig.m+1))
#pragma omp declare reduction(myred6:A:omp_out.m*=omp_in.m)

//CHECK: define{{.*}}foo3
//CHECK: [[N1_ADDR:%.+]] = alloca i32,
//CHECK: [[N2_ADDR:%.+]] = alloca %struct.A,
//CHECK: [[N3_ADDR:%.+]] = alloca %struct.B,
//CHECK: [[N4_ADDR:%.+]] = alloca %struct.B,
//CHECK: [[N5_ADDR:%.+]] = alloca %struct.B,
void foo3() {
  int i;
  A a1(1,0);
  B a2(1), a3(2);
  B a4; init_an_A(&a4, 1, 0);
// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr [[N5_ADDR]],
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1B.omp.destr,
// CHECK-SAME: ptr @.omp_combiner.,
// CHECK-SAME: ptr @.omp_initializer.)
  #pragma omp parallel  reduction(myred2: a4)
  for(i=1; i<10; i++) {
  }
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr [[N2_ADDR]],
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1A.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..1,
// CHECK-SAME: ptr @.omp_initializer..2),
  #pragma omp parallel  reduction(myred1:a1)
  for(i=1; i<10; i++) {
    a1.m = 10;
  }
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr [[N3_ADDR]],
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1B.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..3,
// CHECK-SAME: ptr @.omp_initializer..4),
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr [[N4_ADDR]],
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1B.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..3,
// CHECK-SAME: ptr @.omp_initializer..4),
  #pragma omp parallel  reduction(myred3: a2,a3) private(a1)
  for(i=1; i<10; i++) {
  }
// CHECK: [ "DIR.OMP.END.PARALLEL"() ]
// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %a1,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1A.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..5
// CHECK-SAME: ptr @.omp_initializer..6),
  #pragma omp parallel  reduction(myred4: a1)
  for(i=1; i<10; i++);
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %a1,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS1A.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..7
// CHECK-SAME: ptr @.omp_initializer..8),
  #pragma omp parallel  reduction(myred5: a1)
  for(i=1; i<10; i++);
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %a1,
// CHECK-SAME: ptr @_ZTS1A.omp.def_constr,
// CHECK-SAME: ptr @_ZTS1A.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..9
// CHECK-SAME: ptr null),
  #pragma omp parallel  reduction(myred6: a1)
  for(i=1; i<10; i++);
}

// CHECK: define internal void @.omp_initializer.(ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_Z9init_an_AP1Bii(ptr noundef %3, i32 noundef 1, i32 noundef 0)
// CHECK-NEXT: ret void

// CHECK: define internal void @.omp_initializer..2(ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_ZN1AC1Eii(ptr noundef {{.*}} %3, i32 noundef 1, i32 noundef 0)
// CHECK-NEXT: ret void

// CHECK: define internal void @.omp_initializer..4(ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_ZN1BC1Eii(ptr noundef {{.*}} %3, i32 noundef 1, i32 noundef 0)
// CHECK-NEXT: ret void

int con, des;

typedef struct Point {
   Point(int p1=0, int p2=0) : x(p1), y(p2) {con++;}
   Point(const Point& p) : x(p.x),y(p.y) {con++;}
   ~Point() {des++;}
   int x, y;
} Point;

#pragma omp declare reduction (pointMax: Point : omp_out.x =  omp_in.x) \
                               initializer(omp_priv = Point(1, 10000))

#pragma omp declare reduction (pointMin: Point : \
            omp_out.y = omp_out.y < omp_in.y ? omp_out.y : omp_in.y ) \
                          initializer(omp_priv = Point(1, 1000))

//CHECK: define{{.*}}findMinMax
void findMinMax(Point* points, int n, Point* minPoint, Point* maxPoint) {
  Point lmaxPoint(points[0].x, points[0].y);
  Point lminPoint(points[0].x, points[0].y);
  Point *pMax = &lmaxPoint, *pMin = &lminPoint;

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %lminPoint,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS5Point.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..10,
// CHECK-SAME: ptr @.omp_initializer..11),
  #pragma omp parallel for reduction(pointMin:lminPoint)
  for (int i = 1; i < n; i++) {}
//CHECK: [ "DIR.OMP.END.PARALLEL.LOOP"() ]
}

// CHECK: define internal void @.omp_initializer..11(ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_ZN5PointC1Eii(ptr noundef {{.*}} %3, i32 noundef 1, i32 noundef 1000)
// CHECK-NEXT: ret void

int dcnt;
struct B1 { int a; };
struct A1 {
  operator B1() { B1 bvar; bvar.a = a - 5; return bvar; }
  int a,b,c;
} avar1;
A1 boo1() { return avar1; }
#pragma omp declare \
        reduction(myred1:B1:omp_out.a*=omp_in.a) \
                            initializer(omp_priv = boo1())
struct A2 {
  int a,b,c;
} avar2;
struct B2 {
  int a; B2() {}
  B2(A2 av) : a(av.a-5) {}
};
A2 boo2() { return avar2; }
#pragma omp declare \
        reduction(myred2:B2:omp_out.a*=omp_in.a) \
                            initializer(omp_priv = boo2())
struct B3 { int a; };
struct A3 { ~A3() { dcnt++; }
            operator B3() { B3 b; b.a = a - 5; return b; }
            int a,b,c; } avar3;
A3 boo3() { return avar3; }
#pragma omp declare \
        reduction(myred3:B3:omp_out.a*=omp_in.a) \
                            initializer(omp_priv = boo3())
struct A4 { ~A4() { dcnt++; } int a,b,c; } avar4;
struct B4 { int a; B4() {} B4(A4 av) : a(av.a-5) {} };
A4 boo4() { return avar4; }
int i = 5;
#pragma omp declare \
        reduction(myred4:B4:omp_out.a*=omp_in.a) \
                            initializer(omp_priv = boo4())
#pragma omp declare \
        reduction(myred4a:A4:omp_out.a*=omp_in.a) \
                            initializer(omp_priv = boo4())

//CHECK: define{{.*}}foo4
void foo4() {
  int i;
  B1 bvar1;
  B2 bvar2;
  B3 bvar3;
  B4 bvar4;
  A4 avar4a;

// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %bvar1,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @.omp_combiner..
// CHECK-SAME: ptr @.[[I1:omp_initializer..[0-9]*]]),
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %bvar2,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @.omp_combiner..
// CHECK-SAME: ptr @.[[I2:omp_initializer..[0-9]*]]),
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %bvar3,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @.omp_combiner..
// CHECK-SAME: ptr @.[[I3:omp_initializer..[0-9]*]]),
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %bvar4,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @.omp_combiner..
// CHECK-SAME: ptr @.[[I4:omp_initializer..[0-9]*]]),
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %avar4a,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS2A4.omp.destr,
// CHECK-SAME: ptr @.omp_combiner..
// CHECK-SAME: ptr @.[[I5:omp_initializer..[0-9]*]])
  #pragma omp parallel \
               reduction(myred1:bvar1)   \
               reduction(myred2:bvar2)   \
               reduction(myred3:bvar3)   \
               reduction(myred4:bvar4)   \
               reduction(myred4a:avar4a)
  {
  }
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]
}

// CHECK: define internal void @.[[I1]](ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: [[A0:%.addr.*]] = alloca ptr,
// CHECK: [[A1:%.addr.*]] = alloca ptr,
// CHECK: [[REFTMP:%ref.tmp.*]] = alloca %struct.A1,
// CHECK: [[LA1:%[0-9]+]] = load ptr, ptr [[A1]]
// CHECK: [[LA0:%[0-9]+]] = load ptr, ptr [[A0]]
// CHECK: [[CALL:%call.*]] = call i32 @_ZN2A1cv2B1Ev(ptr {{[^,]*}} [[REFTMP]]
// CHECK-NEXT: [[CO:%coerce.dive.*]] = getelementptr inbounds %struct.B1, ptr [[LA0]], i32 0, i32 0
// CHECK-NEXT: store i32 [[CALL]], ptr [[CO]], align 4
// CHECK-NEXT: ret void

// CHECK: define internal void @.[[I2]](ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_ZN2B2C1E2A2(ptr
// CHECK: ret void

// CHECK: define internal void @.[[I3]](ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK:call void @_ZN2A3D1Ev(ptr
// CHECK-NEXT ret void


// CHECK: define internal void @.[[I4]](ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: [[A0:%.addr.*]] = alloca ptr,
// CHECK: [[A1:%.addr.*]] = alloca ptr,
// CHECK: [[AGGTMP:%agg.tmp.*]] = alloca %struct.A4
// CHECK: [[LA1:%[0-9]+]] = load ptr, ptr [[A1]]
// CHECK: [[LA0:%[0-9]+]] = load ptr, ptr [[A0]]
// CHECK: call void @_ZN2B4C1E2A4(ptr noundef {{.*}}[[LA0]], ptr noundef [[AGGTMP]])
// CHECK-NEXT: call void @_ZN2A4D1Ev(ptr {{[^,]*}}[[AGGTMP]])
// CHECK-NEXT: ret void

// CHECK: define internal void @.[[I5]](ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: [[A0:%.addr.*]] = alloca ptr,
// CHECK: [[A1:%.addr.*]] = alloca ptr,
// CHECK: [[LA1:%[0-9]+]] = load ptr, ptr [[A1]]
// CHECK: [[LA0:%[0-9]+]] = load ptr, ptr [[A0]]
// CHECK: call void @_Z4boo4v(ptr sret(%struct.A4) {{(align 4 )?}}[[LA0]])
// CHECK-NEXT: ret void


int x;
struct AA3 {AA3() {x++;}
           ~AA3() {x++; }
           int a,b,c; } avar;
AA3 boo(AA3*) { return avar;}
#pragma omp declare reduction(myred6:AA3:omp_out.a*=omp_in.a) \
                              initializer(boo(&omp_priv))
//CHECK: define{{.*}}foo5
void foo5()
{
  AA3 bvar3;
// CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %bvar3,
// CHECK-SAME: ptr null,
// CHECK-SAME: ptr @_ZTS3AA3.omp.destr,
// CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
// CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  #pragma omp parallel reduction(myred6:bvar3)
  {
  }
//CHECK: [ "DIR.OMP.END.PARALLEL"() ]
}

// CHECK: define internal void @.omp_initializer..{{[0-9]+}}(ptr noalias noundef %0, ptr noalias noundef %1)
// CHECK: call void @_Z3booP3AA3(ptr sret(%struct.AA3) {{(align 4 )?}}%agg.tmp.ensured, ptr noundef %3)
// CHECK-NEXT: call void @_ZN3AA3D1Ev(ptr {{[^,]*}} %agg.tmp.ensured)
// CHECK-NEXT: ret void

struct Foo { int i; };
void init_Foo(Foo *f) { f->i = 0; }
#pragma omp declare reduction (+:Foo:omp_out.i+=omp_in.i) \
                                      initializer(init_Foo(&omp_priv))
#pragma omp declare reduction (add:int,long,double:omp_out += omp_in) \
                                      initializer(omp_priv = 0)
#pragma omp declare reduction (add2:int,long,double:omp_out += omp_in)

typedef int myint;
typedef myint redint;
#pragma omp declare reduction (add3:redint:omp_out += omp_in)

void test1()
{
  int j;

  /* predefined 'int', user defined 'Foo'. */
  int z = 0;
  Foo f;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %f,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr %z),
  #pragma omp parallel reduction(+:f,z)
  for (j = 0; j < 10; ++j);
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr %z),
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %f,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  #pragma omp parallel reduction(+:z,f)
  for (j = 0; j < 10; ++j);
// CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  Foo f1;
  int z1 = 0;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr %z1),
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %f1,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  #pragma omp parallel reduction(+:z1) reduction(+:f1)
  for (j = 0; j < 10; ++j);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  Foo f2;
  int z2 = 0;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %f2,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr %z2),
  #pragma omp parallel reduction(+:f2) reduction(+:z2)
  for (j = 0; j < 10; ++j);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  double dz = 0.0;
  long lz = 0;
  int z3 = 0;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %dz,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %lz,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr [[INIT:@.omp_initializer..[0-9]*]]
  // CHECK-SAME: QUAL.OMP.REDUCTION.UDR"(ptr %z3,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr  [[INIT:@.omp_initializer..[0-9]*]]
  #pragma omp parallel reduction(add:dz,lz,z3)
  for (j = 0; j < 10; ++j);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]

  double dz1 = 0.0;
  long lz1 = 0;
  int z4 = 0;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %dz1,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %lz1,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %z4,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  #pragma omp parallel reduction(add2:dz1,lz1,z4)
  for (j = 0; j < 10; ++j);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]

  int z5 = 0;
  myint z6 = 0;
  // CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %z5,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %z6,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr null,
  // CHECK-SAME: ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  #pragma omp parallel reduction(add3:z5,z6)
  for (j = 0; j < 10; ++j);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
}
// CHECK: define{{.*}}[[INIT]]
volatile double g, g_orig;
volatile double &g1 = g_orig;
template <class T> struct S {
  T f;
  S(T a) : f(a + g) {}
  S() : f(g) {}
  operator T() { return T(); }
  S &operator&(const S &) { return *this; }
  ~S() {}
};

// CHECK: define{{.*}}tmain
template <typename T, int length>
T tmain()
{
  T t;
  S<T> test;
  T t_var = T(), t_var1;
  T vec[] = {1, 2} ;
  S<T> s_arr[] = {1, 2};
  S<T> &var = test;
  S<T> var1;
  S<T> arr[length];
  // CHECK: [[T0:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL"
  // CHECK: [[T1:%[0-9]*]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.LOOP"
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %arr,
  // CHECK-SAME:  ptr [[COMB1:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR:BYREF"(ptr %var,
  // CHECK-SAME: ptr [[COMB2:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(ptr %var1,
  // CHECK-SAME: ptr [[COMB3:@.omp_combiner..[0-9]*]]
  // CHECK-SAME: ptr null)
#pragma omp parallel
#pragma omp for reduction(&:arr, var, var1)
  for (int i = 0; i < 2; ++i) {
  }
  // CHECK: [ "DIR.OMP.END.LOOP"() ]
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  return T();
}

//CHECK: define {{.*}}[[COMB1]]
//CHECK: define {{.*}}[[COMB2]]
//CHECK: define {{.*}}[[COMB3]]

int main()
{
  return tmain<int, 42>();
}

typedef struct my_struct {
  int a;
  int b;
} TYPE;
bool operator<(const TYPE &t1, const TYPE &t2) { return (t1.a < t2.a) || (t1.b < t2.b); }
void my_add(TYPE &lhs, TYPE const &rhs) {
  lhs.a += rhs.a;
  lhs.b += rhs.b;
}
#pragma omp declare reduction(my_reduction_add:TYPE \
                              : my_add(omp_out, omp_in))

void test2(TYPE ypas[100], int N) {
  #pragma omp parallel reduction(my_reduction_add:ypas[:])
  // CHECK: call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR:ARRSECT"(ptr %ypas.addr,
  // CHECK-SAME: ptr @_ZTS9my_struct.omp.def_constr,
  // CHECK-SAME: ptr @_ZTS9my_struct.omp.destr,
  for (int i = 0; i < 100; i++);
  // CHECK: [ "DIR.OMP.END.PARALLEL"() ]
  #pragma omp parallel reduction(my_reduction_add:ypas[1:N])
  // CHECK: call token @llvm.directive.region.entry()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR:ARRSECT"(ptr %ypas.addr,
  // CHECK-SAME: ptr @_ZTS9my_struct.omp.def_constr,
  for (int i = 0; i < 100; i++);
}

// end INTEL_COLLAB
