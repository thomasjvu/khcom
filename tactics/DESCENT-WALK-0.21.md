# KH Tactics 0.21 descent-and-walking development patch

Apply `kh-tactics-0.21-descent-walk-dev.bps` to the original US ROM (SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`). Verified byte-exact BPS round-trip;
32 MiB native ROM, suspend format 11, 8180 bytes of the reserved 8192-byte EWRAM.
Native source commit: `48a3482ccc905296812421893a02cdd501ecde00`.

This build retains procedural original-art Traverse Town, Agrabah and Castle
rooms, separately controlled Sora/Donald/Goofy, distinct attacks, round-start
party cards, optional Cloud recruitment, personal powers/sleights and bosses.
While attached to stairs, L+Up/Down opens height selection and R toggles the
landing-floor cursor. D-pad selects a reachable tile; A descends and walks to it
as one paid route. Original animations execute each segment. Actual landing
geometry is revalidated before walking; unstarted blocked segments refund.
B cancels without spending resources. Reach diamonds and original card digits
show affordability and the combined cost. See `tactics/PLAY.md` for controls.

The exact ROM passes 113 focused native checks: 27 composed route/cursor
switch/cancel/save/reset/party-resource checks and 86 existing stair regressions.
These use explicit approach/occupancy position fixtures and native controls.
A separate fresh-SRAM input-only replay visits a real generated staircase and
verifies combined descent/walking at frame 1308 with cost 2 and action preserved.
Cloud is recruited at 13423 and deployed at 15001; suspend at 15015 and reset
verification at 15345 preserve run state. Three-world victory occurs at 106696
with Sora HP80; stable PASS 106816, 16 kills and 745 movement commands.
This replay never writes emulated memory or forces exits. The local manifest
records exact ROM, patch, driver and log hashes. Host sanitizer checks pass.
Three earlier campaign victories failed required composition coverage because
their navigator never visited attached stairs; their failed logs are preserved.

This remains an unfinished development build. Complete standing-surface
walk/climb/jump composition, physical generation validation across seeds,
further recruits and content, richer loadouts, companion balance and hardware
verification remain open. Current full-run proof covers one seed and does not
include every optional room/chest. Aerith assets exist but recruitment remains
unimplemented; Tifa assets have not been located. The full polish goal is active.
