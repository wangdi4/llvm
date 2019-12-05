/* Wrapper library to keep mutexes out of kernel code */

namespace Intel {
namespace OpenCL {
namespace Utils {

void *_ihc_mutex_create();
int _ihc_mutex_delete(void *handle);
int _ihc_mutex_lock(void *handle);
int _ihc_mutex_unlock(void *handle);

void *_ihc_cond_create();
int _ihc_cond_delete(void *cv);
int _ihc_cond_notify_one(void *cv);
int _ihc_cond_wait(void *m, void *cv);

void *_ihc_pthread_create(void *(*func)(void *), void *arg);
int _ihc_pthread_join(void *handle);
int _ihc_pthread_detach(void *handle);

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
