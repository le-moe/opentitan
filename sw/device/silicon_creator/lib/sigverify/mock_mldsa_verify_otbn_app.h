// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MOCK_MLDSA_VERIFY_OTBN_APP_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MOCK_MLDSA_VERIFY_OTBN_APP_H_

#include "sw/device/lib/base/global_mock.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify_otbn_app.h"
#include "sw/device/silicon_creator/testing/rom_test.h"

namespace rom_test {
namespace internal {

/**
 * Mock class for mldsa_verify_otbn_app.c.
 */
class MockMldsaVerifyOtbnApp
    : public global_mock::GlobalMock<MockMldsaVerifyOtbnApp> {
 public:
  MOCK_METHOD(rom_error_t, BootAppLoad, ());
};

}  // namespace internal

using MockMldsaVerifyOtbnApp =
    testing::StrictMock<internal::MockMldsaVerifyOtbnApp>;

}  // namespace rom_test

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MOCK_MLDSA_VERIFY_OTBN_APP_H_
