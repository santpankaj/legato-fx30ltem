#!/bin/bash -e

BASE_URL="https://oss.sonatype.org/content/repositories/releases/net/sourceforge/plantuml/plantuml"

VERSION=$1

if [ -z "$VERSION" ]; then
  echo "Version, not specified, getting latest"

  VERSION=$(curl -q $BASE_URL/maven-metadata.xml | xmllint --xpath '//version[last()]/text()' -)
fi

echo "Version: ${VERSION}"

JAR="plantuml-${VERSION}.jar"

if [ -e "${JAR}" ]; then
  echo "File '${JAR}' already present"
  exit 0
fi

URL="${BASE_URL}/${VERSION}/${JAR}"

echo "Getting ${JAR} from ${URL}"
wget ${URL}

echo "Getting SHA1 for ${JAR} from ${URL}.sha1"
SHA1=$(curl ${URL}.sha1)
[ -n "$SHA1" ]

# Check sha1
if [[ "$(sha1sum plantuml-${VERSION}.jar | awk '{print $1}')" != "$SHA1" ]]; then
  echo "Checksum control failed"
  exit 1
else
  echo "Checksum OK"
fi

