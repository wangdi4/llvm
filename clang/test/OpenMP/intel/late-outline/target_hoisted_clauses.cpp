// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//CHECK-LABEL: foo_target_teams
void foo_target_teams(long long int n)
{
  int local = 2;
  //CHECK: [[N_ADDR:%n.*]] = alloca i64,
  //CHECK: [[LOCAL:%local.*]] = alloca i32,
  //CHECK: [[TMP:%omp.clause.tmp.*]] = alloca i32,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[N_ADDR]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i64* [[N_ADDR]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 42)
  #pragma omp target
  #pragma omp teams num_teams(n) thread_limit(42)
  { }

  //CHECK: load i32, i32* [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[TMP]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[TMP]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[TMP]])
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
  //CHECK: [[CAP1:%.capture_expr.*]] = alloca i64,
  //CHECK: [[CAP2:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP3:%.capture_expr.*]] = alloca i32,

  //CHECK: load i64, i64* [[N_ADDR]]
  //CHECK-NEXT: store{{.*}}[[CAP1]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[CAP1]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i64* [[CAP1]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32 42)
  #pragma omp target teams num_teams(n) thread_limit(42)
  { }

  //CHECK: load i32, i32* [[LOCAL]]
  //CHECK-NEXT: store{{.*}}[[CAP2]]
  //CHECK: load i32, i32* [[LOCAL]]
  //CHECK-NEXT: add{{.*}}2
  //CHECK-NEXT: store{{.*}}[[CAP3]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP2]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP3]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[CAP2]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[CAP3]])
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
  //CHECK: [[CAP5:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP6:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP7:%.capture_expr.*]] = alloca i32,
  //CHECK: [[CAP8:%.capture_expr.*]] = alloca i32,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[TMP1]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[TMP1]])
  #pragma omp target
  #pragma omp teams distribute num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[TMP2]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[TMP2]])
  #pragma omp target
  #pragma omp teams distribute simd num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[TMP3]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[TMP3]])
  #pragma omp target
  #pragma omp teams distribute parallel for \
                       num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[TMP4]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[LOCAL]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[TMP4]])
  #pragma omp target
  #pragma omp teams distribute parallel for simd \
                      num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP1]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP2]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[CAP1]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[CAP2]])
  #pragma omp target teams distribute num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP3]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP4]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[CAP3]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[CAP4]])
  #pragma omp target teams distribute simd \
                            num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP5]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP6]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[CAP5]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[CAP6]])
  #pragma omp target teams distribute parallel for \
                             num_teams(local) thread_limit(local+2)
  for(int i=0; i<16; ++i) { }

  local++;
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP7]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP8]])
  //CHECK: "DIR.OMP.TEAMS"()
  //CHECK-SAME: "QUAL.OMP.NUM_TEAMS"(i32* [[CAP7]])
  //CHECK-SAME: "QUAL.OMP.THREAD_LIMIT"(i32* [[CAP8]])
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
  //CHECK: [[CAP2:%.capture_expr.*]] = alloca i32,
  //CHECK: [[T:%tmp]] = alloca i32,
  //CHECK: [[LB:%.omp.lb]] = alloca i32,
  //CHECK: [[UB:%.omp.ub]] = alloca i32,
  //CHECK: [[I:%i]] = alloca i32,
  //CHECK: "DIR.OMP.TASK"()
  //CHECK-SAME: "QUAL.OMP.IF"
  //CHECK-SAME: "QUAL.OMP.TARGET.TASK"()
  //CHECK-SAME: "QUAL.OMP.DEPEND.IN"(i32* [[LO]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP1]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP2]])
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 12)
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[T]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP1]])
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[CAP2]])
  #pragma omp target teams distribute num_teams(local) thread_limit(local+2) depend(in:local)
  for(int i=0; i<16; ++i) { }
}
// end INTEL_COLLAB
