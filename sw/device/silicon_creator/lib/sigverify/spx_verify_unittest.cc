// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/sigverify/spx_verify.h"

#include "gtest/gtest.h"
#include "sw/device/lib/base/hardened.h"
#include "sw/device/silicon_creator/lib/drivers/lifecycle.h"
#include "sw/device/silicon_creator/lib/drivers/mock_otp.h"
#include "sw/device/silicon_creator/testing/rom_test.h"

#include "hw/top/otp_ctrl_regs.h"

namespace spx_verify_unittest {
namespace {
using ::testing::Return;

class SpxVerifyPolicyTest : public rom_test::RomTest {
 protected:
  rom_test::MockOtp otp_;
};

TEST_F(SpxVerifyPolicyTest, TestUnlockedAlwaysDisabled) {
  // TEST_UNLOCKED* states ignore OTP and force PQ disabled.
  EXPECT_EQ(sigverify_pq_policy_get(kLcStateTest), kSigverifyPqPolicyDisabled);
}

TEST_F(SpxVerifyPolicyTest, OperationalLcStatesDecodeOtp) {
  const lifecycle_state_t kOperationalStates[] = {
      kLcStateDev,
      kLcStateProd,
      kLcStateProdEnd,
      kLcStateRma,
  };
  for (lifecycle_state_t lc : kOperationalStates) {
    EXPECT_CALL(otp_,
                read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
        .WillOnce(Return(kSigverifySpxDisabledOtp));
    EXPECT_EQ(sigverify_pq_policy_get(lc), kSigverifyPqPolicyDisabled);

    EXPECT_CALL(otp_,
                read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
        .WillOnce(Return(0u));
    EXPECT_EQ(sigverify_pq_policy_get(lc), kSigverifyPqPolicySpx);

    EXPECT_CALL(otp_,
                read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
        .WillOnce(Return(kHardenedBoolTrue));
    EXPECT_EQ(sigverify_pq_policy_get(lc), kSigverifyPqPolicySpx);

    EXPECT_CALL(otp_,
                read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
        .WillOnce(Return(kSigverifyPqMldsaEnabledOtp));
    EXPECT_EQ(sigverify_pq_policy_get(lc), kSigverifyPqPolicyMldsa);
  }
}

class SpxVerifyPolicyDeathTest : public SpxVerifyPolicyTest {};

TEST_F(SpxVerifyPolicyDeathTest, UnknownOtpValueTraps) {
  EXPECT_DEATH(
      {
        EXPECT_CALL(
            otp_, read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
            .WillOnce(Return(0xdeadbeef));
        OT_DISCARD(sigverify_pq_policy_get(kLcStateProd));
      },
      "");

  EXPECT_DEATH(
      {
        EXPECT_CALL(
            otp_, read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET))
            .WillOnce(Return(kHardenedBoolFalse));
        OT_DISCARD(sigverify_pq_policy_get(kLcStateDev));
      },
      "");
}

}  // namespace
}  // namespace spx_verify_unittest
