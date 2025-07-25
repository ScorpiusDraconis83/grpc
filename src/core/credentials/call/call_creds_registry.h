//
// Copyright 2025 gRPC authors.
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
//

#ifndef GRPC_SRC_CORE_CREDENTIALS_CALL_CALL_CREDS_REGISTRY_H
#define GRPC_SRC_CORE_CREDENTIALS_CALL_CALL_CREDS_REGISTRY_H

#include <map>
#include <memory>
#include <type_traits>
#include <utility>

#include "absl/strings/string_view.h"
#include "src/core/util/json/json.h"
#include "src/core/util/json/json_args.h"
#include "src/core/util/ref_counted.h"
#include "src/core/util/ref_counted_ptr.h"
#include "src/core/util/validation_errors.h"

struct grpc_call_credentials;

namespace grpc_core {

class CallCredsConfig : public RefCounted<CallCredsConfig> {
 public:
  virtual absl::string_view type() const = 0;

  virtual bool Equals(const CallCredsConfig& other) const = 0;

  virtual std::string ToString() const = 0;
};

template <typename T = grpc_call_credentials>
class CallCredsFactory final {
 public:
  virtual ~CallCredsFactory() {}
  virtual absl::string_view type() const = delete;
  virtual RefCountedPtr<CallCredsConfig> ParseConfig(
      const Json& config, const JsonArgs& args,
      ValidationErrors* errors) const = delete;
  virtual RefCountedPtr<T> CreateCallCreds(
      RefCountedPtr<CallCredsConfig> config) const = delete;
};

template <>
class CallCredsFactory<grpc_call_credentials> {
 public:
  virtual ~CallCredsFactory() {}
  virtual absl::string_view type() const = 0;
  virtual RefCountedPtr<CallCredsConfig> ParseConfig(
      const Json& config, const JsonArgs& args,
      ValidationErrors* errors) const = 0;
  virtual RefCountedPtr<grpc_call_credentials> CreateCallCreds(
      RefCountedPtr<CallCredsConfig> config) const = 0;
};

template <typename T = grpc_call_credentials>
class CallCredsRegistry {
 private:
  using FactoryMap =
      std::map<absl::string_view, std::unique_ptr<CallCredsFactory<T>>>;

 public:
  static_assert(std::is_base_of<grpc_call_credentials, T>::value,
                "CallCredsRegistry must be instantiated with "
                "grpc_call_credentials.");

  class Builder {
   public:
    void RegisterCallCredsFactory(
        std::unique_ptr<CallCredsFactory<T>> factory) {
      absl::string_view type = factory->type();
      factories_[type] = std::move(factory);
    }
    CallCredsRegistry Build() {
      return CallCredsRegistry<T>(std::move(factories_));
    }

   private:
    FactoryMap factories_;
  };

  bool IsSupported(absl::string_view type) const {
    return factories_.find(type) != factories_.end();
  }

  RefCountedPtr<CallCredsConfig> ParseConfig(absl::string_view type,
                                             const Json& config,
                                             const JsonArgs& args,
                                             ValidationErrors* errors) const {
    const auto it = factories_.find(type);
    if (it == factories_.cend()) return nullptr;
    return it->second->ParseConfig(config, args, errors);
  }

  RefCountedPtr<T> CreateCallCreds(
      RefCountedPtr<CallCredsConfig> config) const {
    if (config == nullptr) return nullptr;
    const auto it = factories_.find(config->type());
    if (it == factories_.cend()) return nullptr;
    return it->second->CreateCallCreds(std::move(config));
  }

 private:
  explicit CallCredsRegistry(FactoryMap factories)
      : factories_(std::move(factories)) {}

  FactoryMap factories_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_CREDENTIALS_CALL_CALL_CREDS_REGISTRY_H
