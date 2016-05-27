// RUN: %clang_cc1 -O0 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define __va_copy(d,s) __builtin_va_copy(d,s)
int printf(const char* restrict format, ...); 

#ifndef TTT
#define TTT struct{double x,y;}
#endif

#define x(p) (((double*)&(p))[0])
#define y(p) (((double*)&(p))[1])

typedef TTT S2;
// CHECK: [[S2:%.+]] = type { double, double }
// CHECK: [[__va_list_tag:%.+]] = type { i32, i32, i8*, i8* }
S2
#ifdef VC
__vectorcall
#endif

AddS2s(S2 p1, ...)
// CHECK: define [[S2]] @AddS2s
{
  S2 p2;
  S2 p3;
  double x5;
  va_list ap;
//CHECK: [[p1:%.+]] = alloca [[S2]]
//CHECK: [[p2:%.+]] = alloca [[S2]]
//CHECK: [[p3:%.+]] = alloca [[S2]]
//CHECK: [[x5:%.+]] = alloca double
//CHECK: [[ap:%.+]] = alloca [1 x [[__va_list_tag]]], 
//CHECK: [[tmp:%.+]] = alloca [[S2]], 

  va_start(ap,p1);
//CHECK: [[arraydecay:%.+]] = getelementptr inbounds [1 x [[__va_list_tag]]], [1 x [[__va_list_tag]]]* %ap, i32 0, i32 0
//CHECK: [[arraydecay1:%.+]] = bitcast [[__va_list_tag]]* [[arraydecay]] to i8*
//CHECK: [[arraydecay2:%.+]] = getelementptr inbounds [1 x [[__va_list_tag]]], [1 x [[__va_list_tag]]]* %ap, i32 0, i32 0
// CHECK:  [[fp_offset_p:%.+]] = getelementptr inbounds [[__va_list_tag]], [[__va_list_tag]]* [[arraydecay2]], i32 0, i32 1

  p2 = va_arg(ap,S2);
//CHECK: [[t2:%.+]] = getelementptr inbounds [[__va_list_tag]], [[__va_list_tag]]* [[arraydecay2]], i32 0, i32 3
//CHECK: [[reg_save_area:%.+]] = load i8*, i8** [[t2]], 
//CHECK: [[t3:%.+]] = getelementptr i8, i8* [[reg_save_area]], i32 %fp_offset
//CHECK: [[t4:%.+]] = getelementptr inbounds i8, i8* [[t3]], i64 16
//CHECK: [[t5:%.+]] = bitcast [[S2]]* %tmp to { double, double }*
//CHECK: [[t6:%.+]] = bitcast i8* [[t3]] to double*
//CHECK: [[t7:%.+]] = load double, double* [[t6]], 
//CHECK: [[t8:%.+]] = getelementptr inbounds { double, double }, { double, double }* [[t5]], i32 0, i32 0
//CHECK: store double [[t7]], double* [[t8]], 
//CHECK: [[t9:%.+]] = bitcast i8* [[t4]] to double*
//CHECK: [[t10:%.+]] = load double, double* [[t9]], 
//CHECK: [[t11:%.+]] = getelementptr inbounds { double, double }, { double, double }* [[t5]], i32 0, i32 1
//CHECK: store double [[t10]], double* [[t11]], 
//CHECK: [[t12:%.+]] = bitcast { double, double }* [[t5]] to [[S2]]*
//CHECK: [[t13:%.+]] = add i32 %fp_offset, 32
//CHECK: store i32 [[t13]], i32* [[fp_offset_p]], 

//CHECK: [[vaarg_in_mem:.+]]:
//CHECK: [[vaarg_end:.+]]:
//CHECK_NEXT: [[vaarg_addr:%.+]] = phi [[S2]]* [ [[t12]],
//CHECK_NEXT: [[t15:%.+]] = bitcast [[S2]]* [[p2]] to i8*
//CHECK_NEXT: [[t16:%.+]] = bitcast [[S2]]* [[vaarg_addr]] to i8*
//CHECK_NEXT: call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[t15]], i8* [[t16]], i64 16, i32 8, i1 false)
//CHECK_NEXT: [[arraydecay3:%.+]] = getelementptr inbounds [1 x [[__va_list_tag]]], [1 x [[__va_list_tag]]]* %ap, i32 0, i32 0
//CHECK_NEXT: [[fp_offset_p4:%.+]] = getelementptr inbounds [[__va_list_tag]],[[__va_list_tag]]* [[arraydecay3]], i32 0, i32 1
//CHECK_NEXT: [[fp_offset5:%.+]] = load i32, i32* %fp_offset_p4,

//CHECK: [[vaarg_in_reg7:.+]]: 
//CHECK_NEXT: [[t17:%.+]] = getelementptr inbounds [[__va_list_tag]], [[__va_list_tag]]* %arraydecay3, i32 0, i32 3
//CHECK_NEXT: [[reg_save_area8:%.+]] = load i8*, i8** [[t17]], 
//CHECK_NEXT: [[t18:%.+]] = getelementptr i8, i8* [[reg_save_area8]], i32 [[fp_offset5]]
//CHECK_NEXT: [[t19:%.+]] = bitcast i8* [[t18]] to double*
//CHECK_NEXT: [[t20:%.+]] = add i32 [[fp_offset5]], 16
//CHECK_NEXT: store i32 [[t20]], i32* [[fp_offset_p4]], 
x5 = va_arg(ap,double);
  x(p3) = x(p1) + 10*x(p2) + 100*x5;
  y(p3) = y(p1) + 10*y(p2) + 100*x5;
  return p3;
}
//CHECK: }

S2
#ifdef VC
__vectorcall
#endif
AddS2s(S2 p1, ...);

S2 foo()
{
  S2 p1 = {1,2}, p2 = {3,4};
  double   x5 = 5;
  S2 p3 = AddS2s(p1, p2, x5);
  return p3;
}

int main()
{
  S2 p3;
  p3 = foo();
  printf ("%g %g\n", x(p3), y(p3));
}

