# Scripts

> Parameterless, self-locating developer scripts. Run them from any working directory; they find the
> repository from their own location.

| Script | Purpose |
|---|---|
| `check-docs.ps1` | Check relative links and local anchors across `docs/`, `README.md`, `AGENTS.md` and `todo/` |
| `check-docs.cmd` | Windows wrapper for `check-docs.ps1`; uses `pwsh` when available, otherwise `powershell` |
| `test-wsl.ps1` | Configure, build and test the Linux build inside WSL using the `wsl-gcc-debug` preset |
| `test-wsl.cmd` | Windows wrapper for `test-wsl.ps1` |

## Usage

PowerShell:

```powershell
pwsh -File scripts/check-docs.ps1
```

Windows `cmd`:

```bat
scripts\check-docs.cmd
scripts\test-wsl.cmd
```

Both forms take no parameters. The scripts exit non-zero when a check or build fails.

## Adding a script

- Put it in this folder.
- Make it runnable without parameters.
- Resolve the repository root from `$PSScriptRoot` (PowerShell) or `%~dp0` (cmd).
- Add a row to the table above.
- Keep output short: a success summary and actionable failures.
