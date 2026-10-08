# KH Tactics 0.26 command-menu development patch

Apply `kh-tactics-0.26-command-menu-dev.bps` to the original US ROM with SHA-1
`10729bd884f8fdca7a310b6d606c52e46657aa48`. The74304-byte patch reproduces
the tested32MiB ROM byte-for-byte. Native source66fa4d42b; replay source
fde748ebd; save format12; reserved EWRAM8140/8192bytes. Patch and exact evidence
hashes are in the local `build/release/command-menu-0.26-manifest.json`.

Select opens tactical Commands. Move offers reachable walking previews and
R→Jump confirmation with visible direction/cost (no landing preview yet).
Attack and Skills confirm before spending; Cure/ranged targets show recovery/
damage previews. Skills supports stocking, sleights and Reload confirmation.
Party directly selects deployed heroes with visible health/resources. End Turn
shows remaining party budgets before enemies act. L+Select remains quick hero
cycling; Start+Select suspends. A lighter single-stroke font and contextual
command explanations replace the earlier bold field HUD presentation.

Rally uses the approved original pink-haired sprites, her own roster identity,
64HP and a four-point recovery bonus. Front/back animation and mirrored facing
use the consistent draft rows; inconsistent opposite-facing rows remain
preserved but unused. Dedicated action/air art, card, encounter and richer
moveset remain unfinished.

On this exact ROM,98 focused native checks pass:10 Attack,23 Skills/targets/
sleights,9 Reload,10 Jump,8 End Turn,26 Rally/menu/save/facing and12 direct
party-selection/resource checks. Attack/target/reload fixtures explicitly set
cards/health/enemies as documented; Jump/turn/Rally/selector checks are input-only.
Build/header/capacity and preferred-preview/retry policy checks pass. Menu and
facing screenshots were inspected.

A fresh-SRAM input-only three-world campaign passes Cloud recruitment12711/
deployment15168, suspend15182/exact resume15512 and composed descent. Worlds
advance31379/68946; victory98474 has Sora80HP and terminal state remains stable
throughPASS98594 (15kills,745movement commands). The120000-frame bound is
unchanged. A prior same-ROM driver timeout is preserved separately; retaining
a native-valid preferred preview eliminates redundant scan/reopen input, while
native confirmation still revalidates every route. This is one main-path seed.
Older all-room/chest evidence belongs to other ROMs and is not current proof.

This is a development patch, not the completed polished ROM hack. Wider seeds,
branches/chests, integrated height destination routing, physical generation
validation, further recruits/content, balance, Rally art/content, final UI
polish and hardware verification remain open. The full goal remains active.
