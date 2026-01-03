from textual.app import App, ComposeResult
from textual.widgets import Static, Button, Log  # <-- Log replaces TextLog

class HackTermUI(App):
    CSS = """
    Screen {
        layout: horizontal;
    }
    #menu { width: 15%; background: rgb(30,30,30); }
    #console { width: 70%; background: rgb(10,10,10); }
    #stats { width: 15%; background: rgb(25,25,25); }
    Button { margin: 1; background: rgb(50,50,50); color: rgb(0,255,0); }
    """

    def compose(self) -> ComposeResult:
        # Left menu
        yield Static(id="menu")
        # Center console
        yield Log(id="console")
        # Right stats
        yield Static(id="stats")

    def on_mount(self) -> None:
        # Populate menu buttons
        menu_panel = self.query_one("#menu")
        for name in ["Shop", "City", "School"]:
            btn = Button(label=name)
            menu_panel.mount(btn)

        # Example console output
        console = self.query_one(Log)
        console.write("Welcome to HackTerm!")

        # Example stats
        stats = self.query_one("#stats")
        stats.update("Money: $10k\nSecurity: 20\nScripts: 2")

    def on_button_pressed(self, event: Button.Pressed) -> None:
        console = self.query_one(Log)
        console.write(f"Menu clicked: {event.button.label}")

if __name__ == "__main__":
    HackTermUI().run()
