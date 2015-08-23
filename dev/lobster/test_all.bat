@rem @echo off
@cls
for /R ..\.. %%F in (*.lobster) do (
    ..\..\lobster\lobster.exe --non-interactive-test --silent %%F
    @rem @if %errorlevel% neq 0 exit /b %errorlevel%
)
@rem pause
