extern "C" {
extern void *malloc (__SIZE_TYPE__ __size) throw () __attribute__ ((__malloc__)) ;

#if INTEL_CUSTOMIZATION
void MyFunc() throw();
#endif // INTEL_CUSTOMIZATION
}

