#!/usr/bin/env python3
# chimera_tui_client.py
# TUI Client for the PRAETORIAN_CHIMERA Server
# Reflecting the Himothy Covenant: Pragmatic Purity in a Textual Interface.
# v0.3 - Removed invalid 'markup' param from Log widget constructor.

import asyncio
import json
import httpx
import shlex # For parsing command arguments safely
from datetime import datetime # For now_str

from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.containers import Container, Vertical, Horizontal
from textual.css.query import DOMQuery # For DOMQuery.DoesNotExist
from textual.reactive import reactive
from textual.screen import ModalScreen, Screen
from textual.widgets import (
    Button,
    Footer,
    Header,
    Input,
    Label,
    Log,
    Markdown,
    Static,
    TextArea,
)

# --- Configuration ---
SERVER_BASE_URL = "http://localhost:7777" # PRAETORIAN_CHIMERA: Adjust if needed
REQUEST_TIMEOUT = 30.0  # seconds

# --- Utility Functions ---
def now_str() -> str:
    return datetime.now().strftime("%H:%M:%S")

# --- Screens ---

class HelpScreen(ModalScreen[None]):
    """Modal screen for displaying help information."""

    BINDINGS = [
        Binding("escape,q", "pop_screen", "Close Help"),
    ]

    def compose(self) -> ComposeResult:
        help_text = """
        ## Chimera TUI Client - Help

        **General Commands (in Chat Input):**
        - `/help`                       : Show this help screen.
        - `/clear`                      : Clear the chat log.
        - `/quit` or `/exit`            : Exit the TUI.
        - `/info`                       : View Agent Information.
        - `/history`                    : Refresh and view Agent History.
        - `/config`                     : View/Edit Agent Configuration (currently shows info).
        - `/reset`                      : Reset the Agent on the server.
        - `/tool <tool_name> <json_params_or_ask>`: Execute a tool.
          Example: `/tool file_tool {"action": "read", "path": "notes.txt"}`
          Example: `/tool get_current_time {}`
          If `<json_params_or_ask>` is `ask`, a dialog will prompt for JSON.

        **Navigation:**
        - `Ctrl+C`                      : Quit application (Textual default).
        - `Tab` / `Shift+Tab`           : Navigate focus.
        - `Up` / `Down` arrows          : Scroll in chat log / history.

        **In Dialogs/Modals:**
        - `Escape`                      : Close dialog/modal.
        - `Enter`                       : Confirm action (usually).
        """
        yield Vertical(
            Label("Chimera Client Help", id="help_title"),
            Markdown(help_text),
            Button("Close", variant="primary", id="close_help_button"),
            id="help_dialog"
        )

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "close_help_button":
            self.app.pop_screen()

class MessageScreen(ModalScreen[bool]):
    """A modal screen to display a message and get a yes/no confirmation."""
    def __init__(self, message: str, title: str = "Confirm"):
        super().__init__()
        self.message = message
        self.title_text = title

    def compose(self) -> ComposeResult:
        yield Vertical(
            Label(self.title_text, classes="dialog_title"),
            Static(self.message, classes="dialog_message"),
            Horizontal(
                Button("Yes", variant="primary", id="yes_button"),
                Button("No", variant="error", id="no_button"),
                classes="dialog_buttons"
            ),
            id="message_dialog_content"
        )

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "yes_button":
            self.dismiss(True)
        elif event.button.id == "no_button":
            self.dismiss(False)

class JsonInputScreen(ModalScreen[str | None]):
    """Modal screen for inputting JSON."""
    def __init__(self, prompt_message: str = "Enter JSON parameters:"):
        super().__init__()
        self.prompt_message = prompt_message
        self._initial_label_text = prompt_message

    def compose(self) -> ComposeResult:
        yield Vertical(
            Label(self.prompt_message, id="json_prompt_label"),
            TextArea(language="json", theme="monokai", id="json_input_area"),
            Horizontal(
                Button("Submit", variant="primary", id="submit_json"),
                Button("Cancel", id="cancel_json")
            ),
            id="json_input_dialog"
        )

    def on_mount(self) -> None:
        self.query_one(TextArea).focus()

    async def on_button_pressed(self, event: Button.Pressed) -> None:
        prompt_label = self.query_one("#json_prompt_label", Label)
        if event.button.id == "submit_json":
            text_area = self.query_one("#json_input_area", TextArea)
            try:
                json.loads(text_area.text)
                self.dismiss(text_area.text)
            except json.JSONDecodeError as e:
                self.app.bell()
                prompt_label.update(f"{self._initial_label_text}\n[bold red]Invalid JSON: {e}[/]")
        elif event.button.id == "cancel_json":
            self.dismiss(None)


class MainScreen(Screen):
    """The main screen for chat, agent info, and other interactions."""

    BINDINGS = [
        Binding("ctrl+h", "show_help_modal", "Help"),
        Binding("ctrl+i", "show_agent_info_action", "Agent Info", show=False),
        Binding("ctrl+s", "update_status_action", "Update Status", show=False),
        Binding("f5", "update_status_action", "Update Status", show=False),
    ]

    server_status: reactive[str] = reactive("Initializing...")
    agent_name: reactive[str] = reactive("Initializing...")

    def compose(self) -> ComposeResult:
        yield Header(name="Chimera TUI Client")
        yield Container(
            Static(id="status_bar"),
            Log(id="chat_log", highlight=True, auto_scroll=True), # Corrected: removed markup=True
            Input(placeholder="Type your prompt or /command...", id="prompt_input"),
            id="main_container"
        )
        yield Footer()

    def _update_status_bar(self) -> None:
        if not self.is_mounted: return
        try:
            status_widget = self.query_one("#status_bar", Static)
            status_widget.update(f"Agent: [b]{self.agent_name}[/b] | Server: [b]{self.server_status}[/b]")
        except DOMQuery.DoesNotExist:
            self.app.log.error("Status bar widget #status_bar not found during _update_status_bar.")

    async def on_mount(self) -> None:
        self.query_one("#prompt_input", Input).focus()
        self._update_status_bar()
        await self.action_update_status_action()

    def watch_server_status(self, old_status: str, new_status:str) -> None:
        self._update_status_bar()

    def watch_agent_name(self, old_name: str, new_name: str) -> None:
        self._update_status_bar()

    def add_chat_log_message(self, sender: str, message: str, style: str = ""):
        if not self.is_mounted: return
        try:
            log_widget = self.query_one("#chat_log", Log)
            timestamp = now_str()
            sender_lower = sender.lower()

            if sender_lower == "user": sender_styled = f"[bold blue]{sender}[/bold blue]"
            elif sender_lower == "agent": sender_styled = f"[bold green]{sender}[/bold green]"
            elif sender_lower == "system": sender_styled = f"[bold yellow]{sender}[/bold yellow]"
            elif sender_lower == "error": sender_styled = f"[bold red]{sender}[/bold red]"
            elif sender_lower.startswith("tool"): sender_styled = f"[bold magenta]{sender}[/bold magenta]"
            else: sender_styled = f"[bold]{sender}[/bold]"

            log_widget.write_line(f"[{timestamp}] {sender_styled}: {message}")
        except DOMQuery.DoesNotExist:
            self.app.log.error("Chat log widget #chat_log not found during add_chat_log_message.")

    async def _make_api_request(self, method: str, endpoint: str, data: dict | None = None, params: dict | None = None) -> httpx.Response | None:
        try:
            async with httpx.AsyncClient(timeout=REQUEST_TIMEOUT) as client:
                url = f"{SERVER_BASE_URL}{endpoint}"
                if method.upper() == "GET": response = await client.get(url, params=params)
                elif method.upper() == "POST": response = await client.post(url, json=data, params=params)
                else: self.add_chat_log_message("Error", f"Unsupported HTTP method: {method}"); return None
                response.raise_for_status()
                return response
        except httpx.ConnectError:
            self.server_status = "Connection Error"
            self.add_chat_log_message("Error", f"Failed to connect to server at {SERVER_BASE_URL}.")
        except httpx.TimeoutException:
            self.server_status = "Timeout"
            self.add_chat_log_message("Error", "Request timed out.")
        except httpx.HTTPStatusError as e:
            self.server_status = f"HTTP Error {e.response.status_code}"
            error_details = e.response.text
            try: error_json = e.response.json(); error_details = json.dumps(error_json, indent=2)
            except json.JSONDecodeError: pass
            self.add_chat_log_message("Error", f"HTTP Error: {e.response.status_code} - {e.request.url}\nDetails:\n{error_details}")
        except Exception as e:
            self.server_status = "Client Error"
            self.add_chat_log_message("Error", f"An unexpected client error occurred: {e}")
        return None

    async def _update_server_status_health_check(self) -> None:
        current_server_status = self.server_status
        response = await self._make_api_request("GET", "/health")
        if response and response.status_code == 200:
            try:
                data = response.json()
                if data.get("status") == "OK": self.server_status = "Connected"
                else: self.server_status = "Health Check Failed"
            except json.JSONDecodeError: self.server_status = "Health Invalid JSON"
        elif current_server_status != "Connection Error" and current_server_status != "Timeout" and not str(self.server_status).startswith("HTTP Error"):
            self.server_status = "Disconnected"

    async def _fetch_agent_info(self, display_to_log: bool = False) -> None:
        response = await self._make_api_request("GET", "/agent/info")
        if response and response.status_code == 200:
            try:
                data = response.json()
                self.agent_name = data.get("name", "Unknown Agent")
                if display_to_log:
                    info_str = json.dumps(data, indent=2)
                    self.add_chat_log_message("Agent Info", f"\n{info_str}")
            except json.JSONDecodeError:
                self.add_chat_log_message("Error", "Failed to parse agent info JSON.")

    async def _fetch_agent_history(self) -> None:
        response = await self._make_api_request("GET", "/agent/history")
        if response and response.status_code == 200:
            try:
                data = response.json()
                history_entries = data.get("history", [])
                if not history_entries: self.add_chat_log_message("System", "Agent history is empty."); return
                self.add_chat_log_message("System", f"Fetched {len(history_entries)} history entries. Displaying...")
                for entry in history_entries:
                    role = entry.get("role", "unknown")
                    content = entry.get("content", "")
                    self.add_chat_log_message(role.capitalize(), content)
            except json.JSONDecodeError:
                self.add_chat_log_message("Error", "Failed to parse agent history JSON.")

    async def _reset_agent_on_server(self) -> None:
        response = await self._make_api_request("POST", "/agent/reset")
        if response and response.status_code == 200:
            try:
                data = response.json()
                self.add_chat_log_message("System", data.get("message", "Agent reset successful."))
                if self.is_mounted: self.query_one("#chat_log", Log).clear()
                await self._fetch_agent_info()
            except json.JSONDecodeError:
                self.add_chat_log_message("Error", "Failed to parse agent reset response.")
        else:
            self.add_chat_log_message("Error", "Agent reset failed or server did not respond as expected.")

    async def on_input_submitted(self, message: Input.Submitted) -> None:
        user_input = message.value.strip()
        input_widget = self.query_one("#prompt_input", Input)
        input_widget.value = ""
        if not user_input: return
        self.add_chat_log_message("User", user_input)

        if user_input.startswith("/"):
            await self.handle_command(user_input)
        else:
            response = await self._make_api_request("POST", "/prompt", data={"prompt": user_input})
            if response and response.status_code == 200:
                try:
                    data = response.json()
                    agent_response = data.get("response", "Agent returned no response content.")
                    self.add_chat_log_message("Agent", agent_response)
                except json.JSONDecodeError:
                    self.add_chat_log_message("Error", "Failed to parse agent's JSON response.")

    async def handle_command(self, command_input: str) -> None:
        try: parts = shlex.split(command_input)
        except ValueError: self.add_chat_log_message("Error", "Invalid command syntax (check quotes)."); return

        command = parts[0].lower()
        args = parts[1:]

        if command == "/help": self.action_show_help_modal()
        elif command == "/clear":
            if self.is_mounted: self.query_one("#chat_log", Log).clear()
            self.add_chat_log_message("System", "Chat log cleared.")
        elif command == "/quit" or command == "/exit": await self.app.action_quit()
        elif command == "/info": await self._fetch_agent_info(display_to_log=True)
        elif command == "/history": await self._fetch_agent_history()
        elif command == "/reset": await self.action_confirm_reset_agent()
        elif command == "/config":
            self.add_chat_log_message("System", "Agent config: Use `/info` to view. Edit via API directly for now.")
            await self._fetch_agent_info(display_to_log=True)
        elif command == "/tool":
            if len(args) < 1: self.add_chat_log_message("Error", "Usage: /tool <tool_name> [<json_params_or_ask>]"); return
            tool_name = args[0]
            params_str = " ".join(args[1:]) if len(args) > 1 else "{}"

            if params_str.lower() == "ask":
                def tool_params_callback(params_json_str: str | None):
                    if params_json_str:
                        try: parsed_params = json.loads(params_json_str)
                        except json.JSONDecodeError as e: self.add_chat_log_message("Error", f"Invalid JSON from dialog: {e}"); return
                        asyncio.create_task(self._execute_tool_on_server(tool_name, parsed_params))
                self.app.push_screen(JsonInputScreen(f"Enter JSON for tool '{tool_name}':"), tool_params_callback)
                return
            else:
                try: json_params = json.loads(params_str)
                except json.JSONDecodeError as e:
                    self.add_chat_log_message("Error", f"Invalid JSON for tool '{tool_name}': {e}\nUse 'ask' or valid JSON.")
                    return
                await self._execute_tool_on_server(tool_name, json_params)
        else:
            self.add_chat_log_message("System", f"Unknown command: {command}. Type `/help`.")

    async def _execute_tool_on_server(self, tool_name: str, params: dict):
        self.add_chat_log_message("System", f"Executing tool '{tool_name}' with params: {json.dumps(params)}")
        endpoint = f"/agent/tools/{tool_name}/execute"
        response = await self._make_api_request("POST", endpoint, data=params)
        if response and response.status_code == 200:
            try:
                data = response.json()
                result = data.get("result", "Tool returned no result content.")
                self.add_chat_log_message(f"Tool ({tool_name})", result)
            except json.JSONDecodeError:
                self.add_chat_log_message("Error", f"Failed to parse tool '{tool_name}' JSON response.")

    def action_show_help_modal(self) -> None:
        self.app.push_screen(HelpScreen())

    async def action_confirm_reset_agent(self) -> None:
        def confirm_callback(do_reset: bool):
            if do_reset:
                self.add_chat_log_message("System", "Resetting agent as confirmed...")
                asyncio.create_task(self._reset_agent_on_server())
        self.app.push_screen(MessageScreen("Reset agent on server? (clears history/state)", title="Confirm Agent Reset"), confirm_callback)

    async def action_show_agent_info_action(self) -> None:
        await self._fetch_agent_info(display_to_log=True)

    async def action_update_status_action(self) -> None:
        self.add_chat_log_message("System", "Refreshing server status and agent info...")
        await self._update_server_status_health_check()
        if self.server_status == "Connected":
            await self._fetch_agent_info()
        else:
            self.agent_name = "N/A (Server Down)"


class ChimeraClientApp(App[None]):
    """The main TUI application for Chimera."""

    TITLE = "PRAETORIAN CHIMERA - TUI Client v0.3"
    CSS_PATH = "chimera_tui.css"

    def on_mount(self) -> None:
        self.push_screen(MainScreen())

if __name__ == "__main__":
    css_file_path = "chimera_tui.css"
    try:
        with open(css_file_path, "r") as f:
            pass 
    except FileNotFoundError:
        css_content = """
Screen {
    background: $surface-darken-1;
    color: $text;
    layout: vertical;
}
#main_container {
    layout: vertical;
    height: 1fr; 
}
#status_bar {
    width: 1fr;
    padding: 0 1;
    background: $primary-background;
    color: $text-muted; 
    dock: top;
    height: 1; 
}
#chat_log {
    height: 1fr; 
    background: $panel;
    border: round $primary-lighten-2;
    padding: 1;
    margin-bottom: 1;
}
#prompt_input {
    dock: bottom;
    border: round $accent;
}
#help_dialog, #message_dialog_content, #json_input_dialog {
    align: center middle;
    padding: 1 2; 
    width: 80%;
    max-width: 80;
    height: auto;
    max-height: 90%;
    border: thick $primary;
    background: $panel-darken-1; 
}
#help_title, .dialog_title { 
    width: 100%;
    text-align: center;
    padding-bottom: 1;
    text-style: bold;
}
#json_prompt_label { 
    padding-bottom: 1;
}
.dialog_message { 
    padding: 1 0;
    text-align: center; 
}
.dialog_buttons {
    width: 100%;
    align-horizontal: center; 
    padding-top: 1;
}
.dialog_buttons Button {
    margin: 0 1; 
}
#json_input_area {
    width: 1fr;
    height: 10;
    border: round $primary;
    margin: 1 0;
}
Input.-valid { border: round $success; }
Input.-invalid { border: round $error; }
TextArea.-valid { border: round $success; }
TextArea.-invalid { border: round $error; }
"""
        with open(css_file_path, "w") as f:
            f.write(css_content)
        print(f"Created default CSS file: {css_file_path}")

    app = ChimeraClientApp()
    app.run()
