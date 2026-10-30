# Legacy hacks balance research

What TheSmithPlays' three Legacy hacks changed compared with the original games, for the 151 Gen 1 Pokémon and the boss level curves. This is reference material for balancing, not game data, and nothing here is wired into the build.

| File | Contents |
| --- | --- |
| `crossref.md` | Gen 1 stat and evolution changes from all three hacks side by side; start here |
| `emerald_legacy.md` / `.json` | [Emerald Legacy](https://github.com/cRz-Shadows/Pokemon_Emerald_Legacy) (commit `6465639`) vs [pret/pokeemerald](https://github.com/pret/pokeemerald) (`731ad5b`): stats, abilities, evolutions, learnsets, TM and tutor lists, boss parties |
| `crystal_legacy.md` / `.json` | [Crystal Legacy](https://github.com/cRz-Shadows/Pokemon_Crystal_Legacy) (`77e6b38`) vs [pret/pokecrystal](https://github.com/pret/pokecrystal) (`619f370`): the same, plus move data |
| `yellow_legacy.md` / `.json` | [Yellow Legacy](https://github.com/cRz-Shadows/Pokemon_Yellow_Legacy) (`15169d1`) vs [pret/pokeyellow](https://github.com/pret/pokeyellow) (`91d503b`): the same, plus move data and the type chart |
| `scripts/` | The extraction scripts |

Credit TheSmithPlays and the Legacy teams for any changes we adopt.

## Rerunning

Clone each hack and its base into one folder with these names: `emerald_legacy` + `pokeemerald`, `crystal_legacy` + `pokecrystal`, `yellow_legacy` + `pokeyellow`. Then pass that folder to each script, for example `python3 scripts/extract.py <folder>`. `crossref.py` reads the JSON files in this folder.

## Notes

- Yellow Legacy is Gen 1, so it has a single Special stat instead of Sp. Atk and Sp. Def.
- Yellow and Emerald Legacy keep their generation's original rules (no physical/special split, no Fairy type), so some of their changes compensate for problems modern rules already fix.
