// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o -  | FileCheck %s

template <class T> struct CI {
public:
  CI();
  CI(int);
};

class CI1 {
public:
  CI1(int);
};

template <class T> struct CDI : public CI<T>, public CI1 {
  CDI() : CI(2), CI1(3) {}
};

template <class T> struct CDJ : public CI<T>, public CI1 {
  int CI;
  CDJ() : CI(4), CI1(5) {}
};

CDI<int> ca;
CDJ<int> cb;

//CHECK: [[CDI:%.+]] = type
//CHECK: [[CDJ:%.+]] = type
//CHECK: [[CI:%.+]] = type
//CHECK: [[CI1:%.+]] = type

//CHECK: define internal void [[__cxx_global_var_init:@.+]]()
//CHECK: call void [[CDI_Ctor:@.+]]([[CDI]]*{{.+}})

//CHECK: define linkonce_odr void [[CDI_Ctor]]([[CDI]]*{{.+}})
//CHECK: call void [[CDI1_Ctor:@.+]]([[CDI]]*{{.+}})

//CHECK: define internal void [[__cxx_global_var_init_1:@.+]]()
//CHECK: call void [[CDJ_Ctor:@.+]]([[CDJ]]*{{.+}})

//CHECK: define linkonce_odr void [[CDJ_Ctor]]([[CDJ]]*{{.+}})
//CHECK: call void [[CDJ1_Ctor:@.+]]([[CDJ]]*{{.+}})

//CHECK: define linkonce_odr void [[CDI1_Ctor]]([[CDI]]*{{.+}})
//CHECK: call void [[CI_Ctor:@.+]]([[CI]]*{{.+}})
//CHECK: call void [[CT1_Ctor:@.+]]([[CI1]]*{{.+}})

//CHECK: declare void [[CI_Ctor]]([[CI]]*{{.+}})

//CHECK: declare void [[CT1_Ctor]]([[CI1]]*{{.+}})

//CHECK: define linkonce_odr void [[CDJ1_Ctor]]([[CDJ]]*{{.+}})
//CHECK: call void [[CI__Ctor:@.+]]([[CI]]*{{.+}})
//CHECK: call void [[CT1_Ctor]]([[CI1]]*{{.+}})

//CHECK: declare void [[CI__Ctor]]([[CI]]*)

//CHECK: define internal void @_GLOBAL__sub_I_cq408231.cpp() #0 section ".text.startup" {
//CHECK: call void [[__cxx_global_var_init]]()
//CHECK: call void [[__cxx_global_var_init_1]]()

