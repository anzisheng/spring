#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "usage: $0 <sqlite-db-path>" >&2
  exit 2
fi

db_path=$1

sqlite3 "$db_path" "select ClockStyle, TimeLeadValue from route limit 1;" >/dev/null
