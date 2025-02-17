#!/bin/bash
set -e -u -x

# Use ruby from brew on Mac OS X, because system ruby is outdated/broken/will be removed in future releases.
case $OSTYPE in darwin*)
  if [ -x /usr/local/opt/ruby/bin/ruby ]; then
    PATH="/usr/local/opt/ruby/bin:$PATH"
  elif [ -x /opt/homebrew/opt/ruby/bin/ruby ]; then
    PATH="/opt/homebrew/opt/ruby/bin:$PATH"
  else
    echo 'Please install Homebrew ruby by running "brew install ruby"'
    exit -1
  fi
esac

OMIM_PATH="$(dirname "$0")/../.."
TWINE="$OMIM_PATH/tools/twine/twine"
STRINGS_PATH="$OMIM_PATH/data/strings"

MERGED_FILE="$(mktemp)"
cat "$STRINGS_PATH"/{strings,partners_strings,types_strings,brands_strings}.txt> "$MERGED_FILE"

# TODO: Add "--untagged --tags android" when tags are properly set.
# TODO: Add validate-strings-file call to check for duplicates (and avoid Android build errors) when tags are properly set.
$TWINE generate-all-localization-files --include translated --format android "$MERGED_FILE" "$OMIM_PATH/android/res/"
$TWINE generate-all-localization-files --format apple "$MERGED_FILE" "$OMIM_PATH/iphone/Maps/LocalizedStrings/"
$TWINE generate-all-localization-files --format apple-plural "$MERGED_FILE" "$OMIM_PATH/iphone/Maps/LocalizedStrings/"
$TWINE generate-all-localization-files --format apple --file-name InfoPlist.strings "$OMIM_PATH/iphone/plist.txt" "$OMIM_PATH/iphone/Maps/LocalizedStrings/"
$TWINE generate-all-localization-files --format jquery "$OMIM_PATH/data/countries_names.txt" "$OMIM_PATH/data/countries-strings/"
$TWINE generate-all-localization-files --format jquery "$OMIM_PATH/data/sound.txt" "$OMIM_PATH/data/sound-strings/"

rm $MERGED_FILE
