// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

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
//CHECK: global_constant11
//CHECK: [[ANN11:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{internal_max_block_ram_depth:32}
//CHECK: global_constant14
//CHECK: [[ANN14:@.str[\.]*[0-9]*]] = {{.*}}{memory:MLAB}{sizeinfo:4}{pump:2}
//CHECK: global_constant15
//CHECK: [[ANN15:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_replicates:2}
//CHECK: global_constant16
//CHECK: [[ANN16:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{simple_dual_port:1}
//CHECK: [[ANN6A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_concurrency:4}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:bar:width}
//CHECK: [[ANN20:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,2}{simple_dual_port:1}
//CHECK: [[ANN21:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:12}{max_replicates:2}
//CHECK: [[ANN22:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:12,2}{max_replicates:2}
//CHECK: @llvm.global.annotations = {{.*}}global_constant1{{.*}}[[ANN2]]{{.*}}global_constant2{{.*}}[[ANN2]]{{.*}}global_constant5{{.*}}[[ANN4]]{{.*}}@global_constant6{{.*}}[[ANN5]]{{.*}}global_constant7{{.*}}[[ANN6]]{{.*}}global_constant8{{.*}}[[ANN7]]{{.*}}global_constant9{{.*}}[[ANN8]]{{.*}}global_constant10{{.*}}[[ANN9]]{{.*}}global_constant11{{.*}}[[ANN11]]{{.*}}global_constant14{{.*}}[[ANN14]]{{.*}}global_constant15{{.*}}[[ANN15]]{{.*}}global_constant16{{.*}}[[ANN16]]

constant int __attribute__((bank_bits(4, 5))) global_constant1 = 0;
constant int __attribute__((numbanks(4), bank_bits(4, 5))) global_constant2 = 0;
constant int __attribute__((register)) global_constant5 = 0;
constant int __attribute__((__memory__)) global_constant6 = 0;
constant int __attribute__((__bankwidth__(4))) global_constant7 = 0;
constant int __attribute__((singlepump)) global_constant8 = 0;
constant int __attribute__((doublepump)) global_constant9 = 0;
constant int __attribute__((merge("foo", "depth"))) global_constant10 = 0;
constant int __attribute__((internal_max_block_ram_depth(32))) global_constant11 = 0;
constant int __attribute__((doublepump, memory("MLAB"))) global_constant14 = 0;
constant int __attribute__((max_replicates(2))) global_constant15 = 0;
constant int __attribute__((simple_dual_port)) global_constant16 = 0;

//__attribute__((ihc_component))
void foo_two() {
  //CHECK: %[[VAR_TWO:[0-9]+]] = bitcast{{.*}}var_two
  //CHECK: %[[VAR_TWO1:var_two[0-9]+]] = bitcast{{.*}}var_two
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWO1]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: %[[VAR_THREE:[0-9]+]] = bitcast{{.*}}var_three
  //CHECK: %[[VAR_THREE1:var_three[0-9]+]] = bitcast{{.*}}var_three
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THREE1]],{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: %[[VAR_FOUR:[0-9]+]] = bitcast{{.*}}var_four
  //CHECK: %[[VAR_FOUR1:var_four[0-9]+]] = bitcast{{.*}}var_four
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FOUR1]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: %[[VAR_SEVEN:[0-9]+]] = bitcast{{.*}}var_seven
  //CHECK: %[[VAR_SEVEN1:var_seven[0-9]+]] = bitcast{{.*}}var_seven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SEVEN1]],{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: %[[VAR_EIGHT:[0-9]+]] = bitcast{{.*}}var_eight
  //CHECK: %[[VAR_EIGHT1:var_eight[0-9]+]] = bitcast{{.*}}var_eight
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_EIGHT1]],{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: %[[VAR_NINE:[0-9]+]] = bitcast{{.*}}var_nine
  //CHECK: %[[VAR_NINE1:var_nine[0-9]+]] = bitcast{{.*}}var_nine
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_NINE1]],{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: %[[VAR_NINE_TWO:[0-9]+]] = bitcast{{.*}}var_nine_two
  //CHECK: %[[VAR_NINE_TWO1:var_nine_two[0-9]+]] = bitcast{{.*}}var_nine_two
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_NINE_TWO1]],{{.*}}[[ANN6A]]
  int __attribute__((__max_concurrency__(4))) var_nine_two;
  //CHECK: %[[VAR_TEN:[0-9]+]] = bitcast{{.*}}var_ten
  //CHECK: %[[VAR_TEN1:var_ten[0-9]+]] = bitcast{{.*}}var_ten
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TEN1]],{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: %[[VAR_ELEVEN:[0-9]+]] = bitcast{{.*}}var_eleven
  //CHECK: %[[VAR_ELEVEN1:var_eleven[0-9]+]] = bitcast{{.*}}var_eleven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_ELEVEN1]],{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: %[[VAR_TWELVE:[0-9]+]] = bitcast{{.*}}var_twelve
  //CHECK: %[[VAR_TWELVE1:var_twelve[0-9]+]] = bitcast{{.*}}var_twelve
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWELVE1]],{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: %[[VAR_THIRTEEN:[0-9]+]] = bitcast{{.*}}var_thirteen
  //CHECK: %[[VAR_THIRTEEN1:var_thirteen[0-9]+]] = bitcast{{.*}}var_thirteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THIRTEEN1]],{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
  //CHECK: %[[VAR_FOURTEEN:[0-9]+]] = bitcast{{.*}}var_fourteen
  //CHECK: %[[VAR_FOURTEEN1:var_fourteen[0-9]+]] = bitcast{{.*}}var_fourteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FOURTEEN1]],{{.*}}[[ANN14]]
  int __attribute__((doublepump, memory("MLAB"))) var_fourteen;
  //CHECK: %[[VAR_FIFTEEN:[0-9]+]] = bitcast{{.*}}var_fifteen
  //CHECK: %[[VAR_FIFTEEN1:var_fifteen[0-9]+]] = bitcast{{.*}}var_fifteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FIFTEEN1]],{{.*}}[[ANN15]]
  int __attribute__((max_replicates(2))) var_fifteen;
  //CHECK: %[[VAR_SIXTEEN:[0-9]+]] = bitcast{{.*}}var_sixteen
  //CHECK: %[[VAR_SIXTEEN1:var_sixteen[0-9]+]] = bitcast{{.*}}var_sixteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SIXTEEN1]],{{.*}}[[ANN16]]
  int __attribute__((simple_dual_port)) var_sixteen;
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
  int __attribute__((internal_max_block_ram_depth(32))) f11;
  int __attribute__((doublepump, memory("MLAB"))) f14;
  int __attribute__((max_replicates(2))) f15;
  int __attribute__((simple_dual_port)) f16;
};

void bar() {
  struct foo_three s1;
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD1]]{{.*}}[[ANN2]]
  s1.f1 = 0;

  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD2]]{{.*}}[[ANN2]]
  s1.f2 = 0;

  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD3]]{{.*}}[[ANN2]]
  s1.f5 = 0;

  //CHECK: %[[FIELD6:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD6]]{{.*}}[[ANN5]]
  s1.f6 = 0;

  //CHECK: %[[FIELD7:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD7]]{{.*}}[[ANN6]]
  s1.f7 = 0;

  //CHECK: %[[FIELD8:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD8]]{{.*}}[[ANN7]]
  s1.f8 = 0;

  //CHECK: %[[FIELD9:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD9]]{{.*}}[[ANN8]]
  s1.f9 = 0;

  //CHECK: %[[FIELD11:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD11]]{{.*}}[[ANN11]]
  s1.f11 = 0;

  //CHECK: %[[FIELD14:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD14]]{{.*}}[[ANN14]]
  s1.f14 = 0;

  s1.f15 = 0;
  //CHECK: %[[FIELD15:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD15]]{{.*}}[[ANN15]]

  s1.f16 = 0;
  //CHECK: %[[FIELD16:.*]] = getelementptr inbounds %struct.foo_three{{.*}}
  //CHECK: call i32* @llvm.ptr.annotation.p0i32{{.*}}%[[FIELD16]]{{.*}}[[ANN16]]
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
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_five, %struct.foo_five* {{.*}}, i32 0, i32 0
  //CHECK: call i8* @llvm.ptr.annotation.p0i8(i8* %[[FIELD1]], {{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f2[0] = 0;
  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_five, %struct.foo_five* {{.*}}, i32 0, i32 1
  //CHECK: call [2 x i8]* @llvm.ptr.annotation.p0a2i8([2 x i8]* %[[FIELD2]], {{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f3.f1[0] = 0;
  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_five, %struct.foo_five* {{.*}}, i32 0, i32 2
  //CHECK: call %struct.foo_four* @llvm.ptr.annotation.p0s_struct.foo_fours(%struct.foo_four* %[[FIELD3]], {{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f4[0].f1[0] = 0;
  //CHECK: %[[FIELD4:.*]] = getelementptr inbounds %struct.foo_five, %struct.foo_five* {{.*}}, i32 0, i32 3
  //CHECK: call [2 x %struct.foo_four]* @llvm.ptr.annotation.p0a2s_struct.foo_fours([2 x %struct.foo_four]* %[[FIELD4]], {{.*}}getelementptr{{.*}}[[ANN4]]
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
  //CHECK: %[[FIELD1:.*]] = getelementptr inbounds %struct.foo_seven, %struct.foo_seven* {{.*}}, i32 0, i32 0
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %[[FIELD1]], {{.*}}getelementptr{{.*}}[[ANN15]]

  s3.f2[1] = 0;
  //CHECK: %[[FIELD2:.*]] = getelementptr inbounds %struct.foo_seven, %struct.foo_seven* {{.*}}, i32 0, i32 1
  //CHECK: call [2 x i32]* @llvm.ptr.annotation.p0a2i32([2 x i32]* %[[FIELD2]],{{.*}}getelementptr{{.*}}[[ANN20]]

  s3.f5.f1[0] = 0;
  //CHECK: %[[FIELD3:.*]] = getelementptr inbounds %struct.foo_seven, %struct.foo_seven* {{.*}}, i32 0, i32 4
  //CHECK: call %struct.foo_six* @llvm.ptr.annotation.p0s_struct.foo_sixs(%struct.foo_six* %[[FIELD3]],{{.*}}[[ANN21]]

  s3.f5.f2 = 0;
  //CHECK: %[[FIELD4:.*]] = getelementptr inbounds %struct.foo_seven, %struct.foo_seven* {{.*}}, i32 0, i32 4
  //CHECK: call %struct.foo_six* @llvm.ptr.annotation.p0s_struct.foo_sixs(%struct.foo_six* %[[FIELD4]],{{.*}}[[ANN21]]

  s3.f6[0].f1[0] = 0;
  //CHECK: %[[FIELD5:.*]] = getelementptr inbounds %struct.foo_seven, %struct.foo_seven* {{.*}}, i32 0, i32 5
  //CHECK: call [2 x %struct.foo_six]* @llvm.ptr.annotation.p0a2s_struct.foo_sixs([2 x %struct.foo_six]* %[[FIELD5]],{{.*}}[[ANN22]]
}

