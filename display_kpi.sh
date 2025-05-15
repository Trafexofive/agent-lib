#!/bin/bash

display_kpi_table() {
  tool=$(jq -r .tool system-kpi.json)
  status=$(jq -r .status system-kpi.json)

  echo "+----------+----------+"
  echo "| Tool     | Status   |"
  echo "+----------+----------+"
  echo "| $tool | $status |"
  echo "+----------+----------+"
}

display_kpi_table
