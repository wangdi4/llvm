#include "mic_tracer.h"

using namespace Intel::OpenCL::MICDevice;

#ifdef ENABLE_MIC_TRACER

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include "cl_sys_info.h"

using namespace Intel::OpenCL::Utils;

void HostTracer::draw_host_to_file()
{
	if (m_cmdTracerDataArr.size() > 0)
	{
		stringstream headerStrStream(stringstream::in | stringstream::out);
		prepare_header_host(headerStrStream);
		draw_to_file((CommandTracer::COMMAND_DATA**)(&m_cmdTracerDataArr[0]), m_cmdTracerDataArr.size(), headerStrStream, "tracer_host.txt");
	}
}

void HostTracer::draw_device_to_file(void* cmdTracers, size_t sizeInBytes, unsigned long long freq)
{
	if (sizeInBytes > 0)
	{
		stringstream headerStrStream(stringstream::in | stringstream::out);
		prepare_header_device(headerStrStream, freq);
		// Convert array of CommandTracer::COMMAND_DATA to array of CommandTracer::COMMAND_DATA*
		CommandTracer::COMMAND_DATA* tCmdTracers = (CommandTracer::COMMAND_DATA*)cmdTracers;
		unsigned int amount = sizeInBytes / sizeof(CommandTracer::COMMAND_DATA);
		vector<CommandTracer::COMMAND_DATA*> cmdTracersPointers;
		for (unsigned int i = 0; i < amount; i++)
		{
			cmdTracersPointers.push_back(&(tCmdTracers[i]));
		}

		draw_to_file((CommandTracer::COMMAND_DATA**)&cmdTracersPointers[0], cmdTracersPointers.size(), headerStrStream, "tracer_device.txt");
	}
}

#define PRINT_START_REC if (false == openPrinted) { fStream << "{" << endl; openPrinted = true; }
#define PRINT_END_REC if (true == openPrinted) { fStream << "}" << endl; }
#define PRINT_LINE_OF_REC(IN) PRINT_START_REC; fStream << "\t" << name << "[" << j - base_idx << "]" << "\t" << cmdTracers[i]->m_valuesArr[j].value.value_##IN << endl

void HostTracer::draw_to_file(CommandTracer::COMMAND_DATA** cmdTracers, size_t size, stringstream& headerStrStream, const char* fileName)
{
	// Get current process name
	void* mainAppImage = dlopen(NULL, RTLD_LAZY);
	void* mainFunc = dlsym(mainAppImage, "main");
	Dl_info info;
	dladdr(mainFunc, &info);
	char* processName = basename((char*)info.dli_fname);
	dlclose(mainAppImage);

	string processNameStr;

	// Find out the output folder (By checking the environment variable "TRACE_OUTPUT_FOLDER"
	char* pOutFolder = NULL;
	pOutFolder = getenv ("TRACE_OUTPUT_FOLDER");
	if (NULL != pOutFolder)
	{
		struct stat st;
		if(stat(pOutFolder ,&st) != 0)
		{
			printf("Error: Trace folder does not exist - %s\n", pOutFolder);
			return;
		}
		processNameStr = pOutFolder;
		processNameStr += "/";
	}

	processNameStr += processName;
	processNameStr += "_";
	// Add pid to output file name
	char b[64];
	snprintf(b, 64, "%d", getpid() );
	processNameStr += b;
	processNameStr += "_";
	processNameStr += fileName;

	fstream fStream(processNameStr.c_str() , fstream::out);

	// Write header
	fStream << headerStrStream.str();

	for (unsigned int i = 0; i < size; i++)
	{
		bool openPrinted = false;
		const char* name = NULL;
		unsigned int base_idx = 0;
		for (unsigned int j = 0; j < CommandTracer::m_last + 1; j++)
		{
			if ( '\0' != cmdTracers[i]->m_valuesArr[j].name[0])
			{
				name = cmdTracers[i]->m_valuesArr[j].name;
				base_idx = j;
			}

			switch (cmdTracers[i]->m_valuesArr[j].type)
			{
			case CommandTracer::TRACE_unneeded:
				break;
			case CommandTracer::TRACE_unsigned_int:
				PRINT_LINE_OF_REC(unsigned_int);
				break;
			case CommandTracer::TRACE_unsigned_long_long:
				PRINT_LINE_OF_REC(unsigned_long_long);
				break;
			case CommandTracer::TRACE_char_pointer:
				PRINT_LINE_OF_REC(char_pointer);
				break;
			default:
				assert(0);
			}
		}
		PRINT_END_REC;
	}

	fStream.close();
}

void HostTracer::prepare_header_host(stringstream& headerStrStream)
{
	if (0 == m_cmdTracerDataArr.size())
	{
		return;
	}
	headerStrStream << "Start header" << endl;
	headerStrStream << "{DEVICE" << endl << "\t" << "HOST" << endl << "DEVICE}" << endl;
	char b[64];
	snprintf(b, 64, "%d", getpid() );
	headerStrStream << "{PID" << endl << "\t" << b << endl << "PID}" << endl;
	headerStrStream << "{FREQUENCY" << endl << "\t" << (MaxClockFrequency() * 1000000) << endl << "FREQUENCY}" << endl;
	headerStrStream << "{COUNTERS" << endl;
	char* lastName = m_cmdTracerDataArr[0]->m_valuesArr[0].name;
	unsigned int counter = 1;
	for (unsigned int i = 1; i < CommandTracer::m_last + 1; i++)
	{
		if ( '\0' == m_cmdTracerDataArr[0]->m_valuesArr[i].name[0])
		{
			counter ++;
			continue;
		}
		headerStrStream << "\t" << lastName << "[" << counter << "];" << endl;
		lastName = m_cmdTracerDataArr[0]->m_valuesArr[i].name;
		counter = 1;
	}
	// Shall print the last counter if not complex counter
	if (counter > 1)
	{
		headerStrStream << "\t" << lastName << "[" << counter << "];" << endl;
	}
	headerStrStream << "COUNTERS}" << endl;
	headerStrStream << "End header" << endl;
}

void HostTracer::prepare_header_device(stringstream& headerStrStream, unsigned long long freq)
{
	headerStrStream << "Start header" << endl;
	headerStrStream << "{DEVICE" << endl << "\t" << "DEVICE" << endl << "DEVICE}" << endl;
	char b[64];
	snprintf(b, 64, "%d", getpid() );
	headerStrStream << "{PID" << endl << "\t" << b << endl << "PID}" << endl;
	headerStrStream << "{FREQUENCY" << endl << "\t" << freq << endl << "FREQUENCY}" << endl;
	headerStrStream << "End header" << endl;
}

#endif
