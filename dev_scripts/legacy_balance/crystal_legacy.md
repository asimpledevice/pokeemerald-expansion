# Crystal Legacy vs vanilla Pokémon Crystal — Gen 1 data diff

- Crystal Legacy: `cRz-Shadows/Pokemon_Crystal_Legacy` @ `77e6b3859349698ad0a7a8a93cd27f78860d3e4a`
- Base: `pret/pokecrystal` @ `619f3703486b3b40f69d31b1911b58aa0a721a6e` (Legacy README states it is built on the pret Crystal disassembly; same file layout)
- Method: parsed `data/pokemon/base_stats/*.asm`, `data/pokemon/evos_attacks.asm`, `data/moves/moves.asm`, `data/trainers/parties.asm` in both trees (scripts in `scripts/crystal_compare.py`, `scripts/crystal_report.py`).
- Unparsed / parse issues: none. Species/move constant order is identical in both trees; move names unchanged; type chart (`data/types/type_matchups.asm`) identical.
- Engine note (not data): Legacy changes the physical/special check so **Ghost-type moves are special and Dark-type moves are physical** (`engine/battle/effect_commands.asm`, `cp GHOST -> .special`, `cp DARK -> .physical`). Still a per-type split, no per-move category.
- Trade-evolution items (King's Rock, Metal Coat, Dragon Scale, Up-Grade, Brick Piece) get a party-menu use in `data/items/attributes.asm` (vanilla: NOUSE), price 9000.
- Legacy has a Hard Mode flag (`ENGINE_HARD_MODE`) that sets `BATTLETYPE_SETNOITEMS` for boss fights (seen in ViridianGym/SilverCaveRoom3).

## Counts (Gen 1, #1–151)

- Base stats changed: 23; types changed: 0; catch rate/base exp/growth changed: 0/0/0
- Evolution changed: 11; level-up learnset changed: 134; TM/HM compatibility changed: 63
- Species with any change: 142 / 151

## Base stat changes

Order HP/Atk/Def/SpA/SpD/Spe (Gen 2 split special). BST delta in last column. No Gen 1 type changes.

| # | Species | Vanilla | Legacy | Changed | ΔBST |
|---|---|---|---|---|---|
| 23 | Ekans | 35/60/44/40/54/55 | 40/65/44/40/54/60 | HP 35→40, Atk 60→65, Spe 55→60 | +15 |
| 24 | Arbok | 60/85/69/65/79/80 | 62/95/69/65/79/90 | HP 60→62, Atk 85→95, Spe 80→90 | +22 |
| 25 | Pikachu | 35/55/30/50/40/90 | 35/55/40/50/50/90 | Def 30→40, SpD 40→50 | +20 |
| 36 | Clefable | 95/70/73/85/90/60 | 95/70/73/95/90/60 | SpA 85→95 | +10 |
| 37 | Vulpix | 38/41/40/50/65/65 | 45/41/45/60/70/75 | HP 38→45, Def 40→45, SpA 50→60, SpD 65→70, Spe 65→75 | +37 |
| 38 | Ninetales | 73/76/75/81/100/100 | 78/71/80/86/100/100 | HP 73→78, Atk 76→71, Def 75→80, SpA 81→86 | +10 |
| 40 | Wigglytuff | 140/70/45/75/50/45 | 140/70/55/85/55/45 | Def 45→55, SpA 75→85, SpD 50→55 | +25 |
| 43 | Oddish | 45/50/55/75/65/30 | 50/50/55/75/65/30 | HP 45→50 | +5 |
| 44 | Gloom | 60/65/70/85/75/40 | 70/65/70/85/75/40 | HP 60→70 | +10 |
| 45 | Vileplume | 75/80/85/100/90/50 | 90/80/95/100/90/50 | HP 75→90, Def 85→95 | +25 |
| 49 | Venomoth | 70/65/60/90/75/90 | 70/75/60/95/75/100 | Atk 65→75, SpA 90→95, Spe 90→100 | +25 |
| 51 | Dugtrio | 35/80/50/50/70/120 | 35/90/50/50/70/120 | Atk 80→90 | +10 |
| 77 | Ponyta | 50/85/55/65/65/90 | 50/85/55/65/65/100 | Spe 90→100 | +10 |
| 78 | Rapidash | 65/100/70/80/80/105 | 65/100/70/80/80/115 | Spe 105→115 | +10 |
| 87 | Dewgong | 90/70/80/70/95/70 | 90/60/80/80/95/70 | Atk 70→60, SpA 70→80 | +0 |
| 90 | Shellder | 30/65/100/45/25/40 | 35/65/100/45/25/40 | HP 30→35 | +5 |
| 105 | Marowak | 60/80/110/50/80/45 | 65/80/110/50/80/45 | HP 60→65 | +5 |
| 106 | Hitmonlee | 50/120/53/35/110/87 | 50/120/58/35/110/87 | Def 53→58 | +5 |
| 107 | Hitmonchan | 50/105/79/35/110/76 | 50/105/79/35/110/81 | Spe 76→81 | +5 |
| 108 | Lickitung | 90/55/75/60/75/30 | 95/70/85/75/85/30 | HP 90→95, Atk 55→70, Def 75→85, SpA 60→75, SpD 75→85 | +55 |
| 110 | Weezing | 65/90/120/85/70/60 | 75/90/120/85/70/60 | HP 65→75 | +10 |
| 126 | Magmar | 65/95/57/100/85/93 | 75/85/57/100/85/93 | HP 65→75, Atk 95→85 | +0 |
| 136 | Flareon | 65/130/60/95/110/65 | 65/130/60/110/95/65 | SpA 95→110, SpD 110→95 | +0 |

## Evolution changes

| # | Species | Vanilla | Legacy |
|---|---|---|---|
| 61 | Poliwhirl | POLIWRATH via WATER_STONE; POLITOED via trade holding KINGS_ROCK | POLIWRATH via WATER_STONE; POLITOED via KINGS_ROCK |
| 64 | Kadabra | ALAKAZAM via trade | ALAKAZAM @Lv42 |
| 67 | Machoke | MACHAMP via trade | MACHAMP @Lv38 |
| 75 | Graveler | GOLEM via trade | GOLEM @Lv38 |
| 79 | Slowpoke | SLOWBRO @Lv37; SLOWKING via trade holding KINGS_ROCK | SLOWBRO @Lv37; SLOWKING via KINGS_ROCK |
| 93 | Haunter | GENGAR via trade | GENGAR @Lv42 |
| 95 | Onix | STEELIX via trade holding METAL_COAT | STEELIX via METAL_COAT |
| 117 | Seadra | KINGDRA via trade holding DRAGON_SCALE | KINGDRA via DRAGON_SCALE |
| 118 | Goldeen | SEAKING @Lv33 | SEAKING @Lv28 |
| 123 | Scyther | SCIZOR via trade holding METAL_COAT | SCIZOR via METAL_COAT |
| 137 | Porygon | PORYGON2 via trade holding UP_GRADE | PORYGON2 via UP_GRADE |

Unchanged (relevant): Golbat→Crobat and Chansey→Blissey stay happiness (TR_ANYTIME, threshold 220 in both); Eevee item/happiness evolutions unchanged; Pichu/Cleffa/Igglybuff happiness unchanged; Smoochum/Elekid/Magby Lv30 unchanged.
- Baby with change: Tyrogue: HITMONCHAN @Lv20 if ATK_LT_DEF; HITMONLEE @Lv20 if ATK_GT_DEF; HITMONTOP @Lv20 if ATK_EQ_DEF → HITMONTOP via BRICK_PIECE; HITMONCHAN @Lv20 if ATK_LT_DEF; HITMONLEE @Lv20 if ATK_GT_DEF

## Level-up learnset changes

Totals across Gen 1: 299 move additions, 11 removals, 475 level changes (mostly STAB moves earlier; evolved forms keep pre-evo moves at Lv1 instead of re-learning late). Most-added: Sludge Bomb 14, Body Slam 13, BubbleBeam 12, Flame Wheel 9, Sludge 9, Water Gun 9, Rock Slide 9, Tackle 8, Fire Blast 7, Mega Drain 7, Psychic 7. Removed: Swift 4, Poison Sting 2, Leer 2, Dragon Rage/Defense Curl/Slam 1 each.

Format: `+MOVE Lv` added, `-MOVE Lv` removed, `MOVE a→b` level changed (Lv1 = start move).

| # | Species | Added | Removed | Level changed |
|---|---|---|---|---|
| 2 | Ivysaur |  |  | Growl 1/4→1, Leech Seed 1/7→7, Razor Leaf 22→20, Sweet Scent 29→25, Growth 38→32, Synthesis 47→39, Solarbeam 56→46 |
| 3 | Venusaur | Body Slam 45, Ancientpower 50 |  | Growl 1/4→1, Leech Seed 1/7→1, Vine Whip 1/10→1, Razor Leaf 22→20, Sweet Scent 29→25, Growth 41→32, Synthesis 53→39, Solarbeam 65→46 |
| 4 | Charmander | Flame Wheel 19, Body Slam 44, Fire Blast 52 |  | Rage 19→15, Scary Face 25→27, Flamethrower 31→34, Slash 37→40, Dragon Rage 43→48, Fire Spin 49→24 |
| 5 | Charmeleon | Flame Wheel 19, Body Slam 44, Fire Blast 52 |  | Ember 1/7→1, Rage 20→15, Slash 41→40, Fire Spin 55→24 |
| 6 | Charizard | Flame Wheel 19, Body Slam 45, Outrage 48, Fire Blast 52 | Dragon Rage 54 | Ember 1/7→1, Smokescreen 1/13→1, Rage 20→15, Slash 44→40, Fire Spin 64→24 |
| 7 | Squirtle | Body Slam 41, Ice Beam 47 |  | Protect 28→29, Rain Dance 33→35, Skull Bash 40→51, Hydro Pump 47→56 |
| 8 | Wartortle | Body Slam 41, Ice Beam 47 |  | Tail Whip 1/4→1, Bubble 1/7→1, Bite 19→18, Rapid Spin 25→23, Protect 31→29, Rain Dance 37→35, Skull Bash 45→51, Hydro Pump 53→56 |
| 9 | Blastoise | Body Slam 41, Ice Beam 47 |  | Tail Whip 1/4→1, Bubble 1/7→1, Withdraw 1/10→1, Bite 19→18, Rapid Spin 25→23, Protect 31→29, Rain Dance 42→35, Skull Bash 55→51, Hydro Pump 68→56 |
| 11 | Metapod | Tackle 1, String Shot 1 |  |  |
| 12 | Butterfree | Leech Life 12 |  |  |
| 14 | Kakuna | Poison Sting 1, String Shot 1 |  |  |
| 15 | Beedrill | Swords Dance 30 |  | Fury Attack 1/10→1, Focus Energy 15→12, Twineedle 20→15, Rage 25→10, Pursuit 30→18, Pin Missile 35→10 |
| 16 | Pidgey | Mud Slap 30, Sky Attack 47 |  | Gust 9→7, Quick Attack 15→13, Whirlwind 21→19, Wing Attack 29→25, Agility 37→42, Mirror Move 47→55 |
| 17 | Pidgeotto | Mud Slap 30, Sky Attack 47 |  | Sand Attack 1/5→1, Gust 1/9→1, Quick Attack 15→13, Whirlwind 23→19, Wing Attack 33→25, Agility 43→42 |
| 18 | Pidgeot | Mud Slap 30, Extremespeed 36, Sky Attack 47 |  | Sand Attack 1/5→1, Gust 1/9→1, Quick Attack 1/15→1, Whirlwind 23→19, Wing Attack 33→25, Agility 46→42, Mirror Move 61→55 |
| 19 | Rattata | Crunch 45 |  | Focus Energy 20→25, Pursuit 27→20, Super Fang 34→40 |
| 20 | Raticate | Focus Energy 25, Crunch 45 |  | Quick Attack 1/7→1 |
| 21 | Spearow |  |  | Fury Attack 13→10, Pursuit 25→15 |
| 22 | Fearow |  |  | Leer 1/7→1, Fury Attack 1/13→1, Pursuit 26→15, Mirror Move 32→31, Drill Peck 40→37, Agility 47→43 |
| 23 | Ekans | Sludge 27, Sludge Bomb 36 |  | Glare 23→25, Screech 29→30, Acid 37→18 |
| 24 | Arbok | Substitute 22, Sludge 27, Sludge Bomb 36 |  | Poison Sting 1/9→1, Bite 1/15→1, Screech 33→30, Acid 43→18, Haze 51→43 |
| 26 | Raichu | Growl 1, Thunder Wave 1, Double Team 15, Slam 20, Agility 33, Thunder 41, Light Screen 50 |  | Quick Attack 1→11, Thunderbolt 1→26 |
| 27 | Sandshrew | Mud Slap 9, Rollout 15, Metal Claw 19, Cut 30, Earthquake 40 | Poison Sting 17, Swift 30 | Slash 23→25, Fury Swipes 37→1, Sandstorm 45→35 |
| 28 | Sandslash | Mud Slap 9, Rollout 15, Metal Claw 19, Dig 22, Cut 30, Earthquake 40, Spike Cannon 48 | Poison Sting 17, Swift 33 | Defense Curl 1/6→6, Sand Attack 1/11→11, Slash 24→25, Fury Swipes 42→1, Sandstorm 52→35 |
| 30 | Nidorina |  |  | Scratch 8→1, Poison Sting 19→17, Tail Whip 27→23, Bite 36→30, Fury Swipes 46→38 |
| 31 | Nidoqueen | Magnitude 27, Sludge Bomb 36 |  |  |
| 33 | Nidorino |  |  | Poison Sting 19→17, Focus Energy 27→23, Fury Attack 36→30, Horn Drill 46→38 |
| 34 | Nidoking | Magnitude 27, Sludge Bomb 36 |  |  |
| 35 | Clefairy | Body Slam 35 |  | Encore 4→6, Metronome 34→30 |
| 36 | Clefable | Pound 1, Growl 1, Encore 6, Minimize 19, Defense Curl 26, Body Slam 35, Light Screen 53 |  | Sing 1→8, Doubleslap 1→13, Metronome 1→30, Moonlight 1→43 |
| 37 | Vulpix | Flame Wheel 20, Shadow Ball 37, Sunny Day 40, Fire Blast 55 |  | Confuse Ray 19→16, Flamethrower 31→32, Fire Spin 37→45 |
| 38 | Ninetales | Tail Whip 1, Roar 1, Flame Wheel 20, Flamethrower 32, Shadow Ball 37, Sunny Day 40, Fire Blast 55 |  | Confuse Ray 1→16, Safeguard 1→25, Fire Spin 43→45 |
| 39 | Jigglypuff | Charm 9, Lovely Kiss 24 |  | Defense Curl 4→6, Pound 9→1, Doubleslap 24→16 |
| 40 | Wigglytuff | Pound 1, Charm 9, Rollout 19, Lovely Kiss 24, Rest 29, Body Slam 34, Double Edge 39 |  | Disable 1→14, Defense Curl 1→6, Doubleslap 1→16 |
| 41 | Zubat | Gust 7, Sludge 27, Sludge Bomb 36 |  | Supersonic 6→5, Wing Attack 27→23, Mean Look 36→42, Haze 46→48 |
| 42 | Golbat | Gust 1, Sludge 27, Sludge Bomb 36 |  | Supersonic 1/6→1, Wing Attack 30→22, Haze 55→48 |
| 43 | Oddish | Mega Drain 22, Giga Drain 32 |  | Acid 23→24, Moonlight 32→35, Petal Dance 39→44 |
| 44 | Gloom | Mega Drain 21, Giga Drain 32 |  | Sweet Scent 1/7→1, Poisonpowder 1/14→1 |
| 45 | Vileplume | Poisonpowder 1, Sleep Powder 18, Mega Drain 21, Acid 24, Giga Drain 32, Moonlight 35 |  | Petal Dance 1→44 |
| 46 | Paras | Absorb 5, Mega Drain 16 |  | Spore 25→24, Growth 37→36, Giga Drain 43→40 |
| 47 | Parasect | Absorb 1, Mega Drain 16 |  | Stun Spore 1/7→1, Poisonpowder 1/13→13, Spore 28→24, Slash 37→31, Growth 46→36, Giga Drain 55→40 |
| 48 | Venonat | Sludge Bomb 43 |  | Leech Life 25→13, Psybeam 33→34, Sleep Powder 36→38, Psychic M 41→50 |
| 49 | Venomoth | Sludge Bomb 43 |  | Supersonic 1/9→1, Leech Life 25→13, Psybeam 36→34, Sleep Powder 42→38, Psychic M 52→50 |
| 51 | Dugtrio |  |  | Growl 1/5→1, Magnitude 1/9→1, Slash 37→33, Earthquake 49→41, Fissure 61→49 |
| 52 | Meowth | Double Edge 46 |  | Bite 11→1, Screech 35→38, Slash 46→33 |
| 53 | Persian | Double Edge 46, Hyper Beam 50 |  | Faint Attack 29→28, Fury Swipes 46→41, Slash 53→33 |
| 54 | Psyduck | Water Gun 8, Bubblebeam 17, Psychic M 42 |  | Confusion 16→15, Fury Swipes 40→37 |
| 55 | Golduck | Water Gun 1, Bubblebeam 17, Psychic M 42 |  | Tail Whip 1/5→1, Disable 1/10→1, Confusion 1/16→15, Fury Swipes 44→37, Hydro Pump 58→50 |
| 57 | Primeape |  |  | Low Kick 1/9→1, Seismic Toss 36→33, Cross Chop 45→39, Screech 54→45, Thrash 63→51 |
| 58 | Growlithe | Growl 5 |  | Bite 1→9, Ember 9→1, Take Down 26→30, Flame Wheel 34→26, Agility 42→36, Flamethrower 50→35 |
| 59 | Arcanine | Agility 1, Bite 1, Flamethrower 45 | Leer 1 | Roar 1→20 |
| 60 | Poliwag | Mist 5, Bubblebeam 20 |  | Water Gun 13→10, Doubleslap 19→14, Belly Drum 37→43, Hydro Pump 43→48 |
| 61 | Poliwhirl | Mist 1, Bubblebeam 20 |  | Hypnosis 1/7→1, Water Gun 1/13→10, Doubleslap 19→14, Rain Dance 27→25, Body Slam 35→31, Hydro Pump 51→48 |
| 62 | Poliwrath | Bubble 1, Mist 1, Bubblebeam 20, Rain Dance 25, Body Slam 31, Belly Drum 43, Hydro Pump 48 |  | Water Gun 1→10, Doubleslap 1→14 |
| 66 | Machop |  |  | Cross Chop 37→43, Scary Face 43→50, Submission 49→55 |
| 67 | Machoke |  |  | Focus Energy 1/8→1, Karate Chop 15→13, Vital Throw 34→31, Scary Face 52→50, Submission 61→55 |
| 68 | Machamp |  |  | Focus Energy 1/8→1, Karate Chop 15→1, Vital Throw 34→31, Scary Face 52→50, Submission 61→55 |
| 69 | Bellsprout | Sludge 30, Sludge Bomb 41 |  | Sweet Scent 30→33, Razor Leaf 37→21 |
| 70 | Weepinbell | Sludge 30, Sludge Bomb 41 |  | Acid 24→21, Razor Leaf 42→23, Slam 54→45 |
| 71 | Victreebel | Growth 1/1, Wrap 1/11, Poisonpowder 17, Stun Spore 19, Acid 21, Sludge 30, Sludge Bomb 41, Slam 45 |  | Sleep Powder 1→15, Sweet Scent 1→33, Razor Leaf 1→23 |
| 72 | Tentacool | Bubble 10, Sludge 25 |  | Acid 19→16, Bubblebeam 25→20, Barrier 36→37 |
| 73 | Tentacruel | Bubble 1, Sludge 20, Sludge Bomb 42 |  | Supersonic 1/6→1, Constrict 1/12→1, Acid 19→16, Barrier 38→37 |
| 74 | Geodude | Rock Slide 40 |  | Harden 26→27, Rollout 31→34, Earthquake 36→45, Explosion 41→48 |
| 75 | Graveler | Rock Slide 40 |  | Defense Curl 1/6→1, Rock Throw 1/11→1, Earthquake 41→45 |
| 76 | Golem | Rock Slide 40 |  | Defense Curl 1/6→1, Rock Throw 1/11→1, Magnitude 1/16→1, Earthquake 41→45 |
| 77 | Ponyta | Flame Wheel 20, Double Kick 25, Flamethrower 33 |  | Ember 13→11, Stomp 19→17, Fire Spin 26→39, Take Down 34→36, Agility 43→45, Fire Blast 53→51 |
| 78 | Rapidash | Flame Wheel 20, Double Kick 25, Flamethrower 36 |  | Growl 1/4→1, Tail Whip 1/8→1, Ember 1/13→1, Stomp 19→17, Fire Spin 26→39, Take Down 34→33, Agility 47→45, Fire Blast 61→51 |
| 79 | Slowpoke | Psybeam 29 |  | Water Gun 15→13, Confusion 20→18, Disable 29→24, Amnesia 43→41, Psychic M 48→45 |
| 80 | Slowbro | Psybeam 29 |  | Growl 1/6→1, Water Gun 1/15→1, Confusion 20→18, Disable 29→24, Amnesia 46→41, Psychic M 54→45 |
| 81 | Magnemite | Spark 25, Thunderbolt 37 |  | Lock On 27→40, Screech 39→43, Zap Cannon 45→51 |
| 82 | Magneton | Spark 25, Thunderbolt 37 |  | Thundershock 1/6→1, Supersonic 1/11→1, Sonicboom 1/16→1, Lock On 27→40, Tri Attack 35→33, Zap Cannon 53→51 |
| 83 | Farfetch D | Baton Pass 45 |  | Slash 37→35, False Swipe 44→40 |
| 84 | Doduo | Double Edge 56 |  | Rage 25→27, Drill Peck 33→38, Agility 37→44 |
| 85 | Dodrio | Double Edge 56 |  | Tri Attack 21→27, Rage 25→21, Agility 47→44 |
| 86 | Seel | Water Gun 5, Bubblebeam 25, Blizzard 54 |  | Growl 5→1, Ice Beam 37→40, Safeguard 48→44 |
| 87 | Dewgong | Water Gun 1, Bubblebeam 25, Blizzard 54 |  | Growl 1/5→1, Aurora Beam 1/16→1, Ice Beam 43→40, Safeguard 60→44 |
| 88 | Grimer |  |  | Acid Armor 40→45, Sludge Bomb 50→38 |
| 89 | Muk |  |  | Harden 1/33→1, Disable 37→1, Sludge 45→16, Sludge Bomb 60→38 |
| 90 | Shellder | Hydro Pump 50 |  | Clamp 41→35, Ice Beam 49→37 |
| 91 | Cloyster | Tackle 1, Clamp 35, Ice Beam 37, Hydro Pump 50 |  | Protect 1→25, Spike Cannon 41→40 |
| 92 | Gastly | Smog 6 |  |  |
| 93 | Haunter | Smog 1, Shadow Ball 42 |  | Spite 1/8→8 |
| 94 | Gengar | Smog 1, Shadow Ball 42 |  | Spite 1/8→1 |
| 95 | Onix | Dig 18, Rock Slide 30, Earthquake 40 |  | Rock Throw 14→12, Slam 40→49 |
| 96 | Drowzee |  |  | Psych Up 43→47, Future Sight 45→54 |
| 97 | Hypno |  |  | Disable 1/10→1, Confusion 1/18→1, Poison Gas 33→31, Meditate 40→38, Psychic M 49→42, Psych Up 55→47, Future Sight 60→54 |
| 98 | Krabby | Metal Claw 29 |  | Protect 34→37, Crabhammer 41→51 |
| 99 | Kingler | Metal Claw 28 |  | Leer 1/5→1, Vicegrip 1/12→1, Guillotine 27→44, Protect 38→37, Crabhammer 49→27/51 |
| 100 | Voltorb | Spark 19, Thunder 40 | Swift 37 | Light Screen 33→34, Explosion 39→44, Mirror Coat 41→48 |
| 101 | Electrode | Spark 1, Thunder 40 | Swift 40 | Screech 1/9→1, Sonicboom 1/17→1, Selfdestruct 1/23→23 |
| 102 | Exeggcute | Mega Drain 20, Psybeam 28, Giga Drain 32, Psychic M 45 |  | Poisonpowder 31→25, Solarbeam 43→42 |
| 103 | Exeggutor | Giga Drain 1, Reflect 1, Leech Seed 9, Mega Drain 20, Stun Spore 25, Poisonpowder 25, Psybeam 28, Sleep Powder 37, Solarbeam 42, Psychic M 45 |  | Confusion 1→17, Stomp 19→13, Egg Bomb 31→32 |
| 104 | Cubone | Tackle 1 |  | Bone Club 9→8, Rage 29→32, False Swipe 33→15 |
| 105 | Marowak | Tackle 1 |  | Tail Whip 1/5→1, Bone Club 1/9→1, Headbutt 1/13→13, False Swipe 39→15, Thrash 46→37, Bone Rush 53→41 |
| 106 | Hitmonlee |  |  | Meditate 6→20, Rolling Kick 11→20, Jump Kick 16→20, Hi Jump Kick 26→36, Foresight 36→25 |
| 107 | Hitmonchan | Dynamicpunch 38 |  | Agility 7→20, Pursuit 13→20, Mach Punch 32→20, Mega Punch 38→32 |
| 108 | Lickitung | Rollout 17, Curse 25, Body Slam 32, Belly Drum 40 | Defense Curl 13 | Stomp 19→13, Wrap 25→1, Disable 31→28, Slam 37→21 |
| 109 | Koffing | Amnesia 33, Sludge Bomb 36 |  | Selfdestruct 17→15, Sludge 21→19, Smokescreen 25→23, Haze 33→29 |
| 110 | Weezing | Amnesia 33, Sludge Bomb 35 |  | Smog 1/9→1, Selfdestruct 1/17→1, Sludge 21→19, Smokescreen 25→23, Haze 33→29, Explosion 44→41, Destiny Bond 51→45 |
| 111 | Rhyhorn | Rock Throw 19, Magnitude 24, Rock Slide 44 |  | Fury Attack 19→8, Horn Drill 37→58, Take Down 49→51, Earthquake 55→39 |
| 112 | Rhydon | Rock Throw 19, Magnitude 24, Rock Slide 44 |  | Stomp 1/13→1, Fury Attack 1/19→1, Horn Drill 37→58, Take Down 54→51, Earthquake 65→39 |
| 113 | Chansey |  |  | Growl 5→4, Tail Whip 9→7, Softboiled 13→10, Doubleslap 17→13, Minimize 23→18, Sing 29→23, Egg Bomb 35→28, Defense Curl 41→33, Light Screen 49→40, Double Edge 57→47 |
| 114 | Tangela | Giga Drain 37 | Slam 40 | Mega Drain 31→22 |
| 115 | Kangaskhan | Body Slam 40 |  |  |
| 116 | Horsea | Bubblebeam 22 |  | Leer 15→13, Water Gun 22→18, Agility 36→40, Hydro Pump 43→51 |
| 117 | Seadra | Bubblebeam 22 |  | Smokescreen 1/8→1, Leer 1/15→1, Water Gun 1/22→1 |
| 118 | Goldeen | Water Gun 5, Drill Peck 30, Megahorn 35, Hydro Pump 53 |  | Horn Attack 15→12, Flail 24→20, Fury Attack 29→24, Waterfall 38→15, Agility 52→48 |
| 119 | Seaking | Water Gun 1, Drill Peck 30, Megahorn 35, Hydro Pump 53 |  | Tail Whip 1/1→1, Supersonic 10→1, Horn Attack 15→12, Flail 24→20, Fury Attack 29→24, Waterfall 41→15, Horn Drill 49→43, Agility 61→48 |
| 120 | Staryu | Psychic M 40 |  | Light Screen 43→27 |
| 121 | Starmie | Harden 1, Water Gun 1, Swift 25, Light Screen 27, Minimize 34, Psychic M 40, Hydro Pump 50 |  | Recover 1→19, Bubblebeam 1→31 |
| 123 | Scyther | Cut 16, Twineedle 42 |  | Swords Dance 42→54 |
| 124 | Jynx | Confusion 13, Icy Wind 17, Psychic M 39 |  | Lovely Kiss 1/9→1/34, Powder Snow 1/13→1, Mean Look 35→36 |
| 125 | Electabuzz | Thundershock 1, Spark 17 |  | Thunderpunch 1/9→30, Light Screen 17→21, Thunderbolt 47→42, Thunder 58→52 |
| 126 | Magmar | Flame Wheel 17, Poison Gas 25 |  | Leer 1/7→1, Smog 1/13→1, Fire Punch 1/19→30, Smokescreen 25→21 |
| 127 | Pinsir | Twineedle 25, Megahorn 48 |  | Harden 25→27 |
| 128 | Tauros | Headbutt 23, Double Edge 58 |  | Take Down 53→48 |
| 130 | Gyarados | Tackle 1, Gust 20 |  | Thrash 1→30, Leer 30→1/22 |
| 131 | Lapras | Icy Wind 20 |  |  |
| 133 | Eevee | Double Kick 16 |  | Growl 16→12, Bite 30→20, Baton Pass 36→30, Take Down 42→36 |
| 134 | Vaporeon | Growl 12, Double Kick 16, Bubblebeam 26 |  | Water Gun 16→20, Haze 42→47, Acid Armor 47→42 |
| 135 | Jolteon | Growl 12, Spark 26, Thunderbolt 36 |  | Thundershock 16→20, Double Kick 30→16, Pin Missile 36→30 |
| 136 | Flareon | Growl 12, Double Kick 16, Flame Wheel 26, Fire Blast 52 | Leer 47 | Ember 16→20, Fire Spin 36→42, Smog 42→47, Flamethrower 52→36 |
| 137 | Porygon | Hyper Beam 50 |  |  |
| 138 | Omanyte | Rock Throw 14, Bubblebeam 18, Spike Cannon 27 |  | Bite 13→11, Water Gun 19→7, Leer 31→23, Ancientpower 49→30, Hydro Pump 55→46 |
| 139 | Omastar | Rock Throw 14, Bubblebeam 18 |  | Bite 1/13→1, Water Gun 19→1, Leer 31→23, Spike Cannon 40→27, Ancientpower 54→30, Hydro Pump 65→46 |
| 140 | Kabuto | Rock Throw 7, Water Gun 14, Swords Dance 43, Rock Slide 46 |  | Absorb 10→19, Leer 19→1/25, Mega Drain 46→35, Ancientpower 55→30 |
| 141 | Kabutops | Rock Throw 7, Water Gun 14, Swords Dance 43, Rock Slide 46 |  | Absorb 1/10→1/19, Leer 19→25, Mega Drain 51→35, Ancientpower 65→30 |
| 142 | Aerodactyl | Gust 1, Rock Slide 40 |  | Wing Attack 1→32 |
| 144 | Articuno | Drill Peck 55, Sky Attack 76 |  |  |
| 145 | Zapdos | Thunderbolt 49, Sky Attack 76 |  | Drill Peck 49→55 |
| 146 | Moltres | Drill Peck 55, Fire Blast 73 |  | Sky Attack 73→76 |
| 147 | Dratini | Tackle 1 |  | Wrap 1→5, Agility 36→35, Safeguard 43→41, Outrage 50→45, Hyper Beam 57→54 |
| 148 | Dragonair | Tackle 1 |  | Thunder Wave 1/8→8, Twister 1/15→15, Outrage 56→50 |
| 149 | Dragonite | Tackle 1 |  | Thunder Wave 1/8→1, Twister 1/15→15, Outrage 61→56, Hyper Beam 75→70 |

Unchanged learnsets (17): Bulbasaur, Caterpie, Weedle, Pikachu, Nidoran F, Nidoran M, Diglett, Mankey, Abra, Kadabra, Alakazam, Mr. Mime, Magikarp, Ditto, Snorlax, Mewtwo, Mew

## TM/HM compatibility changes (Gen 1)

| # | Species | Added | Removed |
|---|---|---|---|
| 1 | Bulbasaur | Sludge Bomb |  |
| 2 | Ivysaur | Sludge Bomb |  |
| 3 | Venusaur | Earthquake, Sludge Bomb |  |
| 12 | Butterfree | Dream Eater |  |
| 16 | Pidgey | Headbutt |  |
| 17 | Pidgeotto | Headbutt |  |
| 18 | Pidgeot | Headbutt |  |
| 19 | Rattata | Cut |  |
| 29 | Nidoran F | Sludge Bomb |  |
| 30 | Nidorina | Dig, Sludge Bomb |  |
| 31 | Nidoqueen | Sludge Bomb |  |
| 32 | Nidoran M | Sludge Bomb |  |
| 33 | Nidorino | Dig, Sludge Bomb |  |
| 34 | Nidoking | Sludge Bomb |  |
| 35 | Clefairy | Icy Wind, Swift |  |
| 36 | Clefable | Icy Wind, Swift |  |
| 37 | Vulpix | Dream Eater, Shadow Ball, Solarbeam |  |
| 39 | Jigglypuff | Icy Wind |  |
| 40 | Wigglytuff | Icy Wind |  |
| 41 | Zubat | Sludge Bomb |  |
| 42 | Golbat | Sludge Bomb |  |
| 50 | Diglett | Sandstorm |  |
| 51 | Dugtrio | Sandstorm |  |
| 52 | Meowth | Cut, Dig |  |
| 53 | Persian | Cut, Dig |  |
| 54 | Psyduck | Psychic M |  |
| 55 | Golduck | Psychic M |  |
| 60 | Poliwag | Dream Eater |  |
| 61 | Poliwhirl | Dream Eater |  |
| 62 | Poliwrath | Dream Eater |  |
| 63 | Abra |  | Fire Punch, Ice Punch |
| 64 | Kadabra |  | Fire Punch, Ice Punch |
| 65 | Alakazam |  | Fire Punch, Ice Punch |
| 72 | Tentacool | Waterfall |  |
| 73 | Tentacruel | Waterfall |  |
| 77 | Ponyta | Solarbeam |  |
| 78 | Rapidash | Solarbeam |  |
| 79 | Slowpoke | Waterfall, Whirlpool |  |
| 80 | Slowbro | Waterfall, Whirlpool |  |
| 83 | Farfetch D | Fury Cutter |  |
| 86 | Seel | Strength |  |
| 87 | Dewgong | Strength |  |
| 90 | Shellder | Waterfall |  |
| 91 | Cloyster | Waterfall |  |
| 93 | Haunter | Thunderpunch |  |
| 94 | Gengar |  | Fire Punch, Ice Punch |
| 98 | Krabby | Dig, Waterfall |  |
| 99 | Kingler | Dig, Waterfall |  |
| 113 | Chansey | Sweet Scent |  |
| 118 | Goldeen | Whirlpool |  |
| 119 | Seaking | Whirlpool |  |
| 126 | Magmar | Solarbeam |  |
| 130 | Gyarados | Fly |  |
| 131 | Lapras | Solarbeam, Waterfall |  |
| 133 | Eevee | Dig |  |
| 134 | Vaporeon | Dig |  |
| 135 | Jolteon | Dig |  |
| 136 | Flareon | Dig |  |
| 138 | Omanyte | Waterfall |  |
| 139 | Omastar | Waterfall |  |
| 140 | Kabuto | Cut, Dig, Surf, Waterfall, Whirlpool |  |
| 141 | Kabutops | Dig, Waterfall |  |
| 146 | Moltres | Solarbeam |  |

## Move data changes (90 moves)

| Move | Power | Acc | PP | Type | Other |
|---|---|---|---|---|---|
| Doubleslap | 15→20 | 85→100 | 10→35 |  |  |
| Comet Punch | 18→25 | 85→100 | 15→25 |  |  |
| Pay Day | 40→60 |  |  |  |  |
| Razor Wind |  | 75→95 | 10→15 |  |  |
| Cut | 50→55 |  |  | NORMAL→BUG |  |
| Fly |  | 95→100 |  |  |  |
| Bind |  | 75→85 |  |  |  |
| Slam |  | 75→100 |  |  |  |
| Vine Whip | 35→40 |  | 10→25 |  |  |
| Mega Kick |  | 75→85 | 5→10 |  |  |
| Jump Kick | 70→90 |  |  |  |  |
| Rolling Kick |  | 85→100 |  |  |  |
| Fury Attack | 15→18 | 85→100 |  |  |  |
| Tackle |  | 95→100 |  |  |  |
| Take Down | 90→95 | 85→100 |  |  |  |
| Poison Sting | 15→30 |  |  |  |  |
| Twineedle | 25→40 |  |  |  |  |
| Pin Missile | 14→20 | 85→100 | 20→30 |  |  |
| Supersonic |  | 55→70 |  |  |  |
| Sonicboom |  | 90→100 |  |  |  |
| Disable |  | 55→75 |  |  |  |
| Hydro Pump |  | 80→85 | 5→10 |  |  |
| Ice Beam |  |  | 10→15 |  |  |
| Blizzard |  | 70→85 |  |  |  |
| Submission | 80→90 | 80→100 |  |  |  |
| Low Kick |  | 90→100 |  |  |  |
| Absorb |  |  | 20→25 |  |  |
| Mega Drain |  |  | 10→20 |  |  |
| Poisonpowder |  | 75→90 |  |  |  |
| Petal Dance | 70→90 |  |  |  |  |
| Dragon Rage |  |  | 10→20 |  |  |
| Fire Spin | 15→35 | 70→100 |  |  |  |
| Rock Throw |  | 90→95 | 15→25 |  |  |
| Dig | 60→70 |  | 10→20 |  |  |
| Psychic M |  |  | 10→15 |  |  |
| Recover |  |  | 20→5 |  |  |
| Egg Bomb |  | 75→100 |  | NORMAL→GRASS |  |
| Lick | 20→30 |  |  |  |  |
| Smog | 20→25 | 70→100 |  |  |  |
| Bone Club |  | 85→100 |  |  |  |
| Waterfall | 80→70 |  |  |  | effect EFFECT_NORMAL_HIT→EFFECT_FLINCH_HIT; chance 0→30 |
| Clamp |  | 75→100 |  |  |  |
| Spike Cannon | 20→22 |  |  | NORMAL→STEEL |  |
| Constrict | 10→40 |  |  |  |  |
| Softboiled |  |  | 10→5 |  |  |
| Hi Jump Kick | 85→120 |  |  |  |  |
| Glare |  | 75→90 |  |  |  |
| Poison Gas |  | 55→85 |  |  |  |
| Barrage | 15→20 | 85→100 |  |  |  |
| Leech Life | 20→40 |  | 15→25 |  |  |
| Sky Attack | 140→120 | 90→85 | 5→10 |  | effect EFFECT_SKY_ATTACK→EFFECT_NORMAL_HIT |
| Dizzy Punch |  |  | 10→20 |  |  |
| Psywave |  | 80→95 |  |  |  |
| Crabhammer | 90→110 | 85→100 |  |  |  |
| Fury Swipes | 18→20 | 80→100 | 15→20 |  |  |
| Bonemerang |  |  | 10→20 |  |  |
| Rest |  |  | 10→5 |  |  |
| Rock Slide | 75→80 | 90→95 | 10→15 |  |  |
| Tri Attack | 80→85 |  | 10→15 |  |  |
| Triple Kick | 10→20 | 90→100 | 10→15 |  |  |
| Thief | 40→60 |  | 10→15 |  |  |
| Flame Wheel | 60→65 |  |  |  |  |
| Aeroblast |  |  | 5→10 |  |  |
| Mach Punch | 40→50 |  |  |  |  |
| Faint Attack | 60→70 |  |  |  |  |
| Sludge Bomb |  |  | 10→15 |  |  |
| Mud Slap |  |  | 10→15 |  |  |
| Octazooka |  | 85→100 | 10→15 |  |  |
| Icy Wind | 55→65 | 95→100 | 15→20 |  |  |
| Bone Rush |  | 80→100 | 10→20 |  |  |
| Outrage | 90→110 |  |  |  |  |
| Giga Drain | 60→75 |  | 5→15 |  |  |
| Milk Drink |  |  | 10→5 |  |  |
| Steel Wing |  | 90→100 |  |  |  |
| Sacred Fire |  |  | 5→10 |  |  |
| Dynamicpunch |  |  | 5→10 |  |  |
| Megahorn |  |  | 10→5 |  |  |
| Dragonbreath | 60→70 |  |  |  |  |
| Iron Tail |  | 75→90 |  |  | chance 30→20 |
| Metal Claw | 50→70 | 95→100 | 35→25 |  |  |
| Cross Chop |  | 80→85 | 5→10 |  |  |
| Twister | 40→70 |  |  |  |  |
| Rain Dance |  |  | 5→10 |  |  |
| Sunny Day |  |  | 5→10 |  |  |
| Crunch |  |  |  |  | effect EFFECT_SP_DEF_DOWN_HIT→EFFECT_DEFENSE_DOWN_HIT |
| Extremespeed |  |  | 5→10 |  |  |
| Ancientpower |  |  | 5→10 |  |  |
| Future Sight | 80→120 |  |  |  |  |
| Rock Smash | 20→60 |  |  |  | chance 50→20 |
| Beat Up | 10→20 |  |  |  |  |

## Boss parties (levels/species)

Vanilla Crystal has one fixed team per boss. Legacy entries labelled by the comment in parties.asm. Ace = highest level.

| Boss | Vanilla party | Van. max | Legacy party | Leg. max | Δ max |
|---|---|---|---|---|---|
| Falkner | Pidgey 7, Pidgeotto 9 | 9 | Pidgey 8, Noctowl 10 | 10 | +1 |
| Falkner — Rematch | ″ |  | Noctowl 54, Skarmory 53, Dodrio 53, Murkrow 54, Pidgeot 55 | 55 | +46 |
| Bugsy | Metapod 14, Kakuna 14, Scyther 16 | 16 | Pineco 15, Ledian 15, Scyther 16 | 16 | +0 |
| Bugsy — Rematch | ″ |  | Ledian 54, Forretress 53, Shuckle 53, Scizor 54, Scyther 55 | 55 | +39 |
| Whitney | Clefairy 18, Miltank 20 | 20 | Clefairy 20, Teddiursa 20, Miltank 21 | 21 | +1 |
| Whitney — Rematch | ″ |  | Clefable 53, Ursaring 54, Blissey 53, Tauros 54, Miltank 55 | 55 | +35 |
| Morty | Gastly 21, Haunter 21, Gengar 25, Haunter 23 | 25 | Haunter 24, Stantler 23, Misdreavus 24, Gengar 25 | 25 | +0 |
| Morty — Rematch | ″ |  | Gengar 54, Stantler 53, Gengar 54, Misdreavus 53, Gengar 55 | 55 | +30 |
| Chuck — 1st of Pryce/Jasmine/Chuck | Primeape 27, Poliwrath 30 | 30 | Hitmontop 30, Primeape 29, Sudowoodo 30, Poliwrath 31 | 31 | +1 |
| Chuck — 2nd | ″ |  | Hitmontop 35, Sudowoodo 35, Primeape 34, Pinsir 35, Poliwrath 36 | 36 | +6 |
| Chuck — 3rd | ″ |  | Hitmontop 37, Sudowoodo 37, Primeape 37, Pinsir 37, Poliwrath 38 | 38 | +8 |
| Chuck — Rematch | ″ |  | Hitmontop 54, Sudowoodo 53, Primeape 54, Pinsir 53, Poliwrath 55 | 55 | +25 |
| Jasmine — 1st of Pryce/Jasmine/Chuck | Magnemite 30, Magnemite 30, Steelix 35 | 35 | Skarmory 35, Corsola 35, Magneton 35, Scizor 36, Steelix 36 | 36 | +1 |
| Jasmine — 2nd | ″ |  | Skarmory 35, Corsola 35, Magneton 35, Scizor 36, Steelix 36 | 36 | +1 |
| Jasmine — 3rd | ″ |  | Skarmory 37, Corsola 37, Magneton 37, Scizor 38, Steelix 38 | 38 | +3 |
| Jasmine — Rematch | ″ |  | Skarmory 53, Corsola 53, Magneton 54, Scizor 55, Steelix 55 | 55 | +20 |
| Pryce — 1st of Pryce/Jasmine/Chuck | Seel 27, Dewgong 29, Piloswine 31 | 31 | Dewgong 31, Sneasel 32, Jynx 31, Piloswine 33 | 33 | +2 |
| Pryce — 2nd | ″ |  | Cloyster 33, Sneasel 34, Jynx 33, Dewgong 34, Piloswine 35 | 35 | +4 |
| Pryce — 3rd | ″ |  | Cloyster 37, Sneasel 37, Jynx 37, Dewgong 37, Piloswine 38 | 38 | +7 |
| Pryce — Rematch | ″ |  | Sneasel 54, Dewgong 53, Jynx 53, Lapras 54, Piloswine 55 | 55 | +24 |
| Clair | Dragonair 37, Dragonair 37, Dragonair 37, Kingdra 40 | 40 | Dragonair 42, Gyarados 43, Lapras 43, Dragonair 44, Kingdra 45 | 45 | +5 |
| Clair — Rematch | ″ |  | Dragonair 54, Gyarados 53, Lapras 53, Dragonair 54, Kingdra 55 | 55 | +15 |
| Brock | Graveler 41, Rhyhorn 41, Omastar 42, Onix 44, Kabutops 42 | 44 | Golem 66, Aerodactyl 66, Kabutops 66, Omastar 66, Tyranitar 66, Steelix 66 | 66 | +22 |
| Misty | Golduck 42, Quagsire 42, Lapras 44, Starmie 47 | 47 | Golduck 62, Quagsire 62, Vaporeon 61, Kingdra 61, Lapras 62, Starmie 63 | 63 | +16 |
| Lt.Surge | Raichu 44, Electrode 40, Magneton 40, Electrode 40, Electabuzz 46 | 46 | Electrode 57, Magneton 59, Lanturn 58, Ampharos 58, Electabuzz 59, Raichu 60 | 60 | +14 |
| Erika | Tangela 42, Jumpluff 41, Victreebel 46, Bellossom 46 | 46 | Jumpluff 61, Sudowoodo 60, Exeggutor 60, Venusaur 61, Victreebel 62, Bellossom 62 | 62 | +16 |
| Janine | Crobat 36, Weezing 36, Weezing 36, Ariados 33, Venomoth 39 | 39 | Weezing 63, Muk 62, Tentacruel 61, Nidoqueen 62, Crobat 63, Venomoth 64 | 64 | +25 |
| Sabrina | Espeon 46, Mr. Mime 46, Alakazam 48 | 48 | Mr. Mime 65, Jynx 64, Slowbro 64, Wobbuffet 64, Hypno 65, Alakazam 66 | 66 | +18 |
| Blaine | Magcargo 45, Magmar 45, Rapidash 50 | 50 | Rapidash 66, Magmar 65, Houndoom 66, Ninetales 65, Moltres 67, Arcanine 67 | 67 | +17 |
| Blue — (1) fought | Pidgeot 56, Alakazam 54, Rhydon 56, Gyarados 58, Exeggutor 58, Arcanine 58 | 58 | Articuno 69, Alakazam 68, Machamp 67, Exeggutor 68, Gyarados 68, Arcanine 69 | 69 | +11 |
| Blue — (2) unused alt, TODO comment | ″ |  | Articuno 69, Alakazam 68, Rhydon 67, Exeggutor 68, Gyarados 68, Arcanine 69 | 69 | +11 |
| Will | Xatu 40, Jynx 41, Exeggutor 41, Slowbro 41, Xatu 42 | 42 | Girafarig 48, Slowking 49, Espeon 48, Slowbro 49, Xatu 50 | 50 | +8 |
| Will — Rematch | ″ |  | Stantler 66, Slowking 67, Exeggutor 67, Ninetales 67, Slowbro 66, Xatu 68 | 68 | +26 |
| Koga | Ariados 40, Venomoth 41, Forretress 43, Muk 42, Crobat 44 | 44 | Ariados 50, Qwilfish 51, Muk 50, Venomoth 51, Crobat 52 | 52 | +8 |
| Koga — Rematch | ″ |  | Tentacruel 67, Muk 67, Gligar 67, Nidoking 67, Hypno 67, Crobat 68 | 68 | +24 |
| Bruno | Hitmontop 42, Hitmonlee 42, Hitmonchan 42, Onix 43, Machamp 46 | 46 | Hitmonchan 53, Heracross 52, Hitmonlee 53, Steelix 53, Machamp 54 | 54 | +8 |
| Bruno — Rematch | ″ |  | Steelix 67, Poliwrath 68, Heracross 67, Donphan 68, Granbull 67, Machamp 68 | 68 | +22 |
| Karen | Umbreon 42, Vileplume 42, Gengar 45, Murkrow 44, Houndoom 47 | 47 | Umbreon 54, Gengar 53, Vileplume 53, Murkrow 53, Houndoom 55 | 55 | +8 |
| Karen — Rematch | ″ |  | Umbreon 68, Gengar 67, Persian 68, Murkrow 68, Blissey 67, Houndoom 69 | 69 | +22 |
| Lance | Gyarados 44, Dragonite 47, Dragonite 47, Aerodactyl 46, Charizard 46, Dragonite 50 | 50 | Gyarados 54, Dragonite 55, Charizard 54, Zapdos 55, Aerodactyl 54, Dragonite 56 | 56 | +6 |
| Lance — Rematch | ″ |  | Tyranitar 69, Dragonite 68, Gyarados 69, Charizard 68, Aerodactyl 69, Dragonite 70 | 70 | +20 |
| Red — (1) | Pikachu 81, Espeon 73, Snorlax 75, Venusaur 77, Charizard 77, Blastoise 77 | 81 | Pikachu 93, Snorlax 75, Charizard 77, Venusaur 77, Blastoise 77, Mewtwo 75 | 93 | +12 |
| Red — (2) rematch | ″ |  | Pikachu 93, Snorlax 75, Charizard 77, Venusaur 77, Blastoise 77, Espeon 80 | 93 | +12 |

Notes: Legacy gym leaders and E4 carry held items on every mon (TRAINERTYPE_ITEM_MOVES); E4/Lance/Red rematch teams also set DVs/stat exp. Pryce/Jasmine/Chuck scale by the order you fight them (Legacy makes the mid-game nonlinear); Jasmine 1st and 2nd are identical. Blue (1) is the team loaded by `ViridianGym.asm` (BLUE1). Red (2) loads when `EVENT_CERULEAN_CAVE_B1F_MEWTWO` is set.
