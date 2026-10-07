# Party tactics and recruitment requirements

User direction, 2026-10-07. These requirements extend the full polish goal.

Each room encounter is a round; each world boss ends a set of rounds. This is
an implementation assumption until corrected by the user.

- Control one selected deployed character at a time; switch freely among
  eligible members while preserving individual position, movement and action.
- Give every character its own moveset and original sprite animations. Donald
  uses magic/healing, Goofy shield attacks/protection, Sora Keyblade/sleights.
- Add a recruitable roster, starting with Cloud, whose original battle sprites
  and summon card are present. Fight him before recruitment. Confirm the actual
  asset inventory before adding Aerith, Tifa or other candidates.
- Summon cards belong to round-start party assembly, not in-combat summon
  attacks. Select a deployed party from unlocked character cards before each
  encounter, then commit deployment and start tactical play.
- Show the selected character's reachable movement tiles on the original 2.5D
  geometry, including occupied spaces, height, climb and jump connections.
- Offer boss rewards: a recruit/summon card for appropriate character bosses,
  or a regular roguelike powerup. Every other encounter offers a sleight unlock
  or small upgrade assigned to a party member.
- Persist roster, deployment, unlocks and character upgrades in suspend saves;
  validate all assembly/reward transitions and full-run reset/recruitment.

## Current implementation

Sora, Donald and Goofy are individually controllable with separate movement
and action budgets. Donald uses ranged magic and stronger healing; Goofy uses
shield spins and protection. Cloud uses a targeted sword slash and his original
battle animations. Character power and matching-sleight upgrades apply in combat.

Round setup shows original character cards. L/R selects a deployed slot;
Up/Down changes an unlocked companion; A/Start confirms. Sora remains required.
Companion swaps preserve each character's health and spent turn resources.
Summon cards appear in setup and recruitment; combat Guard uses the original
Guard Armor enemy card. Walking previews show reachable terrain tiles. Attached stair previews show both affordable vertical directions and clamp
the final descent to the supporting floor. Thirty-two native fixture checks
pass, including exhausted movement, cancellation and attached suspend/resume;
markers are visually verified in `climb-reach-visible-evidence`. Routes combining
walking, climbing and jumping still need integration into the reachable overlay.

Traverse Town's optional room-9 Cloud challenge offers his recruitment card or
a personal power/sleight upgrade. Every second unique room clear and world boss
also offers a personal upgrade; revisits cannot duplicate rewards. Cloud can
replace Donald or Goofy in either companion slot after recruitment.

Suspend format 10 stores roster identities, deployment, personal upgrades,
bench health and turn resources. Recorded format-8 and format-9 saves migrate.
Sixteen native deployment checks verify locked-character rejection, controller
selection, original art, Cloud damage, bench health/action and reboot recovery.
Both Cloud challenge reward choices have fifteen native fixture checks.

A fresh-SRAM input-only run using the starter party completes all three worlds,
including suspend/reset with the complete roster comparison: victory at frame
108366, stable terminal PASS at 108486, 14 kills and 899 movement commands.
Evidence: `build/tactics-us/cloud-party-map-suspend-full-run-evidence`.
Fixtures are distinct from full-run proof. An additional fresh input-only replay fights and recruits Cloud, deploys him,
saves at frame 13411 and verifies the complete resumed roster at 13741. It
completes all three worlds with 15 kills and 777 movement commands, stable PASS
at frame 92877. Evidence: `cloud-recruit-resume-full-run-evidence`. Cloud and
Goofy are KO later in the run; this proves route completion and persistence,
not balanced companion survival or a Cloud-led combat strategy.

World transitions now rest all recruited heroes, including the bench: full
health, revival, three movement points and one action. Rest preserves identities,
recruitment and upgrades, requires assembly phase, and does not occur on ordinary
room travel. Host checks and native build pass. A fresh input-only Cloud-route replay
verifies suspend/reset and full deployed HP (80,72,72) at both later world
entries. It wins at frame 93534 with Sora HP80 and reaches stable PASS at
93654: 15 kills, 769 movement commands. Evidence:
`world-rest-cloud-full-run-evidence`. A two-run replay uses native retry to advance the seed, recruits/deploys Cloud
and verifies suspend/reset in both runs. Victories at frames 93487 and 230989
end with Sora HP80; stable final PASS is 231109, with 1875 cumulative movement
commands. Evidence: `two-seed-rest-cloud-evidence`.

A fresh input-only full-route replay visits all twelve rooms in each world,
opens nine chests, recruits/deploys Cloud and verifies suspend/reset. All room
masks are 4095; victory at frame 252406 ends with Sora HP80, and terminal
PASS is 252526 (32 kills, 2014 movement commands). Evidence:
`all-rooms-facing-chests-evidence`. This is one-seed coverage, not a physical
room-generation guarantee.

## Remaining scope

- Verify more party compositions, companion survival and character-led combat.
- Integrate height traversal into reachable movement previews and verify physical
  procedural-room connectivity across seeds and optional branches.
- Improve character loadouts and tactical guidance. Capped power and owned
  sleights have readable warnings; seven native checks verify rejection preserves
  the pending reward and an available choice applies once. Screenshots and log:
  `build/tactics-us/reward-availability-evidence`.
- Add further recruitable characters after checking original assets. Aerith's
  field sprites (`gEarF00`, `gEarB00`, `gEarF01`, `gEarB01`, `gEarisPalette`)
  and portrait card (`gCardNpcEx06`) exist in the decomp. No Tifa assets have
  been located; neither character is currently recruitable.
- Broaden encounter/content coverage and package a verified current BPS patch.
  The local 0.18 development BPS includes party assembly, recruitment and
  world recovery, with byte-exact apply verification and an exact-ROM successful
  Cloud-route replay. Full release readiness remains unproven.

Current source stores per-hero facing in suspend format 11, preserving direction
when switching or benching a character. Eight input-only switching/resume checks
pass (`hero-facing-save-evidence`). The corrected Cloud fixture passes eighteen
checks, including actual bench identity and Donald health/action/facing after
resume (`cloud-bench-facing-evidence`). The older sixteen-check fixture had
Donald deployed again and did not prove its claimed bench-specific behavior.
Recorded format-10 saves migrate with default facing and byte-exact format-11
re-encoding; a recorded format-9 save still decodes. Format-8 support is retained
in the decoder. A full Cloud-route regression passes in
`hero-facing-cloud-full-run-evidence`: format-11 suspend/reset compares the
complete 33-byte roster, and stable three-world PASS occurs at 88532
(15 kills, 730 movement commands). The packaged 0.19 BPS uses format 10 and
predates this direction-persistence change.

The height-route foundation now searches separate movement/action resource
states for up to 162 standing surfaces. Directed walking, climbing and jumping
links can therefore retain a longer walking route that saves the action needed
for a later jump. Invalid geometry callbacks invalidate the result; path
extraction is bounded. The native ROM builds and host sanitizer checks compare
all resource-state costs against an independent relaxation solver on 1,000
generated graphs, alongside explicit mixed walk/climb/jump cases.
Walking previews and controller execution now use this core with native
collision/occupancy links. Only affordable routes are selectable. Geometry and
budget changes invalidate the preview cache; A always revalidates geometry.
Integer cost-level scans, lazy geometry links and boundary-node pruning avoid
rebuilding expensive collision probes during unchanged cursor inspection.
Eight overlay checks and nine crossing/execution checks pass in the
`resource-planner-cached-reach-evidence` and
`resource-planner-cached-crossing-evidence` directories. The route fixture now
confirms assembly before movement and uses the new 16-bit path node layout;
all 33 checks pass (`resource-planner-route-execution-evidence`). Its first 27
checks use controller input; the final six use explicit frozen/progress fixtures
to verify timeout and movement refunds. Earlier slow preview attempts and
stopped full runs remain preserved.

A fresh input-only three-world Cloud replay on the walking-integration ROM
at `22c01c96f` recruits
and deploys Cloud, suspends at 16701, and verifies reset restoration at 17031.
Victory occurs at 94943 with Sora HP80; stable terminal PASS is 95063
(15 kills, 654 movement commands), recorded in
`resource-planner-cached-cloud-full-run-evidence`. This is one seed and does
not cover every optional branch or chest. Walking and attached-stair previews
remain separate: combined standing-surface walk/climb/jump links and route
animation execution are still unfinished. The packaged 0.19 BPS predates these changes.

Current source connects attached-stair links to the resource-state planner.
Up/Down can select several vertical segments, with reachable heights and total
movement cost. The original controller advances through each segment and
completes floor/top boundary transitions; unstarted segments are refunded if
the controller ends the route early. Descent predicts the native landing
offset, and party/enemy occupancy blocks vertical routes. HUD labels distinguish
an unavailable route from a zero-cost origin.

The exact current ROM passes 25 multi-segment climb/descent/save/reset checks,
32 single-step regression checks, 15 party/enemy occupancy checks and 14
top-platform landing/cost checks in
`multi-climb-polished-evidence`, `climb-polished-regression-evidence` and
`climb-occupancy-evidence` and `climb-top-boundary-evidence`. Stair approach/actor placements are explicit
fixtures; movement, route confirmation and execution use native input. The
multi-segment scenario also verifies exact attached-height reset restoration,
three-level floor landing position, costs and party switching after descent.
Screenshots were visually inspected. Earlier failed attempts are preserved,
including a fixture requesting a taller stair than this generated room has
and a real controller boundary issue fixed by holding the final direction
until landing. Full walking/climbing/jumping route composition and physical
generation guarantees remain unfinished.

A fresh input-only Cloud recruitment/deployment replay on this climb-route ROM
suspends at 14442, verifies reset at 14772, and completes all three worlds.
Victory is 92563 with Sora HP75; stable PASS is 92683 (15 kills, 649 movement
commands), recorded in `multi-climb-cloud-full-run-evidence`. This one-seed
route does not cover every optional room or chest, and it does not establish
physical generation guarantees across seeds.

Round setup now displays the highlighted hero's own health and turn budgets,
role, personal power and owned matching-sleight bonuses. Twenty-six native
rendered checks in `assembly-distinct-budgets-evidence` cover all four heroes,
Cloud replacement, independent budgets, upgrade isolation and deployment.
The test reads uploaded VRAM glyphs and uses explicit budget, upgrade and
recruitment fixtures; it does not prove fresh-run Cloud recruitment. Earlier
staging-buffer inspection failures remain recorded and were corrected to read
the actual display. These UI checks belong to a newer ROM than packaged 0.21;
that patch and its complete-run proof retain their original exact ROM hash.
