// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_DEV_MLDSA87_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_DEV_MLDSA87_H_

// First 48 bytes (12 LE words) of tr = SHAKE256(pk, 64) for app_dev_mldsa87.
#define APP_DEV_MLDSA87_TR                                                \
  {.digest = {0x82fc063b, 0x217f97ea, 0xc29ec0f9, 0xe378a0ec, 0x8a06e821, \
              0x3218196c, 0x8ccb92aa, 0x79298159, 0xcdc9ce0b, 0x97381c56, \
              0x8304f5fb, 0x21cef748}}

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_DEV_MLDSA87_H_
