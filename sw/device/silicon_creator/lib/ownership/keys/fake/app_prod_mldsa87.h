// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_PROD_MLDSA87_H_
#define OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_PROD_MLDSA87_H_

// First 48 bytes (12 LE words) of tr = SHAKE256(pk, 64) for app_prod_mldsa87.
#define APP_PROD_MLDSA87_TR                                               \
  {.digest = {0x112a6e34, 0x68b3aa5b, 0xe35959c9, 0x2cabc450, 0xed34168e, \
              0x4026253c, 0xf185f691, 0x9ca0bdd6, 0x4f5c5361, 0x26672e70, \
              0x7a924951, 0xcc56345c}}

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_OWNERSHIP_KEYS_FAKE_APP_PROD_MLDSA87_H_
