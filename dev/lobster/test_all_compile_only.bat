@rem @echo off
@cls
@for /R ..\.. %%F in (*.lobster) do @(
    @start /B ..\..\lobster\lobster.exe --compile-only --silent %%F
    @rem @if %errorlevel% neq 0 exit /b %errorlevel%
)
pause
