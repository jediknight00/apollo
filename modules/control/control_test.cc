/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "modules/control/control_component.h"

#include <string>
#include <utility>

#include "cybertron/common/log.h"
#include "gtest/gtest.h"
#include "modules/common/util/file.h"
#include "modules/control/common/control_gflags.h"
#include "modules/control/proto/control_conf.pb.h"

namespace apollo {
namespace control {

class ControlTest : public ::testing::Test {
 public:
  virtual void SetUp() {}

 protected:
  ControlComponent control_;
};

TEST_F(ControlTest, Init) {
  FLAGS_control_conf_file =
      "/apollo/modules/control/testdata/conf/lincoln.pb.txt";
  EXPECT_EQ(true, control_.Init());
}

}  // namespace control
}  // namespace apollo
