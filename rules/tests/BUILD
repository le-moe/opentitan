# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load(
    "@opentitan_signing_infra//signing:defs.bzl",
    "offline_fake_ecdsa_sign",
    "offline_fake_mldsa_sign",
    "offline_signature_attach",
    "signature_test",
)
load("//hw/top:defs.bzl", "opentitan_require_top")
load("//rules:const.bzl", "CONST")
load("//rules:signing.bzl", "offline_presigning_artifacts")
load(
    "//rules/opentitan:defs.bzl",
    "ecdsa_key_for_lc_state",
    "mldsa_key_for_lc_state",
    "opentitan_binary",
)
load("//rules/opentitan:keyutils.bzl", "ECDSA_MLDSA_KEY_STRUCTS")
load("//rules/tests:const_unittest.bzl", "const_test_suite")
load("//rules/tests:otp_unittest.bzl", "otp_test_suite")

const_test_suite()

otp_test_suite()

opentitan_binary(
    name = "mldsa_signed_test_bin",
    testonly = True,
    srcs = ["//sw/device/examples/hello_world:hello_world.c"],
    copts = ["-Wno-date-time"],
    ecdsa_key = ecdsa_key_for_lc_state(
        ECDSA_MLDSA_KEY_STRUCTS,
        CONST.LCV.PROD,
    ),
    exec_env = ["//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys"],
    mldsa_key = mldsa_key_for_lc_state(
        ECDSA_MLDSA_KEY_STRUCTS,
        CONST.LCV.PROD,
    ),
    deps = ["//sw/device/examples/hello_world:hello_world_lib"],
)

filegroup(
    name = "signed_mldsa_binaries",
    testonly = True,
    srcs = [":mldsa_signed_test_bin"],
    output_group = "fpga_cw340_rom_with_fake_keys_signed_bin",
    target_compatible_with = opentitan_require_top("earlgrey"),
)

signature_test(
    name = "mldsa_signature_test",
    srcs = [":signed_mldsa_binaries"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "prod_key_0"},
    target_compatible_with = opentitan_require_top("earlgrey"),
)

signature_test(
    name = "wrong_mldsa_key_test",
    srcs = [":signed_mldsa_binaries"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "dev_key_0"},
    negative_test = True,
    target_compatible_with = opentitan_require_top("earlgrey"),
)

offline_presigning_artifacts(
    name = "offline_mldsa_presigning",
    testonly = True,
    srcs = ["//sw/device/examples/hello_world"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "test_key_0"},
    manifest = "//sw/device/silicon_creator/rom_ext:manifest",
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "test_key_0"},
    target_compatible_with = opentitan_require_top("earlgrey"),
)

offline_fake_ecdsa_sign(
    name = "offline_mldsa_fake_ecdsa",
    testonly = True,
    srcs = [":offline_mldsa_presigning"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "test_key_0"},
    target_compatible_with = opentitan_require_top("earlgrey"),
)

offline_fake_mldsa_sign(
    name = "offline_mldsa_fake_mldsa",
    testonly = True,
    srcs = [":offline_mldsa_presigning"],
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "test_key_0"},
    target_compatible_with = opentitan_require_top("earlgrey"),
)

offline_signature_attach(
    name = "offline_mldsa_signed",
    testonly = True,
    srcs = [":offline_mldsa_presigning"],
    ecdsa_signatures = [":offline_mldsa_fake_ecdsa"],
    mldsa_signatures = [":offline_mldsa_fake_mldsa"],
    target_compatible_with = opentitan_require_top("earlgrey"),
)

signature_test(
    name = "offline_mldsa_signature_test",
    srcs = [":offline_mldsa_signed"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "test_key_0"},
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "test_key_0"},
    target_compatible_with = opentitan_require_top("earlgrey"),
)

# Test that external signing rules remain working with the latest OpenTitanTool

signature_test(
    name = "corrupt_signature_test",
    srcs = [
        "//sw/device/silicon_creator/rom/e2e:empty_test_slot_a_corrupted_fpga_cw340_rom_with_fake_keys.prod_key_0.signed.bin",
    ],
    negative_test = True,
)

signature_test(
    name = "wrong_ecdsa_key_test",
    srcs = ["//sw/device/silicon_creator/rom_ext:signed_spx_binaries"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "dev_key_0"},
    negative_test = True,
)

signature_test(
    name = "wrong_spx_key_test",
    srcs = ["//sw/device/silicon_creator/rom_ext:signed_spx_binaries"],
    negative_test = True,
    spx_key = {"//sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset": "dev_key_0"},
)

signature_test(
    name = "wrong_domain_signature_test",
    srcs = [
        "//sw/device/silicon_creator/rom_ext:signed_spx_binaries",
    ],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    negative_test = True,
    spx_domain = "PrehashedSha256",  # "Pure", "PrehashedSha256"
    spx_key = {"//sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset": "prod_key_0"},
)
