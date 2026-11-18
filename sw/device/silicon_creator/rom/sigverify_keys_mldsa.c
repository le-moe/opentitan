// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/rom/sigverify_keys_mldsa.h"

#include <assert.h>
#include <stddef.h>

#include "sw/device/lib/base/hardened.h"
#include "sw/device/lib/base/hardened_memory.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify.h"
#include "sw/device/silicon_creator/rom/sigverify_otp_keys.h"

rom_error_t sigverify_mldsa_key_get(
    const sigverify_otp_key_ctx_t *sigverify_ctx,
    const sigverify_mldsa87_key_t *manifest_key, lifecycle_state_t lc_state,
    uint32_t *tr, const sigverify_rom_pq_key_t **key) {
  if (key == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }
  *key = NULL;
  if (sigverify_ctx == NULL || manifest_key == NULL || tr == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }

  HARDENED_RETURN_IF_ERROR(mldsa_compute_tr(manifest_key, tr));

  const sigverify_rom_key_header_t *rom_key = NULL;
  rom_error_t error = sigverify_otp_keys_get(
      (sigverify_otp_keys_get_params_t){
          .key_id = tr[0],
          .lc_state = lc_state,
          .key_array =
              (const sigverify_rom_key_header_t *)(sigverify_ctx->keys.pq),
          .key_cnt = kSigVerifyOtpKeysPqCount,
          .key_size = sizeof(sigverify_rom_pq_key_t),
          .key_states = (uint32_t *)&sigverify_ctx->states.pq[0],
          .extra_digest = tr,
          .extra_digest_offset =
              offsetof(sigverify_rom_pq_key_t, mldsa.mldsa_tr),
          .extra_digest_words = kSigverifyMldsa87KeyDigestWords,
      },
      &rom_key);

  if (launder32(error) != kErrorOk) {
    return kErrorSigverifyBadMldsaKey;
  }
  HARDENED_CHECK_EQ(error, kErrorOk);

  const sigverify_rom_pq_key_t *cand_key =
      (const sigverify_rom_pq_key_t *)rom_key;
  static_assert(
      sizeof(cand_key->mldsa.mldsa_tr) == kSigverifyMldsa87KeyDigestBytes,
      "Unexpected ML-DSA key digest size");
  if (launder32(hardened_memeq(tr, cand_key->mldsa.mldsa_tr,
                               kSigverifyMldsa87KeyDigestWords)) !=
      kHardenedBoolTrue) {
    return kErrorSigverifyBadMldsaKey;
  }
  HARDENED_CHECK_EQ(hardened_memeq(tr, cand_key->mldsa.mldsa_tr,
                                   kSigverifyMldsa87KeyDigestWords),
                    kHardenedBoolTrue);

  *key = cand_key;
  return kErrorOk;
}
