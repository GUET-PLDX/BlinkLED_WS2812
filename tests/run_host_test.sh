#!/usr/bin/env bash
set -euo pipefail
module_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
workspace="${module_dir}"
while [[ ! -d "${workspace}/Middlewares/Third_Party/LibXR" && "${workspace}" != / ]]; do
  workspace="$(dirname "${workspace}")"
done
libxr_dir="${LIBXR_ROOT:-${workspace}/Middlewares/Third_Party/LibXR}"
[[ -d "${libxr_dir}/src" ]] || { echo 'Set LIBXR_ROOT to a LibXR checkout' >&2; exit 1; }
binary="$(mktemp "${TMPDIR:-/tmp}/ws2812-test.XXXXXX")"
trap 'rm -f "${binary}"' EXIT
includes=(-I"${module_dir}" -isystem "${libxr_dir}/system/linux"
          -isystem "${libxr_dir}/lib/Eigen")
while IFS= read -r directory; do includes+=(-isystem "${directory}"); done < <(find "${libxr_dir}/src" -type d)
"${CXX:-c++}" -std=c++20 -Wall -Wextra -Werror -pedantic \
  -DXR_LOG_MESSAGE_MAX_LEN=64 -DLIBXR_DEFAULT_SCALAR=float \
  "${includes[@]}" "${module_dir}/tests/blink_led_ws_test.cpp" -o "${binary}"
"${binary}"
echo 'PASS: WS2812 GRB encoding, bit order and reset padding'
