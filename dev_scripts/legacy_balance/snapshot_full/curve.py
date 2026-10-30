"""Compare the draft snapshot levels with the Gym level curve at each point of a run.

Pacing model: about 9 fights per route and town, and a jump costs about 13.5 fights (one jump every 1.5 stops) for a
Medium Fast Pokemon used in every fight, scaled by its growth curve at the level jumped to.
Reads full.json (from full.py). Usage: python3 -I curve.py
"""
import json
import os
import statistics

HERE = os.path.dirname(os.path.abspath(__file__))
FIGHTS_PER_STOP, BASE_JUMP = 9, 13.5
# Option B: a base cost per jump (Medium Fast fights), chosen so the team reaches tier 1 before
# Gym 1, tier 3 just before Gym 5 (FRLG's level cliff), tier 5 for Gym 8 and tier 6 for the finale.
PER_TIER = [8, 12, 15, 12, 18, 16]
GYMS = [("Gym 1", 12, 14), ("Gym 2", 18, 21), ("Gym 3", 18, 24), ("Gym 4", 24, 29), ("Gym 5", 37, 43),
        ("Gym 6", 37, 43), ("Gym 7", 40, 47), ("Gym 8", 42, 50), ("Elite Four", 51, 60)]

lines = json.load(open(os.path.join(HERE, "full.json")))["lines"]
import sys
MODE = sys.argv[1] if len(sys.argv) > 1 else "flat"
rows = []
for k, (name, lo, hi) in enumerate(GYMS, start=1):
    fights = FIGHTS_PER_STOP * k
    levels, by_growth = [], {}
    for e in lines:
        start = e["start_floor"] if e["base"] in ("BULBASAUR", "SQUIRTLE", "CHARMANDER") else 5
        spent, level = 0, max(start or 5, 5)
        for i, t in enumerate(e["tiers"]):
            base = BASE_JUMP if MODE == "flat" else PER_TIER[i]
            spent += base * max(0.6, t["jump_cost"])
            if spent > fights:
                break
            level = t["level"]
        levels.append(level)
        by_growth.setdefault(e["forms"][0]["growth"], []).append(level)
    rows.append({"stop": name, "gym_low": lo, "gym_high": hi, "team_low": min(levels),
                 "team_median": statistics.median(levels), "team_high": max(levels),
                 "median_by_growth": {g: statistics.median(v) for g, v in sorted(by_growth.items())}})
json.dump(rows, open(os.path.join(HERE, "curve_%s.json" % MODE), "w"), indent=1)
for r in rows:
    print("%-10s gym %2d-%2d  team %2d / %4.1f / %2d  %s" % (
        r["stop"], r["gym_low"], r["gym_high"], r["team_low"], r["team_median"], r["team_high"],
        {g.lower(): m for g, m in r["median_by_growth"].items()}))
