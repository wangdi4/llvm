#include "../../notification_port.h"

#include <common/COIResult_common.h>
#include <common/COIEvent_common.h>
#include <source/COIEvent_source.h>

#include <source/COIProcess_source.h>
#include <source/COIEngine_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIBuffer_source.h>

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::MICDevice;


class MyObject : NotificationPort::CallBack
{

public:

	MyObject(unsigned int id) :m_id(id)
	{
	}

	void fireCallBack(void* arg)
	{
		volatile bool* flag = (volatile bool*)arg;
		MyObject* self = (MyObject*)this;
		printf("My ID is %d\n", self->getId());
		fflush(stdout);
		*flag = false;
	}

	unsigned int getId()
	{
		return m_id;
	}

private:
	unsigned int m_id;
};


///////////////////////////////   MULTI THREADED TEST   /////////////////////////////////

struct threadData
{
	NotificationPort* nf;
	unsigned int id;
	volatile bool flag;
	bool* isCompleteArr;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	volatile bool condFlag;
};

void* threadFunc(void* threadObj)
{
	threadData* td = (threadData*)threadObj;
	COIEVENT b1;
	COIEventRegisterUserEvent(&b1);

	MyObject m(td->id);
	td->nf->addBarrier(b1, (NotificationPort::CallBack*)(&m), (void*)&(td->flag));
	usleep(rand() % 10000);
	COIEventSignalUserEvent(b1);
	while (td->flag)
	{
		usleep(100);
	}

	td->isCompleteArr[td->id] = true;

	pthread_mutex_lock(&td->mutex);
	td->condFlag = true;
	pthread_cond_signal(&td->cond);
	pthread_mutex_unlock(&td->mutex);
	return NULL;
}

bool multiThreadedTest2(NotificationPort* cp, unsigned int numOfThreads)
{
	bool result = true;

	bool* isCompleteArr = (bool*)malloc(sizeof(bool) * numOfThreads);
	memset(isCompleteArr, 0, sizeof(bool) * numOfThreads);

	threadData* threadsData = (threadData*)malloc(sizeof(threadData) * numOfThreads);
	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		threadsData[i].id = i;
		threadsData[i].nf = cp;
		threadsData[i].flag = true;
		threadsData[i].isCompleteArr = isCompleteArr;
		threadsData[i].condFlag = false;
		pthread_mutex_init(&(threadsData[i].mutex), NULL);
		pthread_cond_init(&(threadsData[i].cond), NULL);
	}

	pthread_t* threadsArr = (pthread_t*)malloc(sizeof(pthread_t) * numOfThreads);

	int err = 0;
	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		err = pthread_create(&(threadsArr[i]), NULL, threadFunc, (void*)(&(threadsData[i])));
		if (err != 0)
		{
			printf("Thread creation failed\n");
		}
	}

	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		pthread_mutex_lock(&threadsData[i].mutex);
		while (threadsData[i].condFlag == false)
			pthread_cond_wait(&threadsData[i].cond, &threadsData[i].mutex);
		pthread_mutex_unlock(&threadsData[i].mutex);
	}

	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		if (isCompleteArr == false)
		{
			printf("Tests failed\n");
			result = false;
		}
	}

	for (unsigned int i = 0; i < numOfThreads; i++)
	{
		pthread_mutex_destroy(&threadsData[i].mutex);
		pthread_cond_destroy(&threadsData[i].cond);
	}
	free(isCompleteArr);
	free(threadsArr);
	free(threadsData);

	return result;
}


///////////////////////////////   MIC & CPU TEST   /////////////////////////////////

#define CHECK_RESULT(_COIFUNC) \
({ \
    COIRESULT result = _COIFUNC; \
    if (result != COI_SUCCESS) \
    { \
        printf("%s returned %s\n", #_COIFUNC, COIResultGetName(result));\
        return false; \
    } \
})

class micNotification
{
public:

	micNotification(NotificationPort* np, COIPROCESS* process, unsigned int numOfBarriers) 
	{
		m_notificationPort = np;
		m_process = process;
		m_numOfBarriers = numOfBarriers;
		m_threadsStartCounter = 0;
		m_threadsEndCounter = 0;
		m_index = 0;
		pthread_mutex_init(&m_mutex, NULL);
		pthread_cond_init(&m_mainCond, NULL);
	}

	virtual ~micNotification() 
	{
		pthread_cond_destroy(&m_mainCond);
		pthread_mutex_destroy(&m_mutex);
	}
	
	bool start()
	{
		int err = 0;
		err = pthread_create(&m_mainThread, NULL, threadEntry, this);
		if (err != 0)
		{
			printf("Thread creation failed\n");
			return false;
		}
		return true;
	}

	void join()
	{
		pthread_join(m_mainThread, NULL);
	}

protected:

	virtual bool performSinkSideFunction(COIPIPELINE* pipeline) = 0;

	static void* threadEntry(void* arg)
	{
		micNotification* self = (micNotification*)arg;

		pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * self->m_numOfBarriers);

		self->m_barriers = (COIEVENT*)malloc(sizeof(COIEVENT) * self->m_numOfBarriers);
		
		assert(self->m_barriers && "malloc failed");

		for (unsigned int i = 0; i < self->m_numOfBarriers; i++)
		{
			CHECK_RESULT(COIEventRegisterUserEvent(&(self->m_barriers[i])));
		}
		
		int err = 0;
		for (unsigned int i = 0; i < self->m_numOfBarriers; i++)
		{
			err = pthread_create(&(threads[i]), NULL, notificationThreadEntry, arg);
			if (err != 0)
			{
				printf("Thread %d creation failed\n", i);
				return false;
			}
		}

		// Wait until all threads start execution
		pthread_mutex_lock(&self->m_mutex);
		while (self->m_threadsStartCounter != 0xffffffff)
		{
			pthread_cond_wait(&self->m_mainCond, &self->m_mutex);
		}
		pthread_mutex_unlock(&self->m_mutex);

		COIPIPELINE             pipeline;
		COIFUNCTION             launch_func;
		
		//Create a pipeline
		CHECK_RESULT(
			COIPipelineCreate(*self->m_process, NULL, NULL, &pipeline));

		self->performSinkSideFunction(&pipeline);

		// Wait until all threads were finished thier work
		pthread_mutex_lock(&self->m_mutex);
		while (self->m_threadsEndCounter != 0xffffffff)
		{
			pthread_cond_wait(&self->m_mainCond, &self->m_mutex);
		}
		pthread_mutex_unlock(&self->m_mutex);

		for (unsigned int i = 0; i < self->m_numOfBarriers; i++)
		{
			CHECK_RESULT(COIEventUnregisterUserEvent(self->m_barriers[i]));
		}

		CHECK_RESULT(
			COIPipelineDestroy(pipeline));

		free(self->m_barriers);
		free(threads);
		return NULL;
	}

	static void* notificationThreadEntry(void* arg)
	{
		volatile bool sleepFlag = true;
		micNotification* self = (micNotification*)arg;
		unsigned int index = __sync_fetch_and_add(&(self->m_index), 1);
		MyObject m(index);
		self->m_notificationPort->addBarrier(self->m_barriers[index], (NotificationPort::CallBack*)(&m), (void*)&sleepFlag);
		unsigned int currentEntered = __sync_fetch_and_add(&(self->m_threadsStartCounter), 1);
		if (currentEntered == self->m_numOfBarriers - 1)
		{
			pthread_mutex_lock(&(self->m_mutex));
			self->m_threadsStartCounter = 0xffffffff;
			pthread_cond_signal(&(self->m_mainCond));
			pthread_mutex_unlock(&(self->m_mutex));
		}

		while (sleepFlag)
		{
			usleep(100);
		}

		unsigned int currentEnd = __sync_fetch_and_add(&(self->m_threadsEndCounter), 1);
		if (currentEnd == self->m_numOfBarriers - 1)
		{
			pthread_mutex_lock(&(self->m_mutex));
			self->m_threadsEndCounter = 0xffffffff;
			pthread_cond_signal(&(self->m_mainCond));
			pthread_mutex_unlock(&(self->m_mutex));
		}
		return NULL;
	}

	NotificationPort* m_notificationPort;

	COIPROCESS* m_process;
	COIEVENT* m_barriers;

	unsigned int m_numOfBarriers;

	unsigned int m_threadsStartCounter;
	unsigned int m_threadsEndCounter;
	unsigned int m_index;

	pthread_t m_mainThread;

	pthread_mutex_t m_mutex;

	pthread_cond_t m_mainCond;


};

class micSyncNotification : public micNotification
{
public:

	micSyncNotification(NotificationPort* np, COIPROCESS* process, unsigned int numOfBarriers) : micNotification(np, process, numOfBarriers) {}

	bool performSinkSideFunction(COIPIPELINE* pipeline)
	{
		COIFUNCTION             launch_func;
		// Retrieve handles to functions belonging to sink side process
		CHECK_RESULT(
			COIProcessGetFunctionHandles(*m_process, 1, (const char*[]){"DelayFunction"}, 
									 &launch_func));

		for (unsigned int i = 0; i < m_numOfBarriers; i++)
		{
		
			CHECK_RESULT(
				COIPipelineRunFunction(
					*pipeline, launch_func,               // Pipeline handle and function 
														 // handle.
					0, NULL, NULL,                  // Buffer
					0, NULL,                             // Dependencies
					&(m_barriers[i]), sizeof(COIEVENT),                             // Misc Data
					NULL, 0,                             // Return Values
					&(m_barriers[i])));
		}
		return true;
	}

};


class micMultiThreadedNotification : public micNotification
{
public:

	micMultiThreadedNotification(NotificationPort* np, COIPROCESS* process, unsigned int numOfBarriers) : micNotification(np, process, numOfBarriers) {}

	bool performSinkSideFunction(COIPIPELINE* pipeline)
	{
		COIFUNCTION             launch_func;

		// Retrieve handles to functions belonging to sink side process
		CHECK_RESULT(
			COIProcessGetFunctionHandles(*m_process, 1, (const char*[]){"DelayFunctionMultiThreaded"}, 
									 &launch_func));

		CHECK_RESULT(
			COIPipelineRunFunction(
				*pipeline, launch_func,               // Pipeline handle and function 
													 // handle.
				0, NULL, NULL,                  // Buffer
				0, NULL,                             // Dependencies
				m_barriers, sizeof(COIEVENT) * m_numOfBarriers,                             // Misc Data
				NULL, 0,                             // Return Values
				NULL));
				
		return true;
	}

};



bool mic_cpu_test()
{
    COIPROCESS              proc;
    COIENGINE               engine;
    uint32_t                num_engines = 0;
    const char*             SINK_NAME = "notification_port_test_sink_mic";

    // Make sure there is a KNF device available
    CHECK_RESULT(
    COIEngineGetCount(COI_ISA_KNF, &num_engines));

    printf("%u engines available\n", num_engines);

    // If there isn't at least one engine, there is something wrong
    if (num_engines < 1)
    {
        printf("ERROR: Need at least 1 engine\n");
        return false;
    }

    // Get a handle to the "first" KNF engine
    CHECK_RESULT(
    COIEngineGetHandle(COI_ISA_KNF, 0, &engine));

    // Process: Represents process created on the device enumerated(engine).
    //          Processes created on sink side are referenced by COIPROCESS
    //          instance
    // The following call creates a process on the sink. COI will 
    // automatically load any dependent libraries and run the "main" function 
    // in the binary.
    //
    CHECK_RESULT( COIProcessCreateFromFile(
        engine,             // The engine to create the process on.
        SINK_NAME,          // The local path to the sink side binary to launch.
        0, NULL,            // argc and argv for the sink process.
        false, NULL,        // Environment variables to set for the sink 
                            // process.
        true, NULL,         // Enable the proxy but don't specify a proxy root 
                            // path.
        1024*1024,          // The amount of memory to reserve for COIBuffers.
                            // If this is zero buffers cannot be created
		NULL, 
        &proc               // The resulting process handle.
    ));

	NotificationPort* notificationPort = NotificationPort::notificationPortFactory(10);
	assert(NULL != notificationPort && "initialize failed");
	micSyncNotification* synchNotifiaction = new micSyncNotification(notificationPort, &proc, 100);

	micMultiThreadedNotification* multiThreadedNotification = new micMultiThreadedNotification(notificationPort, &proc, 100);

	synchNotifiaction->start();

	multiThreadedNotification->start();

	sleep(1);

	multiThreadedTest2(notificationPort, 200);

	printf("CPU done\n");
	fflush(0);

	synchNotifiaction->join();
	printf("MIC1 done\n");
	fflush(0);
	multiThreadedNotification->join();
	printf("MIC2 done\n");
	fflush(0);

	notificationPort->release();
	NotificationPort::waitForAllNotificationPortThreads();

    // Destroy the process
    //
    CHECK_RESULT(
    COIProcessDestroy(
        proc,           // Process handle to be destroyed
        -1,             // Wait indefinitely till main() (on sink side) returns
        false,          // Don't force to exit. Let it finish executing
                        // functions enqueued and exit gracefully
        NULL,
        NULL ));
    
	delete(multiThreadedNotification);
	delete(synchNotifiaction);

	printf("Exiting\n");

    return true;

}



int main(int argc, char* argv[])
{
    unsigned int rounds = 50;
	/* initialize random seed: */
	srand ( (unsigned int)time(NULL) );
	for (unsigned int i = 0; i < rounds; i++)
	{
	    printf("Starting round %d of %d\n", i, rounds);
		fflush(0);
		mic_cpu_test();
	}
	printf("Test done\n");
	fflush(0);
	return 0;
}