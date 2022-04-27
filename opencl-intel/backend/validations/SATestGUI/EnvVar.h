// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
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

#ifndef ENVVAR_H
#define ENVVAR_H
#include <QString>

namespace Validation
{
namespace GUI
{
/**
 * @brief The EnvVar struct
 * @detailed This struct contains information about user pecific enviroment variables
 */
struct EnvVar
{
    QString name;
    QString value;
};

}
}
#endif // ENVVAR_H
