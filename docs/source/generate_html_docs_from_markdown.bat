for %%F in (*.md) do pandoc -s -f markdown-smart -t html5 -c github.css -H head.html -o ..\%%~nF.html %%F
pandoc -s -f markdown-smart -t html5 -c github.css -H head.html -o ..\RESULTS.html ..\..\tests\compare\RESULTS.md
pause
