import re, sys, json, os
from collections import defaultdict, OrderedDict

ROOT = sys.argv[1]
V = os.path.join(ROOT, 'pokeemerald')
L = os.path.join(ROOT, 'emerald_legacy')
problems = []

def rd(base, p):
    with open(os.path.join(base, p), encoding='utf-8', errors='replace') as f:
        return f.read()

def strip_comments(s):
    s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    return re.sub(r'//[^\n]*', '', s)

# species 1..151
sp = rd(V, 'include/constants/species.h')
GEN1 = []
for m in re.finditer(r'#define SPECIES_(\w+)\s+(\d+)', sp):
    if 1 <= int(m.group(2)) <= 151:
        GEN1.append((int(m.group(2)), m.group(1)))
GEN1.sort()
assert len(GEN1) == 151, len(GEN1)
G1 = {n for _, n in GEN1}
NUM = {n: i for i, n in GEN1}

def nice(c):
    c = re.sub(r'^(SPECIES|MOVE|ABILITY|TYPE|ITEM)_', '', c)
    special = {'NIDORAN_F': 'Nidoran-F', 'NIDORAN_M': 'Nidoran-M', 'MR_MIME': 'Mr. Mime', 'FARFETCHD': "Farfetch'd"}
    if c in special: return special[c]
    return ' '.join(w.capitalize() for w in c.split('_'))

def blocks(text, key_re):
    """split text into {name: body} by occurrences of [KEY_name]"""
    out = OrderedDict()
    ms = list(re.finditer(key_re, text))
    for i, m in enumerate(ms):
        end = ms[i + 1].start() if i + 1 < len(ms) else len(text)
        out.setdefault(m.group(1), text[m.end():end])
    return out

# ---------- species info ----------
def species_info(base):
    t = strip_comments(rd(base, 'src/data/pokemon/species_info.h'))
    t = t[t.index('gSpeciesInfo'):]
    res = {}
    for name, body in blocks(t, r'\[SPECIES_(\w+)\]\s*=').items():
        if name not in G1: continue
        d = {}
        for m in re.finditer(r'\.(\w+)\s*=\s*(\{[^}]*\}|[^,\n]+)', body):
            d[m.group(1)] = re.sub(r'\s+', ' ', m.group(2).strip())
        res[name] = d
    return res

def parse_list(v):
    return [x.strip() for x in v.strip('{} ').split(',') if x.strip()]

STATS = [('baseHP', 'HP'), ('baseAttack', 'Atk'), ('baseDefense', 'Def'), ('baseSpAttack', 'SpA'), ('baseSpDefense', 'SpD'), ('baseSpeed', 'Spe')]

def norm_species(d):
    try:
        st = {k2: int(d[k]) for k, k2 in STATS}
    except Exception as e:
        problems.append(f'stat parse failed: {d}')
        st = {}
    st['BST'] = sum(st.values())
    types = [nice(x) for x in parse_list(d.get('types', ''))]
    if len(types) == 2 and types[0] == types[1]: types = types[:1]
    abil = [nice(x) for x in parse_list(d.get('abilities', '')) if x != 'ABILITY_NONE']
    other = {k: v for k, v in d.items() if k not in dict(STATS) and k not in ('types', 'abilities')}
    return {'stats': st, 'types': types, 'abilities': abil, 'other': other}

# ---------- evolutions ----------
def evolutions(base):
    t = strip_comments(rd(base, 'src/data/pokemon/evolution.h'))
    res = defaultdict(list)
    for name, body in blocks(t, r'\[SPECIES_(\w+)\]\s*=').items():
        for m in re.finditer(r'\{\s*(EVO_\w+)\s*,\s*([^,]+?)\s*,\s*SPECIES_(\w+)\s*\}', body):
            res[name].append((m.group(1), m.group(2), m.group(3)))
    return res

def fmt_evo(e):
    method, p, tgt = e
    tgt = nice(tgt)
    if method == 'EVO_LEVEL': return f'Lv {p} -> {tgt}'
    if method == 'EVO_ITEM': return f'{nice(p)} -> {tgt}'
    if method == 'EVO_TRADE': return f'Trade -> {tgt}'
    if method == 'EVO_TRADE_ITEM': return f'Trade holding {nice(p)} -> {tgt}'
    if method == 'EVO_FRIENDSHIP': return f'Friendship -> {tgt}'
    if method == 'EVO_FRIENDSHIP_DAY': return f'Friendship (day) -> {tgt}'
    if method == 'EVO_FRIENDSHIP_NIGHT': return f'Friendship (night) -> {tgt}'
    return f'{method.replace("EVO_", "")} {p} -> {tgt}'

# ---------- learnsets ----------
def learnsets(base):
    ptrs = strip_comments(rd(base, 'src/data/pokemon/level_up_learnset_pointers.h'))
    pm = dict(re.findall(r'\[SPECIES_(\w+)\]\s*=\s*(\w+)', ptrs))
    t = strip_comments(rd(base, 'src/data/pokemon/level_up_learnsets.h'))
    arrs = {}
    for m in re.finditer(r'(\w+)\[\]\s*=\s*\{(.*?)\};', t, re.S):
        arrs[m.group(1)] = [(int(a), b) for a, b in re.findall(r'LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*MOVE_(\w+)\s*\)', m.group(2))]
    res = {}
    for n in G1:
        if n not in pm or pm[n] not in arrs:
            problems.append(f'learnset missing for {n} in {base}')
            continue
        res[n] = arrs[pm[n]]
    return res

def learnset_diff(a, b):
    va, vb = defaultdict(list), defaultdict(list)
    for lv, mv in a: va[mv].append(lv)
    for lv, mv in b: vb[mv].append(lv)
    added, removed, changed = [], [], []
    for mv in sorted(set(va) | set(vb), key=lambda m: (min(vb.get(m, va.get(m))), m)):
        x, y = sorted(va.get(mv, [])), sorted(vb.get(mv, []))
        if x == y: continue
        if not x: added.append((mv, y))
        elif not y: removed.append((mv, x))
        else: changed.append((mv, x, y))
    return added, removed, changed

def fmt_ls(added, removed, changed):
    parts = []
    for mv, y in added: parts.append(f'+{nice(mv)} {"/".join(map(str, y))}')
    for mv, x in removed: parts.append(f'-{nice(mv)} {"/".join(map(str, x))}')
    for mv, x, y in changed: parts.append(f'{nice(mv)} {"/".join(map(str, x))}→{"/".join(map(str, y))}')
    return ', '.join(parts)

# ---------- TM / tutor ----------
def tmhm(base):
    t = strip_comments(rd(base, 'src/data/pokemon/tmhm_learnsets.h'))
    t = t[t.index('gTMHMLearnsets'):]
    return {n: set(re.findall(r'\.(\w+)\s*=\s*TRUE', b)) for n, b in blocks(t, r'\[SPECIES_(\w+)\]\s*=').items() if n in G1}

def tutor(base):
    t = strip_comments(rd(base, 'src/data/pokemon/tutor_learnsets.h'))
    t = t[t.index('sTutorLearnsets') if 'sTutorLearnsets' in t else 0:]
    return {n: set(re.findall(r'TUTOR\(MOVE_(\w+)\)', b)) for n, b in blocks(t, r'\[SPECIES_(\w+)\]\s*=').items() if n in G1}

# ---------- trainers ----------
def parties(base):
    t = strip_comments(rd(base, 'src/data/trainer_parties.h'))
    res = {}
    for m in re.finditer(r'(sParty_\w+)\[\]\s*=\s*\{(.*?)\n\};', t, re.S):
        mons = []
        for mm in re.finditer(r'\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\}', m.group(2)):
            body = mm.group(1)
            lv = re.search(r'\.lvl\s*=\s*(\d+)', body)
            spc = re.search(r'\.species\s*=\s*SPECIES_(\w+)', body)
            if not (lv and spc): continue
            it = re.search(r'\.heldItem\s*=\s*ITEM_(\w+)', body)
            mv = re.search(r'\.moves\s*=\s*\{([^}]*)\}', body)
            mons.append({'species': nice(spc.group(1)), 'level': int(lv.group(1)),
                         'item': nice(it.group(1)) if it and it.group(1) != 'NONE' else None,
                         'moves': [nice(x) for x in parse_list(mv.group(1)) if x != 'MOVE_NONE'] if mv else None})
        res[m.group(1)] = mons
    return res

def trainers(base):
    t = strip_comments(rd(base, 'src/data/trainers.h'))
    res = {}
    for name, body in blocks(t, r'\[TRAINER_(\w+)\]\s*=').items():
        p = re.search(r'(sParty_\w+)', body)
        db = re.search(r'\.doubleBattle\s*=\s*(\w+)', body)
        res[name] = {'party': p.group(1) if p else None, 'double': db and db.group(1) == 'TRUE'}
    return res

# Boss slots: (label, vanilla trainer id, legacy trainer id(s))
BOSSES = [
    ('Gym 1 Roxanne', 'ROXANNE_1', ['ROXANNE_1']),
    ('Gym 2 Brawly', 'BRAWLY_1', ['BRAWLY_1', 'BRAWLY_1_2', 'BRAWLY_1_3']),
    ('Gym 3 Wattson', 'WATTSON_1', ['WATTSON_1']),
    ('Gym 4 Flannery', 'FLANNERY_1', ['FLANNERY_1']),
    ('Gym 5 Norman', 'NORMAN_1', ['NORMAN_1']),
    ('Gym 6 Winona', 'WINONA_1', ['WINONA_1']),
    ('Gym 7 Tate & Liza', 'TATE_AND_LIZA_1', ['TATE_AND_LIZA_1']),
    ('Gym 8 Sootopolis', 'JUAN_1', ['WALLACE_1']),
    ('E4 Sidney', 'SIDNEY', ['SIDNEY']),
    ('E4 Phoebe', 'PHOEBE', ['PHOEBE']),
    ('E4 Glacia', 'GLACIA', ['GLACIA']),
    ('E4 Drake', 'DRAKE', ['DRAKE']),
    ('Champion', 'WALLACE', ['STEVEN_1']),
]
POST = [('Postgame Champion (Wallace)', 'WALLACE_2'),
        ('E4 rematch Sidney', 'SIDNEY_2'), ('E4 rematch Phoebe', 'PHOEBE_2'), ('E4 rematch Glacia', 'GLACIA_2'),
        ('E4 rematch Drake', 'DRAKE_2'), ('Steven (Meteor Falls)', 'STEVEN_2')]

def main():
    out = {'meta': {}, 'species': {}, 'bosses': [], 'postgame_bosses': [], 'problems': problems}
    si = {k: species_info(b) for k, b in (('v', V), ('l', L))}
    ev = {k: evolutions(b) for k, b in (('v', V), ('l', L))}
    ls = {k: learnsets(b) for k, b in (('v', V), ('l', L))}
    tm = {k: tmhm(b) for k, b in (('v', V), ('l', L))}
    tu = {k: tutor(b) for k, b in (('v', V), ('l', L))}
    for num, n in GEN1:
        if n not in si['v'] or n not in si['l']:
            problems.append(f'species_info missing {n}'); continue
        a, b = norm_species(si['v'][n]), norm_species(si['l'][n])
        rec = {'num': num, 'name': nice(n)}
        if a['stats'] != b['stats']:
            rec['stats'] = {'vanilla': a['stats'], 'legacy': b['stats']}
        if a['types'] != b['types']:
            rec['types'] = {'vanilla': a['types'], 'legacy': b['types']}
        if a['abilities'] != b['abilities']:
            rec['abilities'] = {'vanilla': a['abilities'], 'legacy': b['abilities']}
        oth = {k: [a['other'].get(k), b['other'].get(k)] for k in set(a['other']) | set(b['other'])
               if a['other'].get(k) != b['other'].get(k)}
        if oth: rec['other_fields'] = oth
        ea, eb = ev['v'].get(n, []), ev['l'].get(n, [])
        if ea != eb:
            rec['evolution'] = {'vanilla': [fmt_evo(e) for e in ea], 'legacy': [fmt_evo(e) for e in eb]}
        if n in ls['v'] and n in ls['l']:
            ad, rm, ch = learnset_diff(ls['v'][n], ls['l'][n])
            if ad or rm or ch:
                rec['learnset'] = {'summary': fmt_ls(ad, rm, ch),
                                   'added': [[nice(m), y] for m, y in ad], 'removed': [[nice(m), x] for m, x in rm],
                                   'level_changed': [[nice(m), x, y] for m, x, y in ch],
                                   'legacy_full': [[lv, nice(m)] for lv, m in ls['l'][n]]}
        tma, tmb = tm['v'].get(n, set()), tm['l'].get(n, set())
        if tma != tmb:
            rec['tmhm'] = {'added': sorted(nice(x) for x in tmb - tma), 'removed': sorted(nice(x) for x in tma - tmb)}
        tua, tub = tu['v'].get(n, set()), tu['l'].get(n, set())
        if tua != tub:
            rec['tutor'] = {'added': sorted(nice(x) for x in tub - tua), 'removed': sorted(nice(x) for x in tua - tub)}
        out['species'][n] = rec
    # evolutions into gen1 from non-gen1 species
    extra = []
    for src in set(ev['v']) | set(ev['l']):
        if src in G1: continue
        a = [e for e in ev['v'].get(src, []) if e[2] in G1]
        b = [e for e in ev['l'].get(src, []) if e[2] in G1]
        if a != b: extra.append({'from': nice(src), 'vanilla': [fmt_evo(e) for e in a], 'legacy': [fmt_evo(e) for e in b]})
    out['evolutions_into_gen1_from_other_gens'] = extra
    # bosses
    pv, pl = parties(V), parties(L)
    tv, tl = trainers(V), trainers(L)
    def party_of(tr, pt, tid):
        if tid not in tr: problems.append(f'trainer {tid} missing'); return None
        p = tr[tid]['party']
        if p not in pt: problems.append(f'party {p} for {tid} missing'); return None
        return {'trainer_id': 'TRAINER_' + tid, 'double': tr[tid]['double'], 'party': pt[p]}
    for label, vid, lids in BOSSES:
        rec = {'slot': label, 'vanilla': party_of(tv, pv, vid), 'legacy': [party_of(tl, pl, x) for x in lids]}
        out['bosses'].append(rec)
    for label, lid in POST:
        out['postgame_bosses'].append({'slot': label, 'legacy': party_of(tl, pl, lid)})
    json.dump(out, sys.stdout, indent=1, ensure_ascii=False)

main()
