// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -disable-llvm-passes -emit-llvm -opaque-pointers %s -o - | FileCheck %s

//CHECK: global_constant1
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:4,5}
//CHECK: global_constant2
//CHECK: global_constant5
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: global_constant6
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: global_constant7
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}
//CHECK: global_constant8
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:1}
//CHECK: global_constant9
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:2}
//CHECK: global_constant10
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:foo:depth}
//CHECK: global_constant14
//CHECK: [[ANN14:@.str[\.]*[0-9]*]] = {{.*}}{memory:MLAB}{sizeinfo:4}{pump:2}
//CHECK: global_constant15
//CHECK: [[ANN15:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_replicates:2}
//CHECK: global_constant16
//CHECK: [[ANN16:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{simple_dual_port:1}
//CHECK: @global_constant17
//CHECK: [[ANN30:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{force_pow2_depth:0}
//CHECK: @global_constant18
//CHECK: [[ANN31:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{force_pow2_depth:1}
//CHECK: [[ANN6A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_concurrency:4}
//CHECK: [[ANN6B:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{private_copies:4}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:bar:width}
//CHECK: [[ANN20:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,2}{simple_dual_port:1}
//CHECK: [[ANN21:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:12}{max_replicates:2}
//CHECK: [[ANN22:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:12,2}{max_replicates:2}
//CHECK: @llvm.global.annotations = {{.*}}global_constant1{{.*}}[[ANN2]]{{.*}}global_constant2{{.*}}[[ANN2]]{{.*}}global_constant5{{.*}}[[ANN4]]{{.*}}@global_constant6{{.*}}[[ANN5]]{{.*}}global_constant7{{.*}}[[ANN6]]{{.*}}global_constant8{{.*}}[[ANN7]]{{.*}}global_constant9{{.*}}[[ANN8]]{{.*}}global_constant10{{.*}}[[ANN9]]{{.*}}global_constant14{{.*}}[[ANN14]]{{.*}}global_constant15{{.*}}[[ANN15]]{{.*}}global_constant16{{.*}}[[ANN16]]{{.*}}global_constant17{{.*}}[[ANN30]]{{.*}}global_constant18{{.*}}[[ANN31]]

constant int __attribute__((bank_bits(4, 5))) global_constant1 = 0;
constant int __attribute__((numbanks(4), bank_bits(4, 5))) global_constant2 = 0;
constant int __attribute__((register)) global_constant5 = 0;
constant int __attribute__((__memory__)) global_constant6 = 0;
constant int __attribute__((__bankwidth__(4))) global_constant7 = 0;
constant int __attribute__((singlepump)) global_constant8 = 0;
constant int __attribute__((doublepump)) global_constant9 = 0;
constant int __attribute__((merge("foo", "depth"))) global_constant10 = 0;
constant int __attribute__((doublepump, memory("MLAB"))) global_constant14 = 0;
constant int __attribute__((max_replicates(2))) global_constant15 = 0;
constant int __attribute__((simple_dual_port)) global_constant16 = 0;
constant int __attribute__((__force_pow2_depth__(0))) global_constant17 = 0;
constant int __attribute__((__force_pow2_depth__(1))) global_constant18 = 0;

//__attribute__((ihc_component))
void foo_two() {
  //CHECK: llvm.var.annotation{{.*}}%var_two,{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: llvm.var.annotation{{.*}}%var_three,{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: llvm.var.annotation{{.*}}%var_four,{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: llvm.var.annotation{{.*}}%var_seven,{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: llvm.var.annotation{{.*}}%var_eight,{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: llvm.var.annotation{{.*}}%var_nine,{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: llvm.var.annotation{{.*}}%var_nine_two,{{.*}}[[ANN6A]]
  int __attribute__((__max_concurrency__(4))) var_nine_two;
  //CHECK: llvm.var.annotation{{.*}}%var_nine_three,{{.*}}[[ANN6B]]
  int __attribute__((__private_copies__(4))) var_nine_three;
  //CHECK: llvm.var.annotation{{.*}}%var_ten,{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: llvm.var.annotation{{.*}}%var_eleven,{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: llvm.var.annotation{{.*}}%var_twelve,{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: llvm.var.annotation{{.*}}%var_thirteen,{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
  //CHECK: llvm.var.annotation{{.*}}%var_fourteen,{{.*}}[[ANN14]]
  int __attribute__((doublepump, memory("MLAB"))) var_fourteen;
  //CHECK: llvm.var.annotation{{.*}}%var_fifteen,{{.*}}[[ANN15]]
  int __attribute__((max_replicates(2))) var_fifteen;
  //CHECK: llvm.var.annotation{{.*}}%var_sixteen,{{.*}}[[ANN16]]
  int __attribute__((simple_dual_port)) var_sixteen;
  //CHECK: llvm.var.annotation{{.*}}%var_seventeen,{{.*}}[[ANN30]]
  int __attribute__((__force_pow2_depth__(0))) var_seventeen;
  //CHECK: llvm.var.annotation{{.*}}%var_eighteen,{{.*}}[[ANN31]]
  int __attribute__((__force_pow2_depth__(1))) var_eighteen;
}

struct foo_three{
  int __attribute__((bank_bits(4,5))) f1;
  int __attribute__((numbanks(4),bank_bits(4,5))) f2;
  int __attribute__((register)) f5;
  int __attribute__((__memory__)) f6;
  int __attribute__((__bankwidth__(4))) f7;
  int __attribute__((singlepump)) f8;
  int __attribute__((doublepump)) f9;
  int __attribute__((merge("foo","depth"))) f10;
  int __attribute__((doublepump, memory("MLAB"))) f14;
  int __attribute__((max_replicates(2))) f15;
  int __attribute__((simple_dual_port)) f16;
  int __attribute__((__force_pow2_depth__(0))) f17;
  int __attribute__((__force_pow2_depth__(0))) f18;
};

void bar() {
  struct foo_three s1;
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD1]]{{.*}}[[ANN2]]
  s1.f1 = 0;

  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD2]]{{.*}}[[ANN2]]
  s1.f2 = 0;

  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD3]]{{.*}}[[ANN2]]
  s1.f5 = 0;

  //CHECK: %[[FIELD6:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD6]]{{.*}}[[ANN5]]
  s1.f6 = 0;

  //CHECK: %[[FIELD7:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD7]]{{.*}}[[ANN6]]
  s1.f7 = 0;

  //CHECK: %[[FIELD8:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD8]]{{.*}}[[ANN7]]
  s1.f8 = 0;

  //CHECK: %[[FIELD9:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD9]]{{.*}}[[ANN8]]
  s1.f9 = 0;

  //CHECK: %[[FIELD14:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD14]]{{.*}}[[ANN14]]
  s1.f14 = 0;

  s1.f15 = 0;
  //CHECK: %[[FIELD15:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD15]]{{.*}}[[ANN15]]

  s1.f16 = 0;
  //CHECK: %[[FIELD16:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD16]]{{.*}}[[ANN16]]

  s1.f17 = 0;
  //CHECK: %[[FIELD17:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD17]]{{.*}}[[ANN30]]

  s1.f18 = 0;
  //CHECK: %[[FIELD18:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call ptr @llvm.ptr.annotation.p0{{.*}}%[[FIELD18]]{{.*}}[[ANN30]]

}

struct foo_four {
  char f1[2];
  short f2;
};

struct foo_five {
  char __attribute__((register)) f1;
  char __attribute__((register)) f2[2];
  struct foo_four __attribute__((register)) f3;
  struct foo_four __attribute__((register)) f4[2];
};

void bar2() {
  struct foo_five s2;
  s2.f1 = 0;
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_five, ptr {{.*}}, i32 0, i32 0
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD1]], ptr [[ANN4]]
  s2.f2[0] = 0;
  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_five, ptr {{.*}}, i32 0, i32 1
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD2]], ptr [[ANN4]]
  s2.f3.f1[0] = 0;
  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_five, ptr {{.*}}, i32 0, i32 2
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD3]], ptr [[ANN4]]
  s2.f4[0].f1[0] = 0;
  //CHECK: %[[FIELD4:.*]] = getelementptr inbounds %struct.foo_five, ptr {{.*}}, i32 0, i32 3
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD4]], ptr [[ANN4]]
}

struct foo_six {
  int f1[2];
  int f2;
};
struct foo_seven {
  int __attribute__((max_replicates(2))) f1;
  int __attribute__((simple_dual_port)) f2[2];
  struct foo_six __attribute__((max_replicates(2))) f3;
  struct foo_six __attribute__((max_replicates(2))) f4[2];
  struct foo_six __attribute__((max_replicates(2))) f5;
  struct foo_six __attribute__((max_replicates(2))) f6[2];
};

void bar3() {
  struct foo_seven s3;
  s3.f1 = 0;
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_seven, ptr {{.*}}, i32 0, i32 0
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD1]], ptr [[ANN15]]

  s3.f2[1] = 0;
  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_seven, ptr {{.*}}, i32 0, i32 1
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD2]], ptr [[ANN20]]

  s3.f5.f1[0] = 0;
  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_seven, ptr {{.*}}, i32 0, i32 4
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD3]],{{.*}}[[ANN21]]

  s3.f5.f2 = 0;
  //CHECK: %[[FIELD4:.*]] = getelementptr inbounds %struct.foo_seven, ptr {{.*}}, i32 0, i32 4
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD4]],{{.*}}[[ANN21]]

  s3.f6[0].f1[0] = 0;
  //CHECK: %[[FIELD5:.*]] = getelementptr inbounds %struct.foo_seven, ptr {{.*}}, i32 0, i32 5
  //CHECK: call ptr @llvm.ptr.annotation.p0(ptr %[[FIELD5]],{{.*}}[[ANN22]]
}

