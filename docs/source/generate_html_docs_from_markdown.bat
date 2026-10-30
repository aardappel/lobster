for %%F in (*.md) do pandoc -s -f markdown-smart -t html5 -c github.css -H head.html -B nav.html --syntax-definition lobster.xml -o ..\%%~nF.html %%F
pandoc -s -f markdown-smart -t html5 -c github.css -H head.html -B nav.html --syntax-definition lobster.xml -o ..\RESULTS.html ..\..\tests\compare\RESULTS.md
pause
