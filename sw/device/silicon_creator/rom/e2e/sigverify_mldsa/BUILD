# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("//hw/top_earlgrey/data/otp:defs.bzl", "EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS")
load(
    "//rules:const.bzl",
    "CONST",
    "get_lc_items",
    "hex_digits",
)
load(
    "//rules:otp.bzl",
    "STD_OTP_OVERLAYS",
    "otp_hex",
    "otp_image",
    "otp_json",
    "otp_partition",
)
load("//rules:rom_e2e.bzl", "maybe_skip_in_ci")
load(
    "//rules/opentitan:defs.bzl",
    "DEFAULT_TEST_FAILURE_MSG",
    "fpga_params",
    "opentitan_binary",
    "opentitan_test",
    "verilator_params",
)
load(
    "//rules/opentitan:keyutils.bzl",
    "ECDSA_MLDSA_KEY_STRUCTS",
    "filter_key_structs_for_lc_state",
)
load(
    "//sw/device/silicon_creator/rom/e2e:defs.bzl",
    "MSG_PASS",
    "MSG_TEMPLATE_BFV_LCV",
)

package(default_visibility = ["//visibility:public"])

MLDSA_OPERATIONAL_LC_STATES = get_lc_items(
    CONST.LCV.DEV,
    CONST.LCV.PROD,
    CONST.LCV.PROD_END,
    CONST.LCV.RMA,
)

# OTP overlays for edge cases not covered by //hw/top_earlgrey/data/otp:img_*_mldsa.
otp_json(
    name = "otp_json_mldsa_en",
    partitions = [
        otp_partition(
            name = "CREATOR_SW_CFG",
            items = {
                "CREATOR_SW_CFG_SIGVERIFY_SPX_EN": otp_hex(CONST.PQ_MLDSA_ENABLED),
            },
        ),
    ],
)

otp_json(
    name = "otp_json_mldsa_key2_revoked",
    partitions = [
        otp_partition(
            name = "ROT_CREATOR_AUTH_STATE",
            items = {
                # Slot 2 holds prod_key_0_mldsa87 in json_rot_keys_mldsa.
                "ROT_CREATOR_AUTH_STATE_SPX_KEY2": otp_hex(CONST.SIGVERIFY.KEY_STATE.REVOKED),
            },
        ),
    ],
)

otp_json(
    name = "otp_json_unknown_pq_policy",
    partitions = [
        otp_partition(
            name = "CREATOR_SW_CFG",
            items = {
                "CREATOR_SW_CFG_SIGVERIFY_SPX_EN": otp_hex(0xdeadbeef),
            },
        ),
    ],
)

otp_image(
    name = "otp_img_test_unlocked0_mldsa",
    src = "//hw/top_earlgrey/data/otp:otp_json_test_unlocked0",
    overlays = STD_OTP_OVERLAYS + EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS + [
        ":otp_json_mldsa_en",
    ],
)

otp_image(
    name = "otp_img_prod_mldsa_revoked",
    src = "//hw/top_earlgrey/data/otp:otp_json_prod",
    overlays = STD_OTP_OVERLAYS + EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS + [
        ":otp_json_mldsa_en",
        ":otp_json_mldsa_key2_revoked",
    ],
)

otp_image(
    name = "otp_img_prod_unknown_pq_policy",
    src = "//hw/top_earlgrey/data/otp:otp_json_prod",
    overlays = STD_OTP_OVERLAYS + EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS + [
        ":otp_json_unknown_pq_policy",
    ],
)

# Signed hybrid binary used to construct bit-level negative test artifacts.
opentitan_binary(
    name = "hybrid_prod_bin",
    testonly = True,
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = ["//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys"],
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "prod_key_0"},
    deps = [
        "//hw/top:otp_ctrl_c_regs",
        "//hw/top/dt",
        "//sw/device/lib/runtime:log",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib/drivers:lifecycle",
        "//sw/device/silicon_creator/lib/drivers:otp",
        "//sw/device/silicon_creator/lib/sigverify:spx_verify",
    ],
)

filegroup(
    name = "hybrid_prod_signed_bin",
    testonly = True,
    srcs = [":hybrid_prod_bin"],
    output_group = "fpga_cw340_rom_with_fake_keys_signed_bin",
)

# Corrupt bit 0 or trailing pad byte 4627 of the ML-DSA signature while preserving ECDSA.
[
    genrule(
        name = "hybrid_prod_{}_bin".format(name),
        testonly = True,
        srcs = [":hybrid_prod_signed_bin"],
        outs = ["hybrid_prod_{}.bin".format(name)],
        cmd = "$(location //sw/device/silicon_creator/lib/sigverify:mldsa_manifest_corrupt) --mode={} $< $@".format(mode),
        tools = ["//sw/device/silicon_creator/lib/sigverify:mldsa_manifest_corrupt"],
    )
    for name, mode in [
        ("bad_sig", "bad-sig"),
        ("bad_padding", "bad-padding"),
    ]
]

_TEST_DEPS = [
    "//hw/top:otp_ctrl_c_regs",
    "//hw/top/dt",
    "//sw/device/lib/runtime:log",
    "//sw/device/lib/testing/test_framework:ottf_main",
    "//sw/device/silicon_creator/lib/drivers:lifecycle",
    "//sw/device/silicon_creator/lib/drivers:otp",
    "//sw/device/silicon_creator/lib/sigverify:spx_verify",
]

# Positive hybrid boot tests across Dev / Prod / ProdEnd / RMA for each valid key.
[
    opentitan_test(
        name = "sigverify_mldsa_{}_{}".format(
            lc_state,
            key.mldsa.name,
        ),
        srcs = ["sigverify_mldsa_test.c"],
        ecdsa_key = {key.ecdsa.label: key.ecdsa.name},
        exec_env = {
            "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
        },
        fpga = fpga_params(
            exit_failure = DEFAULT_TEST_FAILURE_MSG,
            exit_success = MSG_PASS,
            otp = "//hw/top_earlgrey/data/otp:img_{}_mldsa".format(lc_state),
            tags = maybe_skip_in_ci(lc_state_val),
        ),
        mldsa_key = {key.mldsa.label: key.mldsa.name},
        deps = _TEST_DEPS,
    )
    for lc_state, lc_state_val in MLDSA_OPERATIONAL_LC_STATES
    for key in filter_key_structs_for_lc_state(ECDSA_MLDSA_KEY_STRUCTS, lc_state_val)
]

# TEST_UNLOCKED0 forces PQ disabled regardless of OTP, accepting both classical and hybrid images.
opentitan_test(
    name = "sigverify_mldsa_test_unlocked0_classical",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "test_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = DEFAULT_TEST_FAILURE_MSG,
        exit_success = MSG_PASS,
        otp = ":otp_img_test_unlocked0_mldsa",
    ),
    deps = _TEST_DEPS,
)

opentitan_test(
    name = "sigverify_mldsa_test_unlocked0_hybrid",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "test_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = DEFAULT_TEST_FAILURE_MSG,
        exit_success = MSG_PASS,
        otp = ":otp_img_test_unlocked0_mldsa",
    ),
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "test_key_0"},
    deps = _TEST_DEPS,
)

# Negative ROM verification tests under MldsaEnabled (Prod).
opentitan_test(
    name = "sigverify_mldsa_bad_sig",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_prod_bad_sig_bin": "firmware"},
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_SIGNATURE),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = "//hw/top_earlgrey/data/otp:img_prod_mldsa",
    ),
    manifest = None,
)

opentitan_test(
    name = "sigverify_mldsa_bad_padding",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_prod_bad_padding_bin": "firmware"},
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_SIGNATURE),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = "//hw/top_earlgrey/data/otp:img_prod_mldsa",
    ),
    manifest = None,
)

opentitan_test(
    name = "sigverify_mldsa_missing_ext",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.MANIFEST.BAD_EXTENSION),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = "//hw/top_earlgrey/data/otp:img_prod_mldsa",
    ),
    deps = _TEST_DEPS,
)

opentitan_test(
    name = "sigverify_mldsa_unauthorized_key",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_KEY),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = "//hw/top_earlgrey/data/otp:img_prod_mldsa",
    ),
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/unauthorized/mldsa:mldsa_keyset": "unauthorized_key_0"},
    deps = _TEST_DEPS,
)

opentitan_test(
    name = "sigverify_mldsa_revoked_key",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_KEY),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = ":otp_img_prod_mldsa_revoked",
    ),
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "prod_key_0"},
    deps = _TEST_DEPS,
)

opentitan_test(
    name = "sigverify_mldsa_unknown_otp_trap",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_with_fake_keys": None,
    },
    fpga = fpga_params(
        exit_failure = MSG_PASS,
        exit_success = MSG_TEMPLATE_BFV_LCV.format(
            hex_digits(CONST.BFV.INTERRUPT.ILLEGAL_INSTRUCTION),
            hex_digits(CONST.LCV.PROD),
        ),
        otp = ":otp_img_prod_unknown_pq_policy",
    ),
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "prod_key_0"},
    deps = _TEST_DEPS,
)

# ROM verification Verilator smoke test under MldsaEnabled (Prod).
opentitan_test(
    name = "sigverify_mldsa_prod_verilator_smoke",
    srcs = ["sigverify_mldsa_test.c"],
    ecdsa_key = {"//sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset": "prod_key_0"},
    exec_env = {
        "//hw/top_earlgrey:sim_verilator_rom_with_fake_keys": None,
    },
    mldsa_key = {"//sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset": "prod_key_0"},
    verilator = verilator_params(
        timeout = "eternal",
        otp = "//hw/top_earlgrey/data/otp:img_prod_mldsa",
        tags = ["manual"],
    ),
    deps = _TEST_DEPS,
)

test_suite(
    name = "rom_e2e_sigverify_mldsa",
    tags = ["manual"],
    tests = [
        "sigverify_mldsa_{}_{}".format(
            lc_state,
            key.mldsa.name,
        )
        for lc_state, lc_state_val in MLDSA_OPERATIONAL_LC_STATES
        for key in filter_key_structs_for_lc_state(ECDSA_MLDSA_KEY_STRUCTS, lc_state_val)
    ] + [
        "sigverify_mldsa_bad_padding",
        "sigverify_mldsa_bad_sig",
        "sigverify_mldsa_missing_ext",
        "sigverify_mldsa_revoked_key",
        "sigverify_mldsa_test_unlocked0_classical",
        "sigverify_mldsa_test_unlocked0_hybrid",
        "sigverify_mldsa_unauthorized_key",
        "sigverify_mldsa_unknown_otp_trap",
    ],
)
