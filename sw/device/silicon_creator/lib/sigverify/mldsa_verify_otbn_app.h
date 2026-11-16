// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_OTBN_APP_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_OTBN_APP_H_

#include "sw/device/lib/base/macros.h"
#include "sw/device/silicon_creator/lib/drivers/otbn.h"
#include "sw/device/silicon_creator/lib/error.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * OTBN binary image and DMEM symbol addresses for `run_mldsa87`.
 */
extern const sc_otbn_app_t kMldsaVerifyOtbnApp;
extern const sc_otbn_addr_t kMldsaVerifyOtbnMode;
extern const sc_otbn_addr_t kMldsaVerifyOtbnPk;
extern const sc_otbn_addr_t kMldsaVerifyOtbnSig;
extern const sc_otbn_addr_t kMldsaVerifyOtbnMu;
extern const sc_otbn_addr_t kMldsaVerifyOtbnResOk;
extern const sc_otbn_addr_t kMldsaVerifyOtbnResCTildePrime;

/**
 * Reloads the OTBN boot-services application after ML-DSA verification.
 *
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_verify_otbn_boot_app_load(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_OTBN_APP_H_
