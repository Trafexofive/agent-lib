#!/usr/bin/env python3
import argparse
import fnmatch
from pathlib import Path
import pyperclip
import sys

def load_ignore_patterns(root_dir: Path) -> list[str]:
    ignore_file = root_dir / '.harvesterignore'
    if not ignore_file.is_file():
        return []
    with open(ignore_file, 'r', encoding='utf-8') as f:
        return [line.strip() for line in f if line.strip() and not line.startswith('#')]

def is_ignored(path: Path, root_dir: Path, patterns: list[str]) -> bool:
    relative_path = path.relative_to(root_dir)
    for pattern in patterns:
        if fnmatch.fnmatch(str(relative_path), pattern) or any(fnmatch.fnmatch(part, pattern) for part in relative_path.parts):
            return True
    return False

def generate_tree(root_dir: Path, patterns: list[str], max_level: int = 3) -> str:
    tree_lines = []
    def recurse(dir_path: Path, prefix: str = '', level: int = 0):
        if level >= max_level: return
        files = sorted([p for p in dir_path.iterdir() if not is_ignored(p, root_dir, patterns)])
        pointers = ['├── '] * (len(files) - 1) + ['└── ']
        for pointer, path in zip(pointers, files):
            tree_lines.append(f"{prefix}{pointer}{path.name}")
            if path.is_dir():
                extension = '│   ' if pointer == '├── ' else '    '
                recurse(path, prefix + extension, level + 1)
    recurse(root_dir)
    return '\n'.join(tree_lines)

def main():
    parser = argparse.ArgumentParser(description='AI Context Harvester')
    parser.add_argument('project_dir', nargs='?', default='.', help='Project directory to analyze.')
    parser.add_argument('-f', '--file', help='Output to file instead of clipboard.')
    args = parser.parse_args()

    root_dir = Path(args.project_dir).resolve()
    if not root_dir.is_dir():
        print(f"Error: Directory not found at '{root_dir}'", file=sys.stderr)
        sys.exit(1)

    ignore_patterns = load_ignore_patterns(root_dir)
    output = ['# AI Project Analysis - {}'.format(root_dir.name)]

    tree_str = generate_tree(root_dir, ignore_patterns)
    output.append('\n## Directory Structure\n```\n{}\n```'.format(tree_str))

    output.append('\n## Project Files')
    all_files = sorted([p for p in root_dir.rglob('*') if p.is_file() and not is_ignored(p, root_dir, ignore_patterns)])

    for file_path in all_files:
        rel_path = file_path.relative_to(root_dir)
        try:
            content = file_path.read_text(encoding='utf-8')
            output.append(f"\n### File: {rel_path}\n```{Path(rel_path).suffix.lstrip('.')}\n{content}\n```")
        except Exception:
            output.append(f"\n### File: {rel_path}\n```\n[Could not read binary file]\n```")
    
    final_output = '\n'.join(output)

    if args.file:
        Path(args.file).write_text(final_output, encoding='utf-8')
        print(f"Project context harvested to '{args.file}'.")
    else:
        try:
            pyperclip.copy(final_output)
            print('Project context copied to clipboard!')
        except pyperclip.PyperclipException:
            print('Error: Clipboard functionality not available. Please install xclip, xsel, or wl-copy.', file=sys.stderr)

if __name__ == '__main__':
    main()
