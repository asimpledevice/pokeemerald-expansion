"""Cross-reference the three Legacy comparisons for the 151 Gen 1 species.

Writes crossref.md next to the comparison files: for every species changed by at
least one hack, its base-stat-total change and evolution change in each.
"""
import json
import os

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
E = json.load(open(os.path.join(HERE, "emerald_legacy.json")))["species"]
C = json.load(open(os.path.join(HERE, "crystal_legacy.json")))["species"]
Y = json.load(open(os.path.join(HERE, "yellow_legacy.json")))["species"]

STAT_NAMES = ["HP", "Atk", "Def", "SpA", "SpD", "Spe"]


def diff(van, leg, names):
    parts = ["%s %+d" % (n, b - a) for n, a, b in zip(names, van, leg) if a != b]
    return "%+d (%s)" % (sum(leg) - sum(van), ", ".join(parts)) if parts else ""


rows = []
C_BY_DEX = {v["dex"]: v for v in C.values()}
for i, key in enumerate(E):
    e, y = E[key], Y[i]
    c = C_BY_DEX[e["num"]]
    name = e["name"]
    es = ""
    if "stats" in e:
        es = diff([e["stats"]["vanilla"][n] for n in STAT_NAMES], [e["stats"]["legacy"][n] for n in STAT_NAMES], STAT_NAMES)
    cs = diff(c["stats"]["vanilla"], c["stats"]["legacy"], STAT_NAMES) if "stats" in c else ""
    yv, yl = y["stats_vanilla"], y["stats_legacy"]
    ys = diff([yv[k] for k in ("hp", "atk", "def", "spc", "spd")], [yl[k] for k in ("hp", "atk", "def", "spc", "spd")],
              ["HP", "Atk", "Def", "Spc", "Spe"])
    ee = "; ".join(e["evolution"]["legacy"]) if "evolution" in e else ""
    ce = "; ".join(c["evo"]["legacy"]) if "evo" in c else ""
    ye = json.dumps(y["evo"]) if y.get("evo") else ""
    agree = sum(bool(s) for s in (es, cs, ys))
    if es or cs or ys or ee or ce or ye:
        rows.append((e["num"], name, es, cs, ys, ee, ce, ye, agree))

with open(os.path.join(HERE, "crossref.md"), "w") as f:
    f.write("# Gen 1 balance changes across the three Legacy hacks\n\n")
    f.write("Base-stat-total change (and which stats) per hack; Yellow is Gen 1, with one Special stat. "
            "\"Hacks\" counts how many of the three changed that species' stats.\n\n")
    f.write("| # | Species | Emerald Legacy stats | Crystal Legacy stats | Yellow Legacy stats | Hacks | Emerald evolution | Crystal evolution | Yellow evolution |\n")
    f.write("| --- | --- | --- | --- | --- | --- | --- | --- | --- |\n")
    for num, name, es, cs, ys, ee, ce, ye, agree in rows:
        f.write("| %d | %s | %s | %s | %s | %d | %s | %s | %s |\n" % (num, name, es, cs, ys, agree, ee, ce, ye))
    stat_rows = [r for r in rows if r[8]]
    f.write("\n%d species have stat changes in at least one hack; %d in two or more; %d in all three.\n" % (
        len(stat_rows), sum(r[8] >= 2 for r in rows), sum(r[8] == 3 for r in rows)))
print(open(os.path.join(HERE, "crossref.md")).read()[-400:])
for r in rows:
    if r[8] >= 2:
        print(r[0], r[1], "|E", r[2], "|C", r[3], "|Y", r[4])
