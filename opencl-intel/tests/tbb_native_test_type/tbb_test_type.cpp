#include <uneven/blocked_range.h>
#include <uneven/parallel_for.h>
#include <opencl_partitioner.h>

#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/task_arena.h>


struct parallel_functor
{
	parallel_functor(tbb::atomic<long>& counter) : my_counter(counter) {}

	void operator()(const tbb::uneven::blocked_range<long>& r) const
	{
		my_counter--;
		while(my_counter > 0)
		{
#ifdef WIN32
			_mm_pause();
#else
			 usleep(0);
#endif
		}
	}

	tbb::atomic<long>& my_counter;
};

struct arena_functor
{
	arena_functor(parallel_functor& parallel, long n) : my_parallel(parallel), my_n(n) {}
	void operator()()
	{
		tbb::opencl_partitioner part;

		tbb::uneven::parallel_for(tbb::uneven::blocked_range<long>(0, my_n, 1), my_parallel, part);
	}

	parallel_functor& my_parallel;
	long my_n;
};

int main(int argc, char* argv[])
{
	int nThreads = tbb::task_scheduler_init::default_num_threads();;
	tbb::atomic<long> counter;
	counter = nThreads;

	tbb::task_scheduler_init tbb_init(nThreads);
	tbb::task_arena arena(nThreads, 1);


	parallel_functor Fparallel(counter);
	arena_functor    Farena(Fparallel, nThreads);
	
	arena.execute(Farena);

	return 0;
}
