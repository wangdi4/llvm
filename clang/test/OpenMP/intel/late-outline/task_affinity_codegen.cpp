// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu -fopenmp-version=51 %s | \
// RUN: FileCheck --check-prefix=CHECK %s

// CHECK-LABEL: @{{.*}}foo
void foo() {
  float *p;
  int a = 10;

  // CHECK: [[P:%.+]] = alloca ptr, align 8
  // CHECK: [[A:%.+]] = alloca i32, align 4
  // CHECK: [[AFFS_ADDR:%.+]] = alloca [1 x %struct.kmp_task_affinity_info_t], align 8
  // CHECK: [[SAVED_STK:%.*saved_stack.*]] = alloca ptr, align 8
  // CHECK: [[VLA0:%.*vla_expr.*]] = alloca i64, align 8
  // CHECK: [[AFF_COUNTER:%.*affs.counter.addr.*]] = alloca i64, align 8
  // CHECK: [[I:%.+]] = alloca i32, align 4
  // CHECK: [[COUNTER_ADDR:%.*counter.addr.*]] = alloca i32, align 4
  // CHECK: store i32 10, ptr [[A]], align 4
  // CHECK: [[AFFS_LST_ADDR:%.+]] = getelementptr inbounds [1 x %struct.kmp_task_affinity_info_t], ptr [[AFFS_ADDR]], i64 0, i64 0
  
  // CHECK: [[TMP1:%[0-9]+]] = load ptr, ptr %p, align 8
  // CHECK: [[TMP2:%[0-9]+]] = load i32, ptr %a, align 4
  // CHECK: [[CONV:%.*conv.*]] = sext i32 [[TMP2]] to i64
  // CHECK: [[TMP3:%[0-9]+]] = mul nuw i64 4, [[CONV]]
  // CHECK: [[TMP4:%[0-9]+]] = mul nuw i64 [[TMP3]], 10
  // CHECK: [[TMP5:%[0-9]+]] = load i32, ptr [[A]], align 4
  // CHECK: [[CONV1:%.*conv.*]] = sext i32 [[TMP5]] to i64
  // CHECK: [[SIZE:%[0-9]+]] = mul nuw i64 [[TMP4]], %conv1
  // CHECK: [[AFFS_0_ADDR:%[0-9]+]] = getelementptr %struct.kmp_task_affinity_info_t, ptr [[AFFS_LST_ADDR]], i64 0
  
  // affs[0].base = p;
  // CHECK: [[AFFS_0_BASE_ADDR:%[0-9]+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_0_ADDR]], i32 0, i32 0
  // CHECK: [[P_INTPTR:%[0-9]+]] = ptrtoint ptr [[TMP1]] to i64
  // CHECK: store i64 [[P_INTPTR]], ptr [[AFFS_0_BASE_ADDR]], align 8
  
  // affs[0].size = sizeof(*p) * a * 10 * a;
  // CHECK: [[AFFS_0_SIZE_ADDR:%[0-9]+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_0_ADDR]], i32 0, i32 1
  // CHECK: store i64 [[SIZE]], ptr [[AFFS_0_SIZE_ADDR]], align 8
  
  // CHECK: [[TMP11:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.AFFARRAY"(i32 1, ptr [[AFFS_LST_ADDR]]) ]
  // CHECK: call void @llvm.directive.region.exit(token [[TMP11]]) [ "DIR.OMP.END.TASK"() ]

#pragma omp task affinity(([a][10][a])p)
  ;

  // CHECK: [[A_VAL:%.+]] = load i32, ptr [[A]], align 4
  // CHECK: [[SUB:%.+]] = sub nsw i32 [[A_VAL]], 0
  // CHECK: [[CONV:%.+]] = zext i32 [[SUB]] to i64

  // <num_elem> = <num_iters> + 1 constant affinity for affinity(a)
  // CHECK: [[NUM_ELEMS:%.+]] = add nuw i64 1, [[CONV]]
  // CHECK: [[SV:%.+]] = call ptr @llvm.stacksave.p0()
  // CHECK: store ptr [[SV]], ptr [[SV_ADDR:%.+]], align 8

  // kmp_task_affinity_info_t affs[<num_elem>];
  // CHECK: [[AFFS_ADDR:%.+]] = alloca %struct.kmp_task_affinity_info_t, i64 [[NUM_ELEMS]],
  // CHECK: store i64 [[NUM_ELEMS]], ptr %__vla_expr0, align 8
  // CHECK: [[NAFFS:%.+]] = trunc i64 [[NUM_ELEMS]] to i32
  // CHECK: [[AFFS_0_ADDR1:%.+]] = getelementptr %struct.kmp_task_affinity_info_t, ptr [[AFFS_ADDR]], i64 0

  // affs[0].base = &a;
  // CHECK: [[AFFS_0_BASE_ADDR1:%.+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_0_ADDR1]], i32 0, i32 0
  // CHECK: [[A_INTPTR:%.+]] = ptrtoint ptr [[A]] to i64
  // CHECK: store i64 [[A_INTPTR]], ptr [[AFFS_0_BASE_ADDR1]],
  
  // affs[0].size = sizeof(a);
  // CHECK: [[AFFS_0_SIZE_ADDR1:%.+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_0_ADDR1]], i32 0, i32 1
  // CHECK: store i64 4, ptr [[AFFS_0_SIZE_ADDR1]],

  // affs_cnt = 1;
  // CHECK: store i64 1, ptr [[AFFS_CNT_ADDR:%.+]],
  // CHECK: [[A_VAL:%.+]] = load i32, ptr [[A]],
  // CHECK: [[NITERS:%.+]] = sub nsw i32 [[A_VAL]], 0
  // CHECK: store i32 0, ptr [[CNT_ADDR:%.+]],
  // CHECK: br label %[[CONT:[^,]+]]
  
  //for (int cnt = 0; cnt < (a-0); ++cnt) {
  // int i = cnt + 0;
  // affs[affs_cnt].base = &p[i];
  // affs[affs_cnt].size = sizeof(p[i]);
  // ++affs_cnt;
  // }

  // CHECK: [[CONT]]:
  // CHECK: [[CNT:%.+]] = load i32, ptr [[CNT_ADDR]],
  // CHECK: [[CMP:%.+]] = icmp slt i32 [[CNT]], [[NITERS]]
  // CHECK: br i1 [[CMP]], label %[[BODY:[^,]+]], label %[[DONE:[^,]+]]

  // CHECK: [[BODY]]:
  // i = cnt + 0;
  // CHECK: [[CNT:%.+]] = load i32, ptr [[CNT_ADDR]],
  // CHECK: [[VAL:%.+]] = add nsw i32 0, [[CNT]]
  // CHECK: store i32 [[VAL]], ptr [[I]],

  // &p[i]
  // CHECK: [[P_VAL:%.+]] = load ptr, ptr [[P]],
  // CHECK: [[I_VAL:%.+]] = load i32, ptr [[I]],
  // CHECK: [[IDX:%.+]] = sext i32 [[I_VAL]] to i64
  // CHECK: [[P_I_ADDR:%.+]] = getelementptr inbounds float, ptr [[P_VAL]], i64 [[IDX]]

  // affs[affs_cnt]
  // CHECK: [[AFFS_CNT:%.+]] = load i64, ptr [[AFFS_CNT_ADDR]],
  // CHECK: [[AFFS_ELEM_ADDR:%.+]] = getelementptr %struct.kmp_task_affinity_info_t, ptr [[AFFS_ADDR]], i64 [[AFFS_CNT]]

  // affs[affs_cnt].base = &p[i];
  // CHECK: [[AFFS_ELEM_BASE_ADDR:%.+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_ELEM_ADDR]], i32 0, i32 0
  // CHECK: [[CAST:%.+]] = ptrtoint ptr [[P_I_ADDR]] to i64
  // CHECK: store i64 [[CAST]], ptr [[AFFS_ELEM_BASE_ADDR]],

  // affs[affs_cnt].size = sizeof(p[i]);
  // CHECK: [[AFFS_ELEM_SIZE_ADDR:%.+]] = getelementptr inbounds %struct.kmp_task_affinity_info_t, ptr [[AFFS_ELEM_ADDR]], i32 0, i32 1
  // CHECK: store i64 4, ptr [[AFFS_ELEM_SIZE_ADDR]],

  // ++affs_cnt;
  // CHECK: [[AFFS_CNT_NEXT:%.+]] = add nuw i64 [[AFFS_CNT]], 1
  // CHECK: store i64 [[AFFS_CNT_NEXT]], ptr [[AFFS_CNT_ADDR]],

  // ++cnt;
  // CHECK: [[CNT:%.+]] = load i32, ptr [[CNT_ADDR]],
  // CHECK: [[CNT_NEXT:%.+]] = add nsw i32 [[CNT]], 1
  // CHECK: store i32 [[CNT_NEXT]], ptr [[CNT_ADDR]],
  // CHECK: br label %[[CONT]]

  // CHECK: [[DONE]]:
  // CHECK: [[TMP34:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.AFFARRAY"(i32 [[NAFFS]], ptr [[AFFS_ADDR]]) ]
  // CHECK: call void @llvm.directive.region.exit(token [[TMP34]]) [ "DIR.OMP.END.TASK"() ]
  // CHECK: [[SV:%.+]] = load ptr, ptr [[SV_ADDR]],
  // CHECK: call void @llvm.stackrestore.p0(ptr [[SV]])
#pragma omp task affinity(iterator(i=0:a): p[i]) affinity(a)
  ;
}
// end INTEL_COLLAB
