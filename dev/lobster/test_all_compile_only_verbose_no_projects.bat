@rem @echo off
@cls
@start /B ..\..\bin\lobster.exe --compile-only ..\..\tests\unittest.lobster
@for /R ..\..\modules\ %%F in (*.lobster) do @(@start /B ..\..\bin\lobster.exe --compile-only %%F)
@for /R ..\..\samples\ %%F in (*.lobster) do @(@start /B ..\..\bin\lobster.exe --compile-only %%F)
@pause
