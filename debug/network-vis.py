from pyvis.network import Network
import json

class Server:
    def __init__(self, id, name, security, money, links, services=None, tier="host"):
        self.id = id
        self.name = name
        self.security = security
        self.money = money
        self.links = links
        self.services = services or []
        self.tier = tier

def load_game(filename):
    # Support JSON exports (export_network.json / save.json) and fallback to legacy text saves.
    with open(filename, "r") as f:
        raw = f.read()

    s = raw.lstrip()
    # JSON path
    if s.startswith('{') or s.startswith('['):
        data = json.loads(raw)
        servers = {}
        home_server = 0
        current_server = 0
        # Determine where the exporter placed the servers array. Support:
        # - top-level {"servers": [...]}
        # - wrapped {"game": {"servers": [...], "home_server": X, "current_server": Y}}
        # - raw list [...]
        if isinstance(data, dict):
            if "game" in data and isinstance(data["game"], dict):
                entries = data["game"].get("servers", []) or []
                home_server = int(data["game"].get("home_server", 0) or 0)
                current_server = int(data["game"].get("current_server", 0) or 0)
            else:
                entries = data.get("servers", []) or []
        else:
            entries = data if isinstance(data, list) else []

        for entry in entries:
            sid = int(entry.get("id", 0))
            name = entry.get("name", "")
            security = int(entry.get("security", 0))
            money = int(entry.get("money", 0))
            links = list(map(int, entry.get("links", []) or []))
            type_str = entry.get("type")
            tier = entry.get("tier")
            # Prefer explicit string "type" from exporter; map backend type strings
            # (e.g. building_switch, access_switch, distribution_router,
            # apartment_router) to the visualiser's canonical tier names.
            type_map = {
                "building_switch": "building",
                "floor_switch": "floor",
                "distribution_router": "router",
                "access_switch": "tor",
                "rack_switch": "rack",
                "apartment_router": "user",
                # legacy/short names
                "tor": "tor",
                "router": "router",
                "user": "user",
                "host": "host",
                "isp": "isp",
                "pop": "pop",
                "area": "area",
                "neighborhood": "neighborhood",
                "backbone": "backbone",
                "building": "building",
                "floor": "floor",
            }
            # Prefer explicit string "type" from exporter; fall back to existing "tier" or name inference
            if type_str:
                t = str(type_str).lower()
                tier = type_map.get(t, type_str)
            # If exporter didn't include a tier, try to infer it from the name
            # so nodes like "tor_d1_p1_r4" or "rtr_floor..." get their own layers.
            if not tier:
                lname = (name or "").lower()
                if lname.startswith("isp"):
                    tier = "isp"
                elif lname.startswith("pop"):
                    tier = "pop"
                elif "area" in lname:
                    tier = "area"
                elif "neigh" in lname:
                    tier = "neighborhood"
                elif "bld" in lname or "building" in lname:
                    tier = "building"
                elif "floor" in lname:
                    tier = "floor"
                elif lname.startswith("tor") or "tor_" in lname or "tor-" in lname:
                    tier = "tor"
                elif "rack" in lname:
                    tier = "rack"
                elif lname.startswith("rtr") or "router" in lname:
                    tier = "router"
                elif lname.startswith("usr") or lname.startswith("user"):
                    tier = "user"
                elif lname.startswith("host"):
                    tier = "host"
                else:
                    tier = "host"
            services = []
            for svc in entry.get("services", []) or []:
                svc_name = svc.get("name") or svc.get("svc") or ""
                port = int(svc.get("port", 0))
                vuln = int(svc.get("vuln", svc.get("vuln_level", 0)))
                services.append((svc_name, port, vuln))

            servers[sid] = Server(sid, name, security, money, links, services, tier)

            if name == "home":
                home_server = sid
                current_server = sid

        return servers, home_server, current_server

    # Legacy text format (best-effort parsing)
    servers = {}
    with open(filename, "r") as f:
        server_count, home_server, current_server = map(int, f.readline().split())
        for _ in range(server_count):
            line = f.readline().strip().split()
            sid = int(line[0])
            name = line[1]
            security = int(line[2])
            money = int(line[3])
            link_count = int(line[4])
            links = []
            if link_count > 0:
                links = list(map(int, f.readline().strip().split()))
            else:
                # consume empty line
                f.readline()

            # read role and services line
            svc_line = f.readline().strip().split()
            role = int(svc_line[0]) if len(svc_line) >= 1 else 0
            svc_count = int(svc_line[1]) if len(svc_line) >= 2 else 0
            services = []
            idx = 2
            for i in range(svc_count):
                if idx + 2 < len(svc_line):
                    port = int(svc_line[idx])
                    name_svc = svc_line[idx + 1]
                    vuln = int(svc_line[idx + 2])
                    services.append((name_svc, port, vuln))
                idx += 3

            servers[sid] = Server(sid, name, security, money, links, role, services)

    return servers, home_server, current_server

def visualize_network(servers, home_server, current_server):
    net = Network(height="750px", width="100%", bgcolor="#222222", font_color="white")
    # Create a deterministic nested-circle layout and enable mild physics
    # so nodes can gently settle while keeping the initial seeded positions.
    net.toggle_physics(True)
    net.set_options("""var options = {
        "physics": {
            "barnesHut": {
                "gravitationalConstant": -1000,
                "centralGravity": 0.01,
                "springLength": 140,
                "springConstant": 0.08,
                "avoidOverlap": 0.5
            },
            "solver": "barnesHut",
            "minVelocity": 0.2,
            "stabilization": { "enabled": true, "iterations": 600 }
        }
    }""")

    # Minimal set of colors for tiers we use in the UI.
    tier_colors = {
        "isp": "#2b7cff",
        "area": "#61dafb",
        "neighborhood": "#ffd700",
        "building": "#ff8c00",
        "floor": "#ffb957",
        "router": "#7b61ff",
        "user": "#5f9ea0",
        "host": "#888888",
    }

    # Build helper maps: by id, by tier, adjacency
    by_id = {s.id: s for s in servers.values()}
    children = {s.id: [] for s in servers.values()}
    parent = {s.id: None for s in servers.values()}

    # Define child->parent tier mapping (child_tier: parent_tier)
    parent_tier = {
        'area': 'isp',
        'neighborhood': 'area',
        'building': 'neighborhood',
        'floor': 'building',
        'router': 'floor',
        'user': 'router',
        'host': 'user',
    }

    # Find parent for each node by looking at its neighbors for the expected parent tier
    for s in servers.values():
        pt = parent_tier.get(s.tier)
        if not pt:
            continue
        for nb in s.links:
            nb_tier = by_id[nb].tier
            if nb_tier == pt:
                parent[s.id] = nb
                children[nb].append(s.id)
                break

    # Roots are nodes without a parent; start placement from ISPs (roots)
    roots = [s.id for s in servers.values() if parent[s.id] is None and s.tier == 'isp']
    if not roots:
        # fallback: any node without parent
        roots = [s.id for s in servers.values() if parent[s.id] is None]

    positions = {}

    import math

    def place_children(p_id, x, y, radius, depth=0):
        ch = children.get(p_id, [])
        n = len(ch)
        if n == 0:
            return
        angle0 = (p_id * 37) % 360  # pseudo-random seed per parent
        for i, cid in enumerate(ch):
            angle = math.radians(angle0 + (360.0 * i / n))
            cx = x + math.cos(angle) * radius
            cy = y + math.sin(angle) * radius
            positions[cid] = (cx, cy)
            # smaller radius for next level
            place_children(cid, cx, cy, max(radius * 0.55, 40), depth + 1)

    # place roots around center if multiple
    if len(roots) == 1:
        positions[roots[0]] = (0, 0)
        place_children(roots[0], 0, 0, 220)
    else:
        R = 120
        for i, rid in enumerate(roots):
            ang = 2 * math.pi * i / len(roots)
            rx = math.cos(ang) * R
            ry = math.sin(ang) * R
            positions[rid] = (rx, ry)
            place_children(rid, rx, ry, 220)

    # Now add nodes with fixed positions (so layout stays as drawn)
    for server in servers.values():
        color = tier_colors.get(server.tier, "#888888")
        if server.id == home_server:
            color = "#00aa00"
        elif server.id == current_server:
            color = "#aa0000"

        svc_text = ""
        if server.services:
            svc_text = "\nServices:\n" + "\n".join([f"{s[0]}:{s[1]} (v{ s[2] })" for s in server.services])

        title = f"Tier: {server.tier}\nSecurity: {server.security}\nMoney: {server.money}{svc_text}"

        pos = positions.get(server.id, (0, 0))
        # size by tier
        size = 36 if server.tier == 'isp' else (24 if server.tier in ('area', 'building') else 14)
        # Provide initial positions (x,y) but do not fix them so physics can act.
        net.add_node(server.id, label=server.name, title=title, color=color, x=pos[0], y=pos[1], size=size)

    for server in servers.values():
        for link in server.links:
            net.add_edge(server.id, link)
    # With hierarchical layout enabled and 'level' set on nodes, vis.js will
    # arrange nodes into layers while physics handles intra-layer spacing.
    net.write_html("server_network.html")
    print("Network saved as server_network.html")

if __name__ == "__main__":
    import sys
    import os

    # Allow passing save file path as first argument, otherwise try common locations.
    candidates = []
    if len(sys.argv) >= 2:
        candidates.append(sys.argv[1])
    # prefer programmatic `save.json` (current game save) before older `export_network.json`
    candidates.extend(["save.json", "export_network.json", "../save.save", "./save.save", "save.save", "../hackterm/save.save"])

    savefile = None
    for c in candidates:
        if os.path.isfile(c):
            savefile = c
            break

    if not savefile:
        print("No save file found. Provide path as first argument or place a save at one of:")
        for c in candidates:
            print("  ", c)
        sys.exit(1)

    print("Using save file:", savefile)

    servers, home_server, current_server = load_game(savefile)
    visualize_network(servers, home_server, current_server)
