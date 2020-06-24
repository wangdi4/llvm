// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: @_Z5test1{{.*}}(
int test1() {
  int sum(100);
  int avar[] = {0,1,2,3,4,5,6,7,8,9,10};

  // CHECK: [[RANGE:%__range.+]] = alloca [11 x i32]*, align 8
  // CHECK-NEXT: [[END:%__end.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE1:%.capture_expr.*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE2:%.capture_expr.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE3:%.capture_expr.+]] = alloca i64, align 8
  // CHECK-NEXT: [[IV:%.omp.iv]] = alloca i64, align 8
  // CHECK-NEXT: [[LB:%.omp.lb]] = alloca i64, align 8
  // CHECK-NEXT: [[UB:%.omp.ub]] = alloca i64, align 8
  // CHECK-NEXT: [[BEGIN:%__begin.+]] = alloca i32*, align 8

  // CHECK: store [11 x i32]* %avar, [11 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load [11 x i32]*, [11 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay.*]] = getelementptr inbounds [11 x i32], [11 x i32]* [[LD]], i64 0, i64 0
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[ADECAY]], i64 11
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[END]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load [11 x i32]*, [11 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay.*]] = getelementptr inbounds [11 x i32], [11 x i32]* [[LD]], i64 0, i64 0
  // CHECK-NEXT: store i32* [[ADECAY]], i32** [[CE1]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32*, i32** [[END]], align 8
  // CHECK-NEXT: store i32* [[LD]], i32** [[CE2]], align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32*, i32** [[CE2]], align 8
  // CHECK-NEXT: [[LD2:%[0-9]+]] =  load i32*, i32** [[CE1]], align 8
  // CHECK-NEXT: %sub.ptr.lhs.cast = ptrtoint i32* [[LD1]] to i64
  // CHECK-NEXT: %sub.ptr.rhs.cast = ptrtoint i32* [[LD2]] to i64
  // CHECK-NEXT: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  // CHECK-NEXT: %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 4
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 %sub.ptr.div, 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = sdiv i64 [[ADD]], 1
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 [[DIV]], 1
  // CHECK-NEXT: store i64 [[SUB]], i64* [[CE3]], align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32*, i32** [[CE1]], align 8
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32*, i32** [[CE2]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ult i32* [[LD1]], [[LD2]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.precond.then{{[0-9]*}}, label %omp.precond.end{{[0-9]*}}

// CHECK-LABEL: omp.precond.then{{[0-9]*}}:
  // CHECK-NEXT: store i64 0, i64* [[LB]], align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load i64, i64* [[CE3]], align 8
  // CHECK-NEXT: store i64 [[LD8]], i64* [[UB]], align 8
  // CHECK-NEXT: [[LD9:%[0-9]+]] = load i32*, i32** %aref, align 8

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(i32* %sum)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD9]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32** [[CE1]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %aref)

// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD4:%[0-9]+]] = load i32*, i32** [[CE1]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[MUL:%mul[0-9]*]] = mul nsw i64 [[LD]], 1
  // CHECK-NEXT: [[ADDPTR:%add.ptr.+]] = getelementptr inbounds i32, i32* [[LD4]], i64 [[MUL]]
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[BEGIN]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32*, i32** [[BEGIN]], align 8
  // CHECK-NEXT: store i32* [[LD]], i32** %aref, align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load i32*, i32** %aref, align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load i32, i32* [[LD7]], align 4
  // CHECK-NEXT: [[ADD7:%add.+]] = add nsw i32 [[LD8]], 2
  // CHECK-NEXT: [[LD9:%[0-9]+]] = load i32, i32* %sum, align 4
  // CHECK-NEXT: [[ADD8:%add.+]] = add nsw i32 [[LD9]], [[ADD7]]
  // CHECK-NEXT: store i32 [[ADD8]], i32* %sum, align 4
  // CHECK-NEXT: br label %omp.body.continue{{[0-9]*}}
// CHECK-LABEL: omp.body.continue{{[0-9]*}}:

  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()

#pragma omp parallel for reduction(+:sum)
  for (const auto &aref : avar) {
    sum += (aref + 2);
  }
  return sum;
}

struct A {
  int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int *begin() { return &arr[0]; }
  int *end()   { return &arr[10]; }
};

template<typename _Container>
inline auto begin(const _Container& __cont) -> decltype(__cont.begin())
    { return __cont.begin(); }

template<typename _Container>
inline auto end(const _Container& __cont) -> decltype(__cont.end())
{ return __cont.end(); }

// CHECK-LABEL: @_Z5test2{{.*}}()
int test2() {
  A avar;
  A avar2;
  int sum(0);

  // CHECK: [[RANGE1:%__range.+]] = alloca %struct.A*, align 8
  // CHECK-NEXT: [[END1:%__end.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE:%.capture_expr.*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE2:%.capture_expr.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE3:%.capture_expr.+]] = alloca i64, align 8
  // CHECK-NEXT: [[IV:%.omp.iv]] = alloca i64, align 8
  // CHECK-NEXT: [[LB:%.omp.lb]] = alloca i64, align 8
  // CHECK-NEXT: [[UB:%.omp.ub]] = alloca i64, align 8
  // CHECK-NEXT: [[BEGIN1:%__begin.+]] = alloca i32*, align 8
  // CHECK-NEXT: %v = alloca i32*, align 8
  // CHECK-NEXT: [[RANGE2:%__range.+]] = alloca %struct.A*, align 8
  // CHECK-NEXT: [[BEGIN2:%__begin.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[END2:%__end.+]] = alloca i32*, align 8
  // CHECK-NEXT: %v2 = alloca i32*, align 8

  // CHECK: store i32 0, i32* %sum, align 4
  // CHECK-NEXT: store %struct.A* %avar, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[LD0:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]*]] = call i32* @_ZN1A3endEv(%struct.A* [[LD0]])
  // CHECK-NEXT: store i32* [[CALL]], i32** [[END1]], align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[CALL1:%call[0-9]+]] = call i32* @_ZN1A5beginEv(%struct.A* %1)
  // CHECK-NEXT: store i32* [[CALL1]], i32** [[CE]], align 8
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32*, i32** [[END1]], align 8
  // CHECK-NEXT: store i32* [[LD2]], i32** [[CE2]], align 8
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32*, i32** [[CE2]], align 8
  // CHECK-NEXT: [[LD4:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: %sub.ptr.lhs.cast = ptrtoint i32* [[LD3]] to i64
  // CHECK-NEXT: %sub.ptr.rhs.cast = ptrtoint i32* [[LD4]] to i64
  // CHECK-NEXT: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  // CHECK-NEXT: %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 4
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 %sub.ptr.div, 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = sdiv i64 [[ADD]], 1
  // CHECK-NEXT: [[SUB4:%sub[0-9]+]] = sub nsw i64 [[DIV]], 1
  // CHECK-NEXT: store i64 [[SUB4]], i64* [[CE3]], align 8
  // CHECK-NEXT: [[LD5:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD6:%[0-9]+]] = load i32*, i32** [[CE2]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ult i32* [[LD5]], [[LD6]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.precond.then{{[0-9]*}}, label %omp.precond.end{{[0-9]*}}
// CHECK-LABEL: omp.precond.then{{[0-9]*}}:

  // CHECK: store i64 0, i64* [[LB]], align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load i64, i64* [[CE3]], align 8
  // CHECK-NEXT: store i64 [[LD7]], i64* [[UB]], align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK-NEXT: [[LD9:%[0-9]+]] = load i32*, i32** %v2, align 8
  // CHECK-NEXT: [[LD10:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(i32* %sum)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 1)
  // CHECK-SAME: "QUAL.OMP.SHARED"(%struct.A* %avar2)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD8]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32** [[CE]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN1]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD9]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[END2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.A* [[LD10]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %v)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.A** [[RANGE2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %v2) ]
  // CHECK: [[LD12:%[0-9]+]] = load i64, i64* [[LB]], align 8
  // CHECK-NEXT: store i64 [[LD12]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.cond{{[0-9]*}}:
  // CHECK: [[LD13:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[LD14:%[0-9]+]] = load i64, i64* [[UB]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]+]] = icmp sle i64 [[LD13]], [[LD14]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.inner.for.body{{[0-9]*}}, label %omp.inner.for.end{{[0-9]*}}


// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD15:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD16:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[MUL:%mul[0-9]*]] = mul nsw i64 [[LD16]], 1
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD15]], i64 [[MUL]]
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[BEGIN1]], align 8
  // CHECK-NEXT: [[LD17:%[0-9]+]] = load i32*, i32** [[BEGIN1]], align 8
  // CHECK-NEXT: store i32* [[LD17]], i32** %v, align 8
  // CHECK-NEXT: store %struct.A* %avar2, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[LD18:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]+]] = call i32* @_ZN1A5beginEv(%struct.A* [[LD18]]) #2
  // CHECK-NEXT: store i32* [[CALL]], i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[LD19:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]+]] = call i32* @_ZN1A3endEv(%struct.A* [[LD19]]) #2
  // CHECK-NEXT: store i32* [[CALL]], i32** [[END2]], align 8
  // CHECK-NEXT: br label %for.cond{{[0-9]*}}

// CHECK-LABEL: for.cond{{[0-9]*}}:
  // CHECK: [[LD20:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[LD21:%[0-9]+]] = load i32*, i32** [[END2]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]+]] = icmp ne i32* [[LD20]], [[LD21]]
  // CHECK-NEXT: br i1 [[CMP]], label %for.body{{[0-9]*}}, label %for.end{{[0-9]*}}

// CHECK-LABEL: for.body{{[0-9]*}}:
  // CHECK: [[LD22:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: store i32* [[LD22]], i32** %v2, align 8
  // CHECK-NEXT: [[LD23:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK-NEXT: [[LD24:%[0-9]+]] = load i32, i32* [[LD23]], align 4
  // CHECK-NEXT: [[LD25:%[0-9]+]] = load i32*, i32** %v2, align 8
  // CHECK-NEXT: [[LD26:%[0-9]+]] = load i32, i32* [[LD25]], align 4
  // CHECK-NEXT: [[ADD9:%add[0-9]+]] = add nsw i32 [[LD24]], [[LD26]]
  // CHECK-NEXT: [[LD27:%[0-9]+]] = load i32, i32* %sum, align 4
  // CHECK-NEXT: [[MUL10:%mul[0-9]+]] = mul nsw i32 [[LD27]], [[ADD9]]
  // CHECK-NEXT: store i32 [[MUL10]], i32* %sum, align 4
  // CHECK-NEXT: br label %for.inc{{[0-9]*}}

// CHECK-LABEL: for.inc{{[0-9]*}}:
  // CHECK: [[LD28:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[INCDEC:%incdec.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD28]], i32 1
  // CHECK-NEXT: store i32* [[INCDEC]], i32** [[BEGIN2]], align 8
  // CHECK-NEXT: br label %for.cond{{[0-9]*}}

// CHECK-LABEL: for.end{{[0-9]*}}:
  // CHECK: br label %omp.body.continue{{[0-9]*}}

// CHECK-LABEL: omp.body.continue{{[0-9]*}}:
  // CHECK: br label %omp.inner.for.inc{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.inc{{[0-9]*}}:
  // CHECK: [[LD29:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[ADD:%add[0-9]+]] = add nsw i64 [[LD29]], 1
  // CHECK-NEXT: store i64 [[ADD]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.end{{[0-9]*}}:
  // CHECK: br label %omp.loop.exit{{[0-9]*}}

// CHECK-LABEL: omp.loop.exit{{[0-9]*}}:
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE.PARLOOP"()

  // Nested range-based for loops.
  // NOTE: Should use collapse(2) but currently causes assert failure.
  #pragma omp distribute parallel for reduction(*:sum) collapse(1)
  for (const auto &v : avar)
    for (auto &v2 : avar2)
      sum *= (v + v2);

  return sum;
}

// CHECK-LABEL: @_Z5test3{{.*}}()
int test3() {
  A avar3;
  A avar4;
  int sum(0);

  // CHECK: %sum = alloca i32, align 4
  // CHECK: [[RANGE1:%__range[0-9]+]] = alloca %struct.A*, align 8
  // CHECK-NEXT: [[END1:%__end[0-9]+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE:%.capture_expr.[0-9]*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE3:%.capture_expr.[0-9]*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE4:%.capture_expr.[0-9]*]] = alloca i64, align 8
  // CHECK-NEXT: [[IV:%.omp.iv[0-9]*]] = alloca i64, align 8
  // CHECK-NEXT: [[LB:%.omp.lb[0-9]*]] = alloca i64, align 8
  // CHECK-NEXT: [[UB:%.omp.ub[0-9]*]] = alloca i64, align 8
  // CHECK-NEXT: [[BEGIN1:%__begin[0-9]+]] = alloca i32*, align 8
  // CHECK-NEXT: %i = alloca i32, align 4
  // CHECK-NEXT: %v = alloca i32*, align 8
  // CHECK-NEXT: call void @_ZN1AC1Ev(%struct.A* %avar3) #2
  // CHECK-NEXT: call void @_ZN1AC1Ev(%struct.A* %avar4) #2
  // CHECK-NEXT: store i32 0, i32* %sum, align 4
  // CHECK-NEXT: store %struct.A* %avar3, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[LD0:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]*]] = call i32* @_ZN1A3endEv(%struct.A* %0)
  // CHECK-NEXT: store i32* [[CALL]], i32** [[END1]], align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE1]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]+]] = call i32* @_ZN1A5beginEv(%struct.A* [[LD1]])
  // CHECK-NEXT: store i32* [[CALL]], i32** [[CE]], align 8
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32*, i32** [[END1]], align 8
  // CHECK-NEXT: store i32* [[LD2]], i32** [[CE3]], align 8
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32*, i32** [[CE3]], align 8
  // CHECK-NEXT: [[LD4:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: %sub.ptr.lhs.cast = ptrtoint i32* [[LD3]] to i64
  // CHECK-NEXT: %sub.ptr.rhs.cast = ptrtoint i32* [[LD4]] to i64
  // CHECK-NEXT: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  // CHECK-NEXT: %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 4
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 %sub.ptr.div, 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = sdiv i64 [[ADD]], 1
  // CHECK-NEXT: [[MUL:%mul[0-9]*]] = mul nsw i64 [[DIV]], 10
  // CHECK-NEXT: [[SUB:%sub[0-9]+]] = sub nsw i64 [[MUL]], 1
  // CHECK-NEXT: store i64 [[SUB]], i64* [[CE4]], align 8
  // CHECK-NEXT: [[LD5:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD6:%[0-9]+]] = load i32*, i32** [[CE3]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ult i32* [[LD5]], [[LD6]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.precond.then{{[0-9]*}}, label %omp.precond.end{{[0-9]*}}

// CHECK-LABEL: omp.precond.then{{[0-9]*}}:
  // CHECK: store i64 0, i64* [[LB]], align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load i64, i64* [[CE4]], align 8
  // CHECK-NEXT: store i64 [[LD7]], i64* [[UB]], align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK: region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.SUB"(i32* %sum)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 2)
  // CHECK-SAME: "QUAL.OMP.SHARED"(%struct.A* %avar4)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD8]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32** [[CE]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN1]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %i)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %v)
  // CHECK-NEXT: [[LD10:%[0-9]+]] = load i64, i64* [[LB]], align 8
  // CHECK-NEXT: store i64 [[LD10]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.cond{{[0-9]*}}:
  // CHECK: [[LD11:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[LD12:%[0-9]+]] = load i64, i64* [[UB]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp sle i64 %11, %12
  // CHECK-NEXT: br i1 [[CMP]], label %omp.inner.for.body{{[0-9]*}}, label %omp.inner.for.end{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD13:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD14:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[DIV:%div[0-9]+]] = sdiv i64 [[LD14]], 10
  // CHECK-NEXT: [[MUL:%mul[0-9]+]] = mul nsw i64 [[DIV]], 1
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD13]], i64 [[MUL]]
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[BEGIN1]], align 8
  // CHECK-NEXT: [[LD15:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[LD16:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[DIV:%div[0-9]+]] = sdiv i64 [[LD16]], 10
  // CHECK-NEXT: [[MUL:%mul[0-9]+]] = mul nsw i64 [[DIV]], 10
  // CHECK-NEXT: [[SUB:%sub[0-9]+]] = sub nsw i64 [[LD15]], [[MUL]]
  // CHECK-NEXT: [[MUL:%mul[0-9]+]] = mul nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[ADD:%add[0-9]+]] = add nsw i64 0, [[MUL]]
  // CHECK-NEXT: [[CONV:%conv[0-9]*]] = trunc i64 [[ADD]] to i32
  // CHECK-NEXT: store i32 [[CONV]], i32* %i, align 4
  // CHECK-NEXT: [[LD17:%[0-9]+]] = load i32*, i32** [[BEGIN1]], align 8
  // CHECK-NEXT: store i32* [[LD17]], i32** %v, align 8
  // CHECK-NEXT: [[LD18:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK-NEXT: [[LD19:%[0-9]+]] = load i32, i32* [[LD18]], align 4
  // CHECK-NEXT: [[ARR:%arr[0-9]*]] = getelementptr inbounds %struct.A, %struct.A* %avar4, i32 0, i32 0
  // CHECK-NEXT: [[LD20:%[0-9]+]] = load i32, i32* %i, align 4
  // CHECK-NEXT: [[IDXPROM:%idxprom[0-9]*]] = sext i32 [[LD20]] to i64
  // CHECK-NEXT: [[AINDEX:%arrayidx[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* [[ARR]], i64 0, i64 [[IDXPROM]]
  // CHECK-NEXT: [[LD21:%[0-9]+]] = load i32, i32* [[AINDEX]], align 4
  // CHECK-NEXT: [[ADD:%add[0-9]+]] = add nsw i32 [[LD19]], [[LD21]]
  // CHECK-NEXT: [[LD22:%[0-9]+]] = load i32, i32* %sum, align 4
  // CHECK-NEXT: [[SUB:%sub[0-9]+]] = sub nsw i32 [[LD22]], [[ADD]]
  // CHECK-NEXT: store i32 [[SUB]], i32* %sum, align 4
  // CHECK-NEXT: br label %omp.body.continue{{[0-9]*}}

// CHECK-LABEL: omp.body.continue{{[0-9]*}}:
  // CHECK: br label %omp.inner.for.inc{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.inc{{[0-9]*}}:
  // CHECK: [[LD23:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[ADD:%add[0-9]+]] = add nsw i64 [[LD23]], 1
  // CHECK-NEXT: store i64 [[ADD]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.end{{[0-9]*}}:
   // CHECK: br label %omp.loop.exit{{[0-9]*}}

// CHECK-LABEL: omp.loop.exit{{[0-9]*}}:
   // CHECK: region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  // Regular for loop nested inside range-based loop.
  #pragma omp distribute parallel for reduction(-:sum) collapse(2)
  for (const auto &v : avar3)
    for (int i=0; i < 10; i++)
      sum -= (v + avar4.arr[i]);

  return sum;
}

// CHECK-LABEL: @_Z5test4{{.*}}()
int test4() {
  A avar5;
  A avar6;
  int sum(0);

  // CHECK: [[IV:%.omp.iv]] = alloca i32, align 4
  // CHECK-NEXT: [[LB:%.omp.lb]] = alloca i32, align 4
  // CHECK-NEXT: [[UB:%.omp.ub]] = alloca i32, align 4
  // CHECK-NEXT: %i = alloca i32, align 4
  // CHECK-NEXT: [[RANGE2:%__range.+]] = alloca %struct.A*, align 8
  // CHECK-NEXT: [[BEGIN2:%__begin.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[END2:%__end.+]] = alloca i32*, align 8
  // CHECK-NEXT: %v = alloca i32*, align 8
  // CHECK: store i32 0, i32* %sum, align 4
  // CHECK-NEXT: store i32 0, i32* [[LB]], align 4
  // CHECK-NEXT: store i32 9, i32* [[UB]], align 4
  // CHECK-NEXT: [[LD0:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.MUL"(i32* %sum)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 1)
  // CHECK-SAME: "QUAL.OMP.SHARED"(%struct.A* %avar5)
  // CHECK-SAME: "QUAL.OMP.SHARED"(%struct.A* %avar6)
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %i)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD0]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[END2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.A* [[LD1]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(%struct.A** [[RANGE2]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %v) ]
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32, i32* [[LB]], align 4
  // CHECK-NEXT: store i32 [[LD3]], i32* [[IV]], align 4
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.cond{{[0-9]*}}:
  // CHECK: [[LD4:%[0-9]+]] = load i32, i32* [[IV]], align 4
  // CHECK-NEXT: [[LD5:%[0-9]+]] = load i32, i32* [[UB]], align 4
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp sle i32 [[LD4]], [[LD5]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.inner.for.body{{[0-9]*}}, label %omp.inner.for.end{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD6:%[0-9]+]] = load i32, i32* [[IV]], align 4
  // CHECK-NEXT: [[MUL:%mul[0-9]*]] = mul nsw i32 [[LD6]], 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i32 0, [[MUL]]
  // CHECK-NEXT: store i32 [[ADD]], i32* %i, align 4
  // CHECK-NEXT: store %struct.A* %avar5, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]*]] = call i32* @_ZN1A5beginEv(%struct.A* [[LD7]]) #2
  // CHECK-NEXT: store i32* [[CALL]], i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load %struct.A*, %struct.A** [[RANGE2]], align 8
  // CHECK-NEXT: [[CALL:%call[0-9]+]] = call i32* @_ZN1A3endEv(%struct.A* [[LD8]])
  // CHECK-NEXT: store i32* [[CALL]], i32** [[END2]], align 8
  // CHECK-NEXT: br label %for.cond{{[0-9]*}}

// CHECK-LABEL: for.cond{{[0-9]*}}:
  // CHECK: [[LD9:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[LD10:%[0-9]+]] = load i32*, i32** [[END2]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ne i32* [[LD9]], [[LD10]]
  // CHECK-NEXT: br i1 [[CMP]], label %for.body{{[0-9]*}}, label %for.end{{[0-9]*}}

// CHECK-LABEL: for.body{{[0-9]*}}:
  // CHECK: [[LD11:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: store i32* [[LD11]], i32** %v, align 8
  // CHECK-NEXT: [[LD12:%[0-9]+]] = load i32*, i32** %v, align 8
  // CHECK-NEXT: [[LD13:%[0-9]+]] = load i32, i32* [[LD12]], align 4
  // CHECK-NEXT: [[ARR:%arr[0-9]*]] = getelementptr inbounds %struct.A, %struct.A* %avar6, i32 0, i32 0
  // CHECK-NEXT: [[LD14:%[0-9]+]] = load i32, i32* %i, align 4
  // CHECK-NEXT: [[IDXPROM:%idxprom[0-9]*]] = sext i32 [[LD14]] to i64
  // CHECK-NEXT: [[AINDEX:%arrayidx[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* [[ARR]], i64 0, i64 [[IDXPROM]]
  // CHECK-NEXT: [[LD15:%[0-9]+]] = load i32, i32* [[AINDEX]], align 4
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i32 [[LD13]], [[LD15]]
  // CHECK-NEXT: [[LD16:%[0-9]+]] = load i32, i32* %sum, align 4
  // CHECK-NEXT: [[ADD2:%add[0-9]*]] = add nsw i32 [[LD16]], [[ADD]]
  // CHECK-NEXT: store i32 [[ADD2]], i32* %sum, align 4
  // CHECK-NEXT: br label %for.inc{{[0-9]*}}

// CHECK-LABEL: for.inc{{[0-9]*}}:
  // CHECK: [[LD17:%[0-9]+]] = load i32*, i32** [[BEGIN2]], align 8
  // CHECK-NEXT: [[INCDECP:%incdec.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD17]], i32 1
  // CHECK-NEXT: store i32* [[INCDECP]], i32** [[BEGIN2]], align 8
  // CHECK-NEXT: br label %for.cond{{[0-9]*}}

// CHECK-LABEL: for.end{{[0-9]*}}:
  // CHECK: br label %omp.body.continue{{[0-9]*}}

// CHECK-LABEL: omp.body.continue{{[0-9]*}}:
  // CHECK: br label %omp.inner.for.inc{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.inc{{[0-9]*}}:
  // CHECK: [[LD18:%[0-9]+]] = load i32, i32* [[IV]], align 4
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i32 [[LD18]], 1
  // CHECK-NEXT: store i32 [[ADD]], i32* [[IV]], align 4
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.end{{[0-9]*}}:
  // CHECK: br label %omp.loop.exit{{[0-9]*}}

// CHECK-LABEL: omp.loop.exit{{[0-9]*}}:
  // CHECK: region.exit(token [[LD2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  // Range-based for loop nested inside regular for loop.
  // NOTE: Should use collapse(2) but currently causes assert failure.
  #pragma omp distribute parallel for reduction(*:sum) collapse(1)
  for (int i=0; i < 10; i++)
    for (const auto &v : avar5)
      sum += (v + avar6.arr[i]);

  return sum;
}

int main() {
  int res1 = test1();
  int res2 = test2();
  int res3 = test3();
  int res4 = test4();
  return (res1 + res2 + res3 + res4);
}
// end INTEL_COLLAB
