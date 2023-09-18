@rem @echo off
@cls
@for /R ..\.. %%F in (*.lobster) do @(
    Echo.%%F | findstr /C:"matvoxels">nul && (
	rem skipping %%F
    ) || (
        @start /B ..\..\bin\lobster.exe --non-interactive-test --silent %%F
        @if %errorlevel% neq 0 exit /b %errorlevel%
    )
)
@pause
