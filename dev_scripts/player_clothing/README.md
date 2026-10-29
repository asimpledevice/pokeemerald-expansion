# Player clothing colours (draft)

Draft palettes that let the player pick a main clothing colour: Red's jacket and hat, or Leaf's hat. Colours: the original (red for Red, white for Leaf), blue, green, yellow, orange, pink, purple and black. Nothing here is wired into the build yet.

- `make_clothing_colours.py` regenerates everything in this folder from the sprites in `graphics/` (and Leaf's hair-split front picture from `../player_skin_tones/`). Run it from anywhere; it needs Pillow.
- `pal/` holds one JASC palette per character, sprite and colour: `<red|leaf>_<overworld|front|back>_<colour>.pal`.
- `sprites/` holds the pictures whose pixels had to move so that only clothing uses the main colour's slots. The palettes for those sprites assume these versions:
  - `red_back_separated.png`: the hair under Red's hat shared the hat's dark red; 283 hair pixels moved to the unused slot 1.
  - `leaf_front_separated.png`: Leaf's socks, shoes and Poke Ball shared the hat's whites; 25 pixels moved to slot 7, freed by merging two near-identical blues on her top (slot 7 into slot 6).
  - `leaf_back_separated.png`: a few stray white pixels (her eye) moved to the unused slot 5.
- `clothing_preview.png` shows every colour on the first walking frames, the front picture and the battle back sprite.

## Notes

- On Leaf's walking sprite her socks change colour with her hat, the same way Red's shoes match his jacket; at that size it reads as an outfit.
- Red's front picture changes his wristband and shoes with his jacket.
- In the game, skin tone and clothing colour would be applied together at load time by writing both ramps into the base palette, rather than storing a palette for every combination.
