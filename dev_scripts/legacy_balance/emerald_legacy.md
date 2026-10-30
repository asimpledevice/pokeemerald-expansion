# Emerald Legacy vs vanilla Emerald: Gen 1 species and boss level curve

- Emerald Legacy: cRz-Shadows/Pokemon_Emerald_Legacy @ `6465639719084852f9bf6eb5b774bbcf8bd2b955`
- Base / vanilla reference: pret/pokeemerald @ `731ad5bfd6e6f265508d0efcca0ba42f9dcf5881` (Legacy README states it is built on the pret Emerald disassembly; data file layout matches).
- Source files compared: `src/data/pokemon/{species_info,evolution,level_up_learnsets,level_up_learnset_pointers,tmhm_learnsets,tutor_learnsets}.h`, `src/data/{trainers,trainer_parties}.h`, map scripts for which trainer IDs are used.
- Parse problems: none; all 151 species, all learnsets and all boss parties parsed.
- Not covered: changes to move data, ability effects or battle code (only species data is compared). Legacy has no new moves, types or abilities (constants files identical apart from 3 extra Deoxys form species).
- Global change not listed per species: egg cycles were cut by roughly 25% for every species (e.g. 20→15, 40→30, 120→90).

## Summary counts (of 151)

| Category | Species changed |
|---|---|
| Base stats | 26 |
| Types | 0 |
| Abilities | 2 |
| Evolution method/level | 11 |
| Level-up learnset | 128 |
| TM/HM compatibility | 14 |
| Move tutor compatibility | 5 |
| Any of stats/types/abilities/evolution/learnset | 130 (unchanged: 21) |

Learnsets: 121 species gained or lost moves; 7 only had level changes. Other species fields changed (besides egg cycles): Eevee {'evYield_Defense': ['0', '2'], 'eggCycles': ['35', '26'], 'evYield_SpDefense': ['1', '0']}.

## Stat / type / ability changes

28 species changed; the other 123 are unchanged.

| # | Species | HP | Atk | Def | SpA | SpD | Spe | BST | Abilities |
|---|---|---|---|---|---|---|---|---|---|
| 23 | Ekans | **35→40** | **60→65** | 44 | 40 | 54 | **55→60** | **288→303** |  |
| 24 | Arbok | **60→62** | **85→95** | 69 | 65 | 79 | **80→90** | **438→460** |  |
| 25 | Pikachu | 35 | 55 | **30→40** | 50 | **40→50** | 90 | **300→320** |  |
| 36 | Clefable | 95 | 70 | 73 | **85→95** | 90 | 60 | **473→483** |  |
| 37 | Vulpix | **38→45** | 41 | **40→45** | **50→60** | **65→70** | **65→75** | **299→336** |  |
| 38 | Ninetales | **73→78** | **76→71** | **75→80** | **81→86** | 100 | 100 | **505→515** |  |
| 40 | Wigglytuff | 140 | 70 | **45→55** | **75→85** | **50→55** | 45 | **425→450** |  |
| 43 | Oddish | **45→50** | 50 | 55 | 75 | 65 | 30 | **320→325** |  |
| 44 | Gloom | **60→70** | 65 | 70 | 85 | 75 | 40 | **395→405** |  |
| 45 | Vileplume | **75→90** | 80 | **85→95** | 100 | 90 | 50 | **480→505** |  |
| 49 | Venomoth | 70 | **65→75** | 60 | **90→95** | 75 | **90→100** | **450→475** |  |
| 51 | Dugtrio | 35 | **80→90** | 50 | 50 | 70 | 120 | **405→415** |  |
| 52 | Meowth | = | = | = | = | = | = | = | Pickup → Pickup / Hustle |
| 53 | Persian | = | = | = | = | = | = | = | Limber → Limber / Hustle |
| 77 | Ponyta | 50 | 85 | 55 | 65 | 65 | **90→100** | **410→420** |  |
| 78 | Rapidash | 65 | 100 | 70 | 80 | 80 | **105→115** | **500→510** |  |
| 87 | Dewgong | 90 | 70 | 80 | **70→80** | 95 | 70 | **475→485** |  |
| 88 | Grimer | 80 | 80 | 50 | 40 | **50→75** | 25 | **325→350** |  |
| 90 | Shellder | **30→35** | 65 | 100 | 45 | 25 | 40 | **305→310** |  |
| 105 | Marowak | **60→65** | 80 | 110 | 50 | 80 | 45 | **425→430** |  |
| 106 | Hitmonlee | 50 | 120 | **53→58** | 35 | 110 | 87 | **455→460** |  |
| 107 | Hitmonchan | 50 | 105 | 79 | 35 | 110 | **76→81** | **455→460** |  |
| 108 | Lickitung | **90→95** | **55→70** | **75→85** | **60→75** | **75→85** | 30 | **385→440** |  |
| 110 | Weezing | **65→75** | 90 | 120 | 85 | 70 | 60 | **490→500** |  |
| 119 | Seaking | 80 | **92→100** | 65 | 65 | 80 | 68 | **450→458** |  |
| 126 | Magmar | **65→75** | 95 | 57 | 100 | 85 | 93 | **495→505** |  |
| 136 | Flareon | 65 | 130 | 60 | **95→110** | **110→95** | 65 | 525 |  |
| 137 | Porygon | **65→75** | **60→70** | 70 | 85 | 75 | 40 | **395→415** |  |

## Evolution changes

| # | Species | Vanilla | Legacy |
|---|---|---|---|
| 61 | Poliwhirl | Water Stone -> Poliwrath; Trade holding Kings Rock -> Politoed | Water Stone -> Poliwrath; Kings Rock -> Politoed |
| 64 | Kadabra | Trade -> Alakazam | Lv 42 -> Alakazam |
| 67 | Machoke | Trade -> Machamp | Lv 38 -> Machamp |
| 75 | Graveler | Trade -> Golem | Lv 38 -> Golem |
| 79 | Slowpoke | Lv 37 -> Slowbro; Trade holding Kings Rock -> Slowking | Lv 37 -> Slowbro; Kings Rock -> Slowking |
| 93 | Haunter | Trade -> Gengar | Lv 42 -> Gengar |
| 95 | Onix | Trade holding Metal Coat -> Steelix | Metal Coat -> Steelix |
| 117 | Seadra | Trade holding Dragon Scale -> Kingdra | Dragon Scale -> Kingdra |
| 118 | Goldeen | Lv 33 -> Seaking | Lv 28 -> Seaking |
| 123 | Scyther | Trade holding Metal Coat -> Scizor | Metal Coat -> Scizor |
| 137 | Porygon | Trade holding Up Grade -> Porygon2 | Up Grade -> Porygon2 |

Trade evolutions: every trade evolution was removed. Plain trade evos (Kadabra, Machoke, Graveler, Haunter) became level-ups (42/38/38/42). Trade-with-item evos (Poliwhirl→Politoed, Slowpoke→Slowking, Onix→Steelix, Seadra→Kingdra, Scyther→Scizor, Porygon→Porygon2) are now used from the bag like evolution stones; Legacy changes King's Rock, Metal Coat, Dragon Scale, Up-Grade and Deep Sea Tooth/Scale to `ItemUseOutOfBattle_EvolutionStone` in `items.h`. The only other change is Goldeen evolving at Lv 28 instead of 33.

## Level-up learnset changes

Format: `+Move L` added, `-Move L` removed, `Move a→b` level changed (`1/17` = learned at both levels).

| # | Species | Changes |
|---|---|---|
| 1 | Bulbasaur | +Acid 13 |
| 2 | Ivysaur | +Acid 10, -Vine Whip 10, Poison Powder 15→15/15, Razor Leaf 22→20, Sweet Scent 29→25, Growth 38→32, Synthesis 47→39, Solar Beam 56→46 |
| 3 | Venusaur | +Acid 10, +Body Slam 45, +Frenzy Plant 55, Razor Leaf 22→20, Sweet Scent 29→25, Growth 41→32, Synthesis 53→39, Solar Beam 65→46 |
| 4 | Charmander | +Fire Punch 22 |
| 5 | Charmeleon | Rage 20→19, Scary Face 27→25, Flamethrower 34→31, Slash 41→37, Dragon Rage 48→43, Fire Spin 55→49 |
| 6 | Charizard | +Body Slam 45, +Blast Burn 55, Rage 20→19, Scary Face 27→25, Flamethrower 34→31, Slash 44→37, Dragon Rage 54→43, Fire Spin 64→49 |
| 7 | Squirtle | +Bubble Beam 21 |
| 8 | Wartortle | +Bubble Beam 21, Bite 19→18, Rapid Spin 25→23, Protect 31→28, Rain Dance 37→33, Skull Bash 45→40, Hydro Pump 53→47 |
| 9 | Blastoise | +Bubble Beam 21, +Body Slam 45, +Ice Beam 50, +Hydro Cannon 55, Bite 19→18, Rapid Spin 25→23, Protect 31→28, Rain Dance 42→33, Skull Bash 55→40, Hydro Pump 68→47 |
| 12 | Butterfree | +Leech Life 16, +Signal Beam 37, +Psychic 47, Whirlwind 23→21, Silver Wind 47→25, Psybeam 34→31 |
| 15 | Beedrill | +Sludge 25, +Swords Dance 35, Pin Missile 35→10, Rage 25→15 |
| 16 | Pidgey | +Mud Slap 27, Whirlwind 19→20, Wing Attack 25→22, Feather Dance 31→34, Agility 39→43, Mirror Move 47→52 |
| 17 | Pidgeotto | +Mud Slap 27, Wing Attack 27→22 |
| 18 | Pidgeot | +Mud Slap 27, +Extreme Speed 36, +Sky Attack 47, Quick Attack 1/13→13, Wing Attack 27→22, Agility 48→43, Mirror Move 62→52 |
| 19 | Rattata | +Dig 23, +Crunch 45 |
| 20 | Raticate | +Focus Energy 20, +Dig 23, +Crunch 45, -Scary Face 20, Quick Attack 1/7→7, Pursuit 30→27, Super Fang 40→34, Endeavor 50→41 |
| 21 | Spearow | +Focus Energy 9, +Sharpen 29 |
| 22 | Fearow | +Focus Energy 9, +Aerial Ace 25, +Sharpen 29, Leer 1/7→7, Fury Attack 1/13→13, Pursuit 26→19, Mirror Move 32→31, Drill Peck 40→37, Agility 47→43 |
| 23 | Ekans | +Substitute 22, +Poison Fang 32, +Sludge Bomb 40, Acid 32→17, Glare 20→25, Screech 25→29 |
| 24 | Arbok | +Substitute 22, +Poison Fang 32, +Sludge Bomb 40, Poison Sting 1/8→8, Bite 1/13→13, Acid 38→17, Glare 20→25, Screech 28→29, Spit Up 46→37, Stockpile 46→37, Swallow 46→37, Haze 56→44 |
| 25 | Pikachu | +Volt Tackle 50, Light Screen 50→45 |
| 26 | Raichu | +Growl 1, +Thunder Wave 8, +Double Team 15, +Slam 20, +Agility 33, +Thunder 41, +Light Screen 45, Tail Whip 1→6, Quick Attack 1→11, Thunderbolt 1→26 |
| 27 | Sandshrew | +Metal Claw 16, +Dig 20, +Cut 28, +Earthquake 33, +Swords Dance 42, +Spike Cannon 46, -Swift 30, Defense Curl 6→1, Sand Attack 11→3, Poison Sting 17→5, Sand Tomb 45→7, Fury Swipes 37→10, Slash 23→24, Sandstorm 53→36 |
| 28 | Sandslash | +Metal Claw 16, +Dig 20, +Cut 28, +Earthquake 33, +Swords Dance 42, +Spike Cannon 46, -Swift 33, Defense Curl 1/6→1, Sand Attack 1/11→1/3, Poison Sting 17→5, Sand Tomb 52→7, Fury Swipes 42→10, Sandstorm 62→36 |
| 29 | Nidoran-F | +Sludge Bomb 32 |
| 30 | Nidorina | +Sludge Bomb 32, Poison Sting 18→17, Bite 22→20, Helping Hand 26→23, Fury Swipes 34→30, Flatter 43→38, Crunch 53→47 |
| 31 | Nidoqueen | +Fury Swipes 1, +Growl 1, +Bite 20, +Helping Hand 23, +Magnitude 23, +Sludge Bomb 32, +Flatter 38, +Earthquake 42, +Crunch 47, +Superpower 52, Tail Whip 1→8, Double Kick 1→12, Poison Sting 1→17, Body Slam 23→27 |
| 33 | Nidorino | +Sludge Bomb 37, Horn Drill 53→52 |
| 34 | Nidoking | +Fury Attack 1, +Leer 1, +Horn Attack 18, +Magnitude 27, +Helping Hand 30, +Earthquake 36/47, +Flatter 43, +Megahorn 52, +Horn Drill 60, Focus Energy 1→8, Double Kick 1→12, Poison Sting 1→15 |
| 36 | Clefable | +Growl 1, +Pound 1, +Encore 5, +Follow Me 17, +Defense Curl 25, +Cosmic Power 33, +Moonlight 37, +Light Screen 41, +Meteor Mash 45, Sing 1→9, Double Slap 1→13, Minimize 1→21, Metronome 1→29 |
| 37 | Vulpix | +Flame Wheel 19, +Shadow Ball 36, +Sunny Day 40, +Fire Blast 43, Tail Whip 5→1, Roar 9→5, Quick Attack 13→8, Fire Spin 41→12, Confuse Ray 21→22, Imprison 25→26, Grudge 37→47 |
| 38 | Ninetales | +Roar 5, +Will O Wisp 17, +Flame Wheel 19, +Imprison 26, +Flamethrower 29, +Shadow Ball 36, +Sunny Day 40, +Fire Blast 43, +Grudge 47, Confuse Ray 1→1/22, Quick Attack 1→1/8, Safeguard 1→1/33, Fire Spin 45→12 |
| 39 | Jigglypuff | +Charm 7, +Fake Tears 19, +Lovely Kiss 22, +Encore 33, Pound 9→1, Disable 14→10, Double Slap 24→13, Rollout 19→16, Body Slam 34→26, Hyper Voice 44→36, Double Edge 49→41 |
| 40 | Wigglytuff | +Charm 7, +Rollout 16, +Fake Tears 19, +Lovely Kiss 22, +Body Slam 26, +Rest 29, +Encore 33, +Hyper Voice 36, +Mimic 39, +Double Edge 41, Defense Curl 1→1/4, Disable 1→1/10, Double Slap 1→1/13 |
| 41 | Zubat | +Gust 7, +Sludge 27, Supersonic 6→5, Astonish 11→10, Bite 16→13, Confuse Ray 26→19, Wing Attack 21→22, Air Cutter 31→32, Poison Fang 41→37, Mean Look 36→42, Haze 46→47 |
| 42 | Golbat | +Gust 7, +Sludge 27, Astonish 1/11→1/10, Supersonic 1/6→1/5, Bite 16→13, Confuse Ray 28→19, Wing Attack 21→22, Air Cutter 35→32, Poison Fang 49→37, Haze 56→47 |
| 43 | Oddish | +Mega Drain 13, +Giga Drain 32, +Sludge Bomb 36, Sweet Scent 7→5, Poison Powder 14→7, Moonlight 32→28, Petal Dance 39→40 |
| 44 | Gloom | +Mega Drain 13, +Giga Drain 32, +Sludge Bomb 36, Poison Powder 1/14→1/7, Sweet Scent 1/7→1/5, Acid 24→23, Moonlight 35→28, Petal Dance 44→40 |
| 45 | Vileplume | +Sweet Scent 5, +Poison Powder 7, +Sleep Powder 18, +Acid 23, +Moonlight 28, +Giga Drain 32, +Sludge Bomb 36, Mega Drain 1→1/13, Stun Spore 1→1/16, Petal Dance 44→40 |
| 46 | Paras | +Absorb 5, +Mega Drain 22, +Twineedle 32, +Swords Dance 37, Slash 31→28 |
| 47 | Parasect | +Absorb 5, +Mega Drain 22, +Twineedle 32, +Swords Dance 37, Stun Spore 1/7→7, Poison Powder 1/13→13, Spore 27→25, Slash 35→28, Growth 43→37, Giga Drain 51→43, Aromatherapy 59→49 |
| 48 | Venonat | +Signal Beam 39, +Sludge Bomb 43, -Confusion 17, Leech Life 25→13/20, Poison Powder 20→17, Stun Spore 28→23, Psybeam 33→27, Sleep Powder 36→33, Psychic 41→50 |
| 49 | Venomoth | +Signal Beam 39, +Sludge Bomb 43, -Confusion 17, Supersonic 1/9→9, Leech Life 25→13/20, Poison Powder 20→17, Stun Spore 28→23, Psybeam 36→27, Silver Wind 1→31, Sleep Powder 42→36, Psychic 52→50 |
| 51 | Dugtrio | Fissure 64→58 |
| 52 | Meowth | +Take Down 28, +Double Edge 38, -Slash 46, Fury Swipes 41→8, Faint Attack 28→25, Screech 35→32, Fake Out 50→45 |
| 53 | Persian | +Take Down 28, +Double Edge 38, +Hyper Beam 52, -Slash 53, Fury Swipes 46→8, Bite 1/11→11, Faint Attack 29→25, Screech 38→32, Fake Out 59→45 |
| 54 | Psyduck | +Water Gun 7, +Bubble Beam 22, +Amnesia 29, +Psychic 37, Confusion 16→14, Fury Swipes 40→18, Screech 23→25, Psych Up 31→32, Hydro Pump 50→43 |
| 55 | Golduck | +Water Gun 7, +Bubble Beam 22, +Amnesia 29, +Psychic 37, Confusion 16→14, Fury Swipes 44→18, Screech 23→25, Psych Up 31→32, Hydro Pump 58→43 |
| 57 | Primeape | Seismic Toss 36→33, Cross Chop 45→39, Screech 54→45, Thrash 63→51 |
| 58 | Growlithe | +Extreme Speed 49, Flame Wheel 31→24, Take Down 25→28, Helping Hand 37→31, Flamethrower 49→35 |
| 59 | Arcanine | +Leer 13, +Flame Wheel 24, +Take Down 28, +Helping Hand 31, +Flamethrower 35, +Agility 43, Ember 1→7, Odor Sleuth 1→19 |
| 60 | Poliwag | +Mind Reader 1, +Mist 10, +Bubble Beam 20, -Body Slam 31, Hypnosis 7→1/7, Water Gun 13→1/13, Double Slap 19→16, Rain Dance 25→29, Belly Drum 37→43, Hydro Pump 43→51 |
| 61 | Poliwhirl | +Mind Reader 1, +Mist 10, +Bubble Beam 20, +Karate Chop 25, Double Slap 19→16, Rain Dance 27→29 |
| 62 | Poliwrath | +Bubble 1, +Mist 10, +Bubble Beam 20, +Karate Chop 25, +Rain Dance 29, +Belly Drum 43, +Hydro Pump 51, Hypnosis 1→1/7, Mind Reader 51→1, Water Gun 1→1/13, Double Slap 1→16, Submission 1/35→35 |
| 64 | Kadabra | Kinesis 1→1/17 |
| 65 | Alakazam | Kinesis 1→1/17 |
| 67 | Machoke | Vital Throw 33→31, Submission 41→37, Cross Chop 46→40, Scary Face 51→43, Dynamic Punch 59→49 |
| 68 | Machamp | Vital Throw 33→31, Submission 41→37, Cross Chop 46→40, Scary Face 51→43, Dynamic Punch 59→49 |
| 69 | Bellsprout | +Sludge Bomb 32, +Leaf Blade 40, Sweet Scent 30→1, Sleep Powder 15→14, Poison Powder 17→15, Stun Spore 19→16, Acid 23→20, Razor Leaf 37→25, Slam 45→54 |
| 70 | Weepinbell | +Sludge Bomb 32, +Leaf Blade 40, Sweet Scent 33→1, Sleep Powder 15→14, Poison Powder 17→15, Stun Spore 19→16, Acid 24→20, Razor Leaf 42→25 |
| 71 | Victreebel | +Growth 1/6, +Wrap 1/11, +Poison Powder 15, +Stun Spore 16, +Acid 20, +Sludge Bomb 32, +Leaf Blade 40, +Slam 54, Sleep Powder 1→14, Razor Leaf 1→25 |
| 72 | Tentacool | +Bubble 1, +Whirlpool 11, +Sludge Bomb 34, Constrict 12→8, Acid 19→14, Bubble Beam 25→19, Wrap 30→27, Barrier 36→38, Hydro Pump 49→48 |
| 73 | Tentacruel | +Sludge Bomb 34, Constrict 1/12→1/9, Acid 19→14, Bubble Beam 25→19, Wrap 30→27, Screech 47→43, Hydro Pump 55→48 |
| 74 | Geodude | +Mud Slap 6, Mud Sport 6→1, Rock Blast 31→37, Earthquake 36→42, Explosion 41→46, Double Edge 46→52 |
| 75 | Graveler | +Mud Slap 6, Mud Sport 1/6→1, Earthquake 45→42, Explosion 53→46, Double Edge 62→52 |
| 76 | Golem | +Mud Slap 6, Mud Sport 1/6→1, Earthquake 45→42, Explosion 53→46, Double Edge 62→52 |
| 77 | Ponyta | +Flame Wheel 20, +Double Kick 25, +Flamethrower 33, Fire Spin 25→1, Take Down 31→36, Agility 38→42, Fire Blast 53→48 |
| 78 | Rapidash | +Flame Wheel 20, +Double Kick 25, +Flamethrower 33, +Hi Jump Kick 40, -Fury Attack 40, Fire Spin 25→1, Take Down 31→36, Agility 38→42, Bounce 50→45, Fire Blast 63→48 |
| 79 | Slowpoke | +Psybeam 25, Yawn 1→1/34, Water Gun 15→13, Confusion 20→17, Headbutt 34→22, Amnesia 43→39, Psychic 48→44 |
| 80 | Slowbro | +Psybeam 25, Growl 1/6→6, Water Gun 15→13, Confusion 20→17, Headbutt 34→22, Withdraw 37→36, Amnesia 46→40, Psychic 54→45 |
| 81 | Magnemite | +Tri Attack 30, +Thunderbolt 36, +Light Screen 40, -Swift 38, Screech 44→19, Thunder Wave 21→22, Lock On 32→50 |
| 82 | Magneton | +Thunderbolt 36, +Light Screen 40, +Refresh 57, Screech 53→19, Thunder Wave 21→22, Tri Attack 44→30, Lock On 35→50, Zap Cannon 62→50 |
| 83 | Farfetch'd | +Air Cutter 24, +Baton Pass 40, Swords Dance 31→30, Slash 41→36, Agility 36→46, False Swipe 46→51 |
| 84 | Doduo | +Bounce 24, +Jump Kick 44, +Double Edge 50, Uproar 33→1, Rage 25→27, Agility 45→33 |
| 85 | Dodrio | +Bounce 24, +Jump Kick 44, +Double Edge 50, Uproar 38→1, Rage 25→27, Agility 60→33, Drill Peck 47→37 |
| 87 | Dewgong | +Powder Snow 1, +Bubble Beam 25, +Blizzard 51, Icy Wind 1/17→17, Take Down 42→28, Rest 29→32, Sheer Cold 34→36, Ice Beam 51→42 |
| 88 | Grimer | +Mud Shot 26, +Toxic 43, Minimize 19→12, Sludge 13→16, Screech 26→21, Acid Armor 34→31, Sludge Bomb 43→36, Memento 53→50 |
| 89 | Muk | +Mud Shot 26, +Toxic 43, Minimize 19→12, Sludge 13→16, Screech 26→21, Acid Armor 34→31, Sludge Bomb 47→36, Memento 61→50 |
| 90 | Shellder | +Bubble Beam 22, +Hydro Pump 55 |
| 91 | Cloyster | +Clamp 1, +Tackle 1, +Bubble Beam 22, +Ice Beam 49, +Hydro Pump 55, Supersonic 1→9, Aurora Beam 1→17, Protect 1→25 |
| 92 | Gastly | +Smog 5, +Shadow Ball 44, Confuse Ray 28→31, Dream Eater 33→39, Destiny Bond 36→48 |
| 93 | Haunter | +Smog 1, +Shadow Ball 44, Spite 1/8→8 |
| 94 | Gengar | +Smog 1, +Shadow Ball 44, Spite 1/8→8 |
| 95 | Onix | +Dig 18, +Rock Slide 33, +Earthquake 41, Sandstorm 33→29 |
| 96 | Drowzee | +Dream Eater 25, Headbutt 25→22 |
| 97 | Hypno | +Dream Eater 25, Headbutt 25→22, Poison Gas 33→31, Meditate 40→36, Psychic 49→40, Psych Up 55→43, Future Sight 60→45 |
| 98 | Krabby | +Bubble Beam 20, +Metal Claw 29, Stomp 27→26, Crabhammer 45→34, Guillotine 34→41, Protect 41→45 |
| 99 | Kingler | +Bubble Beam 20, +Metal Claw 29, Stomp 27→26, Crabhammer 57→34, Guillotine 38→41, Protect 49→45 |
| 100 | Voltorb | +Thunder Wave 14, +Thunderbolt 36, +Rain Dance 42, +Thunder 45, Sonic Boom 15→11, Swift 42→17, Self Destruct 27→26, Light Screen 37→40, Explosion 46→47 |
| 101 | Electrode | +Volt Tackle 1, +Thunder Wave 14, +Thunderbolt 36, +Rain Dance 42, +Thunder 45, +Refresh 59, Sonic Boom 1/15→1/11, Swift 48→17, Self Destruct 27→26, Rollout 34→32, Light Screen 41→40, Explosion 54→47, Mirror Coat 59→49 |
| 102 | Exeggcute | +Mega Drain 19, +Psybeam 28, +Giga Drain 31, +Psychic 48, Confusion 19→16, Poison Powder 31→35 |
| 103 | Exeggutor | +Uproar 1, +Reflect 7, +Leech Seed 13, +Stun Spore 25, +Psybeam 28, +Poison Powder 35, +Sleep Powder 37, +Solar Beam 43, +Psychic 48, Confusion 1→16 |
| 107 | Hitmonchan | +Dizzy Punch 41, +Dynamic Punch 41 |
| 108 | Lickitung | +Rollout 18, +Curse 29, +Body Slam 34, +Belly Drum 40, Disable 34→1, Wrap 29→1, Defense Curl 12→9, Stomp 23→14, Knock Off 18→22, Slam 40→24 |
| 109 | Koffing | +Amnesia 26, +Sludge Bomb 36, +Pain Split 49, Smokescreen 25→15, Sludge 21→17, Self Destruct 17→19, Haze 33→22, Memento 49→30, Explosion 41→39 |
| 110 | Weezing | +Amnesia 26, +Sludge Bomb 36, +Pain Split 49, Self Destruct 1/17→1/19, Smokescreen 25→15, Sludge 21→17, Haze 33→22, Memento 58→30, Explosion 44→39, Destiny Bond 51→45 |
| 111 | Rhyhorn | +Rock Throw 12, +Magnitude 23, +Rock Slide 48, Stomp 10→9, Scary Face 24→26, Take Down 43→35, Earthquake 52→40, Megahorn 57→45, Horn Drill 38→52 |
| 112 | Rhydon | +Rock Throw 12, +Magnitude 23, +Rock Slide 48, Stomp 1/10→1/9, Scary Face 24→26, Take Down 46→35, Earthquake 58→40, Megahorn 66→45, Horn Drill 38→52 |
| 114 | Tangela | +Giga Drain 31, +Body Slam 40, -Slam 40, Vine Whip 22→7, Poison Powder 19→15, Mega Drain 31→22, Tickle 46→28, Bind 28→46 |
| 115 | Kangaskhan | +Double Edge 54 |
| 116 | Horsea | +Bubble Beam 22, +Aurora Beam 31, +Flail 36, Smokescreen 8→5, Leer 15→9, Water Gun 22→13, Twister 29→18, Agility 36→26 |
| 117 | Seadra | +Bubble Beam 22, +Aurora Beam 31, +Dragon Breath 36, +Body Slam 47, Leer 1/15→1/9, Smokescreen 1/8→1/5, Water Gun 1/22→1/13, Twister 29→18, Agility 40→26, Hydro Pump 51→43, Dragon Dance 62→50 |
| 118 | Goldeen | +Water Gun 5, +Drill Peck 30, +Rain Dance 32, +Megahorn 36, +Swords Dance 42, +Hydro Pump 50, Waterfall 38→18, Flail 24→22, Fury Attack 29→26, Agility 52→46, Horn Drill 43→55 |
| 119 | Seaking | +Water Gun 5, +Drill Peck 30, +Rain Dance 32, +Megahorn 36, +Swords Dance 42, +Hydro Pump 50, Waterfall 41→18, Flail 24→22, Fury Attack 29→26, Agility 61→46, Horn Drill 49→55 |
| 120 | Staryu | +Confuse Ray 33, +Psychic 37, +Refresh 55, Rapid Spin 10→9, Camouflage 19→14, Recover 15→18, Swift 24→22, Bubble Beam 28→25, Light Screen 37→30, Minimize 33→35, Cosmic Power 42→41 |
| 121 | Starmie | +Camouflage 14, +Bubble Beam 25, +Light Screen 30, +Minimize 35, +Psychic 37, +Cosmic Power 41, +Hydro Pump 46, +Refresh 55, Rapid Spin 1→1/9, Recover 1→1/18, Swift 1→1/22, Water Gun 1→1/6 |
| 123 | Scyther | +Cut 18, +Twineedle 42, Fury Cutter 46→1, Double Team 41→46 |
| 124 | Jynx | +Icy Wind 13, +Psychic 42, Powder Snow 1/13→1, Mean Look 35→32, Fake Tears 41→37, Body Slam 51→48, Perish Song 57→51, Blizzard 67→55 |
| 125 | Electabuzz | +Thunder Wave 20 |
| 126 | Magmar | +Will O Wisp 22, Sunny Day 33→30, Flamethrower 41→39, Confuse Ray 49→42, Fire Blast 57→50 |
| 127 | Pinsir | +Twineedle 28 |
| 128 | Tauros | +Refresh 1, +Headbutt 26, +Double Edge 50, Pursuit 26→22, Rest 34→30, Take Down 53→36 |
| 130 | Gyarados | +Tackle 1, +Gust 20, +Waterfall 35, +Bounce 48, Leer 30→1, Twister 35→30, Hyper Beam 55→50, Dragon Dance 50→57 |
| 131 | Lapras | +Icy Wind 15 |
| 133 | Eevee | +Double Kick 16, Growl 16→11 |
| 134 | Vaporeon | +Bubble Beam 30, -Bite 30, Acid Armor 47→46, Hydro Pump 52→49 |
| 135 | Jolteon | +Spark 30, -Double Kick 30, Agility 47→46, Thunder 52→49 |
| 136 | Flareon | +Flame Wheel 30, +Double Edge 36, -Bite 30, -Fire Spin 36, Leer 47→46, Flamethrower 52→49 |
| 137 | Porygon | +Hyper Beam 52 |
| 138 | Omanyte | +Rock Throw 17, +Bubble Beam 19, +Spike Cannon 40, +Spikes 51, Leer 31→1, Water Gun 19→7, Ancient Power 49→31, Tickle 43→46, Hydro Pump 55→56 |
| 139 | Omastar | +Rock Throw 17, +Bubble Beam 19, +Spikes 51, Leer 31→1, Water Gun 19→7, Bite 1/13→13, Ancient Power 55→31, Hydro Pump 65→56 |
| 140 | Kabuto | +Slash 36, +Swords Dance 42, +Rock Slide 46, +Hydro Pump 55, Sand Attack 31→1, Metal Sound 43→21, Mega Drain 49→28, Ancient Power 55→33, Endure 37→50 |
| 141 | Kabutops | +Swords Dance 42, +Rock Slide 46, +Hydro Pump 55, Sand Attack 31→1, Absorb 1/13→13, Metal Sound 46→21, Mega Drain 55→28, Ancient Power 65→33, Slash 40→36, Endure 37→50 |
| 142 | Aerodactyl | +Rock Slide 40 |
| 144 | Articuno | +Extrasensory 1, +Heal Bell 1, +Haze 13, +Drill Peck 42, +Sky Attack 55 |
| 145 | Zapdos | +Extrasensory 1, +Metal Sound 1, +Thunderbolt 49, +Sky Attack 55, +Baton Pass 61, Drill Peck 49→42 |
| 146 | Moltres | +Extrasensory 1, +Drill Peck 42, +Fire Blast 85, Sky Attack 85→55 |
| 149 | Dragonite | +Heal Bell 1, +Extreme Speed 81 |

Unchanged learnsets (23): Caterpie, Metapod, Weedle, Kakuna, Nidoran-M, Clefairy, Diglett, Mankey, Abra, Machop, Seel, Cubone, Marowak, Hitmonlee, Chansey, Mr. Mime, Magikarp, Ditto, Snorlax, Dratini, Dragonair, Mewtwo, Mew.

## TM/HM and tutor compatibility additions

| # | Species | TM/HM added | Tutor added |
|---|---|---|---|
| 37 | Vulpix | Flash, Shadow Ball |  |
| 38 | Ninetales | Flash, Shadow Ball, Solar Beam |  |
| 41 | Zubat | Fly |  |
| 42 | Golbat | Fly |  |
| 54 | Psyduck | Psychic |  |
| 55 | Golduck | Psychic |  |
| 58 | Growlithe | Reflect |  |
| 72 | Tentacool | Flash |  |
| 73 | Tentacruel | Flash |  |
| 81 | Magnemite | Light Screen |  |
| 82 | Magneton | Light Screen |  |
| 83 | Farfetch'd |  | Fury Cutter |
| 117 | Seadra |  | Body Slam |
| 118 | Goldeen |  | Swords Dance |
| 119 | Seaking |  | Swords Dance |
| 126 | Magmar | Overheat |  |
| 130 | Gyarados | Fly |  |
| 140 | Kabuto |  | Swords Dance |
| 146 | Moltres | Solar Beam |  |

## Boss level curve (first battles)

Legacy changes who fills some slots: Brawly has three versions picked by how many other gyms you have beaten (Dewford gym order is flexible: `BRAWLY_1`, `BRAWLY_1_2` if Mauville gym already beaten, `BRAWLY_1_3` if Lavaridge gym already beaten); the 8th gym is fought against Wallace (`TRAINER_WALLACE_1`) before the Hall of Fame, with Juan returning post-game (the Legacy `sParty_Juan1` is commented as an Original Base Gym Leader Placeholder and is identical to vanilla, so it is omitted below; his post-game fights use the rematch tables); the first Champion is Steven (`TRAINER_STEVEN_1`), and Wallace (`TRAINER_WALLACE_2`) becomes Champion on rematch.

| Slot | Vanilla party (lv) | Vanilla ace | Legacy party (lv) | Legacy ace | Ace Δ |
|---|---|---|---|---|---|
| Gym 1 Roxanne | Geodude 12, Geodude 12, Nosepass 15 | 15 | Anorith 13, Lileep 12, Nosepass 15 | 15 | +0 |
| Gym 2 Brawly (BRAWLY_1) | Machop 16, Meditite 16, Makuhita 19 | 19 | Makuhita 17, Meditite 16, Breloom 19 | 19 | +0 |
| Gym 2 Brawly (BRAWLY_1_2) | 〃 | 〃 | Meditite 24, Machoke 25, Breloom 24, Hariyama 26 | 26 | +7 |
| Gym 2 Brawly (BRAWLY_1_3) | 〃 | 〃 | Meditite 31, Machoke 30, Breloom 31, Hariyama 32 | 32 | +13 |
| Gym 3 Wattson | Voltorb 20, Electrike 20, Magneton 22, Manectric 24 | 24 | Chinchou 24, Magneton 23, Plusle 24, Manectric 25 | 25 | +1 |
| Gym 4 Flannery | Numel 24, Slugma 24, Camerupt 26, Torkoal 29 | 29 | Vulpix 29, Magcargo 28, Camerupt 30, Torkoal 32 | 32 | +3 |
| Gym 5 Norman | Spinda 27, Vigoroth 27, Linoone 29, Slaking 31 | 31 | Vigoroth 31, Girafarig 30, Linoone 31, Slaking 33 | 33 | +2 |
| Gym 6 Winona | Swablu 29, Tropius 29, Pelipper 30, Skarmory 31, Altaria 33 | 33 | Tropius 36, Swellow 37, Pelipper 36, Skarmory 37, Altaria 38 | 38 | +5 |
| Gym 7 Tate & Liza | Claydol 41, Xatu 41, Lunatone 42, Solrock 42 (double) | 42 | Claydol 44, Xatu 44, Medicham 43, Chimecho 43, Lunatone 45, Solrock 45 (double) | 45 | +3 |
| Gym 8 Sootopolis | Luvdisc 41, Whiscash 41, Sealeo 43, Crawdaunt 43, Kingdra 46 | 46 | Tentacruel 49, Masquerain 48, Whiscash 49, Ludicolo 50, Milotic 51 | 51 | +5 |
| E4 Sidney | Mightyena 46, Shiftry 48, Cacturne 46, Crawdaunt 48, Absol 49 | 49 | Mightyena 51, Murkrow 51, Shiftry 52, Crawdaunt 52, Absol 53 | 53 | +4 |
| E4 Phoebe | Dusclops 48, Banette 49, Sableye 50, Banette 49, Dusclops 51 | 51 | Sableye 52, Banette 53, Ninetales 52, Medicham 52, Dusclops 54 | 54 | +3 |
| E4 Glacia | Sealeo 50, Glalie 50, Sealeo 52, Glalie 52, Walrein 53 | 53 | Glalie 54, Sneasel 53, Castform 53, Piloswine 54, Walrein 55 | 55 | +2 |
| E4 Drake | Shelgon 52, Altaria 54, Kingdra 53, Flygon 53, Salamence 55 | 55 | Flygon 54, Altaria 55, Huntail 53, Kingdra 55, Salamence 56 | 56 | +1 |
| Champion | Wailord 57, Tentacruel 55, Ludicolo 56, Whiscash 56, Gyarados 56, Milotic 58 | 58 | Skarmory 57, Claydol 56, Aggron 56, Cradily 57, Armaldo 57, Metagross 58 | 58 | +0 |

### Post-game / rematch bosses (Legacy only)

| Slot | Legacy party (lv) |
|---|---|
| Postgame Champion (Wallace) (TRAINER_WALLACE_2) | Wailord 77, Tentacruel 75, Ludicolo 76, Whiscash 76, Gyarados 76, Milotic 78 |
| E4 rematch Sidney (TRAINER_SIDNEY_2) | Ninjask 67, Shiftry 68, Houndoom 68, Sharpedo 68, Zangoose 67, Absol 69 |
| E4 rematch Phoebe (TRAINER_PHOEBE_2) | Misdreavus 68, Wobbuffet 68, Ninetales 69, Medicham 69, Gengar 70, Dusclops 70 |
| E4 rematch Glacia (TRAINER_GLACIA_2) | Lapras 72, Piloswine 72, Glalie 71, Jynx 72, Castform 71, Walrein 72 |
| E4 rematch Drake (TRAINER_DRAKE_2) | Feraligatr 72, Dragonite 73, Charizard 72, Sceptile 72, Kingdra 73, Salamence 75 |
| Steven (Meteor Falls) (TRAINER_STEVEN_2) | Armaldo 76, Tyranitar 75, Aerodactyl 77, Cradily 76, Steelix 76, Metagross 78 |

Full held items and movesets for every boss are in emerald_legacy.json.
