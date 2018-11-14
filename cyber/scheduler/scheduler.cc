/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Vesched_infoon 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "cyber/scheduler/scheduler.h"

#include <utility>

#include "cyber/common/global_data.h"
#include "cyber/common/util.h"
#include "cyber/data/data_visitor.h"
#include "cyber/event/perf_event_cache.h"
#include "cyber/scheduler/policy/choreography.h"
#include "cyber/scheduler/policy/classic.h"
#include "cyber/scheduler/policy/scheduler_choreography.h"
#include "cyber/scheduler/policy/scheduler_classic.h"
#include "cyber/scheduler/processor.h"
#include "cyber/scheduler/processor_context.h"
#include "cyber/common/environment.h"
#include "cyber/common/file.h"

namespace apollo {
namespace cyber {
namespace scheduler {

using apollo::cyber::common::GlobalData;
using apollo::cyber::event::PerfEventCache;
using apollo::cyber::event::SchedPerf;
using apollo::cyber::common::GetAbsolutePath;
using apollo::cyber::common::PathExists;
using apollo::cyber::common::GetProtoFromFile;
using apollo::cyber::common::WorkRoot;

Scheduler* Scheduler::Instance() {
  static Scheduler* instance = nullptr;

  if (unlikely(!instance)) {
    std::string policy = "classic";

    // Get sched policy from conf
    std::string conf("conf/");
    conf.append(GlobalData::Instance()->SchedName()).append(".conf");
    auto cfg_file = GetAbsolutePath(WorkRoot(), conf);

    apollo::cyber::proto::CyberConfig cfg;
    if (PathExists(cfg_file) && GetProtoFromFile(cfg_file, &cfg)) {
      policy = cfg.scheduler_conf().policy();
    } else {
      AERROR << "Pls make sure schedconf exist and which format is correct.\n";
    }

    if (!policy.compare(std::string("classic"))) {
      instance = new SchedulerClassic();
    } else if (!policy.compare(std::string("choreography"))) {
      instance = new SchedulerChoreography();
    } else {
      instance = new SchedulerClassic();
    }
  }

  return instance;
}

void Scheduler::ShutDown() {
  if (stop_.exchange(true)) {
    return;
  }

  for (auto& ctx : pctxs_) {
    ctx->ShutDown();
  }
  pctxs_.clear();
}

bool Scheduler::CreateTask(const RoutineFactory& factory,
                           const std::string& name) {
  return CreateTask(factory.create_routine(), name, factory.GetDataVisitor());
}

bool Scheduler::CreateTask(std::function<void()>&& func,
                           const std::string& name,
                           std::shared_ptr<DataVisitorBase> visitor) {
  if (stop_) {
    AERROR << "scheduler is stoped, cannot create task!";
    return false;
  }

  auto task_id = GlobalData::RegisterTaskName(name);

  auto cr = std::make_shared<CRoutine>(func);
  cr->set_id(task_id);
  cr->set_name(name);

  if (!DispatchTask(cr)) {
    return false;
  }

  if (visitor != nullptr) {
    visitor->RegisterNotifyCallback([this, task_id, name]() {
      if (stop_) {
        return;
      }
      this->NotifyProcessor(task_id);
    });
  }
  return true;
}

bool Scheduler::NotifyTask(uint64_t crid) {
  if (stop_) {
    return true;
  }
  return NotifyProcessor(crid);
}

}  // namespace scheduler
}  // namespace cyber
}  // namespace apollo
