# KH Tactics 0.23 prop-support development patch

Apply `kh-tactics-0.23-prop-support-dev.bps` to the original US ROM with SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`. The verified patch round-trip
reproduces the tested 32 MiB ROM byte-for-byte. Native source:
`4fcbb990d9ee0d37163e6353f829cb13adeceeb5`. Save format 11 remains compatible;
reserved native EWRAM uses 8132/8192 bytes.

Retains procedural original-art Traverse Town, Agrabah and Castle rooms,
independent Sora/Donald/Goofy control, distinct attacks, original cards,
round-start deployment, Cloud recruitment, personal upgrades, bosses and
attached-stair descent followed by walking. Walking previews now preserve
original scenery top height and the underlying terrain rather than snapping
back to the floor. Jumping onto and leaving props uses the native controller.

Suspend loading restores the original camera and platform contacts before the
player updates. This repairs a verified bug where entrance-camera culling
left a saved Castle pillar disabled long enough for Sora to fall. The saved
position remains exact; regeneration does not substitute another surface.

On this exact ROM, 17 explicit pillar approach/card/save fixture checks verify
jumping, top walking preview and cost, exact suspend/reset position, native
round deployment, far-side floor landing and movement/action accounting.
Read-only readiness-guarded traces confirm native standing support. Another 27
explicit staircase checks verify composed descent/walking, exact suspend
recovery and independent Sora/Donald/Goofy budgets after selection. The native
build/header/capacity checks and complete host sanitizer suite pass. Focused
fixtures do not prove full campaign completion or all generated geometry.

This is an unfinished development build. The second-seed Castle replay still
requires prop-surface connections in its navigator. Combined standing-surface
walk/climb/jump routes, physical procedural-map validation across seeds,
further recruits/content, richer loadouts, companion balance and hardware
verification remain open. Aerith assets exist but recruitment is unimplemented;
Tifa assets have not been located. Older release evidence retains its own ROM
hash and is not treated as current-ROM coverage. Full goal remains active.

A fresh-SRAM input-only campaign on this exact ROM verifies composed descent
and walking, Cloud recruitment at 13924 and deployment at 15610, suspend at 15624
and exact resume at 15954. Three-world victory occurs at 99973 with Sora HP80;
stable PASS 100093, 15 kills and 721 movement commands. Mandatory suspend
coverage reaches stage 3. The local manifest records exact ROM, patch, driver
and log hashes. This covers one seed and does not visit all optional rooms or
chests. An earlier victory on the same ROM skipped the requested save room;
that replay remains recorded as failed requested coverage, and the driver now
requires an actual verified resume before passing.
