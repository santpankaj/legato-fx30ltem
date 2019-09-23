#!/bin/sh
set -eux
IN_HTML="$LEGATO_ROOT/docManagement/new_html/"
IN_PICS="$LEGATO_ROOT/build/doc/user/html/"
IN_RESOURCES="$LEGATO_ROOT/docManagement/templater/sources/static/resources/"
TOC="$LEGATO_ROOT/build/doc/user/html/toc.json"
TEMP="temp/"
OUT="$LEGATO_ROOT/build/doc/user/Documentation.pdf"
rm -rf $TEMP
mkdir $TEMP
cp -R "$IN_RESOURCES/css" "$IN_RESOURCES/fonts" $TEMP/
sed -i '/\/\*NO PDF START\*\//,/\/\*NO PDF END\*\//d' $TEMP/css/style.css
echo "pre{overflow:visible !important;}" >> $TEMP/css/style.css
find $IN_PICS -not -name '*.html' -exec file {} \; | grep -o -P '^.+: \w+ image' | cut -d':' -f1 | xargs -I{} cp {} $TEMP
FILE="$TEMP/concatenated.html"

echo "<head>" >> $FILE
echo "<meta charset="UTF-8">" >> $FILE
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/style.css\">" >> $FILE
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"css/font-awesome.css\">" >> $FILE
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../pdf_style.css\">" >> $FILE
echo "</head><body>" >> $FILE
python concat.py $TOC $IN_HTML -p "$@" >> $FILE
echo "</body>" >> $FILE
wkhtmltopdf --lowquality  --outline-depth 99 --margin-left 25 --margin-right 25 \
    cover coversheet.html \
    toc --xsl-style-sheet toc.xsl --toc-header-text "Contents" \
    license.html "$FILE" \
    "$OUT"
rm -rf $TEMP
