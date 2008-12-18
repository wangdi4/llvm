///////////////////////////////////////////////////////////
//  enqueue_commands.cpp
//  Implementation of the Class ReadBufferCommand
//  Created on:      16-Dec-2008 10:11:31 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "enqueue_commands.h"


ReadBufferCommand::ReadBufferCommand(){

}



ReadBufferCommand::~ReadBufferCommand(){

}





void ReadBufferCommand::Init(MemoryObject buffer, size_t offset, size_t cb, void){

}


WriteImageCommand::WriteImageCommand(){

}



WriteImageCommand::~WriteImageCommand(){

}


NDRangeKernelCommand::NDRangeKernelCommand(){

}



NDRangeKernelCommand::~NDRangeKernelCommand(){

}


TaskCommand::TaskCommand(){

}



TaskCommand::~TaskCommand(){

}


ReadImageCommand::ReadImageCommand(){

}



ReadImageCommand::~ReadImageCommand(){

}


ImageToBufferCommand::ImageToBufferCommand(){

}



ImageToBufferCommand::~ImageToBufferCommand(){

}


BufferToImageCommand::BufferToImageCommand(){

}



BufferToImageCommand::~BufferToImageCommand(){

}


MapBufferCommand::MapBufferCommand(){

}



MapBufferCommand::~MapBufferCommand(){

}


MapImageCommand::MapImageCommand(){

}



MapImageCommand::~MapImageCommand(){

}


UnmapMemObjectCommand::UnmapMemObjectCommand(){

}



UnmapMemObjectCommand::~UnmapMemObjectCommand(){

}


MarkerCommand::MarkerCommand(){

}



MarkerCommand::~MarkerCommand(){

}


WaitForEventsCommand::WaitForEventsCommand(){

}



WaitForEventsCommand::~WaitForEventsCommand(){

}


BarrierCommand::BarrierCommand(){

}



BarrierCommand::~BarrierCommand(){

}


NativeKernelCommand::NativeKernelCommand(){

}



NativeKernelCommand::~NativeKernelCommand(){

}


CopyBufferCommand::CopyBufferCommand(){

}



CopyBufferCommand::~CopyBufferCommand(){

}


Command::Command(){

}



Command::~Command(){

}





cl_command_type Command::GetCommandType() const 
{

	return  NULL;
}




CopyImageCommand::CopyImageCommand(){

}


CopyImageCommand::~CopyImageCommand(){

}