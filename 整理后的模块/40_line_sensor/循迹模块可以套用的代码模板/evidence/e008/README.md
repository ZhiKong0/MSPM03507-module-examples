# E-008 G-017 Hardware Gate Plan

This folder stores generated hardware-gate plans for the line-trace module.

`line_trace_hardware_gate_plan.py` is non-invasive by default. It resolves the
fresh ELF symbols, checks the expected probe UID and TI DFP pack hash, and emits
a reviewable JSON/Markdown plan for:

- G-017 safety confirmation.
- E-002H raw DAP/status-only gate.
- E-003 bench sensor sampling order.
- E-005 SWD RAM tuning-block steps.
- E-007 last-known-good profile freeze after real evidence.

Files generated in default mode are `dry-run-plan` evidence only. They do not
prove real hardware access, bench mapping, car-test stability, or SWD hot tuning.

Real run commands are omitted unless the generator is called with:

```powershell
python tools\line_trace_hardware_gate_plan.py `
  --include-run-commands `
  --safety-token G017-HARDWARE-SAFE
```

Even then, the script only writes a plan. It does not execute pyOCD, attach to
the target, flash, reset, read SWD, or write RAM.
