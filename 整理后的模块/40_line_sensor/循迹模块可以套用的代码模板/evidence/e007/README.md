# E-007 Profile Freeze Evidence Folder

This folder is for reusable line-trace tuning profiles exported from validated
E-005 tuning plans.

Dry-run profiles are allowed only as `dry-run-candidate`; they are useful for
reviewing generated config files but are not last-known-good vehicle parameters.

Example:

```powershell
python tools\line_trace_profile_freeze.py `
  --plan evidence\e005\e005-hot-kp-step.json `
  --profile-class dry-run-candidate `
  --allow-dry-run-source `
  --name mspm0g3507-8ch-dryrun-kp42 `
  --out-json evidence\e007\profile-dryrun-kp42.json `
  --out-header evidence\e007\line_trace_profile_dryrun_kp42.h
```

Only freeze `last-known-good` after real E-003/E-004/E-005 hardware evidence
passes and the input plan has `hardware_evidence=true`.

