#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import shutil
from pathlib import Path


def copy_file(src: Path, dst: Path) -> None:
    if not src.is_file():
        raise FileNotFoundError(src)
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def copy_tree(src: Path, dst: Path) -> None:
    if not src.is_dir():
        raise FileNotFoundError(src)
    ignore = shutil.ignore_patterns("obj", "lib", "*.a", "*.obj", "*.o", "*.d")
    shutil.copytree(src, dst, dirs_exist_ok=True, ignore=ignore)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--stage", required=True)
    args = parser.parse_args()

    stage = Path(args.stage).resolve()
    app_dir = Path(__file__).resolve().parents[1]
    module_root = app_dir.parents[1]
    repo_root = app_dir.parents[2]
    sdk = repo_root / "待整理的文件夹" / "official_ti_sdk" / "mspm0_sdk_2_11_00_07"
    bsp = module_root / "20_board_bsp" / "2026_天猛星拓展板例程模板"
    line = module_root / "40_line_sensor" / "循迹模块可以套用的代码模板"

    copy_file(bsp / "include" / "tmx_expansion_board.h",
              stage / "bsp" / "include" / "tmx_expansion_board.h")
    copy_file(bsp / "src" / "tmx_expansion_board.c",
              stage / "bsp" / "src" / "tmx_expansion_board.c")
    copy_file(bsp / "examples" / "mspm0g3507_syscfg_adapter_example.c",
              stage / "bsp" / "examples" / "mspm0g3507_syscfg_adapter_example.c")

    copy_file(line / "include" / "line_trace_template.h",
              stage / "line" / "include" / "line_trace_template.h")
    copy_file(line / "src" / "line_trace_template.c",
              stage / "line" / "src" / "line_trace_template.c")

    copy_tree(sdk / "source" / "ti" / "driverlib",
              stage / "sdk" / "source" / "ti" / "driverlib")
    copy_tree(sdk / "source" / "ti" / "devices",
              stage / "sdk" / "source" / "ti" / "devices")
    copy_tree(sdk / "source" / "third_party" / "CMSIS" / "Core" / "Include",
              stage / "sdk" / "source" / "third_party" / "CMSIS" / "Core" / "Include")

    (stage / ".stamp").write_text("staged\n", encoding="ascii")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
