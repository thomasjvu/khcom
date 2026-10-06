# Play KH Tactics 0.1.0 alpha

Open `build/tactics-us/kh_tactics.gba` in mGBA after building, or apply the
BPS patch to your supported original US ROM with a BPS patcher. The game's
product code is KTCE so it has a distinct identity from the original B8CE.
Use a separate emulator save file for this hack.

## Castle run

On the title screen, Left/Right chooses a seed; A starts a new run. B resumes
a saved run when available, otherwise it shows the controls. Each seed fixes
the branching room types, terrain and reward/draw sequence for the same actions.

Choose one of two rooms using Up/Down and A. Fight, rest, claim rewards and
reach the boss on each of three floors. HP and deck carry between rooms;
losing all HP ends the run. After victory, choose one of three cards or +2
maximum HP. A full deck converts card rewards into power, up to +10.

## Combat controls

| GBA button | Action |
| --- | --- |
| D-pad | Move the cursor or menu selection |
| A in move mode | Move to the selected reachable cell |
| B | Switch move/card mode; cancel an active confirmation |
| L/R in card mode | Select a hand card |
| L+R in card mode | Toggle three-card sleight |
| A in card mode | Preview; A again commits the card |
| Start | Open end-turn/reload menu |
| Select | Suspend menu; A saves and returns to title, B cancels |

You may move up to three orthogonal cells and play one card in either order
per player phase. Blue cells are reachable. Red cells are enemy attack zones;
small red outlines show their planned destinations. Movement targets are
locked before your phase, so moving out of the red zone dodges an attack.
The right HUD shows the cursor target's HP/value and threatened damage.
Enemies attack in a stable order. M+/C+ means the move/card budget is
available; a minus indicates it is spent.

- **Key:** adjacent attack. **Fire:** target within four cells.
- **Cure:** restores HP to Sora. **Guard:** absorbs damage during the next enemy phase.
- An attack's value must meet or beat the enemy's value. Value zero always breaks.
  A successful break also cancels that enemy's next intent; a lower value spends
  the card/action without damage.
- A **sleight** combines the selected card with the first two other hand cards.
  It hits the target and adjacent cells within range four and always breaks.
  The selected card exhausts for that encounter; the others go to discard.
- **Reload** returns discards to the draw pile and fills the hand, spending the
  card action. Exhausted cards return when the next encounter begins.
- End-turn refills the hand from the draw pile. No discards reload automatically.

Shadows chase and strike; Red Nocturnes threaten a small fire area;
Wight Knights move slowly and cleave; Neoshadow bosses threaten a larger cross.
This alpha uses a small content pool, with stronger enemies on later floors.

## Saving

Select opens suspend from combat, the route screen or rewards. Saving stores
HP, deck, room graph, current board/hand/intents, seed and RNG state. On the
title screen press B to continue. Two checksum-protected slots retain a
fallback if a write or the newest slot is damaged. Starting a new run does
not replace the previous suspend until you save the new run.
