// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>

#include "hw/top/dt/otp_ctrl.h"
#include "sw/device/lib/runtime/log.h"
#include "sw/device/lib/testing/test_framework/ottf_main.h"
#include "sw/device/silicon_creator/lib/drivers/lifecycle.h"
#include "sw/device/silicon_creator/lib/drivers/otp.h"
#include "sw/device/silicon_creator/lib/sigverify/spx_verify.h"

#include "hw/top/otp_ctrl_regs.h"

OTTF_DEFINE_TEST_CONFIG();

bool test_main(void) {
  lifecycle_state_t lc = lifecycle_state_get();
  LOG_INFO("lc_state=0x%08x, pq_policy=0x%08x, spx_en_otp=0x%08x", lc,
           sigverify_pq_policy_get(lc),
           otp_read32(OTP_CTRL_PARAM_CREATOR_SW_CFG_SIGVERIFY_SPX_EN_OFFSET));
  return true;
}
