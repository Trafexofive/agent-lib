#!/bin/bash
PARAMS_JSON="$1"
TARGET_PATH=$(echo "$PARAMS_JSON" | jq -r .path)
if [ -z "$TARGET_PATH" ] || [ "$TARGET_PATH" == "null" ]; then echo '{"error":"path parameter missing"}'; exit 1; fi
if [ ! -e "$TARGET_PATH" ]; then echo "{\"error\":\"Path not found: $TARGET_PATH\"}"; exit 1; fi
TYPE=$(if [ -f "$TARGET_PATH" ]; then echo "file"; elif [ -d "$TARGET_PATH" ]; then echo "directory"; else echo "other"; fi)
SIZE=$(stat -c%s "$TARGET_PATH")
MTIME=$(stat -c%Y "$TARGET_PATH")
MTIME_HUMAN=$(date -d@"$MTIME" --iso-8601=seconds)
PERMS=$(stat -c%A "$TARGET_PATH")
OWNER=$(stat -c%U "$TARGET_PATH")
GROUP=$(stat -c%G "$TARGET_PATH")
jq -n --arg path "$TARGET_PATH" --arg type "$TYPE" --argjson size "$SIZE" \
      --argjson mtime_unix "$MTIME" --arg mtime_iso "$MTIME_HUMAN" \
      --arg perms "$PERMS" --arg owner "$OWNER" --arg group "$GROUP" \
      '{path: $path, type: $type, size_bytes: $size, modified_at_unix: $mtime_unix, modified_at_iso: $mtime_iso, permissions: $perms, owner: $owner, group: $group}'
