// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int);
// CHECK: define{{.*}}test1
void test1()
{
// CHECK: [[S_OUTER:%s_outer.*]] = alloca i32,
// CHECK: [[S_INNER:%s_inner.*]] = alloca i32,
// CHECK: [[S_OTHER_INNER:%s_other_inner.*]] = alloca i32,
// CHECK: [[P_ONE:%p_one.*]] = alloca i32,
   int s_outer = 2;
// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[S_INNER]]
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
   #pragma omp parallel
   {
     int s_inner = 3;
     int s_other_inner = 4;

// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[S_INNER]]
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
     #pragma omp parallel private(s_other_inner)
     {
       int p_one = 1;
       bar(s_outer+s_inner+p_one+s_other_inner);
     }

   }
}

// CHECK: define{{.*}}test2
// CHECK: [[T_OUTER:%t_outer.*]] = alloca i32,
// CHECK: [[T_OUTER2:%t_outer2.*]] = alloca i32,
// CHECK: [[T_INNER:%t_inner.*]] = alloca i32,
// CHECK: [[T_ONE:%t_one.*]] = alloca i32,
void test2()
{
  int t_outer = 2;
  int t_outer2 = 2;
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  //CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[T_OUTER2]])
  //CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_INNER]])
  #pragma omp parallel
  {
    //CHECK: region.entry() [ "DIR.OMP.TASK"()
    // CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_OUTER2]])
    // CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
    // CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
    // CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_INNER]])
    #pragma omp task private(t_outer2)
    {
      int t_inner = 4;
      //CHECK: region.entry() [ "DIR.OMP.TASK"()
      // CHECK-SAME-DAG: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
      // CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
      // CHECK-SAME-DAG: "QUAL.OMP.FIRSTPRIVATE"(i32* [[T_INNER]])
      // CHECK-SAME-DAG: "QUAL.OMP.FIRSTPRIVATE"(i32* [[T_OUTER2]])
      #pragma omp task
      {
        int t_one = 1;
        bar(t_outer+t_one+t_inner+t_outer2);
      }
    }
  }
}

// CHECK: define{{.*}}test3
void test3()
{
// CHECK: [[J:%j.*]] = alloca i32,
// CHECK: [[A:%a.*]] = alloca i32,
// CHECK: [[IV:%.omp.iv.*]] = alloca i32,
// CHECK: [[LB:%.omp.lb.*]] = alloca i32,
// CHECK: [[UB:%.omp.ub.*]] = alloca i32,
  int j,a=3;
// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
  #pragma omp parallel
  {
// CHECK: region.entry() [ "DIR.OMP.LOOP"()
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[J]])
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[IV]])
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LB]])
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[UB]])
    #pragma omp for schedule(static, 1) firstprivate(a)
    for ( j = 0; j < 10; j++ ) { }
  }
}

struct A {
  void test4_1(int i);
  void test4_2(int i);
  int val;
};

// CHECK: define{{.*}}test4_1
// CHECK: [[THISADDR:%this.addr.*]] = alloca{{.*}}A*,
// CHECK: [[IPARM:%iparm.*]] = alloca i32,
// CHECK: [[VARV:%varV.*]] = alloca i32,
// CHECK: [[AP:%ap.*]] = alloca{{.*}}A*,
// CHECK: [[LOCI:%loci.*]] = alloca i32,
// CHECK: [[LOCI6:%loci.*]] = alloca i32,
// CHECK: [[LOCI13:%loci.*]] = alloca i32,
// CHECK: [[THIS1:%this.*]] = load{{.*}}A*, {{.*}}[[THISADDR]],
void A::test4_1(int iparm)
{
  int varV = 0;
  A *ap = new A;
// CHECK: region.entry{{.*}}"DIR.OMP.TASK"
// CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[THIS1]]
  #pragma omp task
  {
    int loci = this->val;
    ap->test4_2(iparm+3+val+loci+varV);
  }
// CHECK: region.exit{{.*}}"DIR.OMP.END.TASK"

// CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
// CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[THIS1]]
  #pragma omp parallel
  {
    int loci = this->val;
    ap->test4_2(iparm+3+val+loci+varV);
  }
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"

// CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
// CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[THIS1]]
  #pragma omp parallel
  {
// CHECK: region.entry{{.*}}"DIR.OMP.TASK"
// CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[THIS1]]
    #pragma omp task
    {
      int loci = this->val;
      ap->test4_2(iparm+3+val+loci+varV);
    }
// CHECK: region.exit{{.*}}"DIR.OMP.END.TASK"
  }
// CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
}

int *get();
void target_test()
{
// CHECK: [[DATAPTR:%dataPtr.*]] = alloca i32*,
// CHECK: [[OTHER:%other.*]] = alloca i32,
// CHECK: [[X:%x.*]] = alloca i32,
  int *dataPtr = get();
  int data_size = 10;
  int other = 20;
// CHECK: region.entry{{.*}}"DIR.OMP.TARGET"
// CHECK-SAME-DAG: "QUAL.OMP.MAP"{{.*}}[[DATAPTR]]
// CHECK-SAME-DAG: "QUAL.OMP.FIRSTPRIVATE"{{.*}}[[OTHER]]
// CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"{{.*}}[[X]]
    #pragma omp target map(to: dataPtr[0:data_size])
    {
       int x = other + dataPtr[5];
    }
}

struct MyClass {
  int h;
  void execute();
};

void MyClass::execute() {
// CHECK: region.entry{{.*}}"DIR.OMP.TARGET"
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"{{.*}}this1
// CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
// CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}this
  #pragma omp target
  #pragma omp parallel
  {
    int yend =  h;
  }
}

#pragma omp declare target
struct vec2 {
    float x, y;
    inline vec2() { }
    inline vec2(float a)
        :x(a), y(a){}
    inline vec2(float a_x, float a_y)
        :x(a_x), y(a_y){}
    inline friend vec2 operator*(const vec2& v1, const vec2& v2) {
        return vec2(v1.x * v2.x, v1.y * v2.y );
    }
};

#pragma omp end declare target

class MyClass2 {
  int sz;
  int *dst;
  void execute_offload ();
};

void MyClass2::execute_offload() {
  //CHECK-DAG: [[REF_TMP:%ref.tmp.*]] = alloca %struct.vec2, align 4
  //CHECK-DAG: [[RESOLUTION:%resolution.*]] = alloca %struct.vec2, align 4
  int *_dst_ = dst;

  //CHECK: region.entry{{.*}}"DIR.OMP.TARGET"
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"{{.*}}[[REF_TMP]]
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"{{.*}}[[RESOLUTION]]
  //CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL.LOOP"
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"{{.*}}[[REF_TMP]]
  //CHECK-SAME-DAG: "QUAL.OMP.PRIVATE"{{.*}}[[RESOLUTION]]
  #pragma omp target map(from: _dst_[0:sz])
  #pragma omp parallel for
  for (int y = 0 ; y < 10 ; y++) {
    //CHECK: call {{.*}}(%struct.vec2* noundef {{.*}} [[RESOLUTION]], float noundef 2.000000e+00)
    //CHECK: call {{.*}}(%struct.vec2* noundef {{.*}} [[REF_TMP]], float noundef 1.000000e+00)
    vec2 resolution(2.0f);
    vec2 vPos (1.0f * resolution);
    _dst_[ 5 ] = 1234;
  }
}
// end INTEL_COLLAB
