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

Current source starts distinct base attacks: Donald's attack card casts targeted
magic; Goofy's attack card spins his shield through nearby enemies. Native ROM
compilation and emulator checks are required before treating these as verified.
Assembly, recruitment, upgrades and full reachable-tile overlays remain work.
The existing shared card model will need character loadouts and a separate
summon inventory; existing three fixed party identities cannot represent the
complete roster without a save/state redesign.
