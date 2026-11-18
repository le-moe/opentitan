// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_ROM_SIGVERIFY_KEYS_MLDSA_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_ROM_SIGVERIFY_KEYS_MLDSA_H_

#include <stdint.h>

#include "sw/device/lib/base/macros.h"
#include "sw/device/silicon_creator/lib/drivers/lifecycle.h"
#include "sw/device/silicon_creator/lib/error.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa87_key.h"
#include "sw/device/silicon_creator/rom/sigverify_key_types.h"
#include "sw/device/silicon_creator/rom/sigverify_otp_keys.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Returns the OTP key entry for an ML-DSA-87 manifest public key.
 *
 * Computes `tr = SHAKE256(manifest_key, 64)` once via `mldsa_compute_tr()`,
 * uses `tr[0]` as the key ID to look up the entry in the post-quantum OTP key
 * array, validates key state and life cycle eligibility via
 * `sigverify_otp_keys_get()`, and compares the truncated 48-byte digest
 * `tr[0..11]` against OTP via `hardened_memeq()`.
 *
 * @param sigverify_ctx OTP key context loaded by `sigverify_otp_keys_init()`.
 * @param manifest_key ML-DSA-87 public key from the manifest extension.
 * @param lc_state Life cycle state of the device.
 * @param[out] tr 16-word buffer receiving the full 64-byte `tr` digest.
 * @param[out] key Matched PQ OTP key entry on success.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t sigverify_mldsa_key_get(
    const sigverify_otp_key_ctx_t *sigverify_ctx,
    const sigverify_mldsa87_key_t *manifest_key, lifecycle_state_t lc_state,
    uint32_t *tr, const sigverify_rom_pq_key_t **key);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_ROM_SIGVERIFY_KEYS_MLDSA_H_
