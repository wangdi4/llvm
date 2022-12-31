//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -fkeep-static-consts -emit-llvm -opaque-pointers -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -fkeep-static-consts -debug-info-kind=limited -emit-llvm -opaque-pointers -o %t %s

//CHECK: @_ZL3gc1 = internal constant i32 0, align 4
//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_replicates:2}
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{simple_dual_port:1}
//CHECK: @[[Struct3:.*]] = internal global %struct.foo_three zeroinitializer, align 4
//CHECK: @[[Struct5:.*]] = internal global %struct.foo_five zeroinitializer, align 1
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:1}{max_replicates:2}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:1,2}{max_replicates:2}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:1}{simple_dual_port:1}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:1,2}{simple_dual_port:1}
//CHECK: @_ZL7array_a = internal global [8 x i32] zeroinitializer, align 16
//CHECK: @_ZL7array_b = internal global [8 x i32] zeroinitializer, align 16
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{staticreset:2}
//CHECK: [[ANN20:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,8}{max_replicates:3}{staticreset:1}
//CHECK: [[ANN21:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,8}{max_replicates:4}{staticreset:1}
//CHECK: [[ANN11:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}{numbanks:8}{max_replicates:2}{max_concurrency:4}
//CHECK: [[ANN12:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}{numbanks:8}{simple_dual_port:1}{max_concurrency:4}
//CHECK: [[ANN13:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}{private_copies:11}{numbanks:8}{simple_dual_port:1}
//CHECK: [[ANN14:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_replicates:3}
//CHECK: [[ANN15:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:8}{bank_bits:4,3,2}{max_replicates:3}
//CHECK: [[ANN16:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:8}{bank_bits:4,3,2}{simple_dual_port:1}
//CHECK: [[ANN17:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:8}{simple_dual_port:1}
//CHECK: [[ANN18:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_replicates:3}{simple_dual_port:1}

const int __attribute__((max_replicates(2))) gc1 = 0;
const int __attribute__((max_replicates(2), simple_dual_port)) gc3 = 0;

void foo() {
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %lc1, ptr [[ANN1]]
  int __attribute__((max_replicates(2))) lc1;
  int __attribute__((simple_dual_port)) lc3 = 0;
  int __attribute__((max_replicates(2), simple_dual_port)) lc4 = 0;
}

template <int bankwidth, int numbanks, int readports, int writeports,
          int bit1, int bit2, int bit3, int max_concurrency, int num_replicates,
          int private_copies>
__attribute__((ihc_component)) void foo_one() {
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_one, ptr [[ANN11]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_two, ptr [[ANN12]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_four, ptr [[ANN13]]
  __attribute__((bankwidth(bankwidth), numbanks(numbanks),
                 max_concurrency(max_concurrency),
                 max_replicates(num_replicates))) int var_one;
  __attribute__((bankwidth(bankwidth), numbanks(numbanks),
                 max_concurrency(max_concurrency),
                 simple_dual_port)) int var_two;
  __attribute__((simple_dual_port, max_replicates(num_replicates))) int var_three;
  __attribute__((bankwidth(bankwidth), numbanks(numbanks),
                 private_copies(private_copies),
                 simple_dual_port)) int var_four;
}


template <int numbanks, int bit, int num_replicates>
__attribute__((ihc_component)) void foo_two() {
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_one, ptr [[ANN14]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_two, ptr [[ANN15]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_four, ptr [[ANN4]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_five, ptr [[ANN16]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_six, ptr [[ANN17]]
  //CHECK: call void @llvm.var.annotation.p0.p0(ptr %var_seven, ptr [[ANN18]]

  __attribute__((max_replicates(num_replicates))) int var_one;
  __attribute__((max_replicates(num_replicates), __bank_bits__(4, bit, 2))) int var_two;
  __attribute__((simple_dual_port)) int var_four;
  __attribute__((simple_dual_port, __bank_bits__(4, bit, 2))) int var_five;
  __attribute__((numbanks(numbanks), simple_dual_port)) int var_six;
  __attribute__((max_replicates(num_replicates), simple_dual_port)) int var_seven;
}

void call() {
  foo_one<4, 8, 2, 3, 2, 3, 4, 4, 2, 11>();
  foo_two<8, 3, 3>();
}

struct foo_three {
  int __attribute__((max_replicates(2))) f3;
  int __attribute__((simple_dual_port)) f4;
};

static foo_three s1;

void bar1() {
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr @[[Struct3]], ptr [[ANN1]]
  s1.f3 = 0;
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%struct.foo_three, ptr @[[Struct3]], i32 0, i32 1), ptr [[ANN4]]
  s1.f4 = 0;
}

struct foo_four {
  char f1[2];
  short f2;
};

struct foo_five {
  char __attribute__((max_replicates(2))) f1;
  char __attribute__((max_replicates(2))) f2[2];
  char __attribute__((simple_dual_port)) f3;
  char __attribute__((simple_dual_port)) f4[2];
};

static foo_five s2;

void bar2() {
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr @[[Struct5]], ptr [[ANN6]]
  s2.f1 = 0;
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%struct.foo_five, ptr @[[Struct5]], i32 0, i32 1), ptr [[ANN7]]
  s2.f2[0] = 0;
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%struct.foo_five, ptr @[[Struct5]], i32 0, i32 2), ptr [[ANN8]]
  s2.f3 = 0;
  //CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%struct.foo_five, ptr @[[Struct5]], i32 0, i32 3), ptr [[ANN9]]
  s2.f4[0] = 0;
}

static int array_a[8] __attribute__((static_array_reset(1), max_replicates(3)));
static int array_b[8] __attribute__((max_replicates(4),static_array_reset(1)));

void goo()
{
  //CHECK: store i32 2, ptr @_ZL7array_a
  array_a[0] = 2;
  //CHECK: store i32 0, ptr getelementptr inbounds ([8 x i32], ptr @_ZL7array_b, i64 0, i64 2)
  array_b[2] = 0;
}
