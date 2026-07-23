# 巡线模块 Evidence Index

## E-001

- Evidence ID: E-001
- Source type: command/test
- Path / command: `gcc -std=c99 -Wall -Wextra -Werror -Iinclude src/line_trace_template.c tests/test_line_trace_mock.c -o %TEMP%/line_trace_mock_tests.exe && %TEMP%/line_trace_mock_tests.exe`
- Linked Requirement ID: G-008, G-009, G-012
- Linked Goal/subgoal: PC/mock algorithm verification for line trace module v0.1
- Result: pass, output `line_trace_mock_tests: PASS`
- Timestamp/context: 2026-07-23, Codex goal loop implementation turn
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

## E-002

- Evidence ID: E-002
- Source type: command/build
- Path / command: `mingw32-make -f Makefile.mspm0g3507 clean all`
- Linked Requirement ID: G-008, G-012, G-014
- Linked Goal/subgoal: MSPM0G3507 GNU target build gate for line trace smoke ELF
- Result: pass; built `line_trace_smoke.elf/.hex/.bin`; previous size `text=3788 data=68 bss=280 dec=4136`; required symbols present
- Timestamp/context: 2026-07-23, Codex goal loop G-014 implementation turn
- Status: verified for target build and ELF symbol availability

## E-002A

- Evidence ID: E-002A
- Source type: command/tool-dry-run
- Path / command: `python tools\line_trace_swd_readback.py --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf --target mspm0g3507 --probe-uid 031305620164`
- Linked Requirement ID: G-006, G-010, G-014
- Linked Goal/subgoal: SWD readback method for RAM tuning block and telemetry frame
- Result: pass; dry-run resolved ELF symbols and generated pyOCD read-only commands
- Timestamp/context: 2026-07-23, Codex goal loop G-014 implementation turn
- Status: verified for symbol resolution and readback command generation; real hardware readback remains pending

## E-003A

- Evidence ID: E-003A
- Source type: command/tool-dry-run
- Path / command: `python tools\line_trace_bench_capture.py --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf --target mspm0g3507 --probe-uid 031305620164 --case dry_run_center`
- Linked Requirement ID: G-003, G-004, G-009, G-015
- Linked Goal/subgoal: E-003 bench sensor evidence collection method
- Result: pass; dry-run resolved `g_line_bench_snapshot` and generated read-only `read32 <addr> 22` command
- Timestamp/context: 2026-07-23, Codex goal loop G-015 implementation turn
- Status: verified for bench capture tooling only; real sensor samples remain pending

## E-003B

- Evidence ID: E-003B
- Source type: command/tooling-check
- Path / command: `python tools\line_trace_bench_validate.py evidence\e003\e003-template.json %TEMP%\e003-validator-sample.json --strict`
- Linked Requirement ID: G-003, G-004, G-009, G-016
- Linked Goal/subgoal: E-003 bench evidence template and validator readiness
- Result: pass; validator skipped `template_only` JSON, accepted synthetic capture, and reported `status=pass`
- Timestamp/context: 2026-07-23, Codex goal loop G-016 implementation turn
- Status: verified for validator/tooling only; synthetic capture is not hardware evidence

## E-003C

- Evidence ID: E-003C
- Source type: command/probe-readiness
- Path / command: `Get-Command pyocd`; TI DFP `Get-FileHash`; `pyocd list --probes`
- Linked Requirement ID: G-008, G-016
- Linked Goal/subgoal: non-invasive readiness check before real E-003 bench execution
- Result: pass; `pyocd.exe` found; TI DFP pack hash matched expected value; probe enumeration saw Horco CMSIS-DAP UIDs `031305620164` and `031305622180`
- Timestamp/context: 2026-07-23, Codex goal loop G-016 implementation turn
- Status: verified for non-invasive readiness only; no target attach, flash, reset, or real sensor sampling was performed

## E-005A

- Evidence ID: E-005A
- Source type: command/tooling-check
- Path / command: `python tools\line_trace_swd_tune_plan.py ...` for `hot_kp_step`, `hot_kd_step`, `hot_base_speed_step`, `hot_max_correction_step`, `reject_bad_kp`, `rollback_cold_param`; then `python tools\line_trace_swd_tune_validate.py %TEMP%\e005-*.json --strict`
- Linked Requirement ID: G-006, G-007, G-010, G-016
- Linked Goal/subgoal: E-005 SWD hot tuning dry-run and safety validator readiness
- Result: pass; six dry-run reports resolved fresh ELF symbols, generated firmware-owned RAM tuning-block write/readback commands, accepted safe RUNNING_TUNE_SAFE steps, and rejected/rolled back unsafe cases with no hardware access
- Timestamp/context: 2026-07-23, Codex goal continuation after G-017 remained unconfirmed
- Status: verified for tooling only; real SWD RAM write and car-test evidence remain pending

## E-006

- Evidence ID: E-006
- Source type: command/git
- Path / command: `git push origin main`
- Linked Requirement ID: repository delivery
- Linked Goal/subgoal: GitHub synchronization for module examples repository
- Result: pass; previous remote SHA `289f503f54ba0a7d089f310a9cf75731f65e8ade`
- Timestamp/context: 2026-07-23, Codex goal loop
- Status: verified for previous checkpoint; current turn push pending

## Pending Hardware Evidence

- E-002H: real hardware SWD readback remains pending.
- E-003: bench sensor `raw_bits/active_bits` mapping remains pending.
- E-004: low-speed 3 laps or 3 minutes car test remains pending.
- E-005: running SWD hot tuning apply/readback/limits/rollback car test remains pending.
