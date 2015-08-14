rem @echo off
for /R ..\.. %%F in (*.lobster) do (
    ..\..\lobster\lobster.exe --non-interactive-test %%F
    @if %errorlevel% neq 0 exit /b %errorlevel%
)
pause
