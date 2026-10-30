"""Draft snapshot tiers for all 151 Gen 1 Pokemon and flag cases that need a human call.

Rules (see the design doc, Progression > Snapshot tiers):
- Tiers of ten levels; one snapshot per tier; tier 6 is level 60.
- Position inside a tier is a throttle: weaker than the average of its stage -> higher in the tier.
- Evolutions happen at the snapshot of the tier holding their usual level; two early
  evolutions can share one snapshot.
- Tier 0 (start) is never below the first damaging same-type move, with exemptions.
- Stats: modern expansion stats plus Emerald Legacy's changes (its delta from Gen 3).
- Evolutions: expansion data with Emerald Legacy's fixes (trade -> level 38/42, item trades
  -> use the item, Goldeen 28).
- Jump cost scales with the original growth curve at the level jumped to.

Inputs: gen1.i and moves.i (preprocessed; see ../snapshot_sample/sample.py for how to make
them), the expansion's modern learnsets and ../emerald_legacy.json.
Usage: python3 -I full.py <expansion repo> <folder with gen1.i and moves.i>
"""
import json
import os
import re
import statistics
import sys

REPO, PRE = sys.argv[1], sys.argv[2]
HERE = os.path.dirname(os.path.abspath(__file__))
STAT_ORDER = ("HP", "Atk", "Def", "SpA", "SpD", "Spe")


def cexpr(e):
    """Evaluate a numeric C expression with ?:, ||, && and comparisons; returns a string."""
    def flat(x):
        x = x.replace("||", " or ").replace("&&", " and ")
        while "?" in x:
            cond, rest = x.split("?", 1)
            a, b = rest.split(":", 1)
            x = str(eval(a) if eval(cond) else eval(flat(b)))
        return str(int(eval(x)))
    while "(" in e:
        e = re.sub(r"\(([^()]*)\)", lambda m: flat(m.group(1)), e, count=1)
    return flat(e)


# --- expansion data ---------------------------------------------------------
species = {}
for m in re.finditer(r"\[SPECIES_(\w+)\] =\s*\{(.*?)\n    \},", open(os.path.join(PRE, "gen1.i")).read(), re.S):
    name, body = m.group(1), m.group(2)
    found = [re.search(r"\.%s = ([^,\n]+),\n" % f, body)
             for f in ("baseHP", "baseAttack", "baseDefense", "baseSpAttack", "baseSpDefense", "baseSpeed")]
    if not all(found) or name in species:
        continue
    exprs = [f.group(1) for f in found]
    if not all(re.fullmatch(r"[\d\s()+\-*/<>=!?:|&]+", e) for e in exprs):
        continue
    tm = re.search(r"\.types = (?:MON_TYPES\()?(.*?)\)?,\s*\n", body)
    tline = tm.group(1) if tm else ""
    tline = re.sub(r"\((\d+) (>=|==|<=|<|>) (\d+) \? (TYPE_\w+) : (TYPE_\w+)\)",
                   lambda m: m.group(4) if eval(m.group(1) + m.group(2) + m.group(3)) else m.group(5), tline)
    evo = re.search(r"\.evolutions = EVOLUTION\((.*?)\),?\s*$", body, re.S)
    ls = re.search(r"\.levelUpLearnset = (\w+)", body)
    species[name] = {
        "stats": [int(cexpr(e)) for e in exprs],
        "types": sorted(set(re.findall(r"TYPE_\w+", tline))),
        "growth": re.search(r"\.growthRate = GROWTH_(\w+)", body).group(1),
        "evos": re.findall(r"\{(EVO_\w+), (\w+), SPECIES_(\w+)", evo.group(1)) if evo else [],
        "learnset": ls.group(1) if ls else None,
    }

moves = {}
for m in re.finditer(r"\[MOVE_(\w+)\] =\s*\{(.*?)\n    \},", open(os.path.join(PRE, "moves.i")).read(), re.S):
    p = re.search(r"\.power = ([^,\n]+),", m.group(2))
    t = re.search(r"\.type = ([^,\n]+),", m.group(2))
    tval = None
    if t:
        tv = t.group(1)
        c = re.fullmatch(r"\(?(\d+) (>=|==|<=|<|>) (\d+) \? (TYPE_\w+) : (TYPE_\w+)\)?", tv.strip())
        tval = (c.group(4) if eval(c.group(1) + c.group(2) + c.group(3)) else c.group(5)) if c else \
            (tv if tv.startswith("TYPE_") else None)
    pval = int(cexpr(p.group(1))) if p and re.fullmatch(r"[\d\s()+\-*/<>=!?:|&]+", p.group(1)) else 0
    moves[m.group(1)] = (pval, tval)

learn_txt = open(os.path.join(REPO, "src/data/pokemon/level_up_learnsets/gen_9.h")).read()
learnsets = {m.group(1): [(int(a), b) for a, b in re.findall(r"LEVEL_UP_MOVE\( *(\d+), MOVE_(\w+)\)", m.group(2))]
             for m in re.finditer(r"(s\w+LevelUpLearnset)\[\] = \{(.*?)\};", learn_txt, re.S)}

legacy = json.load(open(os.path.join(HERE, "..", "emerald_legacy.json")))["species"]
ROSTER = list(legacy)  # dex order, 1-151
ALIASES = {"FARFETCH_D": "FARFETCHD", "MR__MIME": "MR_MIME"}
roster = [ALIASES.get(k, k) for k in ROSTER]
missing = [n for n in roster if n not in species]
assert not missing, missing

# --- apply Emerald Legacy ---------------------------------------------------
for key, name in zip(ROSTER, roster):
    s = species[name]
    s["el_delta"] = [0] * 6
    if "stats" in legacy[key]:
        v, l = legacy[key]["stats"]["vanilla"], legacy[key]["stats"]["legacy"]
        s["el_delta"] = [l[k] - v[k] for k in STAT_ORDER]
    s["final_stats"] = [a + b for a, b in zip(s["stats"], s["el_delta"])]
    s["bst"] = sum(s["final_stats"])

LEGACY_LEVEL = {"KADABRA": 42, "MACHOKE": 38, "GRAVELER": 38, "HAUNTER": 42}
for name, lvl in LEGACY_LEVEL.items():
    species[name]["evos"] = [("EVO_LEVEL", str(lvl), t) if m == "EVO_TRADE" else (m, p, t)
                             for m, p, t in species[name]["evos"]]
species["GOLDEEN"]["evos"] = [("EVO_LEVEL", "28", t) if m == "EVO_LEVEL" else (m, p, t)
                              for m, p, t in species["GOLDEEN"]["evos"]]

# --- families and stages ----------------------------------------------------
in_roster = set(roster)
parent = {}
for n in roster:
    for m, p, t in species[n]["evos"]:
        if t in in_roster:
            parent.setdefault(t, n)


def stage(n):
    return 0 if n not in parent else 1 + stage(parent[n])


def kind(n):
    has_child = any(t in in_roster for _, _, t in species[n]["evos"])
    if not has_child and n not in parent:
        return "single"
    if not has_child:
        return "final"
    return "basic" if n not in parent else "middle"


groups = {}
for n in roster:
    groups.setdefault(kind(n), []).append(species[n]["bst"])
AVG = {k: statistics.mean(v) for k, v in groups.items()}
SD = {k: statistics.pstdev(v) for k, v in groups.items()}


def offset(n):
    """Position inside a tier, 1-9: weaker than its stage's average -> higher."""
    z = (species[n]["bst"] - AVG[kind(n)]) / SD[kind(n)]
    return max(1, min(9, round(5 - 2.5 * z)))


def first_stab(n):
    for lvl, mv in learnsets.get(species[n]["learnset"], []):
        power, mtype = moves.get(mv, (0, None))
        if power > 1 and mtype in species[n]["types"]:
            return lvl, mv
    return None


def growth_factor(growth, level):
    n = level
    curves = {"FAST": 4 * n ** 3 / 5, "MEDIUM_FAST": n ** 3, "SLOW": 5 * n ** 3 / 4,
              "MEDIUM_SLOW": 6 * n ** 3 / 5 - 15 * n ** 2 + 100 * n - 140}
    return curves.get(growth, n ** 3) / n ** 3


# Starters are set by hand (design doc).
STARTERS = {
    "BULBASAUR": [7, 14, 24, 32, 44, 51, 60], "SQUIRTLE": [8, 16, 26, 34, 42, 49, 60],
    "CHARMANDER": [9, 18, 28, 36, 46, 54, 60],
}


def build_line(base):
    """Main evolution line from a basic form: [(form, method, param)], first entry is the base."""
    line, cur = [(base, None, None)], base
    while True:
        nxt = [(m, p, t) for m, p, t in species[cur]["evos"] if t in in_roster]
        if not nxt:
            return line
        m, p, t = nxt[0]
        line.append((t, m, p))
        cur = t


lines, flags = [], []
for n in roster:
    if n in parent:
        continue
    line = build_line(n)
    level_evos = {t: int(p) for t, m, p in line[1:] if m == "EVO_LEVEL"}
    other_evos = {t: (m, p) for t, m, p in line[1:] if m != "EVO_LEVEL"}
    # Which form is current at each tier; evolutions apply at the tier of their usual level.
    cur_i = 0
    names = [f for f, _, _ in line]
    tiers = []
    for tier in range(1, 7):
        evolved = []
        while cur_i + 1 < len(names) and names[cur_i + 1] in level_evos and \
                max(1, level_evos[names[cur_i + 1]] // 10) <= tier:
            cur_i += 1
            evolved.append(names[cur_i])
        form = names[cur_i]
        level = 60 if tier == 6 else tier * 10 + offset(form)
        if evolved and tier < 6:
            # An evolution snapshot stays within 3 levels of the usual evolution level.
            usual = level_evos[evolved[-1]]
            level = max(tier * 10 + 1, min(tier * 10 + 9, max(usual - 3, min(usual + 3, level))))
        tiers.append({"tier": tier, "level": level, "form": form, "evolves_into": evolved})
    if n in STARTERS:
        for t, lvl in zip(tiers, STARTERS[n][1:]):
            t["level"] = lvl
    # Moves unlocked at each snapshot from the current form's learnset.
    fs = first_stab(n)
    start = STARTERS[n][0] if n in STARTERS else (max(1, fs[0]) if fs else None)
    prev = start or 1
    for t in tiers:
        ls = learnsets.get(species[t["form"]]["learnset"], [])
        t["new_moves"] = [mv for lvl, mv in ls if (prev < lvl <= t["level"]) or (lvl == 0 and t["evolves_into"])]
        t["jump_cost"] = round(growth_factor(species[t["form"]]["growth"], t["level"]), 2)
        prev = t["level"]
    above_60 = sorted({(f, mv, lvl) for f in names for lvl, mv in learnsets.get(species[f]["learnset"], []) if lvl > 60})
    entry = {"base": n, "forms": [{"name": f, "kind": kind(f), "bst": species[f]["bst"],
                                   "el_delta": species[f]["el_delta"], "growth": species[f]["growth"],
                                   "offset": offset(f)} for f in names],
             "start_floor": start, "first_stab": fs, "tiers": tiers,
             "other_evolutions": other_evos, "moves_above_60": above_60}
    # Flags.
    if fs is None:
        flags.append((n, "no damaging same-type move before evolving: exempt from the start rule"))
    for f, (m, p) in other_evos.items():
        flags.append((n, "%s evolves by %s %s: not placed on a snapshot" % (f, m[4:].lower(), p)))
    for t in tiers:
        if len(t["evolves_into"]) > 1:
            flags.append((n, "tier %d evolves twice (%s)" % (t["tier"], ", ".join(t["evolves_into"]))))
        for f in t["evolves_into"]:
            usual = level_evos[f]
            if abs(t["level"] - usual) > 3:
                flags.append((n, "%s evolves at %d, usual %d" % (f, t["level"], usual)))
    if above_60:
        flags.append((n, "%d moves above 60 to pull down" % len(above_60)))
    lines.append(entry)

json.dump({"averages": AVG, "sd": SD, "lines": lines, "flags": flags},
          open(os.path.join(HERE, "full.json"), "w"), indent=1)

# --- report -----------------------------------------------------------------
with open(os.path.join(HERE, "full.md"), "w") as f:
    f.write("# Draft snapshot tiers for the 151\n\nGenerated by full.py. Stage averages (BST): %s.\n\n" %
            ", ".join("%s %d" % (k, v) for k, v in AVG.items()))
    f.write("| Line | Start | T1 | T2 | T3 | T4 | T5 | Growth |\n| --- | --- | --- | --- | --- | --- | --- | --- |\n")
    for e in lines:
        cells = []
        for t in e["tiers"][:5]:
            evo = " **%s**" % "/".join(x.title() for x in t["evolves_into"]) if t["evolves_into"] else ""
            cells.append("%d%s" % (t["level"], evo))
        f.write("| %s | %s | %s | %s |\n" % (
            " > ".join(x["name"].title() for x in e["forms"]),
            e["start_floor"] if e["start_floor"] else "exempt", " | ".join(cells), e["forms"][0]["growth"].lower()))
    f.write("\n## Flags\n\n")
    for n, msg in flags:
        f.write("- %s: %s\n" % (n.title(), msg))

print("lines", len(lines), "flags", len(flags))
print("averages", {k: round(v) for k, v in AVG.items()}, "sd", {k: round(v) for k, v in SD.items()})
for tier in range(1, 6):
    lv = [e["tiers"][tier - 1]["level"] for e in lines]
    print("tier", tier, "min", min(lv), "median", statistics.median(lv), "max", max(lv))
