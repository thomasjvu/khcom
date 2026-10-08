# Rally card development patch 0.33

Apply `build/release/kh-tactics-0.33-rally-card-dev.bps` to the original US
ROM SHA1 `10729bd884f8fdca7a310b6d606c52e46657aa48`. Patch size89478 bytes;
application reproduces the built32MiB ROM byte-identically. ROM SHA256
`64bb02ac04b1e662fc839bbd5c4c3479a626f856cd02f28e752658364c661505`.

Rally now has original recruitment portrait art in assembly, alongside the
original game character cards. Its32x32 OBJ and16-entry palette reuse the
setup's borrowed combat-card banks. Field sprites and compact menu remain.
Native23 checks verify setup/menu/deployment/suspend/reset, exact512 tile
bytes and32 palette bytes uploaded to the allocated hardware banks, and
setup resources freed on deployment. Setup screenshot inspected. Host suite
and30-frame Rally golden verifier pass. Native reserved RAM8140/8192.

This is a development patch. Complete0.32 campaigns are historical proof on
a different exact ROM. Fresh two-seed and all-room tests on0.33 are running;
no complete0.33 campaign is claimed yet. The second seed's repeated connector
jump policy now explores alternatives; this is test-driver work and does
not alter native collision or guarantee universal procedural reachability.
Rally hurt/climb/casting poses and wider content/balance polish remain open.
