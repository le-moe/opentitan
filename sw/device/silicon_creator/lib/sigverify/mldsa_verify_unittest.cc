// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify.h"

#include <array>
#include <cstring>

#include "gtest/gtest.h"
#include "hw/top/dt/kmac.h"
#include "hw/top/dt/otbn.h"
#include "sw/device/lib/base/mock_abs_mmio.h"
#include "sw/device/lib/base/mock_crc32.h"
#include "sw/device/silicon_creator/lib/base/mock_sec_mmio.h"
#include "sw/device/silicon_creator/lib/drivers/mock_rnd.h"
#include "sw/device/silicon_creator/lib/sigverify/mock_mldsa_verify_otbn_app.h"
#include "sw/device/silicon_creator/testing/rom_test.h"

#include "hw/top/kmac_regs.h"
#include "hw/top/otbn_regs.h"

namespace mldsa_verify_unittest {
namespace {
using ::testing::_;
using ::testing::NotNull;
using ::testing::Return;

constexpr uint32_t kIntrStateDone = 1 << OTBN_INTR_COMMON_DONE_BIT;
constexpr uint32_t kEcdsaSuccessForTest = 0x2f06b4e0;
constexpr uint32_t kFlashExecForTest = 0xa26a38f7;

class MldsaVerifyTest : public rom_test::RomTest {
 protected:
  void ExpectKmacPoll(uint32_t flag, bool err = false) {
    EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET,
                      err ? (1 << KMAC_INTR_STATE_KMAC_ERR_BIT) : 0);
    EXPECT_ABS_READ32(kmac_base_ + KMAC_STATUS_REG_OFFSET, 1 << flag);
  }

  void ExpectKmacConfigureShake256() {
    ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
    uint32_t entropy_cfg =
        (0 << KMAC_ENTROPY_CFG_SHADOWED_ENTROPY_BOOT_MODE_BIT) |
        (KMAC_ENTROPY_CFG_SHADOWED_ENTROPY_MODE_VALUE_EDN_MODE
         << KMAC_ENTROPY_CFG_SHADOWED_ENTROPY_MODE_BIT) |
        (0 << KMAC_ENTROPY_CFG_SHADOWED_IGNORE_CMD_ENTROPY_REQ_BIT) |
        (128 << KMAC_ENTROPY_CFG_SHADOWED_EDN_MODE_REFRESH_THRESHOLD_OFFSET);
    EXPECT_ABS_WRITE32_SHADOWED(
        kmac_base_ + KMAC_ENTROPY_CFG_SHADOWED_REG_OFFSET, entropy_cfg);
    uint32_t cfg =
        (KMAC_CFG_SHADOWED_KSTRENGTH_VALUE_L256
         << KMAC_CFG_SHADOWED_KSTRENGTH_OFFSET) |
        (KMAC_CFG_SHADOWED_MODE_VALUE_SHAKE << KMAC_CFG_SHADOWED_MODE_OFFSET);
    EXPECT_ABS_WRITE32_SHADOWED(kmac_base_ + KMAC_CFG_SHADOWED_REG_OFFSET, cfg);
    ExpectKmacPoll(KMAC_STATUS_ENTROPY_READY_BIT);
  }

  void ExpectKmacStartShake256() {
    ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_START << KMAC_CMD_CMD_OFFSET);
    ExpectKmacPoll(KMAC_STATUS_SHA3_ABSORB_BIT);
  }

  void ExpectKmacSqueeze(const std::array<uint32_t, 16> &expected_words) {
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_PROCESS << KMAC_CMD_CMD_OFFSET);
    ExpectKmacPoll(KMAC_STATUS_SHA3_SQUEEZE_BIT);
    for (size_t i = 0; i < expected_words.size(); ++i) {
      EXPECT_ABS_READ32(kmac_base_ + KMAC_STATE_REG_OFFSET + i * 4,
                        expected_words[i]);
      EXPECT_ABS_READ32(kmac_base_ + KMAC_STATE_REG_OFFSET +
                            KMAC_STATE_SIZE_BYTES / 2 + i * 4,
                        0);
    }
    ExpectKmacPoll(KMAC_STATUS_SHA3_SQUEEZE_BIT);
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                       KMAC_CMD_CMD_VALUE_DONE << KMAC_CMD_CMD_OFFSET);
  }

  void ExpectOtbnBusyWaitIdle() {
    EXPECT_ABS_READ32(otbn_base_ + OTBN_STATUS_REG_OFFSET, kScOtbnStatusIdle);
    EXPECT_ABS_READ32(otbn_base_ + OTBN_STATUS_REG_OFFSET, kScOtbnStatusIdle);
  }

  void ExpectOtbnCmdRun(sc_otbn_cmd_t cmd, uint32_t err_bits = 0,
                        sc_otbn_status_t status = kScOtbnStatusIdle) {
    EXPECT_ABS_WRITE32(otbn_base_ + OTBN_INTR_STATE_REG_OFFSET, kIntrStateDone);
    EXPECT_ABS_WRITE32(otbn_base_ + OTBN_CMD_REG_OFFSET, cmd);
    ExpectOtbnCmdFinish(err_bits, status);
  }

  void ExpectOtbnCmdFinish(uint32_t err_bits = 0,
                           sc_otbn_status_t status = kScOtbnStatusIdle) {
    EXPECT_ABS_READ32(otbn_base_ + OTBN_INTR_STATE_REG_OFFSET, kIntrStateDone);
    EXPECT_ABS_WRITE32(otbn_base_ + OTBN_INTR_STATE_REG_OFFSET, kIntrStateDone);
    EXPECT_ABS_READ32(otbn_base_ + OTBN_ERR_BITS_REG_OFFSET, err_bits);
    EXPECT_ABS_READ32(otbn_base_ + OTBN_STATUS_REG_OFFSET, status);
    if (err_bits == 0 && status == kScOtbnStatusIdle) {
      EXPECT_ABS_READ32(otbn_base_ + OTBN_STATUS_REG_OFFSET, status);
    }
  }

  void ExpectOtbnWritePublic(uint32_t reg_offset, sc_otbn_addr_t dest,
                             const uint32_t *src, size_t num_words,
                             uint32_t expected_checksum = 0x12345678,
                             bool mismatch = false) {
    EXPECT_ABS_WRITE32(otbn_base_ + OTBN_LOAD_CHECKSUM_REG_OFFSET, 0);
    EXPECT_CALL(crc32_, Init(NotNull()));
    for (size_t i = 0; i < num_words; ++i) {
      EXPECT_ABS_WRITE32(otbn_base_ + reg_offset + dest + i * 4, src[i]);
      EXPECT_CALL(crc32_, Add32(NotNull(), src[i]));
      EXPECT_CALL(crc32_, Add8(NotNull(), _));
      EXPECT_CALL(crc32_, Add8(NotNull(), _));
    }
    EXPECT_CALL(crc32_, Finish(NotNull())).WillOnce(Return(expected_checksum));
    EXPECT_ABS_READ32(otbn_base_ + OTBN_LOAD_CHECKSUM_REG_OFFSET,
                      mismatch ? (expected_checksum ^ 1) : expected_checksum);
  }

  void ExpectLoadMldsaVerifyApp() {
    ExpectOtbnBusyWaitIdle();
    ExpectOtbnCmdRun(kScOtbnCmdSecWipeDmem);
    ExpectOtbnCmdRun(kScOtbnCmdSecWipeImem);
    ExpectOtbnWritePublic(OTBN_IMEM_REG_OFFSET, 0,
                          kMldsaVerifyOtbnApp.imem_start,
                          static_cast<size_t>(kMldsaVerifyOtbnApp.imem_end -
                                              kMldsaVerifyOtbnApp.imem_start));
    ExpectOtbnWritePublic(
        OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnApp.dmem_data_start_addr,
        kMldsaVerifyOtbnApp.dmem_data_start,
        static_cast<size_t>(kMldsaVerifyOtbnApp.dmem_data_end -
                            kMldsaVerifyOtbnApp.dmem_data_start));
  }

  void ExpectWriteMldsaVerifyMode() {
    uint32_t mode = kMldsa87ModeVerify;
    ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnMode, &mode, 1);
  }

  void ExpectCleanupAndBootReload() {
    ExpectOtbnCmdRun(kScOtbnCmdSecWipeDmem);
    EXPECT_CALL(mldsa_otbn_app_, BootAppLoad()).WillOnce(Return(kErrorOk));
  }

  uint32_t kmac_base_ = dt_kmac_primary_reg_block(kDtKmac);
  uint32_t otbn_base_ = dt_otbn_primary_reg_block(kDtOtbn);
  rom_test::MockAbsMmio abs_mmio_;
  rom_test::MockSecMmio sec_mmio_;
  rom_test::MockCrc32 crc32_;
  rom_test::MockRnd rnd_;
  rom_test::MockMldsaVerifyOtbnApp mldsa_otbn_app_;
};

TEST_F(MldsaVerifyTest, ConstantsAndSuccessToOk) {
  EXPECT_EQ(sigverify_mldsa_success_to_ok(kSigverifyMldsaSuccess), kErrorOk);
  EXPECT_NE(sigverify_mldsa_success_to_ok(kSigverifyMldsaSuccess ^ 1),
            kErrorOk);
  EXPECT_EQ(kEcdsaSuccessForTest ^ kSigverifyMldsaSuccess, kFlashExecForTest);
}

TEST_F(MldsaVerifyTest, ComputeTrAndKeyIdSuccessAndError) {
  sigverify_mldsa87_key_t key{};
  key.data[0] = 0x11223344;
  std::array<uint32_t, 16> expected_tr{};
  expected_tr[0] = 0xdeadbeef;
  expected_tr[15] = 0xcafebabe;

  ExpectKmacConfigureShake256();
  ExpectKmacStartShake256();
  for (size_t i = 0; i < kSigverifyMldsa87PkWords; ++i) {
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, key.data[i]);
  }
  ExpectKmacSqueeze(expected_tr);

  uint32_t tr[kMldsa87TrWords] = {};
  EXPECT_EQ(mldsa_compute_tr(&key, tr), kErrorOk);
  EXPECT_EQ(tr[0], 0xdeadbeef);
  EXPECT_EQ(tr[15], 0xcafebabe);

  // Also test sigverify_mldsa87_key_id_get returns tr[0] and propagates KMAC
  // errors.
  ExpectKmacConfigureShake256();
  ExpectKmacStartShake256();
  for (size_t i = 0; i < kSigverifyMldsa87PkWords; ++i) {
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, key.data[i]);
  }
  ExpectKmacSqueeze(expected_tr);
  uint32_t key_id = 0;
  EXPECT_EQ(sigverify_mldsa87_key_id_get(&key, &key_id), kErrorOk);
  EXPECT_EQ(key_id, 0xdeadbeef);

  ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT, /*err=*/true);
  EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET,
                    1u << KMAC_INTR_STATE_KMAC_ERR_BIT);
  EXPECT_EQ(sigverify_mldsa87_key_id_get(&key, &key_id),
            kErrorKmacInvalidStatus);

  EXPECT_EQ(sigverify_mldsa87_key_id_get(nullptr, &key_id),
            kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(sigverify_mldsa87_key_id_get(&key, nullptr),
            kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(mldsa_compute_tr(nullptr, tr), kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(mldsa_compute_tr(&key, nullptr), kErrorSigverifyBadMldsaKey);
}

TEST_F(MldsaVerifyTest, MuStreamingAbsorbsEmptyContextAndPrefixes) {
  uint32_t tr[kMldsa87TrWords] = {};
  for (size_t i = 0; i < kMldsa87TrWords; ++i) {
    tr[i] = static_cast<uint32_t>(0x100 + i);
  }
  const std::array<uint32_t, 16> expected_mu = {0xa5a55a5a};
  const uint32_t p1 = 0x44332211;
  const uint32_t p2 = 0x88776655;
  const uint32_t msg = 0xccddeeff;

  ExpectKmacConfigureShake256();
  ExpectKmacStartShake256();
  for (size_t i = 0; i < kMldsa87TrWords; ++i) {
    EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, tr[i]);
  }
  // Empty context: 0x00 || 0x00.
  EXPECT_ABS_WRITE8(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, 0x00);
  EXPECT_ABS_WRITE8(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, 0x00);
  EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, p1);
  EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, p2);
  EXPECT_ABS_WRITE32(kmac_base_ + KMAC_MSG_FIFO_REG_OFFSET, msg);
  ExpectKmacSqueeze(expected_mu);

  uint32_t mu[kMldsa87MuWords] = {};
  EXPECT_EQ(mldsa_compute_mu(tr, &p1, sizeof(p1), &p2, sizeof(p2), &msg,
                             sizeof(msg), mu),
            kErrorOk);
  EXPECT_EQ(mu[0], 0xa5a55a5a);

  EXPECT_EQ(mldsa_mu_init(nullptr), kErrorSigverifyBadMldsaKey);
  EXPECT_EQ(mldsa_mu_update(nullptr, 0), kErrorOk);

  // Idle KMAC: abort check observes !SHA3_ABSORB and exits without commands.
  ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
  EXPECT_EQ(mldsa_mu_update(nullptr, 4), kErrorSigverifyBadMldsaSignature);
  ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);
  EXPECT_EQ(mldsa_mu_final(nullptr), kErrorSigverifyBadMldsaSignature);
}

TEST_F(MldsaVerifyTest, MuUpdateErrorRecoversKmacFromAbsorb) {
  EXPECT_ABS_READ32(kmac_base_ + KMAC_INTR_STATE_REG_OFFSET, 0);
  EXPECT_ABS_READ32(kmac_base_ + KMAC_STATUS_REG_OFFSET,
                    1u << KMAC_STATUS_SHA3_ABSORB_BIT);
  EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                     KMAC_CMD_CMD_VALUE_PROCESS << KMAC_CMD_CMD_OFFSET);
  ExpectKmacPoll(KMAC_STATUS_SHA3_SQUEEZE_BIT);
  EXPECT_ABS_WRITE32(kmac_base_ + KMAC_CMD_REG_OFFSET,
                     KMAC_CMD_CMD_VALUE_DONE << KMAC_CMD_CMD_OFFSET);
  ExpectKmacPoll(KMAC_STATUS_SHA3_IDLE_BIT);

  EXPECT_EQ(mldsa_mu_update(nullptr, 4), kErrorSigverifyBadMldsaSignature);
}

TEST_F(MldsaVerifyTest, VerifyStartRejectsNonZeroPaddingWithoutLoadingOtbn) {
  sigverify_mldsa87_key_t key{};
  sigverify_mldsa87_signature_t sig{};
  uint32_t mu[kMldsa87MuWords] = {};
  uint32_t flash_exec = 0;

  reinterpret_cast<uint8_t *>(sig.data)[kSigverifyMldsa87SigBytes] = 0x01;
  EXPECT_EQ(mldsa_verify(&key, &sig, mu, &flash_exec),
            kErrorSigverifyBadMldsaSignature);
  EXPECT_EQ(flash_exec, UINT32_MAX);
}

TEST_F(MldsaVerifyTest, VerifyStartWriteFailureCleansUpAndReloadsBootApp) {
  sigverify_mldsa87_key_t key{};
  sigverify_mldsa87_signature_t sig{};
  uint32_t mu[kMldsa87MuWords] = {};

  ExpectLoadMldsaVerifyApp();
  ExpectWriteMldsaVerifyMode();
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnPk, key.data,
                        kSigverifyMldsa87PublicKeyWords);
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnSig, sig.data,
                        kSigverifyMldsa87SignatureWords);
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnMu, mu,
                        kMldsa87MuWords, /*expected_checksum=*/0xdeadbeef,
                        /*mismatch=*/true);
  ExpectCleanupAndBootReload();

  EXPECT_EQ(mldsa_verify_start(&key, &sig, mu), kErrorOtbnBadChecksum);
}

TEST_F(MldsaVerifyTest, VerifySuccess) {
  sigverify_mldsa87_key_t key{};
  sigverify_mldsa87_signature_t sig{};
  uint32_t mu[kMldsa87MuWords] = {};
  for (size_t i = 0; i < kMldsa87CTildeWords; ++i) {
    sig.data[i] = static_cast<uint32_t>(0xa000 + i);
  }

  ExpectLoadMldsaVerifyApp();
  ExpectWriteMldsaVerifyMode();
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnPk, key.data,
                        kSigverifyMldsa87PublicKeyWords);
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnSig, sig.data,
                        kSigverifyMldsa87SignatureWords);
  ExpectOtbnWritePublic(OTBN_DMEM_REG_OFFSET, kMldsaVerifyOtbnMu, mu,
                        kMldsa87MuWords);
  ExpectOtbnBusyWaitIdle();
  EXPECT_SEC_WRITE32(otbn_base_ + OTBN_CTRL_REG_OFFSET,
                     1 << OTBN_CTRL_SOFTWARE_ERRS_FATAL_BIT);
  EXPECT_ABS_WRITE32(otbn_base_ + OTBN_INTR_STATE_REG_OFFSET, kIntrStateDone);
  EXPECT_ABS_WRITE32(otbn_base_ + OTBN_CMD_REG_OFFSET, kScOtbnCmdExecute);

  ExpectOtbnCmdFinish();
  EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET + kMldsaVerifyOtbnResOk,
                    kMldsa87StatusOk);
  for (size_t i = 0; i < kMldsa87CTildeWords; ++i) {
    EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET +
                          kMldsaVerifyOtbnResCTildePrime + i * 4,
                      sig.data[i]);
  }
  ExpectCleanupAndBootReload();

  uint32_t flash_exec = kEcdsaSuccessForTest;
  EXPECT_EQ(mldsa_verify(&key, &sig, mu, &flash_exec), kErrorOk);
  EXPECT_EQ(flash_exec, kFlashExecForTest);
}

TEST_F(MldsaVerifyTest, VerifyFinishBadCTildeAndBadOkReloadBootApp) {
  sigverify_mldsa87_signature_t sig{};
  for (size_t i = 0; i < kMldsa87CTildeWords; ++i) {
    sig.data[i] = static_cast<uint32_t>(0xb000 + i);
  }

  // Case 1: c_tilde_prime mismatch even when ok == kMldsa87StatusOk.
  ExpectOtbnCmdFinish();
  EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET + kMldsaVerifyOtbnResOk,
                    kMldsa87StatusOk);
  for (size_t i = 0; i < kMldsa87CTildeWords; ++i) {
    EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET +
                          kMldsaVerifyOtbnResCTildePrime + i * 4,
                      i == 5 ? (sig.data[i] ^ 1) : sig.data[i]);
  }
  ExpectCleanupAndBootReload();

  uint32_t flash_exec = 0;
  EXPECT_EQ(mldsa_verify_finish(&sig, &flash_exec),
            kErrorSigverifyBadMldsaSignature);
  EXPECT_EQ(flash_exec, UINT32_MAX);

  // Case 2: status ok != kMldsa87StatusOk even when c_tilde_prime matches.
  ExpectOtbnCmdFinish();
  EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET + kMldsaVerifyOtbnResOk,
                    kMldsa87StatusFail);
  for (size_t i = 0; i < kMldsa87CTildeWords; ++i) {
    EXPECT_ABS_READ32(otbn_base_ + OTBN_DMEM_REG_OFFSET +
                          kMldsaVerifyOtbnResCTildePrime + i * 4,
                      sig.data[i]);
  }
  ExpectCleanupAndBootReload();

  flash_exec = kEcdsaSuccessForTest;
  EXPECT_EQ(mldsa_verify_finish(&sig, &flash_exec),
            kErrorSigverifyBadMldsaSignature);
  EXPECT_EQ(flash_exec, kEcdsaSuccessForTest ^ UINT32_MAX);
}

TEST_F(MldsaVerifyTest, VerifyFinishOtbnExecErrorStillCleansUpAndFailsClosed) {
  sigverify_mldsa87_signature_t sig{};

  ExpectOtbnCmdFinish(1 << OTBN_ERR_BITS_FATAL_SOFTWARE_BIT,
                      kScOtbnStatusLocked);
  ExpectCleanupAndBootReload();

  uint32_t flash_exec = 0;
  EXPECT_EQ(mldsa_verify_finish(&sig, &flash_exec), kErrorOtbnExecutionFailed);
  EXPECT_EQ(flash_exec, UINT32_MAX);
}

}  // namespace
}  // namespace mldsa_verify_unittest
