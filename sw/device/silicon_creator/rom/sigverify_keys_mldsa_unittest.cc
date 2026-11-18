// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/rom/sigverify_keys_mldsa.h"

#include <array>
#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "hw/top/dt/kmac.h"
#include "sw/device/lib/base/mock_abs_mmio.h"
#include "sw/device/silicon_creator/lib/drivers/lifecycle.h"
#include "sw/device/silicon_creator/lib/sigverify/mldsa87_key.h"
#include "sw/device/silicon_creator/rom/mock_sigverify_otp_keys.h"
#include "sw/device/silicon_creator/rom/sigverify_key_types.h"
#include "sw/device/silicon_creator/rom/sigverify_otp_keys.h"
#include "sw/device/silicon_creator/testing/rom_test.h"

#include "hw/top/kmac_regs.h"

namespace sigverify_keys_mldsa_unittest {
namespace {
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::Field;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgPointee;

class SigverifyKeysMldsaTest : public rom_test::RomTest {
 protected:
  void ExpectKmacPoll(uint32_t flag) {
    EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET, 0);
    EXPECT_ABS_READ32(kmac_base_ + KMAC_STATUS_REG_OFFSET, 1u << flag);
  }

  void ExpectComputeTr(const sigverify_mldsa87_key_t &key,
                       const std::array<uint32_t, kMldsa87TrWords> &tr) {
    InSequence seq;
    // kmac_shake256_configure()
    ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
    EXPECT_CALL(
        abs_mmio_,
        Write32Shadowed(kmac_base_ + KMAC_ENTROPY_CFG_SHADOWED_REG_OFFSET, _));
    EXPECT_CALL(abs_mmio_,
                Write32Shadowed(kmac_base_ + KMAC_CFG_SHADOWED_REG_OFFSET, _));
    ExpectKmacPoll(KMAC_STATUS_ENTROPY_READY_BIT);
    // kmac_shake256_start()
    ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_START << KMAC_CMD_CMD_OFFSET);
    ExpectKmacPoll(KMAC_STATUS_SHA3_ABSORB_BIT);
    // kmac_shake256_absorb_words(pk)
    for (size_t i = 0; i < kSigverifyMldsa87PkWords; ++i) {
      EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, key.data[i]);
    }
    // kmac_shake256_squeeze_start()
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_PROCESS << KMAC_CMD_CMD_OFFSET);
    // kmac_shake256_squeeze_end(tr)
    ExpectKmacPoll(KMAC_STATUS_SHA3_SQUEEZE_BIT);
    for (size_t i = 0; i < kMldsa87TrWords; ++i) {
      EXPECT_ABS_READ32(kmac_base_ + KMAC_STATE_REG_OFFSET + i * 4, tr[i]);
      EXPECT_ABS_READ32(kmac_base_ + KMAC_STATE_REG_OFFSET +
                            KMAC_STATE_SIZE_BYTES / 2 + i * 4,
                        0);
    }
    ExpectKmacPoll(KMAC_STATUS_SHA3_SQUEEZE_BIT);
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_DONE << KMAC_CMD_CMD_OFFSET);
  }

  uint32_t kmac_base_ = dt_kmac_primary_reg_block(kDtKmac);
  rom_test::MockAbsMmio abs_mmio_;
  rom_test::MockSigverifyOtpKeys otp_keys_;
};

TEST_F(SigverifyKeysMldsaTest, Success) {
  sigverify_otp_key_ctx_t ctx{};
  sigverify_mldsa87_key_t manifest_key{};
  std::memset(&manifest_key, 0x5a, sizeof(manifest_key));

  std::array<uint32_t, kMldsa87TrWords> expected_tr{};
  for (size_t i = 0; i < kMldsa87TrWords; ++i) {
    expected_tr[i] = 0x12340000u + static_cast<uint32_t>(i);
  }

  sigverify_rom_pq_key_t otp_key{};
  otp_key.mldsa.key_type = kSigverifyKeyTypeProd;
  std::memcpy(otp_key.mldsa.mldsa_tr, expected_tr.data(),
              sizeof(otp_key.mldsa.mldsa_tr));

  ExpectComputeTr(manifest_key, expected_tr);
  EXPECT_CALL(
      otp_keys_,
      OtpKeysGet(
          AllOf(Field(&sigverify_otp_keys_get_params_t::key_id, expected_tr[0]),
                Field(&sigverify_otp_keys_get_params_t::extra_digest_offset,
                      offsetof(sigverify_rom_pq_key_t, mldsa.mldsa_tr)),
                Field(&sigverify_otp_keys_get_params_t::extra_digest_words,
                      kSigverifyMldsa87KeyDigestWords)),
          _))
      .WillOnce(DoAll(SetArgPointee<1>(&otp_key.key_header), Return(kErrorOk)));

  uint32_t tr[kMldsa87TrWords] = {};
  const sigverify_rom_pq_key_t *res_key = nullptr;
  EXPECT_EQ(
      sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, tr, &res_key),
      kErrorOk);
  EXPECT_EQ(res_key, &otp_key);
  EXPECT_THAT(tr, testing::ElementsAreArray(expected_tr));
}

TEST_F(SigverifyKeysMldsaTest, NullArgumentsRejected) {
  sigverify_otp_key_ctx_t ctx{};
  sigverify_mldsa87_key_t manifest_key{};
  uint32_t tr[kMldsa87TrWords] = {};
  const sigverify_rom_pq_key_t *res_key = nullptr;

  EXPECT_EQ(sigverify_mldsa_key_get(nullptr, &manifest_key, kLcStateProd, tr,
                                    &res_key),
            kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(sigverify_mldsa_key_get(&ctx, nullptr, kLcStateProd, tr, &res_key),
            kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, nullptr,
                                    &res_key),
            kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(
      sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, tr, nullptr),
      kErrorSigverifyBadMldsaKey);
}

TEST_F(SigverifyKeysMldsaTest, ComputeTrFailed) {
  sigverify_otp_key_ctx_t ctx{};
  sigverify_mldsa87_key_t manifest_key{};
  std::memset(&manifest_key, 0x5a, sizeof(manifest_key));

  // Trigger a KMAC error while polling IDLE in kmac_shake256_configure().
  EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET,
                    1u << KMAC_INTR_STATE_KMAC_ERR_BIT);
  EXPECT_ABS_READ32(kmac_base_ + KMAC_STATUS_REG_OFFSET,
                    1u << KMAC_STATUS_SHA3_IDLE_BIT);
  // Error recovery checks KMAC_INTR_STATE_KMAC_ERR_BIT before aborting.
  EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET,
                    1u << KMAC_INTR_STATE_KMAC_ERR_BIT);

  uint32_t tr[kMldsa87TrWords] = {};
  const sigverify_rom_pq_key_t *res_key = nullptr;
  EXPECT_EQ(
      sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, tr, &res_key),
      kErrorKmacInvalidStatus);
  EXPECT_EQ(res_key, nullptr);
}

TEST_F(SigverifyKeysMldsaTest, OtpLookupFailed) {
  sigverify_otp_key_ctx_t ctx{};
  sigverify_mldsa87_key_t manifest_key{};
  std::memset(&manifest_key, 0xa5, sizeof(manifest_key));

  std::array<uint32_t, kMldsa87TrWords> expected_tr{};
  expected_tr[0] = 0x73eaee41;

  ExpectComputeTr(manifest_key, expected_tr);
  EXPECT_CALL(
      otp_keys_,
      OtpKeysGet(
          Field(&sigverify_otp_keys_get_params_t::key_id, expected_tr[0]), _))
      .WillOnce(Return(kErrorSigverifyBadKey));

  uint32_t tr[kMldsa87TrWords] = {};
  const sigverify_rom_pq_key_t *res_key = nullptr;
  EXPECT_EQ(
      sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, tr, &res_key),
      kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(res_key, nullptr);
}

TEST_F(SigverifyKeysMldsaTest, DigestMismatchWithMatchingKeyId) {
  sigverify_otp_key_ctx_t ctx{};
  sigverify_mldsa87_key_t manifest_key{};
  std::memset(&manifest_key, 0x5a, sizeof(manifest_key));

  std::array<uint32_t, kMldsa87TrWords> computed_tr{};
  for (size_t i = 0; i < kMldsa87TrWords; ++i) {
    computed_tr[i] = 0x12340000u + static_cast<uint32_t>(i);
  }

  sigverify_rom_pq_key_t otp_key{};
  otp_key.mldsa.key_type = kSigverifyKeyTypeProd;
  std::memcpy(otp_key.mldsa.mldsa_tr, computed_tr.data(),
              sizeof(otp_key.mldsa.mldsa_tr));
  // Keep key_id = mldsa_tr[0] matching, but flip a bit in word 11.
  otp_key.mldsa.mldsa_tr[kSigverifyMldsa87KeyDigestWords - 1] ^= 1u;

  ExpectComputeTr(manifest_key, computed_tr);
  EXPECT_CALL(
      otp_keys_,
      OtpKeysGet(
          Field(&sigverify_otp_keys_get_params_t::key_id, computed_tr[0]), _))
      .WillOnce(DoAll(SetArgPointee<1>(&otp_key.key_header), Return(kErrorOk)));

  uint32_t tr[kMldsa87TrWords] = {};
  const sigverify_rom_pq_key_t *res_key = nullptr;
  EXPECT_EQ(
      sigverify_mldsa_key_get(&ctx, &manifest_key, kLcStateProd, tr, &res_key),
      kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(res_key, nullptr);
}

}  // namespace
}  // namespace sigverify_keys_mldsa_unittest
