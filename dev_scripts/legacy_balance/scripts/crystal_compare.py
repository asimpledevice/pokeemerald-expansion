import re, json, sys, os

ROOT = sys.argv[1]
V = os.path.join(ROOT, 'pokecrystal')
L = os.path.join(ROOT, 'crystal_legacy')
unparsed = []


def rd(base, p):
    with open(os.path.join(base, p), encoding='utf-8', errors='replace') as f:
        return f.read()


def strip_comments(s):
    out = []
    for line in s.split('\n'):
        # remove ; comments not inside quotes
        res, q = '', False
        for ch in line:
            if ch == '"':
                q = not q
            if ch == ';' and not q:
                break
            res += ch
        out.append(res)
    return '\n'.join(out)


# ---------- species list ----------
def species_list(base):
    txt = rd(base, 'constants/pokemon_constants.asm')
    names = []
    for m in re.finditer(r'^\s*const\s+(\w+)', txt, re.M):
        n = m.group(1)
        if n == 'NO_MON':
            continue
        names.append(n)
        if len(names) == 251:
            break
    return names


SPV = species_list(V)
SPL = species_list(L)
assert SPV[:151] == SPL[:151], 'species order differs'
GEN1 = SPV[:151]


# ---------- base stats ----------
def base_stats(base):
    inc = rd(base, 'data/pokemon/base_stats.asm')
    files = re.findall(r'INCLUDE\s+"(data/pokemon/base_stats/[^"]+)"', inc)
    res = {}
    for f in files:
        t = strip_comments(rd(base, f))
        sp = re.search(r'^\s*db\s+(\w+)\s*$', t, re.M).group(1)
        dbs = re.findall(r'^\s*db\s+(.+)$', t, re.M)
        stats = [int(x) for x in dbs[1].split(',')]  # hp atk def spd sat sdf
        types = [x.strip() for x in dbs[2].split(',')]
        catch = int(dbs[3]); bexp = int(dbs[4])
        growth = re.search(r'db\s+(GROWTH_\w+)', t).group(1)
        tm = re.search(r'tmhm\s+(.*)', t)
        tms = [x.strip() for x in tm.group(1).split(',')] if tm and tm.group(1).strip() else []
        hp, atk, de, spe, sat, sdf = stats
        res[sp] = dict(hp=hp, atk=atk, def_=de, spa=sat, spd=sdf, spe=spe,
                       types=sorted(set(types), key=types.index), catch=catch, base_exp=bexp,
                       growth=growth, tmhm=tms)
    return res


BSV, BSL = base_stats(V), base_stats(L)


# ---------- evos/attacks ----------
def evos(base):
    ptr = strip_comments(rd(base, 'data/pokemon/evos_attacks_pointers.asm'))
    labels = re.findall(r'dw\s+(\w+)', ptr)
    sp = species_list(base)
    lab2sp = {lab: sp[i] for i, lab in enumerate(labels) if i < len(sp)}
    t = strip_comments(rd(base, 'data/pokemon/evos_attacks.asm'))
    res = {}
    cur = None; stage = None
    for line in t.split('\n'):
        line = line.strip()
        if not line:
            continue
        m = re.match(r'^(\w+):$', line)
        if m:
            cur = lab2sp.get(m.group(1)); stage = 'evo'
            if cur:
                res[cur] = {'evos': [], 'moves': []}
            continue
        if cur is None:
            continue
        m = re.match(r'^db\s+(.*)$', line)
        if not m:
            continue
        parts = [x.strip() for x in m.group(1).split(',')]
        if parts == ['0']:
            stage = 'moves' if stage == 'evo' else 'done'
            continue
        if stage == 'evo':
            res[cur]['evos'].append(parts)
        elif stage == 'moves':
            res[cur]['moves'].append((int(parts[0]), parts[1]))
    return res


EV, EL = evos(V), evos(L)


def fmt_evo(e):
    meth = e[0].replace('EVOLVE_', '')
    if meth == 'LEVEL':
        return f'{e[2]} @Lv{e[1]}'
    if meth == 'ITEM':
        return f'{e[2]} via {e[1]}'
    if meth == 'TRADE':
        return f'{e[2]} via trade' + ('' if e[1] in ('-1', '$ff') else f' holding {e[1]}')
    if meth == 'HAPPINESS':
        return f'{e[2]} via happiness ({e[1]})'
    if meth == 'STAT':
        return f'{e[3]} @Lv{e[1]} if {e[2]}'
    return ' '.join(e)


# ---------- moves ----------
def moves(base):
    names = []
    for m in re.finditer(r'^\s*const\s+(\w+)', rd(base, 'constants/move_constants.asm'), re.M):
        names.append(m.group(1))
    t = strip_comments(rd(base, 'data/moves/moves.asm'))
    res = {}
    for m in re.finditer(r'^\s*move\s+(.*)$', t, re.M):
        p = [x.strip() for x in m.group(1).split(',')]
        res[p[0]] = dict(effect=p[1], power=int(p[2]), type=p[3], acc=int(p[4]), pp=int(p[5]), chance=int(p[6]))
    return res


MV, ML = moves(V), moves(L)
if len(MV) != 251 or len(ML) != 251:
    unparsed.append(f'move counts V={len(MV)} L={len(ML)}')


def move_names(base):
    t = rd(base, 'data/moves/names.asm')
    return re.findall(r'li\s+"([^"]*)"', t)


MNV, MNL = move_names(V), move_names(L)
mconst = list(MV.keys())
renamed = {}
for i, k in enumerate(mconst):
    if i < len(MNV) and i < len(MNL) and MNV[i] != MNL[i]:
        renamed[k] = [MNV[i], MNL[i]]

# ---------- trainers ----------
BOSS_GROUPS = ['FalknerGroup', 'BugsyGroup', 'WhitneyGroup', 'MortyGroup', 'ChuckGroup', 'JasmineGroup', 'PryceGroup', 'ClairGroup',
               'BrockGroup', 'MistyGroup', 'LtSurgeGroup', 'ErikaGroup', 'JanineGroup', 'SabrinaGroup', 'BlaineGroup', 'BlueGroup',
               'WillGroup', 'KogaGroup', 'BrunoGroup', 'KarenGroup', 'ChampionGroup', 'RedGroup']


def parties(base):
    t = strip_comments(rd(base, 'data/trainers/parties.asm'))
    # split by group labels
    groups = {}
    pos = [(m.group(1), m.start(), m.end()) for m in re.finditer(r'^\s*(\w+Group):', t, re.M)]
    for i, (g, s, e) in enumerate(pos):
        end = pos[i + 1][1] if i + 1 < len(pos) else len(t)
        groups[g] = t[e:end]
    out = {}
    for g in BOSS_GROUPS:
        body = groups.get(g)
        if body is None:
            unparsed.append(f'{base}: missing {g}'); continue
        toks = []
        for m in re.finditer(r'^\s*(db|dw)\s+(.*)$', body, re.M):
            for x in re.findall(r'"[^"]*"|[^,]+', m.group(2)):
                x = x.strip()
                if x:
                    toks.append(x)
        i = 0; entries = []
        while i < len(toks):
            if not toks[i].startswith('"'):
                unparsed.append(f'{base}:{g}: unexpected token {toks[i]}'); break
            name = toks[i].strip('"').rstrip('@'); ttype = toks[i + 1]; i += 2
            flags = set(x.strip() for x in ttype.split('|'))
            f = lambda k: any(k in x for x in flags)
            moves_f = f('TRAINERTYPE_MOVES') or f('ITEM_MOVES')
            item_f = f('TRAINERTYPE_ITEM') or f('ITEM_MOVES')
            mons = []
            while toks[i] != '-1':
                lv = int(toks[i]); sp = toks[i + 1]; i += 2
                mon = {'lv': lv, 'species': sp}
                if f('NICKNAME'): mon['nick'] = toks[i]; i += 1
                if f('DVS'): i += 2
                if f('STAT_EXP'): i += 5
                if f('HAPPINESS'): i += 1
                if item_f: mon['item'] = toks[i]; i += 1
                if moves_f: mon['moves'] = toks[i:i + 4]; i += 4
                mons.append(mon)
            i += 1
            entries.append({'name': name, 'type': ttype, 'party': mons})
        out[g] = entries
    return out


PV, PL = parties(V), parties(L)

# ---------- assemble ----------
STATK = [('hp', 'HP'), ('atk', 'Atk'), ('def_', 'Def'), ('spa', 'SpA'), ('spd', 'SpD'), ('spe', 'Spe')]
species = {}
for i, s in enumerate(GEN1, 1):
    a, b = BSV[s], BSL[s]
    d = {'dex': i}
    if [a[k] for k, _ in STATK] != [b[k] for k, _ in STATK]:
        d['stats'] = {'vanilla': [a[k] for k, _ in STATK], 'legacy': [b[k] for k, _ in STATK]}
    if a['types'] != b['types']:
        d['types'] = {'vanilla': a['types'], 'legacy': b['types']}
    for k in ('catch', 'base_exp', 'growth'):
        if a[k] != b[k]:
            d[k] = {'vanilla': a[k], 'legacy': b[k]}
    tv, tl = set(a['tmhm']), set(b['tmhm'])
    if tv != tl:
        d['tmhm'] = {'added': sorted(tl - tv), 'removed': sorted(tv - tl)}
    ev, el = EV[s]['evos'], EL[s]['evos']
    if ev != el:
        d['evo'] = {'vanilla': [fmt_evo(e) for e in ev], 'legacy': [fmt_evo(e) for e in el]}
    mv, ml = EV[s]['moves'], EL[s]['moves']
    if mv != ml:
        from collections import defaultdict
        dv = defaultdict(list); dl = defaultdict(list)
        for lv, m in mv: dv[m].append(lv)
        for lv, m in ml: dl[m].append(lv)
        added = {m: dl[m] for m in dl if m not in dv}
        removed = {m: dv[m] for m in dv if m not in dl}
        changed = {m: [dv[m], dl[m]] for m in dv if m in dl and dv[m] != dl[m]}
        d['learnset'] = {'added': added, 'removed': removed, 'level_changed': changed,
                         'vanilla': mv, 'legacy': ml}
    species[s] = d

# babies of gen1 (pre-evolutions) evolution changes
baby_evo = {}
for s in ['PICHU', 'CLEFFA', 'IGGLYBUFF', 'SMOOCHUM', 'ELEKID', 'MAGBY', 'TYROGUE']:
    if EV[s]['evos'] != EL[s]['evos']:
        baby_evo[s] = {'vanilla': [fmt_evo(e) for e in EV[s]['evos']], 'legacy': [fmt_evo(e) for e in EL[s]['evos']]}
    else:
        baby_evo[s] = {'unchanged': [fmt_evo(e) for e in EV[s]['evos']]}

move_changes = {}
for k in MV:
    a, b = MV[k], ML.get(k)
    if b is None:
        unparsed.append(f'move {k} missing in legacy'); continue
    diff = {f: [a[f], b[f]] for f in a if a[f] != b[f]}
    if diff:
        move_changes[k] = diff

result = dict(
    commits=dict(crystal_legacy=sys.argv[2], pokecrystal=sys.argv[3]),
    unparsed=unparsed,
    species=species, baby_evolutions=baby_evo,
    moves_changed=move_changes, move_renames=renamed,
    bosses={'vanilla': PV, 'legacy': PL},
)
json.dump(result, open(os.path.join(ROOT, 'crystal_legacy.json'), 'w'), indent=1)
print('ok', len(move_changes), sum(1 for s in species.values() if len(s) > 1), unparsed)
