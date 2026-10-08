# KH Tactics 0.25 target-menu development patch

Apply `kh-tactics-0.25-target-menu-dev.bps` to the original US ROM with SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`. The 71243-byte BPS round-trip
reproduces the tested 32 MiB native ROM byte-for-byte. Native source b0530f892;
save format 12; reserved EWRAM 8140/8192 bytes.

Retains Rally and the five-hero roster, original-art procedural worlds, card
combat, Cloud recruitment and the tactical command panel. Skills now browses
cards before opening target confirmation. Up/Down cycles eligible Cure or
ranged targets, A confirms and B returns without spending. Recovery/damage
previews use the authoritative native calculation; ranged attacks with no
eligible target remain uncommitted. Up stocks a card and Down clears stock in
the Skills selection menu. Three stocked cards open sleight confirmation,
with existing per-actor recovery/damage previews. Attack routes a stocked hand
to Skills rather than unexpectedly executing a recipe.

On this exact ROM, 23 focused native checks pass for Rally's 18-point Cure,
target cycling/cancel safety, empty-target ranged rejection, selected-enemy
Fire damage and menu-driven Rally Curaga with independent party healing caps.
These use explicit health/card/enemy placement and HP fixtures, followed by
native menu input. Another 14 input-only checks pass for Rally deployment,
menu/preview resource accounting and menu suspend/reset. Build/header/capacity
checks and the five-hero retry-policy mock pass. The campaign verifier snapshots
all 39 meaningful roster bytes and expects starter unlock mask 23 on retry.
Current campaign evidence is recorded separately in the local manifest.

This remains unfinished. Dedicated Rally action/air artwork, directional
refinement, portrait/card/recruit encounter, richer movesets, fuller menu
presentation, integrated walking/climb/jump routes, broader physical map/seed
validation and hardware verification remain open. Historical complete-run
proofs retain their older ROM hashes; focused menu checks do not prove a full
campaign or all procedural maps. The full polish goal remains active.

The fresh-SRAM input-only campaign verifies Cloud recruitment13431 and
deployment15810, suspend15825 and exact resume16155. Traverse Town completes
at47407 and Agrabah at96003. The replay reaches Castle room5 before exceeding
the120000-frame bound (FAIL120001,17 kills,766 movement commands). No full
victory is claimed for this ROM. The failure records incomplete bounded
coverage, not evidence that Castle is physically impossible.
