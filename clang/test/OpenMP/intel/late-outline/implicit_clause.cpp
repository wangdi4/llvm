// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
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
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[S_INNER]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
   #pragma omp parallel
   {
     int s_inner = 3;
     int s_other_inner = 4;

// CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[S_OTHER_INNER]]
// CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[S_OUTER]]
// CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[S_INNER]]
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[P_ONE]]
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
  //CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[T_OUTER2]])
  //CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_INNER]])
  #pragma omp parallel
  {
    //CHECK: region.entry() [ "DIR.OMP.TASK"()
    // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_OUTER2]])
    // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
    // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
    // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_INNER]])
    #pragma omp task private(t_outer2)
    {
      int t_inner = 4;
      //CHECK: region.entry() [ "DIR.OMP.TASK"()
      // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[T_OUTER]])
      // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T_ONE]])
      // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[T_INNER]])
      // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[T_OUTER2]])
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
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LB]])
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[IV]])
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[UB]])
// CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[J]])
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
// end INTEL_COLLAB
