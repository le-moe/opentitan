# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("@opentitan_signing_infra//signing:defs.bzl", "keyset")

package(default_visibility = ["//visibility:public"])

exports_files(glob(["*.pem"]))

keyset(
    name = "mldsa_keyset",
    build_setting_default = "",
    keys = {
        "test_key_0_mldsa87.pem": "test_key_0",
        "dev_key_0_mldsa87.pem": "dev_key_0",
        "prod_key_0_mldsa87.pem": "prod_key_0",
        "prod_key_1_mldsa87.pem": "prod_key_1",
    },
    profile = "local",
    tool = "//signing/tokens:local",
)
