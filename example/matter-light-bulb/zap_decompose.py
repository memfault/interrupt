#!/usr/bin/env python3
"""
Decompose a Matter ZAP file into composable fragments.

Usage:
    python3 zap_decompose.py input.zap output_dir/

Creates:
    output_dir/base.json              — root endpoint + global config
    output_dir/<endpoint_name>.json   — one file per application endpoint
"""

import json
import os
import re
import sys


def slugify(name):
    """Convert endpoint name to a filename-safe slug."""
    return re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")


def decompose(zap_path, output_dir):
    with open(zap_path) as f:
        zap = json.load(f)

    os.makedirs(output_dir, exist_ok=True)

    endpoint_types = zap["endpointTypes"]

    # Base fragment: global config + first endpointType (root)
    base = {
        "fileFormat": zap["fileFormat"],
        "featureLevel": zap["featureLevel"],
        "creator": zap["creator"],
        "keyValuePairs": zap["keyValuePairs"],
        "package": zap["package"],
        "endpointTypes": [endpoint_types[0]],
    }

    base_path = os.path.join(output_dir, "base.json")
    with open(base_path, "w") as f:
        json.dump(base, f, indent=2)
        f.write("\n")
    print(f"  {base_path} ({endpoint_types[0]['name']})")

    # Application fragments: one per remaining endpointType
    for ep_type in endpoint_types[1:]:
        frag = {"endpointTypes": [ep_type]}
        name = slugify(ep_type["name"])
        frag_path = os.path.join(output_dir, f"{name}.json")
        with open(frag_path, "w") as f:
            json.dump(frag, f, indent=2)
            f.write("\n")
        print(f"  {frag_path} ({ep_type['name']})")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.zap output_dir/", file=sys.stderr)
        sys.exit(1)

    decompose(sys.argv[1], sys.argv[2])


if __name__ == "__main__":
    main()
