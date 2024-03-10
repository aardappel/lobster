for file in *.md
do
    base=${file%.*}
    pandoc "$file" -o "../$base".html -s -f markdown-smart -t html5 -c github.css -H head.html
done
