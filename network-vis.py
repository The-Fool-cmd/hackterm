from pyvis.network import Network

class Server:
    def __init__(self, id, name, security, money, links):
        self.id = id
        self.name = name
        self.security = security
        self.money = money
        self.links = links

def load_game(filename):
    servers = {}
    with open(filename, "r") as f:
        server_count, home_server, current_server = map(int, f.readline().split())
        for _ in range(server_count):
            line = f.readline().strip().split()
            id = int(line[0])
            name = line[1]
            security = int(line[2])
            money = int(line[3])
            link_count = int(line[4])
            links = list(map(int, f.readline().strip().split()))
            servers[id] = Server(id, name, security, money, links)
    return servers, home_server, current_server

def visualize_network(servers, home_server, current_server):
    net = Network(height="750px", width="100%", bgcolor="#222222", font_color="white")
    
    for server in servers.values():
        color = "gray"
        if server.id == home_server:
            color = "green"
        elif server.id == current_server:
            color = "red"

        net.add_node(server.id, label=server.name, title=f"Security: {server.security}\nMoney: {server.money}", color=color)

    for server in servers.values():
        for link in server.links:
            net.add_edge(server.id, link)

    net.write_html("server_network.html")
    print("Network saved as server_network.html")

if __name__ == "__main__":
    savefile = "save.save"
    servers, home_server, current_server = load_game(savefile)
    visualize_network(servers, home_server, current_server)
