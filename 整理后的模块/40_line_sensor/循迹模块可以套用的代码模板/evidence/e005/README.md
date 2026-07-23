# E-005 Hot-Tuning Evidence Folder

Put SWD hot-tuning plan files and real car-test evidence here.

Dry-run planning does not touch hardware:

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case hot_kp_step `
  --seq 1 `
  --kp 42 `
  --out-json evidence\e005\e005-hot-kp-step.json
```

Validate collected JSON:

```powershell
python tools\line_trace_swd_tune_validate.py evidence\e005\e005-*.json --strict
```

Only run a real RAM write after the hardware safety gate is confirmed:

```powershell
python tools\line_trace_swd_tune_plan.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --pack C:\Users\DZ\AppData\Local\Temp\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.1.3.1.pack `
  --case hot_kp_step `
  --seq 1 `
  --kp 42 `
  --run `
  --allow-ram-write `
  --out-json evidence\e005\e005-hot-kp-step.json
```

Real E-005 evidence must also include SWD readback or telemetry proving
`status=APPLIED` for safe changes and `REJECTED/ROLLED_BACK` for unsafe changes.

