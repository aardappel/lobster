echo off
for /R ..\.. %%F in (*.lobster) do ..\..\lobster\lobster.exe --non-interactive-test %%F
pause
