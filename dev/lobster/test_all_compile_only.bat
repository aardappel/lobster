@rem @echo off
@cls
@for /R ..\.. %%F in (*.lobster) do @(
    Echo.%%F | findstr "matvoxels testFixture">nul && (
	rem skipping %%F
    ) || (
        @start /B ..\..\bin\lobster.exe --compile-only --silent %%F
        @rem @if %errorlevel% neq 0 exit /b %errorlevel%
    )
)
@pause
