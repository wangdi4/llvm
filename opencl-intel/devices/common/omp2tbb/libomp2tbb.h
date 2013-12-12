#include <vector>
#include <tbb\critical_section.h>
#include <tbb\atomic.h>

#ifdef _WIN32
#define LIBOMP2TBB_API	__declspec(dllexport)
#define THREAD_LOCAL	__declspec(thread)
#else
#define THREAD_LOCAL	__thread
#define LIBOMP2TBB_API
// put pthread definition hhere
#endif

struct ident_t;
struct kmp_critical_name;
typedef int kmp_int32;
typedef long long kmp_int64;
typedef void kmpc_micro(kmp_int32* global_tid, kmp_int32* bound_tid,...);

#ifndef TRUE
#define TRUE 1
#endif

namespace omp2tbb {

enum sched_type {
    kmp_sch_static_chunked            = 33,
    kmp_sch_static                    = 34,   /**< static unspecialized */
    kmp_sch_dynamic_chunked           = 35,
    kmp_sch_guided_chunked            = 36,   /**< guided unspecialized */
};

class global_thread_table;

extern global_thread_table* g_thread_info;

extern unsigned int get_current_team_concurency();

// Global information which is required to handle for each OMP master thread
class global_thread_table {
public:
	struct thread_info_entry;

	global_thread_table() {}
	~global_thread_table() {}

	kmp_int32 allocate_master_thread();
	
	kmp_int32 get_master_count() {
		return (kmp_int32)masters.size();
	}
	
	thread_info_entry* get_master_entry(kmp_int32 gtid) {
		return &masters[gtid];
	}

	struct thread_info_entry {
		thread_info_entry();
		thread_info_entry(const thread_info_entry& o);
		~thread_info_entry();

		// requried by "omp critical" handling
		tbb::critical_section*	my_critical;
		// required by "omp single" handling
		tbb::atomic<ident_t*>	my_last_single_location;
		tbb::atomic<long>		my_single_count;
		// required by "omp barrier" or implicit barrier handling
		long					my_concurency;
		volatile long			my_barrier_epoch;
		// Need to change to more efficient implementation
		tbb::atomic<long>		my_barrier_count;
	};

protected:
	tbb::critical_section			masters_guard;
	std::vector<thread_info_entry>	masters;
};
}
