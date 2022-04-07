// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "RunnerFactoryGenerator.h"
#include "OpenCLFactory.h"

using namespace Validation;

IRunnerFactory* RunnerFactory::s_oclFactory = NULL;

RunnerFactory::RunnerFactory()
{
}

IRunnerFactory& RunnerFactory::GetInstance()
{
    if( NULL == s_oclFactory)
    {
        s_oclFactory = new OpenCLFactory();
    }
    return *s_oclFactory;
}
