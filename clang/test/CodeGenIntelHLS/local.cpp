//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -fkeep-static-consts -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -fkeep-static-consts -debug-info-kind=limited -emit-llvm -o %t %s

//CHECK: @_ZL13global_const1 = internal constant i32 0, align 4
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: @_ZL13global_const2 = internal constant i32 0, align 4
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:1}
//CHECK: _ZL13global_const3 = internal constant i32 0, align 4
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:2}
//CHECK: @_ZL13global_const5 = internal constant i32 0, align 4
//CHECK: [[ANN5A:@.str[\.]*[0-9]*]] = {{.*}}{memory:MLAB}{sizeinfo:4}
//CHECK: @_ZL13global_const6 = internal constant i32 0, align 4
//CHECK: [[ANN5B:@.str[\.]*[0-9]*]] = {{.*}}{memory:BLOCK_RAM}{sizeinfo:4}
//CHECK: @_ZL13global_const7 = internal constant i32 0, align 4
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:4,5}
//CHECK: @_ZL13global_const8 = internal constant i32 0, align 4
//CHECK: @_ZL14global_const11 = internal constant i32 0, align 4
//CHECK: [[ANN13:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{internal_max_block_ram_depth:32}
//CHECK: @_ZL14global_const12 = internal constant i32 0, align 4
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}
//CHECK: @_ZL14global_const13 = internal constant i32 0, align 4
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:foo:depth}
//CHECK: @_ZL14global_const16 = internal constant i32 0, align 4
//CHECK: [[ANN14:@.str[\.]*[0-9]*]] = {{.*}}{staticreset:1}
//CHECK: @_ZL14global_const17 = internal constant i32 0, align 4
//CHECK: [[ANN15:@.str[\.]*[0-9]*]] = {{.*}}{memory:MLAB}{sizeinfo:4}{pump:2}
//CHECK: [[ANN2A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:5,4}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: [[ANN6A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{max_concurrency:4}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:bar:width}
//CHECK: @[[Struct1:.*]] = internal global %struct.foo_three zeroinitializer, align 4
//CHECK: @[[Struct2:.*]] = internal global %struct.foo_five zeroinitializer, align 2
//CHECK: [[ANN40:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:512,10}
//CHECK: [[ANN41:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,120}
//CHECK: [[ANN42:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,16,8}{numbanks:2}{bank_bits:4}
//CHECK: [[ANN43:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:2}{bank_bits:4}
//CHECK: [[ANN44:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:2,10}{numbanks:2}{bank_bits:4}
//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:1}{bankwidth:4}{numbanks:8}{merge:merge_foo_one:depth}{max_concurrency:4}{bank_bits:2,3,4}
//CHECK: [[ANN1A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:8}{bank_bits:4,3,2}
//CHECK: @llvm.global.annotations = appending global{{.*}}@_ZL13global_const1 {{.*}}[[ANN4]]{{.*}}@_ZL13global_const2{{.*}}[[ANN7]]{{.*}}@_ZL13global_const3{{.*}}[[ANN8]]{{.*}}@_ZL13global_const5{{.*}}@_ZL13global_const6{{.*}}[[ANN5B]]{{.*}}@_ZL13global_const7{{.*}}[[ANN2]]{{.*}}@_ZL13global_const8{{.*}}[[ANN2]]{{.*}}@_ZL14global_const11{{.*}}[[ANN13]]{{.*}}@_ZL14global_const12{{.*}}[[ANN6]]{{.*}}@_ZL14global_const13{{.*}}[[ANN9]]{{.*}}@_ZL14global_const16{{.*}}[[ANN14]]{{.*}}@_ZL14global_const17{{.*}}[[ANN15]]

const int __attribute__((register)) global_const1 = 0;
const int __attribute__((singlepump)) global_const2 = 0;
const int __attribute__((doublepump)) global_const3 = 0;
const int __attribute__((__memory__("MLAB"))) global_const5 = 0;
const int __attribute__((__memory__("BLOCK_RAM"))) global_const6 = 0;
const int __attribute__((bank_bits(4, 5))) global_const7 = 0;
const int __attribute__((numbanks(4), bank_bits(4, 5))) global_const8 = 0;
const int __attribute__((internal_max_block_ram_depth(32))) global_const11 = 0;
const int __attribute__((__bankwidth__(4))) global_const12 = 0;
const int __attribute__((merge("foo", "depth"))) global_const13 = 0;
const int __attribute__((static_array_reset(1))) global_const16 = 0;
const int __attribute__((doublepump, memory("MLAB"))) global_const17 = 0;

__attribute__((ihc_component))
void foo_two() {
  //CHECK: llvm.var.annotation{{.*}}var_two{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: llvm.var.annotation{{.*}}var_three{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: llvm.var.annotation{{.*}}var_four{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: llvm.var.annotation{{.*}}var_four_A{{.*}}[[ANN2A]]
  int __attribute__((bank_bits(5,4),numbanks(4))) var_four_A;
  //CHECK: llvm.var.annotation{{.*}}var_seven{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: llvm.var.annotation{{.*}}var_eight{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: llvm.var.annotation{{.*}}var_eightA{{.*}}[[ANN5A]]
  int __attribute__((__memory__("MLAB"))) var_eightA;
  //CHECK: llvm.var.annotation{{.*}}var_eightB{{.*}}[[ANN5B]]
  int __attribute__((__memory__("BLOCK_RAM"))) var_eightB;
  //CHECK: llvm.var.annotation{{.*}}var_nine{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: llvm.var.annotation{{.*}}var_nine_two{{.*}}[[ANN6A]]
  int __attribute__((__max_concurrency__(4))) var_nine_two;
  //CHECK: llvm.var.annotation{{.*}}var_ten{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: llvm.var.annotation{{.*}}var_eleven{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: llvm.var.annotation{{.*}}var_twelve{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: llvm.var.annotation{{.*}}var_thirteen{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
  //CHECK: llvm.var.annotation{{.*}}var_sixteen{{.*}}[[ANN15]]
  int __attribute__((doublepump, memory("MLAB"))) var_sixteen = 0;
}

template <int bankwidth, int numbanks, int readports, int writeports,
          int bit1, int bit2, int bit3, int max_concurrency>
__attribute__((ihc_component))
void foo_one()
{
  __attribute__((bankwidth(bankwidth),numbanks(numbanks),
                 merge("merge_foo_one","depth"), memory, singlepump,
                 __bank_bits__(bit1,bit2,bit3),
                 max_concurrency(max_concurrency)))
  int var_one;
}

//CHECK: define{{.*}}foo_one{{.*}}!ihc_component
//CHECK: llvm.var.annotation{{.*}}var_one{{.*}}[[ANN1]]

template <int numbanks, int bit>
__attribute__((ihc_component))
void foo_two()
{
  __attribute__((numbanks(numbanks), __bank_bits__(4,bit,2)))
  int var_two;
}

//CHECK: define{{.*}}foo_two{{.*}}!ihc_component
//CHECK: llvm.var.annotation{{.*}}var_two{{.*}}[[ANN1A]]

void call()
{
  foo_one<4,8,2,3,2,3,4,4>();
  foo_two<8,3>();
}

struct foo_three {
  int __attribute__((register)) f1;
  int __attribute__((singlepump)) f2;
  int __attribute__((doublepump)) f3;
  int __attribute__((__memory__)) f4;
  int __attribute__((__memory__("MLAB"))) f5;
  int __attribute__((__memory__("BLOCK_RAM"))) f6;
  int __attribute__((bank_bits(4, 5))) f7;
  int __attribute__((numbanks(4), bank_bits(4, 5))) f8;
  int __attribute__((internal_max_block_ram_depth(32))) f11;
  int __attribute__((__bankwidth__(4))) f12;
  int __attribute__((merge("foo", "depth"))) f13;
  int __attribute__((static_array_reset(1))) f16;
  int __attribute__((doublepump, memory("MLAB"))) f17;
};

static foo_three s1;

void bar1() {
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 0){{.*}}getelementptr{{.*}}[[ANN4]]
  s1.f1 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 1){{.*}}getelementptr{{.*}}[[ANN7]]
  s1.f2 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 2){{.*}}getelementptr{{.*}}[[ANN8]]
  s1.f3 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 3){{.*}}getelementptr{{.*}}[[ANN5]]
  s1.f4 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 4){{.*}}getelementptr{{.*}}[[ANN5A]]
  s1.f5 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 5){{.*}}getelementptr{{.*}}[[ANN5B]]
  s1.f6 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 6){{.*}}getelementptr{{.*}}[[ANN2]]
  s1.f7 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 7){{.*}}getelementptr{{.*}}[[ANN2]]
  s1.f8 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 8){{.*}}getelementptr{{.*}}[[ANN13]]
  s1.f11 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 9){{.*}}getelementptr{{.*}}[[ANN6]]
  s1.f12 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 10){{.*}}getelementptr{{.*}}[[ANN9]]
  s1.f13 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 11){{.*}}getelementptr{{.*}}[[ANN14]]
  s1.f16 = 0;
  //CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%struct.foo_three, %struct.foo_three* @[[Struct1]], i32 0, i32 12){{.*}}getelementptr{{.*}}[[ANN15]]
  s1.f17 = 0;
}

struct foo_four {
  char f1[2];
  short f2;
};

struct foo_five {
  char __attribute__((register)) f1;
  char __attribute__((register)) f2[2];
  foo_four __attribute__((register)) f3;
  foo_four __attribute__((register)) f4[2];
};

static foo_five s2;

void bar2() {
  s2.f1 = 0;
  //CHECK: call i8* @llvm.ptr.annotation.p0i8(i8* getelementptr inbounds (%struct.foo_five, %struct.foo_five* @[[Struct2]], i32 0, i32 0){{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f2[0] = 0;
  //CHECK: call [2 x i8]* @llvm.ptr.annotation.p0a2i8([2 x i8]* getelementptr inbounds (%struct.foo_five, %struct.foo_five* @[[Struct2]], i32 0, i32 1){{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f3.f1[0] = 0;
  //CHECK: call %struct.foo_four* @llvm.ptr.annotation.p0s_struct.foo_fours(%struct.foo_four* getelementptr inbounds (%struct.foo_five, %struct.foo_five* @[[Struct2]], i32 0, i32 2){{.*}}getelementptr{{.*}}[[ANN4]]
  s2.f4[0].f1[0] = 0;
  //CHECK: call [2 x %struct.foo_four]* @llvm.ptr.annotation.p0a2s_struct.foo_fours([2 x %struct.foo_four]* getelementptr inbounds (%struct.foo_five, %struct.foo_five* @[[Struct2]], i32 0, i32 3){{.*}}getelementptr{{.*}}[[ANN4]]
}

struct pack {
  int mem[120] __attribute__((memory));
  int reg[8];
};

int foo() {
  struct pack p1;
  struct pack p2[10] __attribute__((memory));
  //CHECK: call void @llvm.var.annotation(i8* %p21, i8* getelementptr{{.*}}[[ANN40]]
  p1.mem[4] = 10;
  //CHECK: call [120 x i32]* @llvm.ptr.annotation.p0a120i32([120 x i32]* %mem, i8* getelementptr{{.*}}[[ANN41]]
  return 0;
}

int main ()
{
  int a[16][8] __attribute__((bank_bits(4)));
  //CHECK: call void @llvm.var.annotation(i8* %a1, i8* getelementptr{{.*}}[[ANN42]]
  int b __attribute__((bank_bits(4)));
  //CHECK: call void @llvm.var.annotation(i8* %b2, i8* getelementptr{{.*}}[[ANN43]]
  short c[10] __attribute__((bank_bits(4)));
  //CHECK: call void @llvm.var.annotation(i8* %c3, i8* getelementptr{{.*}}[[ANN44]]

  return 0;
}
