import json, sys, os
from collections import Counter

ROOT = sys.argv[1]
r = json.load(open(os.path.join(ROOT, 'crystal_legacy.json')))
S = r['species']
o = []
w = o.append
T = lambda s: s.title().replace('__', '. ').replace('_', ' ')

cnt = Counter(k for d in S.values() for k in d if k != 'dex')
w('# Crystal Legacy vs vanilla Pokémon Crystal — Gen 1 data diff\n')
w(f"- Crystal Legacy: `cRz-Shadows/Pokemon_Crystal_Legacy` @ `{r['commits']['crystal_legacy']}`")
w(f"- Base: `pret/pokecrystal` @ `{r['commits']['pokecrystal']}` (Legacy README states it is built on the pret Crystal disassembly; same file layout)")
w('- Method: parsed `data/pokemon/base_stats/*.asm`, `data/pokemon/evos_attacks.asm`, `data/moves/moves.asm`, `data/trainers/parties.asm` in both trees (scripts in `scripts/crystal_compare.py`, `scripts/crystal_report.py`).')
w(f"- Unparsed / parse issues: {'none' if not r['unparsed'] else r['unparsed']}. Species/move constant order is identical in both trees; move names unchanged; type chart (`data/types/type_matchups.asm`) identical.")
w('- Engine note (not data): Legacy changes the physical/special check so **Ghost-type moves are special and Dark-type moves are physical** (`engine/battle/effect_commands.asm`, `cp GHOST -> .special`, `cp DARK -> .physical`). Still a per-type split, no per-move category.')
w('- Trade-evolution items (King\'s Rock, Metal Coat, Dragon Scale, Up-Grade, Brick Piece) get a party-menu use in `data/items/attributes.asm` (vanilla: NOUSE), price 9000.')
w('- Legacy has a Hard Mode flag (`ENGINE_HARD_MODE`) that sets `BATTLETYPE_SETNOITEMS` for boss fights (seen in ViridianGym/SilverCaveRoom3).\n')
w(f"## Counts (Gen 1, #1–151)\n")
w(f"- Base stats changed: {cnt['stats']}; types changed: {cnt.get('types', 0)}; catch rate/base exp/growth changed: {cnt.get('catch', 0)}/{cnt.get('base_exp', 0)}/{cnt.get('growth', 0)}")
w(f"- Evolution changed: {cnt['evo']}; level-up learnset changed: {cnt['learnset']}; TM/HM compatibility changed: {cnt['tmhm']}")
w(f"- Species with any change: {sum(1 for d in S.values() if len(d) > 1)} / 151\n")

w('## Base stat changes\n')
w('Order HP/Atk/Def/SpA/SpD/Spe (Gen 2 split special). BST delta in last column. No Gen 1 type changes.\n')
w('| # | Species | Vanilla | Legacy | Changed | ΔBST |')
w('|---|---|---|---|---|---|')
lab = ['HP', 'Atk', 'Def', 'SpA', 'SpD', 'Spe']
for s, d in S.items():
    if 'stats' in d:
        a, b = d['stats']['vanilla'], d['stats']['legacy']
        ch = ', '.join(f'{lab[i]} {a[i]}→{b[i]}' for i in range(6) if a[i] != b[i])
        w(f"| {d['dex']} | {T(s)} | {'/'.join(map(str, a))} | {'/'.join(map(str, b))} | {ch} | {sum(b) - sum(a):+d} |")

w('\n## Evolution changes\n')
w('| # | Species | Vanilla | Legacy |')
w('|---|---|---|---|')
for s, d in S.items():
    if 'evo' in d:
        w(f"| {d['dex']} | {T(s)} | {'; '.join(d['evo']['vanilla'])} | {'; '.join(d['evo']['legacy'])} |")
w('\nUnchanged (relevant): Golbat→Crobat and Chansey→Blissey stay happiness (TR_ANYTIME, threshold 220 in both); Eevee item/happiness evolutions unchanged; Pichu/Cleffa/Igglybuff happiness unchanged; Smoochum/Elekid/Magby Lv30 unchanged.')
for s, d in r['baby_evolutions'].items():
    if 'vanilla' in d:
        w(f"- Baby with change: {T(s)}: {'; '.join(d['vanilla'])} → {'; '.join(d['legacy'])}")

w('\n## Level-up learnset changes\n')
w('Format: `+MOVE Lv` added, `-MOVE Lv` removed, `MOVE a→b` level changed (Lv1 = start move).\n')
w('| # | Species | Added | Removed | Level changed |')
w('|---|---|---|---|---|')
f = lambda lv: '/'.join(map(str, lv))
for s, d in S.items():
    if 'learnset' in d:
        L = d['learnset']
        add = ', '.join(f"{T(m)} {f(v)}" for m, v in L['added'].items())
        rem = ', '.join(f"{T(m)} {f(v)}" for m, v in L['removed'].items())
        chg = ', '.join(f"{T(m)} {f(v[0])}→{f(v[1])}" for m, v in L['level_changed'].items())
        w(f"| {d['dex']} | {T(s)} | {add} | {rem} | {chg} |")
unch = [T(s) for s, d in S.items() if 'learnset' not in d]
w(f"\nUnchanged learnsets ({len(unch)}): {', '.join(unch)}")

w('\n## TM/HM compatibility changes (Gen 1)\n')
w('| # | Species | Added | Removed |')
w('|---|---|---|---|')
for s, d in S.items():
    if 'tmhm' in d:
        w(f"| {d['dex']} | {T(s)} | {', '.join(T(x) for x in d['tmhm']['added'])} | {', '.join(T(x) for x in d['tmhm']['removed'])} |")

w(f"\n## Move data changes ({len(r['moves_changed'])} moves)\n")
w('| Move | Power | Acc | PP | Type | Other |')
w('|---|---|---|---|---|---|')
for m, d in r['moves_changed'].items():
    g = lambda k: f"{d[k][0]}→{d[k][1]}" if k in d else ''
    other = '; '.join(f"{k} {d[k][0]}→{d[k][1]}" for k in d if k in ('effect', 'chance'))
    w(f"| {T(m)} | {g('power')} | {g('acc')} | {g('pp')} | {g('type')} | {other} |")

w('\n## Boss parties (levels/species)\n')
w('Vanilla Crystal has one fixed team per boss. Legacy entries labelled by the comment in parties.asm. Ace = highest level.\n')
w('| Boss | Vanilla party | Van. max | Legacy party | Leg. max | Δ max |')
w('|---|---|---|---|---|---|')
labels = {
    'PryceGroup': ['1st of Pryce/Jasmine/Chuck', '2nd', '3rd', 'Rematch'],
    'JasmineGroup': ['1st of Pryce/Jasmine/Chuck', '2nd', '3rd', 'Rematch'],
    'ChuckGroup': ['1st of Pryce/Jasmine/Chuck', '2nd', '3rd', 'Rematch'],
    'BlueGroup': ['(1) fought', '(2) unused alt, TODO comment'],
    'RedGroup': ['(1)', '(2) rematch'],
}
P = lambda p: ', '.join(f"{T(m['species'])} {m['lv']}" for m in p)
for g, ves in r['bosses']['vanilla'].items():
    les = r['bosses']['legacy'][g]
    v = ves[0]; vmax = max(m['lv'] for m in v['party'])
    for i, e in enumerate(les):
        lmax = max(m['lv'] for m in e['party'])
        lab = labels.get(g, ['', 'Rematch'])[i] if i < len(labels.get(g, ['', 'Rematch'])) else f'({i + 1})'
        name = T(e['name']) + (f" — {lab}" if lab else '')
        w(f"| {name} | {P(v['party']) if i == 0 else '″'} | {vmax if i == 0 else ''} | {P(e['party'])} | {lmax} | {lmax - vmax:+d} |")
w('\nNotes: Legacy gym leaders and E4 carry held items on every mon (TRAINERTYPE_ITEM_MOVES); E4/Lance/Red rematch teams also set DVs/stat exp. Pryce/Jasmine/Chuck scale by the order you fight them (Legacy makes the mid-game nonlinear); Jasmine 1st and 2nd are identical. Blue (1) is the team loaded by `ViridianGym.asm` (BLUE1). Red (2) loads when `EVENT_CERULEAN_CAVE_B1F_MEWTWO` is set.')
open(os.path.join(ROOT, 'crystal_legacy.md'), 'w').write('\n'.join(o) + '\n')
print(len(o))
