#!/bin/bash

# --doc-tweaks enables Legato docs specific tweaks, which for now is just moving
#              the Reference node under C Prototypes
# --pdf        generates a pdf version of the docs

set -eu

DEFAULT_IN="$LEGATO_ROOT/build/doc/user/html/"

if [ -z ${INPUT_DIR+x} ]; then
    IN="$DEFAULT_IN"
    echo "Using default input dir: $IN"
    echo "Set INPUT_DIR to override."
else
    IN="$INPUT_DIR"
    echo "Using custom input dir: $IN"
fi

if [ -z ${OUTPUT_DIR+x} ]; then
    # Remove trailing slash so we can append a "_converted" suffix to the dir name
    IN_trimmed=${IN%/}
    OUT="${IN_trimmed}_converted/"
    # not calling it 'default' because it's automatically generated
    echo "Using standard output dir: $OUT"
    echo "Set OUTPUT_DIR to override."
else
    OUT="$OUTPUT_DIR"
    echo "Using custom output dir: $OUT"
fi


TEMPLATER_DIR=./templater/

# compares two version numbers and returns true if the first is bigger or equal.
function version_gt() { test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1"; }


MIN_DOXY_VER="1.8.11"

CUR_DOXY_VER=`doxygen -v` ||
    { echo -e "\033[31mWARNING: Couldn't verify doxygen version. Things might look wrong. \033[0m"; CUR_DOXY_VER=""; }

if [ "$CUR_DOXY_VER" ] && version_gt $MIN_DOXY_VER $CUR_DOXY_VER; then
    echo -e "\033[31mWARNING: Your doxygen seems to be outdated.\033[0m This script is designed for >$MIN_DOXY_VER while you have $CUR_DOXY_VER"
    echo -e "\033[31mThings might look wrong. \033[0m"
fi

if [ -d "$OUT" ]; then
    echo "Cleaning $OUT"
    rm -rf $OUT
fi

mkdir -p $OUT

echo "Generating table of contents ..."

if [[ "$*" == *--doc-tweaks* ]]; then
    EXTRACT_DOC_OPTS="--move-reference"
    TEMPLATER_OPTS="--split-top-level"
else
    EXTRACT_DOC_OPTS=""
    TEMPLATER_OPTS="--split-top-level"
fi

./extractDocStructure.py $IN/toc.xml $EXTRACT_DOC_OPTS # make toc.json

echo "Extracting HTML ..."
if [ -d "new_html" ]; then
    rm -rf new_html/
fi
./extractHTML.py $IN new_html/ # extract content into their own partial html files
cp $IN/toc.json ./new_html/

if [ -z ${TMPL_RES_DIR+x} ]; then
    # set default template resource dir
    TMPL_RES_DIR='default'
fi
echo "Running templater ..."
rm -rf $TEMPLATER_DIR/out/
$TEMPLATER_DIR/templater.py new_html $TEMPLATER_DIR/out/ $TEMPLATER_OPTS --resource-dir $TMPL_RES_DIR
mv $TEMPLATER_DIR/out/* $OUT
rm -rf $TEMPLATER_DIR/out/

if [[ "$*" == *--pdf* ]]; then
    echo "Creating PDF ..."
    (cd pdf; ./generate.sh)
    # if you want to include only specific sections, use this syntax:
    # (cd pdf; ./generate.sh "Build Apps > Concepts" "Build Apps > Tools")
else
    echo "--pdf isn't specified, skipping it."
fi

echo "Copying images ..."
for f in $(find $IN -not -name '*.html'); do
    if file --mime-type $f | grep -q 'image/'; then
        cp $f $OUT/
    fi
done

echo "Copying PDFs"
find $IN -name "*.pdf" -exec cp -v {} $OUT \;

if ! [ -e "$OUT/build_apps_main.html" ]; then
    echo "Handling doxygen >= 1.6.11"
    sed -i 's/build_apps_main/buildAppsMain/g' $OUT/index.html
    sed -i 's/build_platform_main/buildPlatformMain/g' $OUT/index.html
    sed -i 's/about_main/aboutMain/g' $OUT/index.html
fi

if [ -e "$OUT/mangohDevelopers.html" ]; then
    mv $OUT/mangohDevelopers.html $OUT/mangOH_developers.html # workaround for a silly mess
fi

rm -rf new_html

if ! which linkchecker; then
    echo "Skipping link tests (linkchecker not found) ..."
else
    CHECK_EXTERN_OPT=''
    if [[ -n "${WITH_EXTERNAL_LINKS_CHECK-}" ]]; then # check if variable is defined first
        if [[ "$WITH_EXTERNAL_LINKS_CHECK" == "1" ]] || [[ "$WITH_EXTERNAL_LINKS_CHECK" == "true" ]]; then
            CHECK_EXTERN_OPT='--check-extern'
        fi
    fi

    linkchecker \
                $CHECK_EXTERN_OPT \
                --ignore-url="^http://nnn.nnn.nnn.nnn.*|^https://...airvantage.net.*" \
                $OUT/index.html
fi

echo "Done."
echo "Output is in $(realpath $OUT)"

