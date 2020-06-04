/*
    Copyright (c) 2005-2020 Intel Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef __TBB_task_H
#define __TBB_task_H

#include "detail/_config.h"
#include "detail/_task.h"

#if __TBB_RESUMABLE_TASKS
namespace tbb {
inline namespace v1 {
namespace task {
    using detail::d1::suspend_point;
    using detail::d1::resume;
    using detail::d1::suspend;
} // namespace task
} // namespace v1
} // namespace tbb
#endif /* __TBB_RESUMABLE_TASKS */

#endif /* __TBB_task_H */
