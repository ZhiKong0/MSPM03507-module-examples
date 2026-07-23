# 巡线模块 Evidence Index

## E-001

- Evidence ID: E-001
- Source type: command/test
- Path / command: `gcc -std=c99 -Wall -Wextra -Werror -Iinclude src/line_trace_template.c tests/test_line_trace_mock.c -o %TEMP%/line_trace_mock_tests.exe && %TEMP%/line_trace_mock_tests.exe`
- Linked Requirement ID: G-008, G-009, G-012
- Linked Goal/subgoal: PC/mock algorithm verification for line trace module v0.1
- Result: pass, output `line_trace_mock_tests: PASS`
- Timestamp/context: 2026-07-23, Codex goal loop implementation turn, Windows MinGW gcc at `F:\mingw64\bin\gcc.exe`
- Status: verified

## E-001A

- Evidence ID: E-001A
- Source type: command/compile
- Path / command: `gcc -std=c99 -Wall -Wextra -Werror -Iinclude -c examples/integration_loop_example.c -o %TEMP%/integration_loop_example.o`
- Linked Requirement ID: G-010, G-012
- Linked Goal/subgoal: MSPM0 integration-loop example API sanity check
- Result: pass, integration example compiles as a host C translation unit with external board/motor symbols
- Timestamp/context: 2026-07-23, Codex goal loop implementation turn
- Status: verified for API syntax only; not a hardware/MSPM0 build substitute

## E-001B

- Evidence ID: E-001B
- Source type: command/static-check
- Path / command: `git diff --check -- 整理后的模块/40_line_sensor/循迹模块可以套用的代码模板`
- Linked Requirement ID: G-012
- Linked Goal/subgoal: repository patch hygiene for first implementation slice
- Result: pass, no whitespace errors reported
- Timestamp/context: 2026-07-23, Codex goal loop implementation turn
- Status: verified

## E-001C

- Evidence ID: E-001C
- Source type: command/artifact-guard
- Path / command: `python C:\Users\DZ\.codex\tools\artifact-guard\scan_prompt_artifact_pollution.py 整理后的模块/40_line_sensor/循迹模块可以套用的代码模板 --json`
- Linked Requirement ID: G-012
- Linked Goal/subgoal: resumed-turn file edit pollution check
- Result: pass, `finding_count=0`
- Timestamp/context: 2026-07-23, Codex goal loop implementation turn
- Status: verified

## Pending Hardware Evidence

- E-002: MSPM0 GNU/CMake or Makefile build remains pending.
- E-003: bench sensor `raw_bits/active_bits` mapping remains pending.
- E-004: low-speed 3 laps or 3 minutes car test remains pending.
- E-005: running SWD hot tuning apply/readback/limits/rollback car test remains pending.
