@echo off

android update project -p .

python -B build_all_android.py -j4 -f NDK_DEBUG=0 -S -k debug.keystore -K android -P debugkey.txt

for %%x in (%cmdcmdline%) do if /i "%%~x"=="/c" set DOUBLECLICKED=1
if defined DOUBLECLICKED pause
