//RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
//RUN: %clang_cc1 -emit-llvm -o - %s -fexceptions -fopenmp \
//RUN:   -fintel-compatibility -fintel-openmp-region       \
//RUN:   -triple x86_64-unknown-linux-gnu | FileCheck %s

int bar(int i);
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
void foo1(int *d) {
  long int n1=0, n2=1000, n3=1, n4=982, n5=0,
           n6=0, n7=0, n8=0, n9=0, n10=999;

//CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKGROUP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(i64* [[N1_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.SUB"(i64* [[N2_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(i64* [[N3_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BAND"(i64* [[N4_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BOR"(i64* [[N5_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BXOR"(i64* [[N6_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.AND"(i64* [[N7_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.OR"(i64* [[N8_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MAX"(i64* [[N9_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MIN"(i64* [[N10_ADDR]])

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
               task_reduction(min:n10)
  {
//CHECK: [[TOK1:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.PARALLEL"()
    #pragma omp parallel
    {
//CHECK: [[TOK2:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(i64* [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK2]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(+:n1)
      n1 += d[0];

//CHECK: [[TOK6:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(i64* [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK6]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(+:n1)
      n1 += d[n1];

//CHECK: [[TOK11:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.SUB"(i64* [[N2_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK11]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(-:n2)
      n2 -= d[n2];

//CHECK: [[TOK16:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MUL"(i64* [[N3_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK16]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(*:n3)
      n3 = bar(3);

//CHECK: [[TOK17:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BAND"(i64* [[N4_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK17]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(&:n4)
      n4 = bar(4);

//CHECK: [[TOK18:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BOR"(i64* [[N5_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK18]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(|:n5)
      n5 = bar(5);

//CHECK: [[TOK19:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BXOR"(i64* [[N6_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK19]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(^:n6)
      n6 = bar(6);

//CHECK: [[TOK20:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.AND"(i64* [[N7_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK20]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(&&:n7)
      n7 = bar(7);

//CHECK: [[TOK21:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.OR"(i64* [[N8_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK21]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(||:n8)
      n8 = bar(8);

//CHECK: [[TOK22:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MAX"(i64* [[N9_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK22]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(max:n9)
      n9 = bar(9);

//CHECK: [[TOK23:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MIN"(i64* [[N10_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK23]])
//CHECK-SAME: [ "DIR.OMP.END.TASK"() ]
      #pragma omp task in_reduction(min:n10)
      n10 = bar(10);
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
void foo2(int *d) {
  long int n1=0, n2=1000, n3=1, n4=982, n5=0,
           n6=0, n7=0, n8=0, n9=0, n10=999;
//CHECK: [[TOK0:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKGROUP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(i64* [[N1_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.SUB"(i64* [[N2_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(i64* [[N3_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BAND"(i64* [[N4_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BOR"(i64* [[N5_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.BXOR"(i64* [[N6_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.AND"(i64* [[N7_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.OR"(i64* [[N8_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MAX"(i64* [[N9_ADDR]])
//CHECK-SAME: "QUAL.OMP.REDUCTION.MIN"(i64* [[N10_ADDR]])
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
               task_reduction(min:n10)
  {
//CHECK: [[TOK6:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.ADD"(i64* [[N1_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK6]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(+:n1)
  for(int i=0;i<20;++i)
    n1 += bar(1);

//CHECK: [[TOK7:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.SUB"(i64* [[N2_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK7]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(-:n2)
  for(int i=0;i<20;++i)
    n2 += bar(2);

//CHECK: [[TOK8:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MUL"(i64* [[N3_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK8]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(*:n3)
  for(int i=0;i<20;++i)
    n3 += bar(3);

//CHECK: [[TOK9:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BAND"(i64* [[N4_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK9]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(&:n4)
  for(int i=0;i<20;++i)
    n4 += bar(4);

//CHECK: [[TOK10:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BOR"(i64* [[N5_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK10]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(|:n5)
  for(int i=0;i<20;++i)
    n5 += bar(5);

//CHECK: [[TOK11:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.BXOR"(i64* [[N6_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK11]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(^:n6)
  for(int i=0;i<20;++i)
    n6 += bar(6);

//CHECK: [[TOK12:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.AND"(i64* [[N7_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK12]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(&&:n7)
  for(int i=0;i<20;++i)
    n7 += bar(7);

//CHECK: [[TOK13:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.OR"(i64* [[N8_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK13]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(||:n8)
  for(int i=0;i<20;++i)
    n8 += bar(8);

//CHECK: [[TOK14:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MAX"(i64* [[N9_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK14]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(max:n9)
  for(int i=0;i<20;++i)
    n9 += bar(9);

//CHECK: [[TOK15:%[0-9]*]] = call token @llvm.directive.region.entry()
//CHECK-SAME: "DIR.OMP.TASKLOOP"()
//CHECK-SAME: "QUAL.OMP.INREDUCTION.MIN"(i64* [[N10_ADDR]])
//CHECK: call void @llvm.directive.region.exit(token [[TOK15]])
//CHECK-SAME: [ "DIR.OMP.END.TASKLOOP"() ]
  #pragma omp taskloop in_reduction(min:n10)
  for(int i=0;i<20;++i)
    n10 += bar(10);
  }
//CHECK: call void @llvm.directive.region.exit(token [[TOK0]])
//CHECK-SAME: [ "DIR.OMP.END.TASKGROUP"() ]
}
