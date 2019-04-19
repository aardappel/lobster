for file in *.md
do
    base=${file%.*}
    pandoc "$file" -o "../$base".html -s -f markdown -t html5 -c github.css
done
