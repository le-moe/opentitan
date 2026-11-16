# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("//rules:const.bzl", "CONST")
load(
    "//rules:otp.bzl",
    "otp_hex",
    "otp_json_rot_keys",
    "otp_partition",
)

package(default_visibility = ["//visibility:public"])

# Each 48-byte ROT_CREATOR_AUTH_CODESIGN_SPX_KEY{i} slot packs:
#   bits   0..255: 32-byte SPHINCS+ key (LSB)
#   bits 256..287: 4-byte config_id
#   bits 288..383: 12-byte zero pad (MSB)
otp_json_rot_keys(
    name = "json_rot_keys",
    partitions = [
        otp_partition(
            name = "ROT_CREATOR_AUTH_CODESIGN",
            items = {
                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "test_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE0": otp_hex(CONST.SIGVERIFY.KEY_TYPE.TEST),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY0": "0xdeb1e092bb56aeb4a5d7eb1df773fc51012abb223c413ac2444da9ded8e17e502e66e6cb6937e2375bcb453d845024cfe9e0f4807bfccca80bba6de3ee07109a",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "dev_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE1": otp_hex(CONST.SIGVERIFY.KEY_TYPE.DEV),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY1": "0xce8c4b5b70df4ad0c9dbc64bf7a103fdb1dfa2cc40529329ba9eb9172fbccaafd0b7ee1365a85787b9e7bf8aa742ec258ce157388a01ceb04f7ed1a3c11c931c",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "prod_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE2": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY2": "0x7e571d04866f12e00a5b3c2e734ebd05611a0325ef6d90140e3cd2091ddda9bd8277988ce6557b33923499d90eeeff0028a525c10fa3a5a438daf96b9bf2dafd",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "prod_key_1"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE3": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY3": "0xb70fd07db1c79bbb87f974e6d3ea06e1f120582d2e8abaf1ddd1167aac4d7547fbf0b76ae2838f6cec711f34204d24202fccd456ad9f2278d80fe551423e545e",

                # //sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset "test_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE0": otp_hex(CONST.SIGVERIFY.KEY_TYPE.TEST),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY0": "0x" + ("00" * 12) + otp_hex(CONST.SPX_CONFIG_ID.SHA2_128S)[2:] + "a7fd92541005bf04f172710eaafa228220ce5b2a73e009c74fdc04eefa2a1ba6",

                # //sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset "dev_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE1": otp_hex(CONST.SIGVERIFY.KEY_TYPE.DEV),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY1": "0x" + ("00" * 12) + otp_hex(CONST.SPX_CONFIG_ID.SHA2_128S)[2:] + "594087c53775908295afa7000d11eec6fc430f8e7858e774e595f1d161f34ce4",

                # //sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset "prod_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE2": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY2": "0x" + ("00" * 12) + otp_hex(CONST.SPX_CONFIG_ID.SHA2_128S)[2:] + "789b490a8c50f4dadee29c091dfe267bfedece98519c46bdb2ade5d32d3e6e02",

                # //sw/device/silicon_creator/rom/keys/fake/spx:spx_keyset "prod_key_1"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE3": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY3": "0x" + ("00" * 12) + otp_hex(CONST.SPX_CONFIG_ID.SHA2_128S_PREHASH)[2:] + "4d83c8d10356fbb033ae5f9a3ee242a21f5a1a8b8c9a5579496271faad63458a",
            },
        ),
        otp_partition(
            name = "ROT_CREATOR_AUTH_STATE",
            items = {
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY0": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY1": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY2": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY3": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY0": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY1": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY2": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY3": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
            },
        ),
    ],
)

# ML-DSA-87 ROT_CREATOR_AUTH_CODESIGN fake keys. Each 48-byte slot holds
# tr[0..47] = SHAKE256(pk, 48) serialized MSB-first so OtpMemImg and
# gen-otp-rot-auth-json store tr[0..3] (LE key_id) at offset 0.
otp_json_rot_keys(
    name = "json_rot_keys_mldsa",
    partitions = [
        otp_partition(
            name = "ROT_CREATOR_AUTH_CODESIGN",
            items = {
                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "test_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE0": otp_hex(CONST.SIGVERIFY.KEY_TYPE.TEST),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY0": "0xdeb1e092bb56aeb4a5d7eb1df773fc51012abb223c413ac2444da9ded8e17e502e66e6cb6937e2375bcb453d845024cfe9e0f4807bfccca80bba6de3ee07109a",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "dev_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE1": otp_hex(CONST.SIGVERIFY.KEY_TYPE.DEV),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY1": "0xce8c4b5b70df4ad0c9dbc64bf7a103fdb1dfa2cc40529329ba9eb9172fbccaafd0b7ee1365a85787b9e7bf8aa742ec258ce157388a01ceb04f7ed1a3c11c931c",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "prod_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE2": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY2": "0x7e571d04866f12e00a5b3c2e734ebd05611a0325ef6d90140e3cd2091ddda9bd8277988ce6557b33923499d90eeeff0028a525c10fa3a5a438daf96b9bf2dafd",

                # //sw/device/silicon_creator/rom/keys/fake/ecdsa:ecdsa_keyset "prod_key_1"
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY_TYPE3": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_ECDSA_KEY3": "0xb70fd07db1c79bbb87f974e6d3ea06e1f120582d2e8abaf1ddd1167aac4d7547fbf0b76ae2838f6cec711f34204d24202fccd456ad9f2278d80fe551423e545e",

                # //sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset "test_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE0": otp_hex(CONST.SIGVERIFY.KEY_TYPE.TEST),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY0": "0x9d7274f5df70b6b2af0615cfba30bda3518d96eb3ae4d73f2b0ef08d83589a997846dc21d22a63ca06718fd55a6c284b",

                # //sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset "dev_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE1": otp_hex(CONST.SIGVERIFY.KEY_TYPE.DEV),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY1": "0xe516dbe5bc931e57a47e96940eb3a4155c34eaaec1316b908ad169e5a1294ee302bed9ae2ec6e27c49d133c4fddc8e18",

                # //sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset "prod_key_0"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE2": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY2": "0x2ff0934e5964a890027f3d20e1f4553b414554a06d821eb4fa737c0a9bd37e3ed4c2fc4ee90e5ec1c184868b760f0be7",

                # //sw/device/silicon_creator/rom/keys/fake/mldsa:mldsa_keyset "prod_key_1"
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY_TYPE3": otp_hex(CONST.SIGVERIFY.KEY_TYPE.PROD),
                "ROT_CREATOR_AUTH_CODESIGN_SPX_KEY3": "0xe1396e38ffe457a9f390e0644c477f1a63be1bd2c750d361805d27becdf71153aefc0a01214079ff00c5721edd866b53",
            },
        ),
        otp_partition(
            name = "ROT_CREATOR_AUTH_STATE",
            items = {
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY0": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY1": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY2": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_ECDSA_KEY3": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY0": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY1": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY2": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
                "ROT_CREATOR_AUTH_STATE_SPX_KEY3": otp_hex(CONST.SIGVERIFY.KEY_STATE.PROVISIONED),
            },
        ),
    ],
)
