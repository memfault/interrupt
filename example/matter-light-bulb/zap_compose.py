#!/usr/bin/env python3
"""
Compose a Matter ZAP file from fragments.

Usage:
    python3 zap_compose.py base.json [fragment1.json ...] > output.zap

The base fragment provides the global config (fileFormat, featureLevel,
keyValuePairs, package) and the root endpoint. Each additional fragment
adds an application endpoint.

Endpoints are assigned IDs sequentially: 0 for the base's endpoint,
1 for the first fragment, 2 for the second, etc.
"""

import json
import sys


def compose(base_path, fragment_paths):
    with open(base_path) as f:
        zap = json.load(f)

    for frag_path in fragment_paths:
        with open(frag_path) as f:
            frag = json.load(f)

        # Append fragment's endpointTypes
        zap["endpointTypes"].extend(frag["endpointTypes"])

    # Rebuild endpoints array from endpointTypes
    zap["endpoints"] = []
    for i, ep_type in enumerate(zap["endpointTypes"]):
        zap["endpoints"].append(
            {
                "endpointTypeName": ep_type["name"],
                "endpointTypeIndex": i,
                "profileId": 259,  # Matter profile
                "endpointId": i,
                "networkId": 0,
                "parentEndpointIdentifier": None,
            }
        )

    return zap


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} base.json [fragment.json ...]", file=sys.stderr)
        sys.exit(1)

    base_path = sys.argv[1]
    fragment_paths = sys.argv[2:]

    zap = compose(base_path, fragment_paths)
    json.dump(zap, sys.stdout, indent=2)
    print()  # trailing newline


if __name__ == "__main__":
    main()
