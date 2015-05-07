/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BINDINGS_TESTING_OPERATIONS_TEST_INTERFACE_H_
#define BINDINGS_TESTING_OPERATIONS_TEST_INTERFACE_H_

#include <string>

#include "base/optional.h"
#include "cobalt/bindings/testing/arbitrary_interface.h"
#include "cobalt/script/wrappable.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace cobalt {
namespace bindings {
namespace testing {

class OperationsTestInterface : public script::Wrappable {
 public:
  MOCK_METHOD0(VoidFunctionNoArgs, void());
  MOCK_METHOD0(StringFunctionNoArgs, std::string());
  MOCK_METHOD0(ObjectFunctionNoArgs, scoped_refptr<ArbitraryInterface>());

  MOCK_METHOD1(VoidFunctionStringArg, void(const std::string&));
  MOCK_METHOD1(VoidFunctionObjectArg,
               void(const scoped_refptr<ArbitraryInterface>&));

  MOCK_METHOD1(OptionalArguments, void(int32_t));
  MOCK_METHOD2(OptionalArguments, void(int32_t, int32_t));
  MOCK_METHOD3(OptionalArguments, void(int32_t, int32_t, int32_t));

  MOCK_METHOD1(OptionalArgumentWithDefault, void(double));
  MOCK_METHOD2(OptionalNullableArgumentsWithDefaults,
               void(base::optional<bool>,
                    const scoped_refptr<ArbitraryInterface>&));

  MOCK_METHOD1(VariadicPrimitiveArguments, void(const std::vector<int32_t>));
  MOCK_METHOD0(VariadicStringArgumentsAfterOptionalArgument, void());
  MOCK_METHOD2(VariadicStringArgumentsAfterOptionalArgument,
               void(bool, const std::vector<std::string>));

  DEFINE_WRAPPABLE_TYPE(OperationsTestInterface);
};

}  // namespace testing
}  // namespace bindings
}  // namespace cobalt

#endif  // BINDINGS_TESTING_OPERATIONS_TEST_INTERFACE_H_
