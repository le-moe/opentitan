// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/sigverify/mock_mldsa_verify_otbn_app.h"

namespace {
constexpr uint32_t kFakeImem[4] = {0x11111111, 0x22222222, 0x33333333,
                                   0x44444444};
constexpr uint32_t kFakeDmem[2] = {0x55555555, 0x66666666};
}  // namespace

extern "C" {
const sc_otbn_app_t kMldsaVerifyOtbnApp = {
    .imem_start = kFakeImem,
    .imem_end = kFakeImem + 4,
    .dmem_data_start = kFakeDmem,
    .dmem_data_end = kFakeDmem + 2,
    .dmem_data_start_addr = 0x0000,
};
const sc_otbn_addr_t kMldsaVerifyOtbnMode = 0x0900;
const sc_otbn_addr_t kMldsaVerifyOtbnPk = 0x2360;
const sc_otbn_addr_t kMldsaVerifyOtbnSig = 0x2d80;
const sc_otbn_addr_t kMldsaVerifyOtbnMu = 0x0940;
const sc_otbn_addr_t kMldsaVerifyOtbnResOk = 0x0b00;
const sc_otbn_addr_t kMldsaVerifyOtbnResCTildePrime = 0x0b40;

rom_error_t mldsa_verify_otbn_boot_app_load(void) {
  return rom_test::MockMldsaVerifyOtbnApp::Instance().BootAppLoad();
}
}  // extern "C"
