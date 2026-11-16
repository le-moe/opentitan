// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA87_KEY_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA87_KEY_H_

#include <stddef.h>
#include <stdint.h>

#include "sw/device/lib/base/macros.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

enum {
  /**
   * Size of a ML-DSA-87 public key.
   */
  kSigverifyMldsa87PkBytes = 2592,
  kSigverifyMldsa87PkWords = kSigverifyMldsa87PkBytes / sizeof(uint32_t),
  kSigverifyMldsa87PublicKeyBytes = kSigverifyMldsa87PkBytes,
  kSigverifyMldsa87PublicKeyWords = kSigverifyMldsa87PkWords,

  /** Size of an ML-DSA-87 public key digest (truncated tr) in words and bytes.
   */
  kSigverifyMldsa87KeyDigestWords = 12,
  kSigverifyMldsa87KeyDigestBytes = 48,

  /**
   * Size of a ML-DSA-87 signature. It is stored zero-padded to a whole number
   * of words.
   */
  kSigverifyMldsa87SigBytes = 4627,
  kSigverifyMldsa87SigWords =
      (kSigverifyMldsa87SigBytes + sizeof(uint32_t) - 1) / sizeof(uint32_t),
  kSigverifyMldsa87SignatureBytes = 4628,
  kSigverifyMldsa87SignatureWords = kSigverifyMldsa87SigWords,

  /** Size of c_tilde in bytes and words. */
  kMldsa87CTildeBytes = 64,
  kMldsa87CTildeWords = kMldsa87CTildeBytes / sizeof(uint32_t),

  /** Size of intermediate hash mu and tr in words (64 bytes each). */
  kMldsa87MuWords = 16,
  kMldsa87TrWords = 16,

  /** Mode selector written to `mldsa87_mode` in OTBN `run_mldsa87` DMEM. */
  kMldsa87ModeVerify = 0x1f9b0e52,

  /**
   * Multi-bit status token written to DMEM by OTBN `run_mldsa87` on success.
   */
  kMldsa87StatusOk = 0x7baf73d2,
  kMldsa87StatusFail = 0xadf1aebd,

  /**
   * Non-trivial constant such that:
   *   kSigverifyEcdsaSuccess ^ kSigverifyMldsaSuccess = kSigverifyFlashExec
   * (0xa26a38f7) With kSigverifyEcdsaSuccess = 0x2f06b4e0,
   * kSigverifyMldsaSuccess = 0x8d6c8c17.
   */
  kSigverifyMldsaSuccess = 0x8d6c8c17,
};

/**
 * Context string of the ML-DSA-87 image signature (FIPS 204, pure mode).
 *
 * Must match `MLDSA87_IMAGE_CONTEXT` in
 * `sw/host/opentitanlib/src/image/manifest.rs`.
 */
extern const uint8_t kSigverifyMldsa87ImageContext[];
extern const size_t kSigverifyMldsa87ImageContextSize;

/**
 * An ML-DSA-87 signature.
 */
typedef struct sigverify_mldsa87_signature {
  uint32_t data[kSigverifyMldsa87SigWords];
} sigverify_mldsa87_signature_t;

/**
 * An ML-DSA-87 public key.
 */
typedef struct sigverify_mldsa87_key {
  uint32_t data[kSigverifyMldsa87PkWords];
} sigverify_mldsa87_key_t;

/** Alias for compatibility. */
typedef sigverify_mldsa87_key_t sigverify_mldsa87_public_key_t;

/** ML-DSA-87 Public Key Digest structure (48 bytes, 12 words). */
typedef struct sigverify_mldsa87_key_digest {
  uint32_t digest[kSigverifyMldsa87KeyDigestWords];
} sigverify_mldsa87_key_digest_t;

OT_ASSERT_SIZE(sigverify_mldsa87_key_t, 2592);
OT_ASSERT_SIZE(sigverify_mldsa87_key_digest_t, 48);
OT_ASSERT_SIZE(sigverify_mldsa87_signature_t, 4628);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_SIGVERIFY_MLDSA87_KEY_H_
