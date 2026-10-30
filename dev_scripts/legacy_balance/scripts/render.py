import json, sys
src, dst, vhash, lhash = sys.argv[1:5]
d = json.load(open(src, encoding='utf-8'))
S = list(d['species'].values())
cats = ['stats', 'types', 'abilities', 'evolution', 'learnset', 'tmhm', 'tutor']
cnt = {k: sum(k in r for r in S) for k in cats}
core = [r for r in S if any(k in r for k in ['stats', 'types', 'abilities', 'evolution', 'learnset'])]
ls_addrem = sum(1 for r in S if 'learnset' in r and (r['learnset']['added'] or r['learnset']['removed']))
ls_only_lv = cnt['learnset'] - ls_addrem
other_nonegg = [r for r in S if set(r.get('other_fields', {})) - {'eggCycles'}]

d['meta'] = {'emerald_legacy_repo': 'https://github.com/cRz-Shadows/Pokemon_Emerald_Legacy', 'emerald_legacy_commit': lhash,
             'vanilla_repo': 'https://github.com/pret/pokeemerald', 'vanilla_commit': vhash,
             'counts_of_151_changed': cnt, 'species_with_any_core_change': len(core),
             'learnset_species_with_add_or_remove': ls_addrem, 'learnset_species_level_shift_only': ls_only_lv}
json.dump(d, open(src, 'w', encoding='utf-8'), indent=1, ensure_ascii=False)

o = []
w = o.append
w('# Emerald Legacy vs vanilla Emerald: Gen 1 species and boss level curve\n')
w(f'- Emerald Legacy: cRz-Shadows/Pokemon_Emerald_Legacy @ `{lhash}`')
w(f'- Base / vanilla reference: pret/pokeemerald @ `{vhash}` (Legacy README states it is built on the pret Emerald disassembly; data file layout matches).')
w('- Source files compared: `src/data/pokemon/{species_info,evolution,level_up_learnsets,level_up_learnset_pointers,tmhm_learnsets,tutor_learnsets}.h`, `src/data/{trainers,trainer_parties}.h`, map scripts for which trainer IDs are used.')
w('- Parse problems: ' + ('; '.join(d['problems']) if d['problems'] else 'none; all 151 species, all learnsets and all boss parties parsed.'))
w('- Not covered: changes to move data, ability effects or battle code (only species data is compared). Legacy has no new moves, types or abilities (constants files identical apart from 3 extra Deoxys form species).')
w('- Global change not listed per species: egg cycles were cut by roughly 25% for every species (e.g. 20→15, 40→30, 120→90).\n')

w('## Summary counts (of 151)\n')
w('| Category | Species changed |\n|---|---|')
lab = {'stats': 'Base stats', 'types': 'Types', 'abilities': 'Abilities', 'evolution': 'Evolution method/level', 'learnset': 'Level-up learnset', 'tmhm': 'TM/HM compatibility', 'tutor': 'Move tutor compatibility'}
for k in cats: w(f'| {lab[k]} | {cnt[k]} |')
w(f'| Any of stats/types/abilities/evolution/learnset | {len(core)} (unchanged: {151 - len(core)}) |')
w(f'\nLearnsets: {ls_addrem} species gained or lost moves; {ls_only_lv} only had level changes. Other species fields changed (besides egg cycles): ' +
  (', '.join(f"{r['name']} {r['other_fields']}" for r in other_nonegg) or 'none') + '.\n')

w('## Stat / type / ability changes\n')
w(f'{sum(1 for r in S if any(k in r for k in ("stats","types","abilities")))} species changed; the other {151 - sum(1 for r in S if any(k in r for k in ("stats","types","abilities")))} are unchanged.\n')
w('| # | Species | HP | Atk | Def | SpA | SpD | Spe | BST | Abilities |\n|---|---|---|---|---|---|---|---|---|---|')
for r in S:
    if not any(k in r for k in ('stats', 'types', 'abilities')): continue
    cells = []
    for s in ['HP', 'Atk', 'Def', 'SpA', 'SpD', 'Spe', 'BST']:
        if 'stats' in r:
            a, b = r['stats']['vanilla'][s], r['stats']['legacy'][s]
            cells.append(f'**{a}→{b}**' if a != b else str(a))
        else: cells.append('=')
    ab = (' / '.join(r['abilities']['vanilla']) + ' → ' + ' / '.join(r['abilities']['legacy'])) if 'abilities' in r else ''
    ty = (' types ' + '/'.join(r['types']['vanilla']) + '→' + '/'.join(r['types']['legacy'])) if 'types' in r else ''
    w(f"| {r['num']} | {r['name']} | " + ' | '.join(cells) + f' | {ab}{ty} |')

w('\n## Evolution changes\n')
w('| # | Species | Vanilla | Legacy |\n|---|---|---|---|')
for r in S:
    if 'evolution' in r:
        w(f"| {r['num']} | {r['name']} | {'; '.join(r['evolution']['vanilla'])} | {'; '.join(r['evolution']['legacy'])} |")
w('\nTrade evolutions: every trade evolution was removed. Plain trade evos (Kadabra, Machoke, Graveler, Haunter) became level-ups (42/38/38/42). '
  'Trade-with-item evos (Poliwhirl→Politoed, Slowpoke→Slowking, Onix→Steelix, Seadra→Kingdra, Scyther→Scizor, Porygon→Porygon2) are now used from the bag like evolution stones; '
  'Legacy changes King\'s Rock, Metal Coat, Dragon Scale, Up-Grade and Deep Sea Tooth/Scale to `ItemUseOutOfBattle_EvolutionStone` in `items.h`. The only other change is Goldeen evolving at Lv 28 instead of 33.')
if d['evolutions_into_gen1_from_other_gens']:
    w('\nEvolutions into Gen 1 from later-gen species: ' + json.dumps(d['evolutions_into_gen1_from_other_gens']))

w('\n## Level-up learnset changes\n')
w('Format: `+Move L` added, `-Move L` removed, `Move a→b` level changed (`1/17` = learned at both levels).\n')
w('| # | Species | Changes |\n|---|---|---|')
for r in S:
    if 'learnset' in r: w(f"| {r['num']} | {r['name']} | {r['learnset']['summary']} |")
unch = [r['name'] for r in S if 'learnset' not in r]
w(f'\nUnchanged learnsets ({len(unch)}): ' + ', '.join(unch) + '.')

w('\n## TM/HM and tutor compatibility additions\n')
w('| # | Species | TM/HM added | Tutor added |\n|---|---|---|---|')
for r in S:
    if 'tmhm' in r or 'tutor' in r:
        t = ', '.join(r.get('tmhm', {}).get('added', [])); u = ', '.join(r.get('tutor', {}).get('added', []))
        rm = r.get('tmhm', {}).get('removed', []) + r.get('tutor', {}).get('removed', [])
        w(f"| {r['num']} | {r['name']} | {t} | {u}{' (removed: ' + ', '.join(rm) + ')' if rm else ''} |")

def fp(p):
    return ', '.join(f"{m['species']} {m['level']}" for m in p['party'])
def lv(p):
    L = [m['level'] for m in p['party']]
    return min(L), max(L)
w('\n## Boss level curve (first battles)\n')
w('Legacy changes who fills some slots: Brawly has three versions picked by how many other gyms you have beaten (Dewford gym order is flexible: `BRAWLY_1`, `BRAWLY_1_2` if Mauville gym already beaten, `BRAWLY_1_3` if Lavaridge gym already beaten); '
  'the 8th gym is fought against Wallace (`TRAINER_WALLACE_1`) before the Hall of Fame, with Juan returning post-game (the Legacy `sParty_Juan1` is commented as an Original Base Gym Leader Placeholder and is identical to vanilla, so it is omitted below; his post-game fights use the rematch tables); the first Champion is Steven (`TRAINER_STEVEN_1`), and Wallace (`TRAINER_WALLACE_2`) becomes Champion on rematch.\n')
w('| Slot | Vanilla party (lv) | Vanilla ace | Legacy party (lv) | Legacy ace | Ace Δ |\n|---|---|---|---|---|---|')
for b in d['bosses']:
    v = b['vanilla']; va = lv(v)[1]
    for i, l in enumerate(b['legacy']):
        la = lv(l)[1]
        slot = b['slot'] + ('' if len(b['legacy']) == 1 else f" ({l['trainer_id'].replace('TRAINER_', '')})")
        dbl = ' (double)' if l['double'] else ''
        vc = (fp(v) + (' (double)' if v['double'] else '')) if i == 0 else '〃'
        w(f"| {slot} | {vc} | {va if i == 0 else '〃'} | {fp(l)}{dbl} | {la} | {la - va:+d} |")
w('\n### Post-game / rematch bosses (Legacy only)\n')
w('| Slot | Legacy party (lv) |\n|---|---|')
for b in d['postgame_bosses']:
    w(f"| {b['slot']} ({b['legacy']['trainer_id']}) | {fp(b['legacy'])} |")
w('\nFull held items and movesets for every boss are in emerald_legacy.json.')
open(dst, 'w', encoding='utf-8').write('\n'.join(o) + '\n')
