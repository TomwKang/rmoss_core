// Copyright 2021 RoboMaster-OSS
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef RM_BASE__TRANSPORTER_INTERFACE_HPP_
#define RM_BASE__TRANSPORTER_INTERFACE_HPP_

#include <memory>

namespace rm_base
{

// Transporter device interface to transport data between embedded systems (stm32,c51) and PC
class TransporterInterface
{
public:
  using SharedPtr = std::shared_ptr<TransporterInterface>;
  virtual bool open() {return false;}
  virtual bool close() {return false;}
  virtual bool is_open() = 0;
  // return recv len>0,return <0 if error
  virtual int read(unsigned char * buffer, int len) = 0;
  // return send len>0,return <0 if error
  virtual int write(const unsigned char * buffer, int len) = 0;
};

}  // namespace rm_base

#endif  // RM_BASE__TRANSPORTER_INTERFACE_HPP_
