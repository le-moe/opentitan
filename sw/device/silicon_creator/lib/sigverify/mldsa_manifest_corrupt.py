#!/usr/bin/env python3
# Copyright lowRISC contributors (OpenTitan project).
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
"""Mutates ML-DSA-87 manifest extensions in signed binaries for negative tests."""

import argparse
from pathlib import Path
import struct
import sys

MANIFEST_SIZE = 1024
EXT_TABLE_OFFSET = 904
EXT_TABLE_ENTRY_COUNT = 15
EXT_ENTRY_FMT = "<II"
EXT_ENTRY_SIZE = struct.calcsize(EXT_ENTRY_FMT)
EXT_HEADER_FMT = "<II"
EXT_HEADER_SIZE = struct.calcsize(EXT_HEADER_FMT)

EXT_ID_MLDSA87_KEY = 0xA3E247B8
EXT_NAME_MLDSA87_KEY = 0x4B384C4D  # "ML8K"
EXT_ID_MLDSA87_SIG = 0x4714AC7D
EXT_NAME_MLDSA87_SIG = 0x53384C4D  # "ML8S"

MLDSA87_SIG_BYTES = 4627
MLDSA87_SIG_TOTAL_SIZE = EXT_HEADER_SIZE + MLDSA87_SIG_BYTES + 1


def _find_extension(
    data: bytearray,
    ext_id: int,
    ext_name: int,
) -> tuple[int, int]:
    """Returns `(table_entry_offset, ext_offset)` for `ext_id`, verifying its header."""
    if len(data) < MANIFEST_SIZE:
        raise ValueError(
            f"Signed image too short ({len(data)} B < {MANIFEST_SIZE} B)"
        )

    for slot in range(EXT_TABLE_ENTRY_COUNT):
        entry_offset = EXT_TABLE_OFFSET + slot * EXT_ENTRY_SIZE
        entry_id, ext_offset = struct.unpack_from(EXT_ENTRY_FMT, data, entry_offset)
        if entry_id != ext_id or ext_offset == 0:
            continue
        if ext_offset + EXT_HEADER_SIZE > len(data):
            raise ValueError(
                f"Extension {ext_id:#010x} offset {ext_offset:#x} out of bounds"
            )
        hdr_id, hdr_name = struct.unpack_from(EXT_HEADER_FMT, data, ext_offset)
        if hdr_id != ext_id or hdr_name != ext_name:
            raise ValueError(
                f"Unexpected extension header at {ext_offset:#x}: "
                f"got ({hdr_id:#010x}, {hdr_name:#010x}), "
                f"expected ({ext_id:#010x}, {ext_name:#010x})"
            )
        return entry_offset, ext_offset

    raise ValueError(f"Extension {ext_id:#010x} not found in manifest table")


def _mutate_image(data: bytearray, mode: str) -> None:
    """Applies the requested negative-test mutation to `data` in place."""
    if mode in ("bad-sig", "bad-padding"):
        _, ext_offset = _find_extension(
            data, EXT_ID_MLDSA87_SIG, EXT_NAME_MLDSA87_SIG
        )
        if ext_offset + MLDSA87_SIG_TOTAL_SIZE > len(data):
            raise ValueError(
                f"ML-DSA signature extension at {ext_offset:#x} truncated"
            )
        if mode == "bad-sig":
            data[ext_offset + EXT_HEADER_SIZE] ^= 0x01
        else:
            pad_offset = ext_offset + EXT_HEADER_SIZE + MLDSA87_SIG_BYTES
            if data[pad_offset] != 0x00:
                raise ValueError(
                    f"Expected zero pad byte at {pad_offset:#x}, got {data[pad_offset]:#04x}"
                )
            data[pad_offset] = 0x01
        return

    targets = (
        ((EXT_ID_MLDSA87_SIG, EXT_NAME_MLDSA87_SIG),)
        if mode == "strip-sig"
        else (
            (EXT_ID_MLDSA87_KEY, EXT_NAME_MLDSA87_KEY),
            (EXT_ID_MLDSA87_SIG, EXT_NAME_MLDSA87_SIG),
        )
    )
    for ext_id, ext_name in targets:
        entry_offset, _ = _find_extension(data, ext_id, ext_name)
        struct.pack_into(EXT_ENTRY_FMT, data, entry_offset, 0, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode",
        required=True,
        choices=("bad-sig", "bad-padding", "strip-sig", "strip-all"),
        help="Mutation to apply to the signed image.",
    )
    parser.add_argument("input_bin", type=Path, help="Input signed binary path.")
    parser.add_argument("output_bin", type=Path, help="Output mutated binary path.")
    args = parser.parse_args()

    data = bytearray(args.input_bin.read_bytes())
    _mutate_image(data, args.mode)
    args.output_bin.write_bytes(data)
    return 0


if __name__ == "__main__":
    sys.exit(main())
