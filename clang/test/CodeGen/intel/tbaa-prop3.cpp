// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu  -std=c++11  -fintel-compatibility -O3 %s -disable-llvm-optzns -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu  -std=c++11  -fintel-compatibility -O3 %s -disable-llvm-optzns -emit-llvm -o - -mllvm -opaque-pointers | FileCheck %s
//
// CQ415654
// Check that we generate fakeload intrinsic for the return pointers when
// the struct type is literal struct type.
//
struct pair;
template < class> struct cons;

struct drop_front 
{
    template < class Tuple > struct apply
    {
        typedef Tuple type;
        static type & call ();
    };
};

template < class HT > 
HT&
get (cons < HT>)
{
    typedef typename drop_front ::apply < cons < HT > >impl;
    return impl::call().head;
}
template < class HT > struct cons
{
    HT head;
};

typedef cons<pair (*)()> index_entry;

void register_dynamic_id_aux ()
{
    get (index_entry{});
}
// CHECK:  call {{.*}}  @llvm.intel.fakeload.
