// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_TEST_MLDSA87_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_TEST_MLDSA87_H_

// First 48 bytes (12 LE words) of tr = SHAKE256(pk, 64) for app_test_mldsa87.
#define APP_TEST_MLDSA87_TR                                               \
  {.digest = {0x08f4cfe0, 0x665adec7, 0x4ffefd88, 0xf17ae99d, 0xc3986af2, \
              0x2a179053, 0x8eec5965, 0x7b294bc5, 0x36304edd, 0x96717b8c, \
              0x611b51d2, 0x8b20246b}}

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_TEST_MLDSA87_H_
