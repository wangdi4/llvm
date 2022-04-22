// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: @_Z5test1i(i32 noundef %n) #0 {
int test1(int n) {
  int avar[n];
  // CHECK: [[NADDR:%n.addr]] = alloca i32, align 4
  // CHECK: [[VLAEXPR:%__vla_expr.+]] = alloca i64, align 8
  // CHECK: [[RANGE:%__range.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[END:%__end.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE:%.capture_expr.*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE1:%.capture_expr.+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE2:%.capture_expr.+]] = alloca i64, align 8
  // CHECK-NEXT: [[IV:%.omp.iv]] = alloca i64, align 8
  // CHECK-NEXT: [[LB:%.omp.lb]] = alloca i64, align 8
  // CHECK-NEXT: [[UB:%.omp.ub]] = alloca i64, align 8
  // CHECK-NEXT: [[OMPVLA:%omp.vla.tmp]] = alloca i64, align 8
  // CHECK-NEXT: [[BEGIN:%__begin.+]] = alloca i32*, align 8
  // CHECK-NEXT: %aref = alloca i32*, align 8
  // CHECK-NEXT: store i32 %n, i32* [[NADDR]], align 4
  // CHECK-NEXT: [[LD0:%[0-9]+]] = load i32, i32* [[NADDR]], align 4
  // CHECK-NEXT: [[LD1:%[0-9]+]] = zext i32 [[LD0]] to i64
  // CHECK-NEXT: [[LD2:%[0-9]+]] = call i8* @llvm.stacksave()
  // CHECK-NEXT: store i8* [[LD2]], i8** %saved_stack, align 8
  // CHECK-NEXT: [[VLA:%vla]] = alloca i32, i64 [[LD1]], align 16
  // CHECK-NEXT: store i64 [[LD1]], i64* [[VLAEXPR]], align 8
  // CHECK-NEXT: store i32* [[VLA]], i32** [[RANGE]], align 8
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load i32*, i32** [[RANGE]], align 8
  // CHECK-NEXT: [[LD4:%[0-9]+]] = mul nuw i64 4, [[LD1]]
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = udiv i64 [[LD4]], 4
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD3]], i64 [[DIV]]
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[END]], align 8

  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32*, i32** [[RANGE]], align 8
  // CHECK-NEXT: store i32* [[LD]], i32** [[CE]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32*, i32** [[END]], align 8
  // CHECK-NEXT: store i32* [[LD]], i32** [[CE1]], align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load i32*, i32** [[CE1]], align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: %sub.ptr.lhs.cast = ptrtoint i32* [[LD7]] to i64
  // CHECK-NEXT: %sub.ptr.rhs.cast = ptrtoint i32* [[LD8]] to i64
  // CHECK-NEXT: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  // CHECK-NEXT: %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 4
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 %sub.ptr.div, 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = sdiv i64 [[ADD]], 1
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 [[DIV]], 1
  // CHECK-NEXT: store i64 [[SUB]], i64* [[CE2]], align 8
  // CHECK-NEXT: [[LD9:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD10:%[0-9]+]] = load i32*, i32** [[CE1]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ult i32* [[LD9]], [[LD10]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.precond.then{{[0-9]*}}, label %omp.precond.end{{[0-9]*}}

// CHECK-LABEL: omp.precond.then{{[0-9]*}}:
  // CHECK: store i64 0, i64* [[LB]], align 8
  // CHECK-NEXT: [[LD11:%[0-9]+]] = load i64, i64* [[CE2]], align 8
  // CHECK-NEXT: store i64 [[LD11]], i64* [[UB]], align 8
  // CHECK-NEXT: store i64 [[LD1]], i64* [[OMPVLA]], align 8
  // CHECK-NEXT: [[LD12:%[0-9]+]] = load i32*, i32** %aref, align 8

  // CHECK: region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD12]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32** [[CE]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i64* [[OMPVLA]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %aref) ]
  // CHECK-NEXT: [[LD14:%[0-9]+]] = load i64, i64* [[OMPVLA]], align 8
  // CHECK-NEXT: [[LD15:%[0-9]+]] = load i64, i64* [[LB]], align 8
  // CHECK-NEXT: store i64 [[LD15]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.cond{{[0-9]*}}:
  // CHECK: [[LD16:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[LD17:%[0-9]+]] = load i64, i64* [[UB]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp sle i64 [[LD16]], [[LD17]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.inner.for.body{{[0-9]*}}, label %omp.inner.for.end{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD18:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD19:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[MUL:%mul[0-9]*]] = mul nsw i64 [[LD19]], 1
  // CHECK-NEXT: [[ADD:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[LD18]], i64 [[MUL]]
  // CHECK-NEXT: store i32* [[ADD]], i32** [[BEGIN]], align 8
  // CHECK-NEXT: [[LD20:%[0-9]+]] = load i32*, i32** [[BEGIN]], align 8
  // CHECK-NEXT: store i32* [[LD20]], i32** %aref, align 8
  // CHECK-NEXT: [[LD21:%[0-9]+]] = load i32*, i32** %aref, align 8
  // CHECK-NEXT: [[LD22:%[0-9]+]] = load i32, i32* [[LD21]], align 4
  // CHECK-NEXT: [[INC:%inc[0-9]*]] = add nsw i32 [[LD22]], 1
  // CHECK-NEXT: store i32 [[INC]], i32* [[LD21]], align 4
  // CHECK-NEXT: br label %omp.body.continue{{[0-9]*}}

// CHECK-LABEL: omp.body.continue{{[0-9]*}}:
  // CHECK: br label %omp.inner.for.inc{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.inc{{[0-9]*}}:
  // CHECK: [[LD23:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[LD23]], 1
  // CHECK-NEXT: store i64 [[ADD]], i64* [[IV]], align 8
  // CHECK-NEXT: br label %omp.inner.for.cond{{[0-9]*}}

// CHECK-LABEL: omp.inner.for.end{{[0-9]*}}:
  // CHECK: br label %omp.loop.exit{{[0-9]*}}

// CHECK-LABEL: omp.loop.exit{{[0-9]*}}:
  // CHECK: region.exit{{.*}}"DIR.OMP.END.DISTRIBUTE.PARLOOP"()
  // CHECK-NEXT: br label %omp.precond.end{{[0-9]*}}

#pragma omp distribute parallel for
  for (auto &aref : avar) {
    aref++;
  }
  return 0;
}

class B {
public:
  B() { }
  int c;
};

class A : virtual B {
public:
  A() { }
  ~A() { }
  A operator=(const A&foo) { return foo; }
  int x[10] = {1,2,3,4,5,6,7,8,9,10};
};

A foobar;

// CHECK-LABEL: @_Z5test2{{.*}}()
int test2() {
  // CHECK: [[RANGE:%__range[0-9]+]] = alloca [10 x i32]*, align 8
  // CHECK-NEXT: [[END:%__end[0-9]+]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE:%.capture_expr.*]] = alloca i32*, align 8
  // CHECK-NEXT: [[CE1:%.capture_expr.+]] = alloca i64, align 8
  // CHECK-NEXT: [[IV:%.omp.iv]] = alloca i64, align 8
  // CHECK-NEXT: [[LB:%.omp.lb]] = alloca i64, align 8
  // CHECK-NEXT: [[UB:%.omp.ub]] = alloca i64, align 8
  // CHECK-NEXT: [[BEGIN:%__begin[0-9]+]] = alloca i32*, align 8
  // CHECK-NEXT: %a = alloca i32*, align 8
  // CHECK-NEXT: store [10 x i32]* bitcast (i8* getelementptr (i8, i8* bitcast (%class.A* @foobar to i8*), i64 8) to [10 x i32]*), [10 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load [10 x i32]*, [10 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* [[LD]], i64 0, i64 0
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]*]] = getelementptr inbounds i32, i32* [[ADECAY]], i64 10
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[END]], align 8
  // CHECK-NEXT: [[LD1:%[0-9]+]] = load i32*, i32** [[END]], align 8
  // CHECK-NEXT: store i32* [[LD1]], i32** [[CE]], align 8
  // CHECK-NEXT: [[LD2:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[LD3:%[0-9]+]] = load [10 x i32]*, [10 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay[0-9]*]] = getelementptr inbounds [10 x i32], [10 x i32]* [[LD3]], i64 0, i64 0
  // CHECK-NEXT: %sub.ptr.lhs.cast = ptrtoint i32* [[LD2]] to i64
  // CHECK-NEXT: %sub.ptr.rhs.cast = ptrtoint i32* [[ADECAY]] to i64
  // CHECK-NEXT: %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  // CHECK-NEXT: %sub.ptr.div = sdiv exact i64 %sub.ptr.sub, 4
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 %sub.ptr.div, 1
  // CHECK-NEXT: [[ADD:%add[0-9]*]] = add nsw i64 [[SUB]], 1
  // CHECK-NEXT: [[DIV:%div[0-9]*]] = sdiv i64 %add, 1
  // CHECK-NEXT: [[SUB:%sub[0-9]*]] = sub nsw i64 [[DIV]], 1

  // CHECK-NEXT: store i64 [[SUB]], i64* [[CE1]], align 8
  // CHECK-NEXT: [[LD4:%[0-9]+]] = load [10 x i32]*, [10 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay[0-9]+]] = getelementptr inbounds [10 x i32], [10 x i32]* [[LD4]], i64 0, i64 0
  // CHECK-NEXT: [[LD5:%[0-9]+]] = load i32*, i32** [[CE]], align 8
  // CHECK-NEXT: [[CMP:%cmp[0-9]*]] = icmp ult i32* [[ADECAY]], [[LD5]]
  // CHECK-NEXT: br i1 [[CMP]], label %omp.precond.then{{[0-9]*}}, label %omp.precond.end{{[0-9]*}}

// CHECK-LABEL: omp.precond.then{{[0-9]*}}:
  // CHECK-NEXT: store i64 0, i64* [[LB]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i64, i64* [[CE1]], align 8
  // CHECK-NEXT: store i64 [[LD]], i64* [[UB]], align 8
  // CHECK-NEXT: [[LD7:%[0-9]+]] = load i32*, i32** %a, align 8
  // CHECK-NEXT: [[LD8:%[0-9]+]] = load [10 x i32]*, [10 x i32]** [[RANGE]], align 8

  // CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.A* @foobar, void (%class.A*, %class.A*)* @_ZTS1A.omp.copy_constr, void (%class.A*)* @_ZTS1A.omp.destr)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[LD7]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64* [[IV]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[LB]])
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[UB]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[BEGIN]])
  // CHECK-SAME: "QUAL.OMP.SHARED:BYREF"([10 x i32]** [[RANGE]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** %a)

// CHECK-LABEL: omp.inner.for.body{{[0-9]*}}:
  // CHECK: [[LD:%[0-9]+]] = load [10 x i32]*, [10 x i32]** [[RANGE]], align 8
  // CHECK-NEXT: [[ADECAY:%arraydecay[0-9]+]] = getelementptr inbounds [10 x i32], [10 x i32]* [[LD]], i64 0, i64 0
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i64, i64* [[IV]], align 8
  // CHECK-NEXT: %mul = mul nsw i64 [[LD]], 1
  // CHECK-NEXT: [[ADDPTR:%add.ptr[0-9]+]] = getelementptr inbounds i32, i32* [[ADECAY]], i64 %mul
  // CHECK-NEXT: store i32* [[ADDPTR]], i32** [[BEGIN]], align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32*, i32** [[BEGIN]], align 8
  // CHECK-NEXT: store i32* [[LD]], i32** %a, align 8
  // CHECK-NEXT: [[LD16:%[0-9]+]] = load i32*, i32** %a, align 8
  // CHECK-NEXT: [[LD:%[0-9]+]] = load i32, i32* [[LD16]], align 4
  // CHECK-NEXT: %inc = add nsw i32 [[LD]], 1
  // CHECK-NEXT: store i32 %inc, i32* [[LD16]], align 4
  // CHECK-NEXT: br label %omp.body.continue{{[0-9]*}}

  // CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL.LOOP"()

#pragma omp parallel for firstprivate(foobar)
  for (auto &a : foobar.x)
    a++;
  return 0;
}

int main() {
  int res1 = test1(100);
  int res2 = test2();
  return 0;
}
// end INTEL_COLLAB
