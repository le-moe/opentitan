// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_H_

#include <stddef.h>
#include <stdint.h>

#include "sw/device/lib/base/hardened.h"
#include "sw/device/lib/base/macros.h"
#include "sw/device/silicon_creator/lib/error.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa87_key.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Computes public key hash `tr = SHAKE256(pk, 64)` per FIPS 204.
 *
 * @param key Pointer to ML-DSA-87 public key.
 * @param[out] tr 64-byte (16-word) buffer receiving `tr`.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_compute_tr(const sigverify_mldsa87_key_t *key, uint32_t *tr);

/**
 * Computes the key ID (first 32-bit word of `tr = SHAKE256(pk, 64)`).
 *
 * @param key Pointer to ML-DSA-87 public key.
 * @param[out] key_id Key ID output buffer.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t sigverify_mldsa87_key_id_get(const sigverify_mldsa87_key_t *key,
                                         uint32_t *key_id);

/**
 * Starts streaming FIPS 204 §6.2 Pure mode message representative `mu`:
 *   mu = SHAKE256(tr || 0x00 || len(ctx) || ctx || ..., 64)
 *
 * Uses the context string defined by `kSigverifyMldsa87ImageContext` and
 * `kSigverifyMldsa87ImageContextSize`.
 * Callers must invoke `mldsa_mu_update` zero or more times followed by
 * `mldsa_mu_final`.
 *
 * @param tr Precomputed 64-byte public key hash `tr` (16 words).
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_mu_init(const uint32_t *tr);

/**
 * Absorbs message bytes into an active `mldsa_mu_init` session.
 *
 * @param data Input buffer (must be non-NULL when `len > 0`).
 * @param len Number of bytes to absorb.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_mu_update(const void *data, size_t len);

/**
 * Finalizes the streaming `mu` calculation and reads 64 bytes (16 words).
 *
 * @param[out] mu 64-byte (16-word) buffer receiving `mu`.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_mu_final(uint32_t *mu);

/**
 * Computes FIPS 204 §6.2 Pure mode message representative with empty context:
 *   mu = SHAKE256(tr || 0x00 || 0x00 || msg_prefix_1 || msg_prefix_2 || msg,
 * 64)
 *
 * @param tr Precomputed 64-byte public key hash `tr` (16 words).
 * @param msg_prefix_1 Optional prefix 1 (may be NULL if `msg_prefix_1_len ==
 * 0`).
 * @param msg_prefix_1_len Length of prefix 1 in bytes.
 * @param msg_prefix_2 Optional prefix 2 (may be NULL if `msg_prefix_2_len ==
 * 0`).
 * @param msg_prefix_2_len Length of prefix 2 in bytes.
 * @param msg Optional message buffer (may be NULL if `msg_len == 0`).
 * @param msg_len Length of message in bytes.
 * @param[out] mu 64-byte (16-word) buffer receiving message representative
 * `mu`.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_compute_mu(const uint32_t *tr, const void *msg_prefix_1,
                             size_t msg_prefix_1_len, const void *msg_prefix_2,
                             size_t msg_prefix_2_len, const void *msg,
                             size_t msg_len, uint32_t *mu);

/**
 * Starts ML-DSA-87 signature verification on OTBN (`run_mldsa87`).
 *
 * Validates that the zero-padding byte at offset 4627 is 0x00 without
 * duplicating the signature on the stack.
 *
 * @param key Public key.
 * @param sig Signature.
 * @param mu Precomputed message representative `mu` (16 words).
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_verify_start(const sigverify_mldsa87_key_t *key,
                               const sigverify_mldsa87_signature_t *sig,
                               const uint32_t *mu);

/**
 * Finishes ML-DSA-87 signature verification on OTBN.
 *
 * Unconditionally wipes OTBN DMEM and reloads the OTBN boot-services app before
 * returning, on both success and failure paths.
 *
 * @param sig Signature.
 * @param[in,out] flash_exec Flash execution token (`kSigverifyMldsaSuccess` is
 *                           XORed in on valid signature; `UINT32_MAX`
 * otherwise).
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_verify_finish(const sigverify_mldsa87_signature_t *sig,
                                uint32_t *flash_exec);

/**
 * Synchronous ML-DSA-87 signature verification entry point.
 *
 * @param key Public key.
 * @param sig Signature.
 * @param mu Precomputed message representative `mu` (16 words).
 * @param[in,out] flash_exec Flash execution token.
 * @return Result of the operation.
 */
OT_WARN_UNUSED_RESULT
rom_error_t mldsa_verify(const sigverify_mldsa87_key_t *key,
                         const sigverify_mldsa87_signature_t *sig,
                         const uint32_t *mu, uint32_t *flash_exec);

/**
 * Transforms `kSigverifyMldsaSuccess` (`0x8d6c8c17`) into `kErrorOk` (`0x739`).
 *
 * @param v A 32-bit flash_exec share value.
 * @return `kErrorOk` if `v == kSigverifyMldsaSuccess`.
 */
OT_WARN_UNUSED_RESULT
inline uint32_t sigverify_mldsa_success_to_ok(uint32_t v) {
  return (v << 22 ^ v << 8 ^ v << 1) >> 20;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA_VERIFY_H_
