// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | \
// RUN:  FileCheck --check-prefixes CHECK,CHECK-NEW %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s -fno-openmp-new-depend-ir | \
// RUN:  FileCheck --check-prefixes CHECK,CHECK-OLD %s

//CHECK-LABEL: foo_target_teams
void foo_target_teams(long long int n)
{
  int local = 2;
  //CHECK: [[N_ADDR:%n.*]] = alloca i64,
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[TMP:%omp.clause.tmp.*]] = alloca i32,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[N_ADDR]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[N_ADDR]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 42)
  #pragma omp target
  #pragma omp teams num_teams(n) thread_limit(42)
  { }

  //CHECK: load i32, ptr [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[TMP]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP]]
  #pragma omp target
  #pragma omp teams num_teams(local) thread_limit(local+2)
  { }
}

//CHECK-LABEL: foo_target_teams_combined
void foo_target_teams_combined(long long int n)
{
  int local = 2;

  //CHECK: [[N_ADDR:%n.*]] = alloca i64,
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[CAP3:%.capture_expr.*]] = alloca i32,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 42)
  #pragma omp target teams num_teams(n) thread_limit(42)
  { }

  //CHECK: load i32, ptr [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[CAP3]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP3]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[CAP3]]
  #pragma omp target teams num_teams(local) thread_limit(local+2)
  { }
}

//CHECK-LABEL: bar
void bar()
{
  int local = 6;
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[TMP1:%omp.clause.tmp.*]] = alloca i32,
  //CHECK: [[TMP2:%omp.clause.tmp.*]] = alloca i32,
  //CHECK: [[TMP3:%omp.clause.tmp.*]] = alloca i32,
  //CHECK: [[TMP4:%omp.clause.tmp.*]] = alloca i32,
  //CHECK: [[CAP1:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP2:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP3:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP4:%.capture_expr.*]] = alloca i32,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP1]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP1]]
  #pragma omp target
  #pragma omp teams distribute num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP2]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP2]]
  #pragma omp target
  #pragma omp teams distribute simd num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP3]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP3]]
  #pragma omp target
  #pragma omp teams distribute parallel for \
                       num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[TMP4]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[TMP4]]
  #pragma omp target
  #pragma omp teams distribute parallel for simd \
                      num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP1]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  #pragma omp target teams distribute num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP2]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[CAP2]]
  #pragma omp target teams distribute simd \
                            num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP3]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[CAP3]]
  #pragma omp target teams distribute parallel for \
                             num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP4]]
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr [[LOCAL]]
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr [[CAP4]]
  #pragma omp target teams distribute parallel for simd \
                             num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }
}

//CHECK-LABEL: bar
void barfoo()
{
  int local = 6;
  //CHECK: [[LO:%.+]] = alloca i32,
  //CHECK: [[CAP1:%.capture_expr.*]] = alloca i32,
  //CHECK: [[T:%tmp]] = alloca i32,
  //CHECK: [[LB:%.omp.lb]] = alloca i32,
  //CHECK: [[UB:%.omp.ub]] = alloca i32,
  //CHECK: [[I:%i]] = alloca i32,
  //CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  //CHECK: "DIR.OMP.TASK"()
  //CHECK-SAME: "QUAL.OMP.IF"
  //CHECK-SAME: "QUAL.OMP.TARGET.TASK"()
  //CHECK-OLD-SAME: "QUAL.OMP.DEPEND.IN"(ptr [[LO]])
  //CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DARR]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP1]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 12)
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[T]]
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP1]]
  #pragma omp target teams distribute num_teams(local) thread_limit(local+2) depend(in:local)
  for(int i=0; i<16; ++i) { }
}

struct C {
  int NT;
  void run0();
  void run1();
};

//CHECK: define{{.*}}run0
void C::run0() {
  //CHECK: "DIR.OMP.TARGET"
  //CHECK: "DIR.OMP.TEAMS"
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr
  #pragma omp target teams loop num_teams(NT)
  for (auto i = 0; i < NT; ++i) {
  }
}
//CHECK: define{{.*}}run1
void C::run1() {
  //CHECK: "DIR.OMP.TARGET"
  //CHECK: "DIR.OMP.TEAMS"
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS:TYPED"(ptr
  #pragma omp target teams distribute parallel for num_teams(NT)
  for (auto i = 0; i < NT; ++i) {
  }
}
// end INTEL_COLLAB
