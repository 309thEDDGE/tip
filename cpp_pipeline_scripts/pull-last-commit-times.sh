#!/usr/bin/env bash

for FILE in $(git ls-files)
do
    TIME=$(git log --pretty=format:%cd -n 1 --date=iso -- "$FILE")
    TIME=$(date -d "$TIME" +%Y%m%d%H%M.%S)
    touch -m -t "$TIME" "$FILE"
done

