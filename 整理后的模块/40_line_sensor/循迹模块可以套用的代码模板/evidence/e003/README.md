# E-003 Bench Evidence Folder

Put real bench capture files here when hardware is ready.

Suggested names:

- `e003-white-idle.json`
- `e003-ch0-black.json`
- `e003-ch1-black.json`
- `e003-ch2-black.json`
- `e003-ch3-black.json`
- `e003-ch4-black.json`
- `e003-ch5-black.json`
- `e003-ch6-black.json`
- `e003-ch7-black.json`
- `e003-left-to-right.json`
- `e003-no-line-after-seen.json`

Generate a capture:

```powershell
python tools\line_trace_bench_capture.py `
  --elf build\mspm0g3507-line-trace-smoke\line_trace_smoke.elf `
  --target mspm0g3507 `
  --probe-uid 031305620164 `
  --case ch0_black `
  --samples 3 `
  --out-json evidence\e003\e003-ch0-black.json `
  --out-csv evidence\e003\e003-ch0-black.csv `
  --run
```

Validate collected JSON:

```powershell
python tools\line_trace_bench_validate.py evidence\e003\e003-*.json --strict
```

Keep generated capture files when they are real evidence. Do not commit throwaway dry-run outputs as if they were hardware evidence.
