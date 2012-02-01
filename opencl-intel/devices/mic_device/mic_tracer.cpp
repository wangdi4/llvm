#include "mic_tracer.h"

using namespace Intel::OpenCL::MICDevice;

#ifdef ENABLE_MIC_TRACER

CommandTracer::CommandTracer() 
{
	m_data = new COMMAND_DATA;
	initializeAll();
}

CommandTracer::~CommandTracer()
{
	// Add this command trace to tracer pool
	Tracer::getTracerInstace()->insert(*this);
}

Tracer* Tracer::m_active = NULL;

Tracer::Tracer()
{
	pthread_mutex_init(&m_mutex, NULL);
	m_active = this;
}

Tracer::~Tracer()
{
	for (unsigned int i = 0; i < m_cmdTracerDataArr.size(); i++)
	{
		delete m_cmdTracerDataArr[i];
	}
	pthread_mutex_destroy(&m_mutex);
}

void Tracer::insert(CommandTracer& cmdTracer)
{
	pthread_mutex_lock(&m_mutex);
	m_cmdTracerDataArr.push_back(cmdTracer.m_data);
	pthread_mutex_unlock(&m_mutex);
}

#endif
