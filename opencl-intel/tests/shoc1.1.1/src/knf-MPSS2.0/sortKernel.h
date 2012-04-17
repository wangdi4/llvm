#ifdef  __MIC__ ||  __MIC2__
#pragma offload_attribute(push,target(mic))


#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <memory.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <pmmintrin.h>
#include <zmmintrin.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <omp.h>

#define MAX_THREAD  248
#define MAX_WORKER  248
unsigned int Thread; // number of threads
int coreFreq;

#include <pthread.h>
#ifdef AFFINITIZE
#include <pthread_affinity_np.h>
#endif

#include <fcntl.h>

#include <sys/sysctl.h>
pthread_t th[128];

typedef struct threadData
{
  long tid;
  long numElements;

}THREAD_DATA_T;

THREAD_DATA_T tdata;

#define BARRIER(X, Y, T) pthread_barrier_wait(&X);
volatile static int _barrier_turn_ = 0;
volatile static int _barrier_go_1;
volatile static int _barrier_1[256];
volatile static int _barrier_go_2;
volatile static int _barrier_2[256];
#define _BARRIER_ \
	if (_barrier_turn_ == 0) \
	{ \
	if (id == 0) \
	{ \
		for (int i=1; i<Thread; i++) \
		{ \
			while(_barrier_1[i] == 0); \
			_barrier_2[i] = 0; \
		} \
		_barrier_turn_ = 1; \
		_barrier_go_2 = 0; \
		_barrier_go_1 = 1; \
	} \
	else \
	{ \
		_barrier_1[id] = 1; \
		while(_barrier_go_1 == 0); \
	} \
	} \
	else \
	{ \
	if (id == 0) \
	{ \
		for (int i=1; i<Thread; i++) \
		{ \
			while(_barrier_2[i] == 0); \
			_barrier_1[i] = 0; \
		} \
		_barrier_turn_ = 0; \
		_barrier_go_1 = 0; \
		_barrier_go_2 = 1; \
	} \
	else \
	{ \
		_barrier_2[id] = 1; \
		while(_barrier_go_2 == 0); \
	} \
	}


#define my_malloc(X) _mm_malloc(X, 4096)

extern "C" void __intel_new_proc_init_R();
void __intel_new_proc_init_R(){}


// Number of radix bits and histogram bins per pass of radix sort
//#define LOG_HIST_BINS 4
//#define HIST_BINS 16
//#define HIST_BINS_1 15
#define LOG_HIST_BINS 8
#define HIST_BINS 256
#define HIST_BINS_1 255

// X = input key, Y = output key
// Z = input value V = output Value
// Hist = histogram
// Buf = local buffer for efficiency
unsigned int *X;
unsigned int *Y;
unsigned int *Z;
unsigned int *V;

unsigned int *pingpong;
unsigned int *Hist;
unsigned int *Buf;
unsigned int *vBuf;
unsigned int *tmp;
unsigned int *masks;

int global_elements_per_task;
#define LOOKUP1(a) ((a>>0)&HIST_BINS_1)
//#define LOOKUP1(a) ((a>>LOG_HIST_BINS)&HIST_BINS_1)
//#define LOOKUP1(a) ((a>>(2*LOG_HIST_BINS))&HIST_BINS_1)
//#define LOOKUP1(a) ((a>>(3*LOG_HIST_BINS))&HIST_BINS_1)


#define BUFFER_SIZE 16
#define BUFFER_SIZE_1 15

#ifndef LARRY
__declspec(target(mic))
static void
get_core_freq (int *pFreq)
{
   *pFreq = 1200;
}
#endif

// The code below only implements one phase of radix sort dealing with 8 bits
// Radix sort has three steps:

// Step1 : histogram computation for each of the HIST_BINS radices
// Step2 : prefix sum of the radices
// Step3 : scatter to proper locations

#define L2DIST 64
__declspec(target(mic))
void Step1_Phase1(long id)
{
    int index, index2;
    long N = tdata.numElements;

    int starting_element_id = id*global_elements_per_task;
    int ending_element_id = starting_element_id + global_elements_per_task;
    if (ending_element_id > N) ending_element_id = N;
    int i;
    //printf("id: %d starting_element_id: %d ending_element_id: %d\n", id, starting_element_id, ending_element_id);                fflush(stdout);       
    unsigned int *Local_Hist = Hist + 2*(HIST_BINS)*id;
    unsigned int *Local_Hist2 = Hist + 2*(HIST_BINS)*id+HIST_BINS;
    
    unsigned int *X_start = X + starting_element_id;
    unsigned int *X_start_2 = X + starting_element_id + ((ending_element_id - starting_element_id)>>1);
    //    for(i=starting_element_id; i<ending_element_id; i+=16)
    //BARRIER(Barrier1, 1, id);
    _BARRIER_;
   // Time_Start[id] = read_tsc();
    for(i=0; i< (((ending_element_id-starting_element_id)>>1)); i+=32)
    {
#pragma unroll (16)
      for(unsigned j = 0; j < 16; j++){
	unsigned int in0 = X_start[i+j];
	unsigned int in1 = X_start_2[i+j];
	unsigned int index0 = LOOKUP1(in0);
	unsigned int index1 = LOOKUP1(in1);
	unsigned int tmp0 = Local_Hist[index0];
	unsigned int tmp1 = Local_Hist2[index1];
	tmp0++;
	tmp1++;
	Local_Hist[index0] = tmp0;
	Local_Hist2[index1] = tmp1;
      }

#pragma unroll (16)
      for(unsigned j = 0; j < 16; j++){
	unsigned int in0 = X_start[i+j+16];
	unsigned int in1 = X_start_2[i+j+16];
	unsigned int index0 = LOOKUP1(in0);
	unsigned int index1 = LOOKUP1(in1);
	unsigned int tmp0 = Local_Hist[index0];
	unsigned int tmp1 = Local_Hist2[index1];
	tmp0++;
	tmp1++;
	Local_Hist[index0] = tmp0;
	Local_Hist2[index1] = tmp1;
      }
    }
    _BARRIER_;
    //Time_Finish[id] = read_tsc();
   // Time_Stage1[id] += (Time_Finish[id] - Time_Start[id]);
//    printf("BB: tid - %d Time Step1 - %lld (start: %lld, end: %lld)\n",id, Time_Stage1[id], Time_Start[id], Time_Finish[id]);
    //BARRIER(Barrier2, 2, id);
    //memcpy(endCounters, myCounters, sizeof(struct cycle_times));
//    printf("AB: tid - %d Time Step1 - %lld\n",id, Time_Stage1[id]);
//    BARRIER(Barrier1, 1, id);

      //printf("Done step 1\n");
fflush(stdout);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__declspec(target(mic))
void Step2_Phase1(long id)
{
    int index, index2;

    long N = tdata.numElements;

    int total_hists = HIST_BINS*Thread*2;
    int num_columns = HIST_BINS/Thread;
    if((HIST_BINS%Thread) != 0) num_columns++;
    //num_columns+= (16 - (num_columns%16));
    int start_col = id*num_columns;
    int end_col = start_col + num_columns;
    if (end_col > HIST_BINS) {end_col = HIST_BINS;}
#if 0 
    if(id == 0){
    printf("before:\n");
    for(int i=0; i<256; i++){
      for(int j=0; j<Thread*2; j++){
        printf("%d ", Hist[j*HIST_BINS + i]);
      }
      printf("\n");
    } 
    }
#endif

    _BARRIER_;
  //  Time_Start[id] = read_tsc();
    int prev_counter = 0;
    int curr_counter;
      
    for(int i=start_col; i<end_col; i++){
      for(int j=0; j<Thread*2; j++){
        curr_counter = Hist[j*(HIST_BINS)+i];
        Hist[j*(HIST_BINS)+i] = prev_counter;
        prev_counter += curr_counter;
      }
    }
    if(start_col < HIST_BINS) Hist[start_col] = prev_counter;
    _BARRIER_;
    if(id == 0){
      int prev_counter = 0, curr_counter;
      for(int i=0; i < Thread; i++){
        if(i*num_columns < HIST_BINS){
          curr_counter = Hist[i*num_columns];
          Hist[i*num_columns] = prev_counter;
          prev_counter += curr_counter;
        }
      }
#if 0 
    if(id == 0){
    printf("prev_counter: %d\n", prev_counter);
    printf("middle:\n");
    for(int i=0; i<256; i++){
      for(int j=0; j<Thread*2; j++){
        printf("%d ", Hist[j*HIST_BINS + i]);
      }
      printf("\n");
    }
    }
#endif

    }
    // _BARRIER_;
    for(int j=1; j<Thread*2; j++)
      Hist[j*HIST_BINS+start_col] += Hist[start_col];
    for(int i=(start_col+1); i < end_col; i++){
      for(int j=0; j<Thread*2; j++){
        Hist[j*HIST_BINS + i] += Hist[start_col];
      }
    } 
  //  _BARRIER_;
   // Time_Finish[id] = read_tsc();
   // Time_Stage2[id] = Time_Finish[id] - Time_Start[id];

#if 0 
    if(id == 0){
    printf("after:\n");
    for(int i=0; i<256; i++){
      for(int j=0; j<Thread*2; j++){
        printf("%d ", Hist[j*HIST_BINS + i]);
      }
      printf("\n");
    } 
    }
#endif
//printf("done with step2\n"); fflush(stdout);
}



// Step3 is scatter step
// uses buffer to speed up scatter


__declspec(target(mic))
void Step3_Buffer_Phase1(long id)
{
  // The logic is the following:
  // Read elements 16 at a time, put them into appropriate positions in the buffer
  // The buffer is an array of HIST_BINS * BUFFER_SIZE, and buffers BUFFER_SIZE entries per radix
  // As soon as a buffer line corresponding to a radix gets full, it is written out to memory.

    const int L2_DIST = 10;
    int index, index2;
    long N = tdata.numElements;


    //BARRIER(Barrier3, 3, id);
//    _BARRIER_;
    int starting_element_id = id*global_elements_per_task;
    int ending_element_id = starting_element_id + global_elements_per_task;
    if (ending_element_id > N) ending_element_id = N;
    int i;

    unsigned int *Local_Hist = Hist + (HIST_BINS)*id*2;
    unsigned int *Local_Hist2 = Hist + (HIST_BINS)*id*2 + HIST_BINS;
    unsigned int *tmpLocal = tmp + id*32; 

    unsigned int *Dest = Buf + BUFFER_SIZE*HIST_BINS*id;
    unsigned int *vDest = vBuf + BUFFER_SIZE*HIST_BINS*id;

    unsigned char steady_state[HIST_BINS];
    unsigned int start_cnt[HIST_BINS];

//printf("In step3 before prefetch hist\n"); fflush(stdout);
    for(unsigned i = 0; i < HIST_BINS; i++){ steady_state[i] = 0; start_cnt[i] = Local_Hist[i]; /* printf("start_cnt[%d]: %d\n", i, start_cnt[i]); */ }
//printf("In step3 before mask\n"); fflush(stdout);

    // set up masks for buffer management
    unsigned int m0 = 0xFFFF;
    unsigned int mask_constants[16] = {
      0xFFFF, 0xFFFE, 0xFFFC, 0xFFF8,
      0xFFF0, 0xFFE0, 0xFFC0, 0xFF80,
      0xFF00, 0xFE00, 0xFC00, 0xF800,
      0xF000, 0xE000, 0xC000, 0x8000
    };
    
    unsigned int *X_start = X + starting_element_id;
    unsigned int *X_start_2 = X + starting_element_id + ((ending_element_id - starting_element_id)>>1);


    unsigned int *Z_start = Z + starting_element_id;
    unsigned int *Z_start_2 = Z + starting_element_id + ((ending_element_id - starting_element_id)>>1);


    unsigned int log_hist_bins = LOG_HIST_BINS;
    unsigned int and_mask = HIST_BINS_1;
    unsigned int buf_size = BUFFER_SIZE;

    //BARRIER(Barrier0, 0, id);
    _BARRIER_;
   // Time_Start[id] = read_tsc();

//printf("In step3 before compute start value\n"); fflush(stdout);
#define STEP_SIZE 32 

    for(i=0; i< ((ending_element_id - starting_element_id)>>1); i+=STEP_SIZE)
    {
        tmpLocal[0:16] = X_start[i:16]&and_mask;
        tmpLocal[16:16] = X_start[i+16:16]&and_mask;

      // for each element, do scalar computation to find the buffer position to write to.
      // this is done with a histogram update followed by a write to local buffer

      for(unsigned j = 0; j < STEP_SIZE; j++){
        unsigned int in = X_start[i+j];
        unsigned int vin = Z_start[i+j];

        unsigned int index = tmpLocal[j]; // computed radix
	unsigned int cnt = Local_Hist[index]++;
	
	// write to buffer
	Dest[index*BUFFER_SIZE + cnt%BUFFER_SIZE] = in;
        vDest[index*BUFFER_SIZE + cnt%BUFFER_SIZE] = vin;
	
	// detect if buffer line is full
	if(cnt%BUFFER_SIZE == BUFFER_SIZE_1) 
	{
	  // if so, write out the line
	  //   this has two special cases: the first time we write a line correpsonding to a radix, and the last time we flush buffer values in a  line
	  //   For the first time, we may start with an unaligned address: then we compute the start of the cache line corresponding to the write value,
	  //       and write only the correct subset of the elements using a masked store.
	  //   The last flush is handled separately.
	  unsigned int * Y_start = Y + cnt - BUFFER_SIZE_1;
          unsigned int * V_start = V + cnt - BUFFER_SIZE_1;
	  
          if(steady_state[index]) {
	    // this is the normal case, when we write the entire buffer line 
              // int offset = index*BUFFER_SIZE;
              // there is a loss in perf here in moving from intirin->C 
               #pragma vector aligned
               Y_start[0:16] = Dest[index*BUFFER_SIZE:16];
               V_start[0:16] = vDest[index*BUFFER_SIZE:16];
	  }
	  else{
	    // special case for first write
	    steady_state[index] = 1;
            unsigned int m1 = mask_constants[start_cnt[index]%BUFFER_SIZE];
            #pragma unroll(16)
            for(int k=0;k<16;k++){
                  if((m1&(1<<k))){
                      Y_start[k] = Dest[k+index*BUFFER_SIZE];
                      V_start[k] = vDest[k+index*BUFFER_SIZE];
                  }
            }
	  }
	}
      }
    }


    // this handles the last flush of remaining buffer elements to memory: this is scalar for now.
    // can also be LRBized - but is not a bottleneck for large input sets.
    for(unsigned i = 0; i < HIST_BINS; i++){
	int cnt = Local_Hist[i];
	int cnt2 = cnt - cnt%BUFFER_SIZE;
	for(unsigned j = 0; j < BUFFER_SIZE; j++){
	  if(cnt > (cnt2+j) && ((cnt2 + j) >= start_cnt[i])){
		 Y[cnt2+j] = Dest[i*BUFFER_SIZE + j];
                 V[cnt2+j] = vDest[i*BUFFER_SIZE + j];
	  }
	}
    }


    // All code below is a repetition of the code above to handle a different element in a thread: 
    //   as in Step1, we use 1 thread to work on two different blocks of inputs to improve Step1 
    
    for(unsigned i = 0; i < HIST_BINS; i++){ steady_state[i] = 0; start_cnt[i] = Local_Hist2[i]; /* printf("start_cnt[%d]: %d\n", i, start_cnt[i]); */ }

    for(i=0; i< ((ending_element_id - starting_element_id)>>1); i+=STEP_SIZE)
    {
        tmpLocal[0:16] = X_start_2[i:16]&and_mask;
        tmpLocal[16:16] = X_start_2[i+16:16]&and_mask;

      
      for(unsigned j = 0; j < STEP_SIZE; j++){
        unsigned int in = X_start_2[i+j];
        unsigned int vin = Z_start_2[i+j];
        unsigned int index = tmpLocal[j];//LOOKUP(in);
	unsigned int cnt = Local_Hist2[index]++;
	Dest[index*BUFFER_SIZE + cnt%BUFFER_SIZE] = in;
        vDest[index*BUFFER_SIZE + cnt%BUFFER_SIZE] = vin;	

	if(cnt%BUFFER_SIZE == BUFFER_SIZE_1)
	{
	  unsigned int * Y_start = Y + cnt - BUFFER_SIZE_1;
          unsigned int * V_start = V + cnt - BUFFER_SIZE_1;

	  if(steady_state[index]) {
             Y_start[0:16]= Dest[index*BUFFER_SIZE:16];
             V_start[0:16]= vDest[index*BUFFER_SIZE:16];
	  }
	  else{
	    steady_state[index] = 1;

            unsigned int m1 = mask_constants[start_cnt[index]%BUFFER_SIZE];

            #pragma unroll(16)
            for(int k=0;k<16;k++){
                  if((m1&(1<<k))){
                      Y_start[k] = Dest[k+index*BUFFER_SIZE];
                      V_start[k] = vDest[k+index*BUFFER_SIZE];
                  }
            }
	  }
	}
      }
    }



    for(unsigned i = 0; i < HIST_BINS; i++){
	int cnt = Local_Hist2[i];
	int cnt2 = cnt - cnt%BUFFER_SIZE;
	for(unsigned j = 0; j < BUFFER_SIZE; j++){
	  if(cnt > (cnt2+j) && ((cnt2 + j) >= start_cnt[i])){
              Y[cnt2+j] = Dest[i*BUFFER_SIZE + j];
              V[cnt2+j] = vDest[i*BUFFER_SIZE + j];
          }
	}
    }
    //BARRIER(Barrier4, 4, id);
    _BARRIER_;
   // Time_Finish[id] = read_tsc();
  //  Time_Stage3[id] += (Time_Finish[id] - Time_Start[id]);

//printf("Done with step 3\n"); fflush(stdout);
}

int ntasks;
int ntasks_per_thread;


__declspec(target(mic))
void *sort(void *id)
{


	for(int i = 0; i < (HIST_BINS)*ntasks*2; i++) Hist[i] = 0;
  	Step1_Phase1((long)id);
  	Step2_Phase1((long)id);
 	Step3_Buffer_Phase1((long)id);
        pthread_exit(NULL);

} 



template <class T>
__declspec(target(mic))
extern void sortKernelMIC(T* hkey, T* hvalue, T* outkey, T* outvalue, const size_t N, int numThreads)
{
   tdata.numElements = N;

   ntasks = Thread = numThreads;
    if (Thread > MAX_WORKER) {
        printf("Cannot create more than %d threads\n", MAX_WORKER);
        exit(1);
    }
    // read in tasks_per_thread for taskQ
    // The +64 is for alignment reasons to ensure that L1 bank conflicts dont occur
    if( (N/ntasks)%64 != 0)
      global_elements_per_task = ((N/ntasks)/64)*64 + 64 + 64;
    else
      global_elements_per_task = (N/ntasks) + 64;
    // For affinity
#ifndef SCALING
//    printf("Thread = %d\n", Thread);
//    printf("ntasks = %d\n", ntasks);
#endif


    // Allocate data
    X =  hkey;
    Y =  outkey; 
    Z =  hvalue;
    V = outvalue; //(unsigned int *)my_malloc(N*sizeof(unsigned int));
    Hist = (unsigned int *)my_malloc(ntasks*2*(HIST_BINS)*sizeof(unsigned int));
    Buf = (unsigned int *)my_malloc(Thread*HIST_BINS*BUFFER_SIZE*sizeof(unsigned int));
    vBuf = (unsigned int *)my_malloc(Thread*HIST_BINS*BUFFER_SIZE*sizeof(unsigned int));
    tmp = (unsigned int *)my_malloc(Thread*32*sizeof(unsigned int));
    masks = (unsigned int *)my_malloc(16*sizeof(unsigned int));
   // printf("Finished allocating\n");

    // Initialize data
    for(unsigned i = 0; i < 16; i++){
      masks[i] = (1<<i);
    }

    //printf("Finished initializing\n");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
#ifdef AFFINITIZE
    pthread_aff_mask_t aff_mask;
    pthread_aff_mask_initialize_np(&aff_mask, 0);
#endif


    { 
      for(int i = 0; i < (HIST_BINS)*ntasks*2; i++) Hist[i] = 0;
      for(long i=1; i<Thread; i++) {

#ifdef AFFINITIZE
          pthread_aff_mask_set_np(&aff_mask, 0, i+(MAX_THREAD-MAX_WORKER));
          pthread_attr_aff_set_np(&attr, aff_mask);
#endif
          tdata.tid = i;
          pthread_create(&th[i], &attr, (void *(*)(void *))sort, (void *) i);
#ifdef AFFINITIZE
          pthread_aff_mask_clear_np(&aff_mask, 0, i+(MAX_THREAD-MAX_WORKER));
#endif
      }
#ifdef AFFINITIZE
      pthread_aff_mask_set_np(&aff_mask, 0, (MAX_THREAD-MAX_WORKER));
      pthread_attr_aff_set_np(&attr, aff_mask);
#endif
    tdata.tid = 0;
    tdata.numElements=N;
    pthread_create(&th[0], &attr, (void *(*)(void *))sort, (void *) 0);
#ifdef AFFINITIZE
      pthread_aff_mask_clear_np(&aff_mask, 0, (MAX_THREAD-MAX_WORKER));
#endif
    }
    for(int i=1; i<Thread; i++) {
        int ret;
        pthread_join(th[i], (void **)&ret);
    }
    int ret;
    pthread_join(th[0], (void**)&ret);

    memcpy(hkey,Y,N*sizeof(T));
    
    _mm_free(Hist);
    _mm_free(Buf);
    _mm_free(tmp);
    _mm_free(masks);


    return;
}
#pragma offload_attribute(pop)

#endif


#include "OptionParser.h"
#include "ResultDatabase.h"
#include "Sort.h"


template <class T>
__declspec(target(mic))
extern void sortKernel(T* hkey, T* hvalue, T* outkey, T* outvalue,const size_t n, int numThreads)
{
 #ifdef __MIC__

   sortKernelMIC(hkey,  hvalue, outkey, outvalue, n, numThreads);
   // sorted output saced in hvalue

 #endif 

 return;
}

