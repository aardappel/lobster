@rem @echo off
@cls
@for /R ..\.. %%F in (modules\*.lobster) do @(@start /B ..\..\bin\lobster.exe --compile-only %%F)
@for /R ..\.. %%F in (tests\*.lobster) do @(@start /B ..\..\bin\lobster.exe --compile-only %%F)
@for /R ..\.. %%F in (samples\*.lobster) do @(@start /B ..\..\bin\lobster.exe --compile-only %%F)
@pause
