#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/tactics
cc -std=c89 -pedantic -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I tactics tactics/tactics.c tactics/save.c tests/tactics_test.c -o build/tactics/rules_test
build/tactics/rules_test
cc -std=c89 -pedantic -Wall -Wextra -Werror -fsanitize=address,undefined \
  -I tactics tactics/worldgen.c tests/tactics_worldgen_test.c -o build/tactics/worldgen_test
build/tactics/worldgen_test
