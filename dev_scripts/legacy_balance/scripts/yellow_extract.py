import re, json, sys, os
D = sys.argv[1]
REPOS = {'vanilla': os.path.join(D,'pokeyellow'), 'legacy': os.path.join(D,'yellow_legacy')}
unparsed = []

def strip(l): return l.split(';')[0].strip()

def base_stats(root):
    order = re.findall(r'base_stats/(\w+)\.asm', open(f'{root}/data/pokemon/base_stats.asm').read())
    out = {}
    for fn in order:
        txt = open(f'{root}/data/pokemon/base_stats/{fn}.asm').read()
        d = {'file': fn}
        m = re.search(r'db DEX_(\w+)', txt); d['dex_const'] = m.group(1)
        m = re.search(r'db\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\s*\n\s*;\s*hp', txt)
        if not m: unparsed.append(f'stats {root} {fn}'); continue
        d['hp'],d['atk'],d['def'],d['spd'],d['spc'] = map(int, m.groups())
        m = re.search(r'db (\w+), (\w+) ; type', txt); d['types'] = sorted(set([m.group(1).replace('_TYPE',''), m.group(2).replace('_TYPE','')]), key=[m.group(1).replace('_TYPE',''), m.group(2).replace('_TYPE','')].index)
        d['catch_rate'] = int(re.search(r'db (\d+) ; catch rate', txt).group(1))
        d['base_exp'] = int(re.search(r'db (\d+) ; base exp', txt).group(1))
        d['lv1'] = [x for x in re.search(r'db ([\w, ]+); level 1 learnset', txt).group(1).replace(' ','').split(',') if x and x!='NO_MOVE']
        d['growth'] = re.search(r'db (\w+) ; growth rate', txt).group(1)
        m = re.search(r'tmhm(.*?)\n\s*; end', txt, re.S)
        d['tmhm'] = sorted(x for x in re.split(r'[\s,\\]+', m.group(1)) if x) if m else None
        if d['tmhm'] is None: unparsed.append(f'tmhm {root} {fn}')
        d['bst'] = d['hp']+d['atk']+d['def']+d['spd']+d['spc']
        out[fn] = d
    return out

def evos(root):
    txt = open(f'{root}/data/pokemon/evos_moves.asm').read()
    res = {}; cur=None; phase=None
    for line in txt.splitlines():
        m = re.match(r'^(\w+)EvosMoves:', line)
        if m: cur = m.group(1); res[cur]={'evos':[], 'moves':[]}; phase='evo'; continue
        if cur is None: continue
        s = strip(line)
        if not s.startswith('db'): continue
        args = [a.strip() for a in s[2:].split(',') if a.strip()]
        if args == ['0']:
            phase = 'moves' if phase=='evo' else 'done'; continue
        if phase=='evo':
            if args[0]=='EVOLVE_LEVEL': res[cur]['evos'].append({'method':'LEVEL','level':int(args[1]),'into':args[2]})
            elif args[0]=='EVOLVE_ITEM': res[cur]['evos'].append({'method':'ITEM','item':args[1],'level':int(args[2]),'into':args[3]})
            elif args[0]=='EVOLVE_TRADE': res[cur]['evos'].append({'method':'TRADE','level':int(args[1]),'into':args[2]})
            else: unparsed.append(f'evo {root} {cur}: {s}')
        elif phase=='moves':
            if len(args)==2 and args[0].isdigit(): res[cur]['moves'].append((int(args[0]), args[1]))
            else: unparsed.append(f'move {root} {cur}: {s}')
    return res

def moves(root):
    out = {}
    for line in open(f'{root}/data/moves/moves.asm'):
        s = strip(line)
        m = re.match(r'move\s+(\w+),\s*(\w+),\s*(\d+),\s*(\w+),\s*(\d+),\s*(\d+)', s)
        if m:
            n,e,p,t,a,pp = m.groups(); out[n]={'effect':e,'power':int(p),'type':t,'acc':int(a),'pp':int(pp)}
        elif s.startswith('move '): unparsed.append(f'moveline {root}: {s}')
    return out

V = {k:{'stats':base_stats(r),'evos':evos(r),'moves':moves(r)} for k,r in REPOS.items()}

# map base_stats file -> evos label: evos labels are CamelCase of names; build normalized key
def norm(s): return re.sub(r'[^a-z0-9]','',s.lower())
def label_for(fn, labels):
    n = norm(fn)
    for l in labels:
        if norm(l)==n: return l
    alias = {'nidoranf':'NidoranF','nidoranm':'NidoranM','mrmime':'MrMime','farfetchd':'Farfetchd'}
    return alias.get(n)

species = []
for fn, vs in V['vanilla']['stats'].items():
    ls = V['legacy']['stats'].get(fn)
    lab = label_for(fn, V['vanilla']['evos'])
    if lab is None or lab not in V['legacy']['evos']: unparsed.append(f'no evos label {fn}')
    ve = V['vanilla']['evos'].get(lab, {'evos':[],'moves':[]}); le = V['legacy']['evos'].get(lab, {'evos':[],'moves':[]})
    rec = {'species': fn, 'label': lab}
    sd = {}
    for k in ['hp','atk','def','spd','spc','bst','types','catch_rate','base_exp','growth','lv1']:
        if vs[k]!=ls[k]: sd[k]={'vanilla':vs[k],'legacy':ls[k]}
    rec['stat_changes']=sd
    rec['stats_vanilla']={k:vs[k] for k in ['hp','atk','def','spd','spc','bst','types']}
    rec['stats_legacy']={k:ls[k] for k in ['hp','atk','def','spd','spc','bst','types']}
    if vs['tmhm']!=ls['tmhm']:
        rec['tmhm_added']=sorted(set(ls['tmhm'])-set(vs['tmhm'])); rec['tmhm_removed']=sorted(set(vs['tmhm'])-set(ls['tmhm']))
    if ve['evos']!=le['evos']: rec['evo']={'vanilla':ve['evos'],'legacy':le['evos']}
    vm, lm = ve['moves'], le['moves']
    if vm!=lm:
        vd={}; ld={}
        for lv,mv in vm: vd.setdefault(mv,[]).append(lv)
        for lv,mv in lm: ld.setdefault(mv,[]).append(lv)
        rec['learnset']={
          'added':[(lv,mv) for lv,mv in lm if mv not in vd],
          'removed':[(lv,mv) for lv,mv in vm if mv not in ld],
          'relevelled':[(mv,vd[mv],ld[mv]) for mv in vd if mv in ld and vd[mv]!=ld[mv]],
          'vanilla':vm,'legacy':lm}
    species.append(rec)

mvchg = []
for n,vm in V['vanilla']['moves'].items():
    lm = V['legacy']['moves'].get(n)
    if lm is None: unparsed.append(f'move missing in legacy {n}'); continue
    d = {k:{'vanilla':vm[k],'legacy':lm[k]} for k in vm if vm[k]!=lm[k]}
    if d: mvchg.append({'move':n,'changes':d,'vanilla':vm,'legacy':lm})
for n in V['legacy']['moves']:
    if n not in V['vanilla']['moves']: unparsed.append(f'new move in legacy {n}')

json.dump({'species':species,'moves':mvchg,'unparsed':unparsed}, open(os.path.join(D,'yellow_tmp.json'),'w'), indent=1)
print(len(species), len(mvchg), unparsed[:20])
