### Setup and Usage

**1. Initial Setup (One-time only):**

This tool uses a self-contained Python virtual environment. To set it up, run:

```bash
make setup
```

This will create a `.venv` directory and install the required `pyperclip` library into it.

**2. Make the Harvester Executable:**

```bash
chmod +x harvester
```

**3. Running the Harvester:**

To analyze the current directory and copy the context to your clipboard:

```bash
./harvester
```

To analyze a different directory:

```bash
./harvester /path/to/your/project
```

To save the output to a file instead of the clipboard:

```bash
./harvester -f project_context.md
```

**4. Customizing Ignores:**

Edit the `.harvesterignore` file to add or remove patterns. The syntax is similar to `.gitignore`.