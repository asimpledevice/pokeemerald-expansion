import json, sys, os, re, subprocess
D = sys.argv[1]
d = json.load(open(os.path.join(D,'yellow_tmp.json')))
def h(r): return subprocess.run(['git','-C',os.path.join(D,r),'log','-1','--format=%H %cs'],capture_output=True,text=True).stdout.strip()
hv, hl = h('pokeyellow'), h('yellow_legacy')
RENAME = {('FREEZE_SIDE_EFFECT1','FREEZE_SIDE_EFFECT')}
moves = []
for m in d['moves']:
    ch = {k:v for k,v in m['changes'].items() if not (k=='effect' and (v['vanilla'],v['legacy']) in RENAME)}
    if ch: moves.append({'move':m['move'],'changes':ch,'vanilla':m['vanilla'],'legacy':m['legacy']})
sp = d['species']
def nm(s): return s.capitalize()

# bosses
BOSSES = ['Brock','Misty','LtSurge','Erika','Koga','Sabrina','Blaine','Giovanni','Lorelei','Bruno','Agatha','Lance','Rival3']
def parties(root):
    txt = open(os.path.join(D,root,'data/trainers/parties.asm')).read()
    out = {}
    for b in BOSSES:
        m = re.search(rf'^{b}Data:[ \t]*\n(.*?)(?=^\w+Data:|\Z)', txt, re.S|re.M)
        lst=[]; rematch=False
        for line in m.group(1).splitlines():
            if 'Rematch' in line: rematch=True
            s=line.split(';')[0].strip()
            if not s.startswith('db $FF'): continue
            a=[x.strip() for x in s[2:].split(',') if x.strip()][1:-1]
            mons=[(int(a[i]),a[i+1]) for i in range(0,len(a),2)]
            lst.append({'rematch':rematch,'party':mons})
        out[b]=lst
    return out
PV, PL = parties('pokeyellow'), parties('yellow_legacy')
NOTES = {('Erika',1):'4th gym (fewer than 4 badges)',('Erika',2):'5th gym (4 badges)',('Erika',3):'6th gym (5+ badges)',
 ('Koga',1):'5th gym (<5 badges)',('Koga',2):'6th gym (5+ badges)',('Sabrina',1):'6th gym (5+ badges, default)',('Sabrina',2):'5th gym (<5 badges)',
 ('Giovanni',1):'Rocket Hideout B4F',('Giovanni',2):'Silph Co. 11F',('Giovanni',3):'Viridian Gym',
 ('Rival3',1):'Champion (Jolteon line)',('Rival3',2):'Champion (Flareon line)',('Rival3',3):'Champion (Vaporeon line)'}
def fmt(p): return ', '.join(f'{s.title()} {l}' for l,s in p)
boss_rows=[]
for b in BOSSES:
    n=max(len(PV[b]),len(PL[b]))
    for i in range(n):
        v=PV[b][i] if i<len(PV[b]) else None; l=PL[b][i] if i<len(PL[b]) else None
        row={'boss':b,'index':i+1,'context':NOTES.get((b,i+1),'Rematch (post-game, new)' if l and l['rematch'] else ''),
             'vanilla':v['party'] if v else None,'legacy':l['party'] if l else None}
        for k in ['vanilla','legacy']:
            p=row[k]; row[k+'_max']=max(x[0] for x in p) if p else None; row[k+'_size']=len(p) if p else None
        boss_rows.append(row)

L=[]; w=L.append
w('# Yellow Legacy vs vanilla Pokemon Yellow: data diff\n')
w(f'- Yellow Legacy: https://github.com/cRz-Shadows/Pokemon_Yellow_Legacy @ `{hl}` (README: based on pret/pokeyellow, so compared directly against it)')
w(f'- Vanilla: https://github.com/pret/pokeyellow @ `{hv}`')
w('- Files compared: data/pokemon/base_stats/*.asm, data/pokemon/evos_moves.asm, data/moves/moves.asm, data/trainers/parties.asm (+ gym scripts for which party index is used), data/types/type_matchups.asm.')
w(f'- Unparsed lines: {d["unparsed"] or "none"} (two Legacy learnset lines have a trailing comma, `db 38, LOVELY_KISS,` for Jigglypuff/Wigglytuff; handled).')
w('- Caveat: pokeyellow master is newer than Legacy\'s fork point; only the cosmetic constant rename FREEZE_SIDE_EFFECT1 -> FREEZE_SIDE_EFFECT was found and filtered (same effect $05). Gen 1 single Special stat recorded as Spc.\n')
cnt=lambda f: sum(1 for r in sp if f(r))
base=lambda r: any(k in r['stat_changes'] for k in ['hp','atk','def','spd','spc'])
w('## Counts\n')
w(f'- Species with base-stat changes: {cnt(base)} / 151; type changes: {cnt(lambda r:"types" in r["stat_changes"])}; catch-rate/base-exp/growth changes: {cnt(lambda r:any(k in r["stat_changes"] for k in ["catch_rate","base_exp","growth"]))}')
w(f'- Species with evolution changes: {cnt(lambda r:"evo" in r)}')
w(f'- Species with level-up learnset changes (lv 2+): {cnt(lambda r:"learnset" in r)}; with level-1 (starting) move changes: {cnt(lambda r:"lv1" in r["stat_changes"])}; union: {cnt(lambda r:"learnset" in r or "lv1" in r["stat_changes"])}')
w(f'- Species with TM/HM compatibility changes: {cnt(lambda r:"tmhm_added" in r)}')
w(f'- Moves with data changes: {len(moves)} (of 165 in the move table)')
w('- Type chart: Bug->Poison super-effective entry removed (now neutral); Ghost->Psychic changed from NO_EFFECT (0x) to SUPER_EFFECTIVE (2x).\n')
w('## Base stat changes\n')
w('| Species | HP | Atk | Def | Spd | Spc | BST |'); w('|---|---|---|---|---|---|---|')
for i,r in enumerate(sp):
    if not base(r): continue
    v,l=r['stats_vanilla'],r['stats_legacy']
    cells=[(f'{v[k]}->**{l[k]}**' if v[k]!=l[k] else str(v[k])) for k in ['hp','atk','def','spd','spc']]
    w(f'| {nm(r["species"])} | '+' | '.join(cells)+f' | {v["bst"]}->{l["bst"]} ({l["bst"]-v["bst"]:+d}) |')
w('\nNo species had its type, catch rate, base exp or growth rate changed.\n')
w('## Evolution changes\n')
w('| Species | Vanilla | Legacy |'); w('|---|---|---|')
def ev(e): return '; '.join((f'Lv {x["level"]}' if x['method']=='LEVEL' else f'{x["item"]}' if x['method']=='ITEM' else 'Trade')+f' -> {x["into"].title()}' for x in e)
for r in sp:
    if 'evo' in r: w(f'| {nm(r["species"])} | {ev(r["evo"]["vanilla"])} | {ev(r["evo"]["legacy"])} |')
w('\nAll four trade evolutions become level-ups (Kadabra 42, Machoke 38, Graveler 38, Haunter 42); no EVOLVE_TRADE entries remain in Legacy. Only other change: Poliwag evolves at 18 instead of 25. Item evolutions (stones) unchanged.\n')
w('## Learnset changes\n')
w('Format: `+Lv MOVE` added, `-Lv MOVE` removed, `MOVE a->b` level moved. "L1" = starting moves from base_stats.\n')
for r in sp:
    parts=[]
    if 'lv1' in r['stat_changes']:
        a=r['stat_changes']['lv1']; parts.append('L1 '+'/'.join(a['vanilla'])+' -> '+'/'.join(a['legacy']))
    if 'learnset' in r:
        ls=r['learnset']
        parts += [f'+{lv} {m}' for lv,m in ls['added']] + [f'-{lv} {m}' for lv,m in ls['removed']] + [f'{m} {"/".join(map(str,a))}->{"/".join(map(str,b))}' for m,a,b in ls['relevelled']]
    if parts: w(f'- **{nm(r["species"])}**: '+'; '.join(parts))
w('\n## TM/HM compatibility additions\n')
for r in sp:
    if 'tmhm_added' in r or 'tmhm_removed' in r:
        s='+'+', +'.join(r.get('tmhm_added',[])) if r.get('tmhm_added') else ''
        if r.get('tmhm_removed'): s+=' ; -'+', -'.join(r['tmhm_removed'])
        w(f'- {nm(r["species"])}: {s}')
w('\n## Move changes\n')
w('Note: Legacy sets most status moves to type BIRD (the unused Gen 1 "typeless" slot), which removes STAB/type interactions for them; listed separately.\n')
w('| Move | Power | Acc | PP | Type | Effect |'); w('|---|---|---|---|---|---|')
bird=[]
for m in moves:
    c=m['changes']
    if set(c)=={'type'} and c['type']['legacy']=='BIRD': bird.append(m['move']); continue
    def f(k):
        return f"{c[k]['vanilla']}->**{c[k]['legacy']}**" if k in c else str(m['legacy'][k])
    w(f"| {m['move']} | {f('power')} | {f('acc')} | {f('pp')} | {f('type')} | {f('effect')} |")
w(f'\nType -> BIRD (typeless) only ({len(bird)}): '+', '.join(bird)+'. (Also GLARE, SUPERSONIC, DISABLE, SOFTBOILED became BIRD with accuracy/PP changes; shown in the table.)\n')
w('## Boss parties (Gym Leaders, Elite Four, Champion)\n')
w('Party index = wTrainerNo. Legacy Erika/Koga/Sabrina have badge-count-dependent teams (gym scripts select by number of badges). Rematch teams are new post-game content.\n')
w('| Boss | # | Context | Vanilla | V max | Legacy | L max | Δ max |'); w('|---|---|---|---|---|---|---|---|')
for r in boss_rows:
    dm = f"{r['legacy_max']-r['vanilla_max']:+d}" if r['vanilla_max'] and r['legacy_max'] else ''
    w(f"| {r['boss']} | {r['index']} | {r['context']} | {fmt(r['vanilla']) if r['vanilla'] else '-'} | {r['vanilla_max'] or '-'} | {fmt(r['legacy']) if r['legacy'] else '-'} | {r['legacy_max'] or '-'} | {dm} |")
open(os.path.join(D,'yellow_legacy.md'),'w').write('\n'.join(L)+'\n')
json.dump({'commits':{'yellow_legacy':hl,'pokeyellow':hv},'unparsed':d['unparsed'],'species':sp,'moves':moves,
  'type_chart':[{'atk':'BUG','def':'POISON','vanilla':'SUPER_EFFECTIVE','legacy':'NORMAL (entry removed)'},{'atk':'GHOST','def':'PSYCHIC','vanilla':'NO_EFFECT','legacy':'SUPER_EFFECTIVE'}],
  'bosses':boss_rows}, open(os.path.join(D,'yellow_legacy.json'),'w'), indent=1)
print(len(moves), len(bird))
