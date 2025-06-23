#!/bin/bash
PARAMS_JSON="$1"
DIR_PATH=$(echo "$PARAMS_JSON" | jq -r .path)
if [ -z "$DIR_PATH" ] || [ "$DIR_PATH" == "null" ]; then echo '{"error":"path parameter missing"}'; exit 1; fi
if [ ! -d "$DIR_PATH" ]; then echo "{\"error\":\"Directory not found: $DIR_PATH\"}"; exit 1; fi
find "$DIR_PATH" -maxdepth 1 -mindepth 1 -printf '%p\0%y\0%s\0%M\0%TY-%Tm-%Td %TH:%TM:%.2TS\0' | \
  jq -R -s 'split("\u0000") | . as $a |
    reduce range(0; length/5) as $i ([]; . + [{
      "name": ($a[$i*5] | ltrimstr("'$DIR_PATH'/") | ltrimstr("$DIR_PATH")) ,
      "path": $a[$i*5],
      "type": (if $a[$i*5+1] == "f" then "file" elif $a[$i*5+1] == "d" then "directory" else "other" end),
      "size_bytes": ($a[$i*5+2] | tonumber? // null),
      "permissions": $a[$i*5+3],
      "modified_at": $a[$i*5+4]
    }]) | map(select(.name != ""))'
