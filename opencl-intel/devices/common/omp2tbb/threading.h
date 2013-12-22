#define TBB_PREVIEW_LOCAL_OBSERVER 1
#define TBB_PREVIEW_TASK_ARENA 1

#include <tbb/task_arena.h>
#include <tbb/task_scheduler_observer.h>

namespace omp2tbb {
	// Defines class that count the number of active threads in the active arrena
	class arena_worker_counter : public tbb::task_scheduler_observer {

		unsigned int active_workers;

	public:
		arena_worker_counter() : tbb::task_scheduler_observer(true), active_workers(0) {}
		unsigned int get_active_worker_count() const { return active_workers;}

		// overidden functions
		virtual void on_scheduler_entry(bool isWorker) { ++active_workers; }
	};

	// Explicit arena for fork execution
	// Is used when num_threads() is specified and __kmp_push_num_threads() is called
	// This class is to fill missing function from TBB task_arena
	// TBB is to return my_max_concurency as default method
	class active_arena : public tbb::task_arena {
		unsigned int max_concurrency;

	public:
		active_arena(unsigned int threads) : tbb::task_arena(threads,1), max_concurrency(threads) {}

		unsigned int get_concurency() const { return max_concurrency;}
	};
}