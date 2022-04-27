/* Wrapper library to keep mutexes out of kernel code */

extern "C" LLVM_BACKEND_API void *_ihc_mutex_create();
extern "C" LLVM_BACKEND_API int _ihc_mutex_delete(void *);
extern "C" LLVM_BACKEND_API int _ihc_mutex_lock(void *);
extern "C" LLVM_BACKEND_API int _ihc_mutex_unlock(void *);

extern "C" LLVM_BACKEND_API void *_ihc_cond_create();
extern "C" LLVM_BACKEND_API int _ihc_cond_delete(void *cv);
extern "C" LLVM_BACKEND_API int _ihc_cond_notify_one(void *);
extern "C" LLVM_BACKEND_API int _ihc_cond_wait(void *, void *);

extern "C" LLVM_BACKEND_API void *_ihc_pthread_create(void *(*func)(void *), void *arg);
extern "C" LLVM_BACKEND_API int _ihc_pthread_join(void *handle);
extern "C" LLVM_BACKEND_API int _ihc_pthread_detach(void *handle);

#ifdef _WIN32
extern "C" LLVM_BACKEND_API void *_Znwy(unsigned long long);
extern "C" LLVM_BACKEND_API void _ZdlPvy(void*, unsigned long long);
extern "C" LLVM_BACKEND_API void _ZSt14_Xlength_errorPKc(char const*);
extern "C" LLVM_BACKEND_API void _ZdlPv(void*);
#endif
