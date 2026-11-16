// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "sw/device/silicon_creator/lib/sigverify/mldsa_verify_otbn_app.h"

#include "sw/device/silicon_creator/lib/drivers/otbn.h"
#include "sw/device/silicon_creator/lib/otbn_boot_services.h"

OTBN_DECLARE_APP_SYMBOLS(run_mldsa87);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_mode);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_pk_rho);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_sig_c_tilde);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_mu);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_verify_res_ok);
OTBN_DECLARE_SYMBOL_ADDR(run_mldsa87, mldsa87_verify_res_c_tilde_prime);

const sc_otbn_app_t kMldsaVerifyOtbnApp = OTBN_APP_T_INIT(run_mldsa87);
const sc_otbn_addr_t kMldsaVerifyOtbnMode =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_mode);
const sc_otbn_addr_t kMldsaVerifyOtbnPk =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_pk_rho);
const sc_otbn_addr_t kMldsaVerifyOtbnSig =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_sig_c_tilde);
const sc_otbn_addr_t kMldsaVerifyOtbnMu =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_mu);
const sc_otbn_addr_t kMldsaVerifyOtbnResOk =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_verify_res_ok);
const sc_otbn_addr_t kMldsaVerifyOtbnResCTildePrime =
    OTBN_ADDR_T_INIT(run_mldsa87, mldsa87_verify_res_c_tilde_prime);

rom_error_t mldsa_verify_otbn_boot_app_load(void) {
  return otbn_boot_app_load();
}
