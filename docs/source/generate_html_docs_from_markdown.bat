for %%F in (*.md) do pandoc -s -f markdown -t html5 -c github.css -H head.html -o ..\%%~nF.html %%F
pause
