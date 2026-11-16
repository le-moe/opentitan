// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify.h"

#include "sw/device/lib/base/hardened.h"
#include "sw/device/silicon_creator/lib/base/sec_mmio.h"
#include "sw/device/silicon_creator/lib/drivers/kmac.h"
#include "sw/device/silicon_creator/lib/drivers/otbn.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify_otbn_app.h"

/*
 * Shares producing `kSigverifyMldsaSuccess` (`0x8d6c8c17`) when XORed together.
 * Combined with `kSigverifyEcdsaSuccess` (`0x2f06b4e0`), this yields
 * `kSigverifyFlashExec` (`0xa26a38f7`).
 */
static const uint32_t kMldsaShares[kMldsa87CTildeWords] = {
    0x39a1f2b4, 0x8c7e01d5, 0x54b3a98e, 0x12f6c07a, 0x7da4e891, 0x9b20d3f8,
    0x4f128c6e, 0x6e5b309a, 0xa09f7c12, 0x2b8d4e9f, 0xef31048b, 0xd47a96c1,
    0x1c8b3e52, 0x63a921d7, 0x8e5f03ba, 0xf81f2de7,
};

/**
 * Checks that the zero-padding byte of `sig` (offset 4627) is strictly 0x00.
 */
OT_WARN_UNUSED_RESULT
static rom_error_t check_sig_padding_zero(
    const sigverify_mldsa87_signature_t *sig) {
  uint8_t pad = ((const uint8_t *)sig->data)[kSigverifyMldsa87SigBytes];
  if (launder32(pad) != 0) {
    return kErrorSigverifyBadMldsaSignature;
  }
  HARDENED_CHECK_EQ(pad, 0);
  return kErrorOk;
}

/**
 * Wipes OTBN DMEM and reloads the OTBN boot-services app.
 */
OT_WARN_UNUSED_RESULT
static rom_error_t cleanup_otbn_to_boot_app(void) {
  rom_error_t wipe_err = sc_otbn_dmem_sec_wipe();
  rom_error_t reload_err = mldsa_verify_otbn_boot_app_load();
  HARDENED_RETURN_IF_ERROR(wipe_err);
  return reload_err;
}

/**
 * Drains KMAC out of `SHA3_ABSORB` back to Idle on error paths and returns
 * `err`.
 */
static rom_error_t recover_kmac(rom_error_t err) {
  kmac_shake256_abort_if_absorbing();
  return err;
}

rom_error_t mldsa_compute_tr(const sigverify_mldsa87_key_t *key, uint32_t *tr) {
  if (key == NULL || tr == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }
  rom_error_t err = kmac_shake256_configure();
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  err = kmac_shake256_start();
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  kmac_shake256_absorb_words(key->data, kSigverifyMldsa87PkWords);
  kmac_shake256_squeeze_start();
  err = kmac_shake256_squeeze_end(tr, kMldsa87TrWords);
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  HARDENED_CHECK_EQ(err, kErrorOk);
  return kErrorOk;
}

rom_error_t sigverify_mldsa87_key_id_get(const sigverify_mldsa87_key_t *key,
                                         uint32_t *key_id) {
  if (key == NULL || key_id == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }
  uint32_t tr[kMldsa87TrWords];
  HARDENED_RETURN_IF_ERROR(mldsa_compute_tr(key, tr));
  *key_id = tr[0];
  return kErrorOk;
}

rom_error_t mldsa_mu_init(const uint32_t *tr) {
  if (tr == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }
  HARDENED_CHECK_LE(kSigverifyMldsa87ImageContextSize, UINT8_MAX);
  rom_error_t err = kmac_shake256_configure();
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  err = kmac_shake256_start();
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  kmac_shake256_absorb_words(tr, kMldsa87TrWords);

  // FIPS 204 §6.2 Pure mode: 0x00 || len(ctx) || ctx.
  const uint8_t domain_and_len[2] = {
      0x00,
      (uint8_t)kSigverifyMldsa87ImageContextSize,
  };
  kmac_shake256_absorb(domain_and_len, sizeof(domain_and_len));
  if (kSigverifyMldsa87ImageContextSize > 0) {
    kmac_shake256_absorb(kSigverifyMldsa87ImageContext,
                         kSigverifyMldsa87ImageContextSize);
  }
  return kErrorOk;
}

rom_error_t mldsa_mu_update(const void *data, size_t len) {
  if (len == 0) {
    return kErrorOk;
  }
  if (data == NULL) {
    return recover_kmac(kErrorSigverifyBadMldsaSignature);
  }
  kmac_shake256_absorb((const uint8_t *)data, len);
  return kErrorOk;
}

rom_error_t mldsa_mu_final(uint32_t *mu) {
  if (mu == NULL) {
    return recover_kmac(kErrorSigverifyBadMldsaSignature);
  }
  kmac_shake256_squeeze_start();
  rom_error_t err = kmac_shake256_squeeze_end(mu, kMldsa87MuWords);
  if (launder32(err) != kErrorOk) {
    return recover_kmac(err);
  }
  HARDENED_CHECK_EQ(err, kErrorOk);
  return kErrorOk;
}

rom_error_t mldsa_compute_mu(const uint32_t *tr, const void *msg_prefix_1,
                             size_t msg_prefix_1_len, const void *msg_prefix_2,
                             size_t msg_prefix_2_len, const void *msg,
                             size_t msg_len, uint32_t *mu) {
  HARDENED_RETURN_IF_ERROR(mldsa_mu_init(tr));
  HARDENED_RETURN_IF_ERROR(mldsa_mu_update(msg_prefix_1, msg_prefix_1_len));
  HARDENED_RETURN_IF_ERROR(mldsa_mu_update(msg_prefix_2, msg_prefix_2_len));
  HARDENED_RETURN_IF_ERROR(mldsa_mu_update(msg, msg_len));
  return mldsa_mu_final(mu);
}

rom_error_t mldsa_verify_start(const sigverify_mldsa87_key_t *key,
                               const sigverify_mldsa87_signature_t *sig,
                               const uint32_t *mu) {
  if (key == NULL || mu == NULL) {
    return kErrorSigverifyBadMldsaKey;
  }
  if (sig == NULL) {
    return kErrorSigverifyBadMldsaSignature;
  }
  HARDENED_RETURN_IF_ERROR(check_sig_padding_zero(sig));

  HARDENED_RETURN_IF_ERROR(sc_otbn_load_app(kMldsaVerifyOtbnApp));

  uint32_t mode = kMldsa87ModeVerify;
  rom_error_t err = sc_otbn_dmem_write_public(1, &mode, kMldsaVerifyOtbnMode);
  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_dmem_write_public(kSigverifyMldsa87PublicKeyWords, key->data,
                                    kMldsaVerifyOtbnPk);
  }
  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_dmem_write_public(kSigverifyMldsa87SignatureWords, sig->data,
                                    kMldsaVerifyOtbnSig);
  }
  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_dmem_write_public(kMldsa87MuWords, mu, kMldsaVerifyOtbnMu);
  }
  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_execute_start();
    if (launder32(err) == kErrorOk) {
      HARDENED_CHECK_EQ(err, kErrorOk);
      SEC_MMIO_WRITE_INCREMENT(kScOtbnSecMmioExecute);
      return kErrorOk;
    }
  }

  OT_DISCARD(cleanup_otbn_to_boot_app());
  return err;
}

rom_error_t mldsa_verify_finish(const sigverify_mldsa87_signature_t *sig,
                                uint32_t *flash_exec) {
  if (flash_exec == NULL) {
    OT_DISCARD(cleanup_otbn_to_boot_app());
    return kErrorSigverifyBadMldsaSignature;
  }
  if (sig == NULL) {
    *flash_exec ^= UINT32_MAX;
    OT_DISCARD(cleanup_otbn_to_boot_app());
    return kErrorSigverifyBadMldsaSignature;
  }

  rom_error_t err = sc_otbn_execute_finish();
  uint32_t ok = 0;
  uint32_t c_tilde_prime[kMldsa87CTildeWords] = {0};

  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_dmem_read(1, kMldsaVerifyOtbnResOk, &ok);
  }
  if (launder32(err) == kErrorOk) {
    HARDENED_CHECK_EQ(err, kErrorOk);
    err = sc_otbn_dmem_read(kMldsa87CTildeWords, kMldsaVerifyOtbnResCTildePrime,
                            c_tilde_prime);
  }

  // Always wipe DMEM and restore the OTBN boot-services app on exit.
  rom_error_t cleanup_err = cleanup_otbn_to_boot_app();
  if (launder32(err) != kErrorOk) {
    *flash_exec ^= UINT32_MAX;
    return err;
  }
  if (launder32(cleanup_err) != kErrorOk) {
    *flash_exec ^= UINT32_MAX;
    return cleanup_err;
  }
  HARDENED_CHECK_EQ(err, kErrorOk);
  HARDENED_CHECK_EQ(cleanup_err, kErrorOk);

  if (launder32(check_sig_padding_zero(sig)) != kErrorOk) {
    *flash_exec ^= UINT32_MAX;
    return kErrorSigverifyBadMldsaSignature;
  }

  size_t i = 0;
  for (size_t j = 0; launder32(j) < kMldsa87CTildeWords; ++j, ++i) {
    c_tilde_prime[i] ^= sig->data[j] ^ kMldsaShares[i];
  }
  HARDENED_CHECK_EQ(i, kMldsa87CTildeWords);

  uint32_t flash_exec_mldsa = 0;
  uint32_t diff = 0;
  for (--i; launder32(i) < kMldsa87CTildeWords; --i) {
    diff |= c_tilde_prime[i] ^ kMldsaShares[i];
    diff |= ~diff + 1;
    diff |= ~(diff >> 31) + 1;
    flash_exec_mldsa ^= c_tilde_prime[i];
    flash_exec_mldsa |= diff;
  }
  HARDENED_CHECK_EQ(i, SIZE_MAX);

  rom_error_t res = sigverify_mldsa_success_to_ok(flash_exec_mldsa);
  if (launder32(ok) != kMldsa87StatusOk || launder32(diff) != 0 ||
      launder32(res) != kErrorOk) {
    *flash_exec ^= UINT32_MAX;
    return kErrorSigverifyBadMldsaSignature;
  }
  HARDENED_CHECK_EQ(ok, kMldsa87StatusOk);
  HARDENED_CHECK_EQ(diff, 0);
  HARDENED_CHECK_EQ(res, kErrorOk);

  *flash_exec ^= flash_exec_mldsa;
  return res;
}

rom_error_t mldsa_verify(const sigverify_mldsa87_key_t *key,
                         const sigverify_mldsa87_signature_t *sig,
                         const uint32_t *mu, uint32_t *flash_exec) {
  if (flash_exec == NULL) {
    return kErrorSigverifyBadMldsaSignature;
  }
  rom_error_t err = mldsa_verify_start(key, sig, mu);
  if (launder32(err) != kErrorOk) {
    *flash_exec ^= UINT32_MAX;
    return err;
  }
  HARDENED_CHECK_EQ(err, kErrorOk);
  return mldsa_verify_finish(sig, flash_exec);
}

// Extern declaration for the inline helper in mldsa_verify.h.
extern uint32_t sigverify_mldsa_success_to_ok(uint32_t v);
