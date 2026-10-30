"""Apply the snapshot-tier rules to a sample of Gen 1 families and flag where they strain.

Inputs: the preprocessed species and move data (gen1.i, moves.i in this folder), the
expansion's modern learnsets, and Emerald Legacy's stat changes.

Make the preprocessed files first, from the repo root (a probe .c file that includes
"global.h" and then the data header):
  gen1.i:  "constants/species.h" + "data/pokemon/species_info/gen_1_families.h"
  moves.i: "constants/moves.h" + "data/moves_info.h"
  arm-none-eabi-gcc -E -P -DFIRERED -DMODERN=1 -Iinclude -Isrc -Igflib -iquote include probe.c -o <name>.i

Usage: python3 -I sample.py <expansion repo>
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = sys.argv[1]

# --- data -------------------------------------------------------------------
def cexpr(e):
    """Evaluate a numeric C expression with ?:, ||, && and comparisons; returns a string."""
    def flat(x):
        # x has no parentheses; resolve the rightmost-nested ternary first.
        x = x.replace("||", " or ").replace("&&", " and ")
        while "?" in x:
            cond, rest = x.split("?", 1)
            a, b = rest.split(":", 1)
            x = str(eval(a) if eval(cond) else eval(flat(b)))
        return str(int(eval(x)))
    while "(" in e:
        e = re.sub(r"\(([^()]*)\)", lambda m: flat(m.group(1)), e, count=1)
    return flat(e)


gen1 = open(os.path.join(HERE, "gen1.i")).read()
species = {}
for m in re.finditer(r"\[SPECIES_(\w+)\] =\s*\{(.*?)\n    \},", gen1, re.S):
    name, body = m.group(1), m.group(2)
    found = [re.search(r"\.%s = ([^,\n]+),\n" % f, body)
             for f in ("baseHP", "baseAttack", "baseDefense", "baseSpAttack", "baseSpDefense", "baseSpeed")]
    if not all(found):
        continue
    exprs = [f.group(1) for f in found]
    if not all(re.fullmatch(r"[\d\s()+\-*/<>=!?:|&]+", e) for e in exprs):
        print("skipped", name, exprs)
        continue
    stats = [int(eval(cexpr(e))) for e in exprs]
    types = re.search(r"\.types = MON_TYPES\((TYPE_\w+)(?:, *(TYPE_\w+))?\)", body)
    evo = re.search(r"\.evolutions = .*?\{\{(.*?)\}\s*,\s*\{EVOLUTIONS_END", body, re.S) or \
        re.search(r"\.evolutions = EVOLUTION\((.*?)\),", body, re.S)
    evos = re.findall(r"\{(EVO_\w+), (\w+), SPECIES_(\w+)", evo.group(1)) if evo else []
    ls = re.search(r"\.levelUpLearnset = (\w+)", body)
    species.setdefault(name, {
        "stats": stats, "bst": sum(stats),
        "types": sorted({t for t in types.groups() if t}) if types else [],
        "growth": re.search(r"\.growthRate = (\w+)", body).group(1),
        "evos": evos, "learnset": ls.group(1) if ls else None})

moves = {}
for m in re.finditer(r"\[MOVE_(\w+)\] =\s*\{(.*?)\n    \},", open(os.path.join(HERE, "moves.i")).read(), re.S):
    p = re.search(r"\.power = (\d+)", m.group(2))
    t = re.search(r"\.type = (TYPE_\w+)", m.group(2))
    moves[m.group(1)] = (int(p.group(1)) if p else 0, t.group(1) if t else None)

learn_txt = open(os.path.join(REPO, "src/data/pokemon/level_up_learnsets/gen_9.h")).read()
learnsets = {}
for m in re.finditer(r"(s\w+LevelUpLearnset)\[\] = \{(.*?)\};", learn_txt, re.S):
    learnsets[m.group(1)] = [(int(a), b) for a, b in re.findall(r"LEVEL_UP_MOVE\( *(\d+), MOVE_(\w+)\)", m.group(2))]

legacy = json.load(open(os.path.join(REPO, "dev_scripts/legacy_balance/emerald_legacy.json")))["species"]

# Average BST of fully evolved / middle / basic Gen 1 forms, for the throttle.
def stage_of(name):
    pre = [n for n, s in species.items() if any(t == name for _, _, t in s["evos"])]
    if not pre:
        return 0
    return 1 + stage_of(pre[0])

def is_final(name):
    return not species[name]["evos"]

GEN1 = [n for n in species if n in legacy]
groups = {}
for n in GEN1:
    key = ("final" if is_final(n) else "unevolved", )
    groups.setdefault(key, []).append(species[n]["bst"])
AVG = {k: sum(v) / len(v) for k, v in groups.items()}

def first_stab(name):
    s = species[name]
    for lvl, mv in learnsets.get(s["learnset"], []):
        power, mtype = moves.get(mv, (0, None))
        if power > 1 and mtype in s["types"]:
            return lvl, mv
    return None

# --- sample -----------------------------------------------------------------
FAMILIES = ["PIDGEY", "RATTATA", "CATERPIE", "GEODUDE", "ABRA", "MAGIKARP", "DRATINI",
            "EEVEE", "PIKACHU", "NIDORAN_M", "FARFETCHD", "SNORLAX", "EKANS"]

def chain(base):
    out, cur = [base], base
    while species[cur]["evos"]:
        method, param, target = species[cur]["evos"][0]
        out.append((method, param, target))
        cur = target
    return out

report = []
for base in FAMILIES:
    if base not in species:
        report.append({"family": base, "error": "not found"})
        continue
    fam = chain(base)
    names = [fam[0]] + [t for _, _, t in fam[1:]]
    rows = []
    for n in names:
        s = species[n]
        leg = legacy.get(n, {}).get("stats")
        el_bst = leg["legacy"]["BST"] if leg else None
        avg = AVG[("final",) if is_final(n) else ("unevolved",)]
        rows.append({"species": n, "bst": s["bst"], "emerald_legacy_bst": el_bst,
                     "vs_avg": round(s["bst"] - avg), "types": s["types"], "growth": s["growth"],
                     "first_stab": first_stab(n),
                     "learnset_max": max((l for l, _ in learnsets.get(s["learnset"], [])), default=None),
                     "learnset_moves_after_1": sum(1 for l, _ in learnsets.get(s["learnset"], []) if l > 1)})
    evos = [{"from": names[i], "to": t, "method": m, "param": p,
             "tier": (int(p) // 10 if m == "EVO_LEVEL" and p.isdigit() else None)}
            for i, (m, p, t) in enumerate(fam[1:])]
    report.append({"family": base, "forms": rows, "evolutions": evos})

json.dump({"averages": {k[0]: v for k, v in AVG.items()}, "families": report}, open(os.path.join(HERE, "sample.json"), "w"), indent=1)
for f in report:
    print("==", f["family"])
    for r in f.get("forms", []):
        print("  %-10s BST %3d (EL %s, %+d vs avg) %s %s first STAB %s, moves after Lv1: %d, last at %s" % (
            r["species"], r["bst"], r["emerald_legacy_bst"], r["vs_avg"], "/".join(t[5:] for t in r["types"]),
            r["growth"][7:], r["first_stab"], r["learnset_moves_after_1"], r["learnset_max"]))
    for e in f.get("evolutions", []):
        print("  evo %s -> %s: %s %s (tier %s)" % (e["from"], e["to"], e["method"], e["param"], e["tier"]))
print("averages", {k[0]: round(v) for k, v in AVG.items()})
