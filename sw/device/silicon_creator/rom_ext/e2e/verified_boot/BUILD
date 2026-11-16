# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("//hw/top_earlgrey/data/otp:defs.bzl", "EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS")
load("//rules:const.bzl", "CONST", "hex", "hex_digits")
load(
    "//rules:manifest.bzl",
    "integrator_specific_firmware_binding",
    "manifest",
    "product_expr",
)
load(
    "//rules:otp.bzl",
    "STD_OTP_OVERLAYS",
    "otp_hex",
    "otp_image",
    "otp_json_immutable_rom_ext",
    "otp_partition",
)
load(
    "//sw/device/silicon_creator/rom_ext/imm_section:defs.bzl",
    "IMMUTABLE_HASH_UNENFORCED_MSG",
    "IMM_SECTION_MAJOR_VERSION",
    "IMM_SECTION_MINOR_VERSION",
)
load(
    "//rules/opentitan:defs.bzl",
    "DEFAULT_TEST_FAILURE_MSG",
    "DEFAULT_TEST_SUCCESS_MSG",
    "fpga_params",
    "opentitan_binary",
    "opentitan_test",
)

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "boot_test",
    srcs = ["boot_test.c"],
)

otp_json_immutable_rom_ext(
    name = "otp_json_with_imm_romext_enabled",
    testonly = True,
    partitions = [
        otp_partition(
            name = "CREATOR_SW_CFG",
            items = {
                "CREATOR_SW_CFG_IMMUTABLE_ROM_EXT_EN": otp_hex(CONST.HARDENED_TRUE),
            },
        ),
    ],
    rom_ext = "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_virtual",
    visibility = ["//visibility:private"],
)

otp_image(
    name = "otp_img_with_imm_romext_enabled",
    testonly = True,
    src = "//hw/top_earlgrey/data/otp:otp_json_prod",
    overlays = STD_OTP_OVERLAYS + [
        "//sw/device/silicon_creator/rom_ext/e2e:otp_json_secret2_locked",
        ":otp_json_with_imm_romext_enabled",
    ],
    visibility = ["//visibility:private"],
)

_POSITIONS = {
    "owner_slot_a": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "bl0_slot = AA__\r\n",
        "otp_img": None,
    },
    "owner_slot_b": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_b",
        "owner_offset": "{owner_slot_b}",
        "success": "bl0_slot = __BB\r\n",
        "otp_img": None,
    },
    "owner_virtual_a": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_virtual",
        "owner_offset": "{owner_slot_a}",
        "success": "bl0_slot = AA__\r\n",
        "otp_img": None,
    },
    "owner_virtual_b": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_virtual",
        "owner_offset": "{owner_slot_b}",
        "success": "bl0_slot = __BB\r\n",
        "otp_img": None,
    },
    "romext_slot_a": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "rom_ext_slot = AA__\r\n",
        "otp_img": None,
    },
    "romext_slot_b": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_b",
        "romext_offset": "{rom_ext_slot_b}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "rom_ext_slot = __BB\r\n",
        "otp_img": None,
    },
    "romext_virtual_a": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_virtual",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "rom_ext_slot = AA__\r\n",
        "otp_img": None,
    },
    "romext_virtual_b": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_virtual",
        "romext_offset": "{rom_ext_slot_b}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "rom_ext_slot = __BB\r\n",
        "otp_img": None,
    },
    "romext_virtual_a_with_imm_romext_enabled": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_virtual",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "rom_ext_slot = AA__\r\n",
        "failure": IMMUTABLE_HASH_UNENFORCED_MSG,
        "otp_img": ":otp_img_with_imm_romext_enabled",
    },
    "imm_section_virtual_a": {
        "romext": "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_virtual",
        "romext_offset": "{rom_ext_slot_a}",
        "linker_script": "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_a",
        "owner_offset": "{owner_slot_a}",
        "success": "IMM_SECTION:{}.{}".format(IMM_SECTION_MAJOR_VERSION, IMM_SECTION_MINOR_VERSION),
        "otp_img": None,
    },
}

[
    opentitan_test(
        name = "position_{}".format(name),
        srcs = [":boot_test"],
        exec_env = {
            "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
        },
        fpga = fpga_params(
            assemble = "{romext}@{romext_offset} {firmware}@{owner_offset}",
            binaries = {
                position["romext"]: "romext",
            },
            exit_failure = position.get("failure", DEFAULT_TEST_FAILURE_MSG),
            exit_success = position["success"],
            otp = position["otp_img"],
            owner_offset = position["owner_offset"],
            romext_offset = position["romext_offset"],
        ),
        linker_script = position["linker_script"],
        deps = [
            "//sw/device/lib/base:status",
            "//sw/device/lib/testing/test_framework:ottf_main",
            "//sw/device/silicon_creator/lib:boot_log",
            "//sw/device/silicon_creator/lib/drivers:retention_sram",
        ],
    )
    for name, position in _POSITIONS.items()
]

test_suite(
    name = "positions",
    tests = ["position_{}".format(name) for name in _POSITIONS],
)

manifest(d = {
    "name": "bad_manifest",
    "address_translation": hex(CONST.HARDENED_FALSE),
    "identifier": hex(CONST.OWNER),
    "manifest_version_major": "0",
    "manifest_version_minor": "0",
    "visibility": ["//visibility:public"],
})

opentitan_test(
    name = "bad_manifest_test",
    srcs = [":boot_test"],
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        exit_failure = "PASS|FAIL|FAULT|BFV:.{8}",
        # Because our bad manifest has an invalid manifest version, we should
        # expect kErrorManifestBadVersionMajor from the ROM_EXT.
        exit_success = "BFV:054d410d",
    ),
    manifest = ":bad_manifest",
    deps = [
        "//sw/device/lib/base:status",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:boot_log",
        "//sw/device/silicon_creator/lib/drivers:retention_sram",
    ],
)

manifest(d = {
    "name": "bad_spx_manifest",
    "address_translation": hex(CONST.HARDENED_FALSE),
    "identifier": hex(CONST.OWNER),
    "visibility": ["//visibility:public"],
    "extensions": [
        "spx_key",
        "",
        "secver_write",
        "isfb",
        "isfb_erase",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "spx_signature",
    ],
})

opentitan_test(
    name = "bad_spx_manifest_test",
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_spx_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        exit_failure = "PASS|FAIL|FAULT|BFV:.{8}",
        # Because our bad manifest placed spx_signature in a wrong slot, we
        # should expect kErrorManifestBadExtension from the ROM_EXT.
        exit_success = "BFV:044d410d",
    ),
    manifest = ":bad_spx_manifest",
    spx_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:spx_keyset": "app_prod_0"},
    deps = [
        "//sw/device/lib/base:status",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:boot_log",
        "//sw/device/silicon_creator/lib/drivers:retention_sram",
    ],
)

_KEYS = {
    "dev": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_dev_0"},
        "spx": {},
        "exit_success": DEFAULT_TEST_SUCCESS_MSG,
        "exit_failure": DEFAULT_TEST_FAILURE_MSG,
    },
    "dev_hybrid_spx_prehashed": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_dev_spx_0"},
        "spx": {"//sw/device/silicon_creator/lib/ownership/keys/fake:spx_keyset": "app_dev_0"},
        "exit_success": DEFAULT_TEST_SUCCESS_MSG,
        "exit_failure": DEFAULT_TEST_FAILURE_MSG,
    },
    "prod": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_0"},
        "spx": {},
        "exit_success": DEFAULT_TEST_SUCCESS_MSG,
        "exit_failure": DEFAULT_TEST_FAILURE_MSG,
    },
    "prod_hybrid_spx_pure": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_spx_0"},
        "spx": {"//sw/device/silicon_creator/lib/ownership/keys/fake:spx_keyset": "app_prod_0"},
        "exit_success": DEFAULT_TEST_SUCCESS_MSG,
        "exit_failure": DEFAULT_TEST_FAILURE_MSG,
    },
    "test": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_test_0"},
        "spx": {},
        "exit_success": DEFAULT_TEST_SUCCESS_MSG,
        "exit_failure": DEFAULT_TEST_FAILURE_MSG,
    },
    "unauthorized": {
        "ecdsa": {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_unauthorized_0"},
        "spx": {},
        "exit_success": DEFAULT_TEST_FAILURE_MSG,
        "exit_failure": DEFAULT_TEST_SUCCESS_MSG,
    },
}

[
    opentitan_test(
        name = "key_{}".format(name),
        srcs = [":boot_test"],
        ecdsa_key = keyinfo["ecdsa"],
        exec_env = {
            "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
        },
        fpga = fpga_params(
            exit_failure = keyinfo["exit_failure"],
            exit_success = keyinfo["exit_success"],
        ),
        spx_key = keyinfo["spx"],
        deps = [
            "//sw/device/lib/base:status",
            "//sw/device/lib/testing/test_framework:ottf_main",
            "//sw/device/silicon_creator/lib:boot_log",
            "//sw/device/silicon_creator/lib/drivers:retention_sram",
        ],
    )
    for name, keyinfo in _KEYS.items()
]

test_suite(
    name = "keys",
    tests = ["key_{}".format(name) for name in _KEYS],
)

manifest(d = {
    "name": "isfb_manifest",
    "address_translation": hex(CONST.HARDENED_FALSE),
    "identifier": hex(CONST.OWNER),
    "visibility": ["//visibility:public"],
    "integrator_specific_firmware_binding": integrator_specific_firmware_binding(
        name = "isfb",
        product_exprs = [
            product_expr(
                name = "none",
                mask = "0xFFFFFFFF",
                value = "0xFFFFFFFF",
            ),
        ],
        strike_mask = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    ),
})

opentitan_test(
    name = "isfb_boot_test",
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_spx_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(),
    manifest = ":isfb_manifest",
    spx_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:spx_keyset": "app_prod_0"},
    deps = [
        "//sw/device/lib/base:status",
        "//sw/device/lib/testing/test_framework:ottf_main",
        "//sw/device/silicon_creator/lib:boot_log",
        "//sw/device/silicon_creator/lib/drivers:retention_sram",
    ],
)

_BOOT_TEST_DEPS = [
    "//sw/device/lib/base:status",
    "//sw/device/lib/testing/test_framework:ottf_main",
    "//sw/device/silicon_creator/lib:boot_log",
    "//sw/device/silicon_creator/lib/drivers:retention_sram",
]

# Hybrid ECDSA + ML-DSA-87 ROM_EXT -> BL0 verified-boot E2E tests.
opentitan_test(
    name = "key_prod_hybrid_mldsa87",
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_mldsa_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_prod_0"},
    deps = _BOOT_TEST_DEPS,
)

opentitan_test(
    name = "key_dev_hybrid_mldsa87",
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_dev_mldsa_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_dev_0"},
    deps = _BOOT_TEST_DEPS,
)

opentitan_test(
    name = "key_unauthorized_mldsa",
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_mldsa_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        exit_failure = DEFAULT_TEST_SUCCESS_MSG,
        exit_success = "BFV:0c4f5705",
    ),
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_unauthorized_0"},
    deps = _BOOT_TEST_DEPS,
)

opentitan_binary(
    name = "hybrid_mldsa_slot_a_bin",
    testonly = True,
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_mldsa_0"},
    exec_env = ["//hw/top_earlgrey:fpga_cw340_rom_ext"],
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_prod_0"},
    deps = _BOOT_TEST_DEPS,
)

opentitan_binary(
    name = "hybrid_mldsa_slot_b_bin",
    testonly = True,
    srcs = [":boot_test"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_mldsa_0"},
    exec_env = ["//hw/top_earlgrey:fpga_cw340_rom_ext"],
    linker_script = "//sw/device/lib/testing/test_framework:ottf_ld_silicon_owner_slot_b",
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_prod_0"},
    deps = _BOOT_TEST_DEPS,
)

filegroup(
    name = "hybrid_mldsa_slot_a_signed_bin",
    testonly = True,
    srcs = [":hybrid_mldsa_slot_a_bin"],
    output_group = "fpga_cw340_rom_ext_signed_bin",
)

[
    genrule(
        name = "hybrid_mldsa_slot_a_{}_bin".format(name),
        testonly = True,
        srcs = [":hybrid_mldsa_slot_a_signed_bin"],
        outs = ["hybrid_mldsa_slot_a_{}.bin".format(name)],
        cmd = "$(location //sw/device/silicon_creator/lib/sigverify:mldsa_manifest_corrupt) --mode={} $< $@".format(mode),
        tools = ["//sw/device/silicon_creator/lib/sigverify:mldsa_manifest_corrupt"],
    )
    for name, mode in [
        ("bad_sig", "bad-sig"),
        ("bad_padding", "bad-padding"),
        ("stripped_sig_ext", "strip-sig"),
        ("stripped_all_ext", "strip-all"),
    ]
]

opentitan_test(
    name = "bad_mldsa_sig_test",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_mldsa_slot_a_bad_sig_bin": "firmware"},
        exit_failure = DEFAULT_TEST_SUCCESS_MSG,
        exit_success = "BFV:{}".format(hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_SIGNATURE)),
    ),
    manifest = None,
)

opentitan_test(
    name = "bad_mldsa_padding_test",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_mldsa_slot_a_bad_padding_bin": "firmware"},
        exit_failure = DEFAULT_TEST_SUCCESS_MSG,
        exit_success = "BFV:{}".format(hex_digits(CONST.BFV.SIGVERIFY.BAD_MLDSA_SIGNATURE)),
    ),
    manifest = None,
)

opentitan_test(
    name = "stripped_mldsa_sig_ext_test",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_mldsa_slot_a_stripped_sig_ext_bin": "firmware"},
        exit_failure = DEFAULT_TEST_SUCCESS_MSG,
        exit_success = "BFV:{}".format(hex_digits(CONST.BFV.MANIFEST.BAD_EXTENSION)),
    ),
    manifest = None,
)

opentitan_test(
    name = "stripped_mldsa_all_ext_test",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        binaries = {":hybrid_mldsa_slot_a_stripped_all_ext_bin": "firmware"},
        exit_failure = DEFAULT_TEST_SUCCESS_MSG,
        exit_success = "BFV:0c4f5705",
    ),
    manifest = None,
)

opentitan_test(
    name = "mldsa_slot_a_bad_sig_slot_b_fallback_test",
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        assemble = "{rom_ext}@{rom_ext_slot_a} {bad_a}@{owner_slot_a} {firmware}@{owner_slot_b}",
        binaries = {
            ":hybrid_mldsa_slot_a_bad_sig_bin": "bad_a",
            ":hybrid_mldsa_slot_b_bin": "firmware",
        },
        exit_failure = "((FAIL|FAULT).*\\n)|(BFV:[0-9a-f]{8})",
        exit_success = "bl0_slot = __BB\\r\\n[\\s\\S]*PASS!\\r\\n",
    ),
    manifest = None,
)

otp_image(
    name = "otp_img_secret2_locked_mldsa_prod",
    src = "//hw/top_earlgrey/data/otp:otp_json_prod",
    overlays = STD_OTP_OVERLAYS + EARLGREY_OTP_SIGVERIFY_FAKE_MLDSA_KEYS + [
        "//sw/device/silicon_creator/rom/e2e/sigverify_mldsa:otp_json_mldsa_en",
        "//sw/device/silicon_creator/rom_ext/e2e:otp_json_secret2_locked",
    ],
)

# Full ROM -> ROM_EXT -> BL0 ML-DSA-87 boot
# with SECRET2 locked and DICE CDI_0/CDI_1 keymgr_dpe verification in BL0.
opentitan_test(
    name = "mldsa_secret2_cdi1_e2e_test",
    srcs = [":boot_test"],
    defines = ["WITH_KEYMGR_DPE"],
    ecdsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:ecdsa_keyset": "app_prod_mldsa_0"},
    exec_env = {
        "//hw/top_earlgrey:fpga_cw340_rom_ext": None,
    },
    fpga = fpga_params(
        exit_failure = "((FAIL|FAULT).*\\n)|(BFV:[0-9a-f]{8})",
        exit_success = "keymgr dpe state = Available[\\s\\S]*PASS!\\r\\n",
        otp = ":otp_img_secret2_locked_mldsa_prod",
        rom_ext = "//sw/device/silicon_creator/rom_ext:rom_ext_dice_x509_slot_a_mldsa",
    ),
    mldsa_key = {"//sw/device/silicon_creator/lib/ownership/keys/fake:mldsa_keyset": "app_prod_0"},
    deps = _BOOT_TEST_DEPS + [
        "//hw/top_earlgrey/sw/autogen:top_earlgrey",
        "//sw/device/lib/dif:keymgr_dpe",
    ],
)

test_suite(
    name = "mldsa_verified_boot_tests",
    tags = ["manual"],
    tests = [
        "bad_mldsa_padding_test",
        "bad_mldsa_sig_test",
        "key_dev_hybrid_mldsa87",
        "key_prod_hybrid_mldsa87",
        "key_unauthorized_mldsa",
        "mldsa_secret2_cdi1_e2e_test",
        "mldsa_slot_a_bad_sig_slot_b_fallback_test",
        "stripped_mldsa_all_ext_test",
        "stripped_mldsa_sig_ext_test",
    ],
)
