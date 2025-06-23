#!/usr/bin/env python3

import argparse
import os
import sys

# --- Configuration ---
# Add or remove extensions you want to process when scanning folders
ALLOWED_EXTENSIONS = {
    '.txt', '.md', '.py', '.js', '.html', '.css', '.json',
    '.yaml', '.yml', '.csv', '.sh', '.rst', '.java', '.c', '.cpp', '.h', '.hpp'
    # Add more text-based extensions if needed
}
# --- End Configuration ---

def count_tokens_in_file(filepath):
    """
    Reads a file and counts tokens based on whitespace splitting.
    Returns the token count, or None if the file cannot be read or is not text.
    """
    try:
        # Try reading as UTF-8 first, the most common encoding
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        return len(content.split())
    except UnicodeDecodeError:
        # If UTF-8 fails, try the system's default encoding as a fallback
        try:
            with open(filepath, 'r', encoding=sys.getdefaultencoding()) as f:
                content = f.read()
            print(f"Warning: File '{filepath}' read with system default encoding '{sys.getdefaultencoding()}' after UTF-8 failed.", file=sys.stderr)
            return len(content.split())
        except Exception as e:
            print(f"Error: Could not read file '{filepath}': {e}", file=sys.stderr)
            return None
    except FileNotFoundError:
        print(f"Error: File not found '{filepath}'", file=sys.stderr)
        return None
    except IOError as e:
        print(f"Error: Could not read file '{filepath}': {e}", file=sys.stderr)
        return None
    except Exception as e:
        # Catch other potential errors (e.g., permission denied)
        print(f"Error processing file '{filepath}': {e}", file=sys.stderr)
        return None


def process_folder(folderpath):
    """
    Recursively scans a folder, counts tokens in allowed file types.
    Returns total tokens and number of files processed.
    """
    total_tokens = 0
    files_processed = 0
    files_skipped_ext = 0
    files_error = 0

    if not os.path.isdir(folderpath):
        print(f"Error: Folder not found or is not a directory: '{folderpath}'", file=sys.stderr)
        return 0, 0, 0, 0

    print(f"Scanning folder: {folderpath}...")
    for root, _, filenames in os.walk(folderpath):
        for filename in filenames:
            filepath = os.path.join(root, filename)
            _, ext = os.path.splitext(filename)

            if ext.lower() in ALLOWED_EXTENSIONS:
                token_count = count_tokens_in_file(filepath)
                if token_count is not None:
                    total_tokens += token_count
                    files_processed += 1
                    # Uncomment the line below for verbose output per file
                    # print(f"  - {filepath}: {token_count} tokens")
                else:
                    files_error += 1
            else:
                files_skipped_ext += 1
                # Uncomment the line below to see which files are skipped by extension
                # print(f"  - Skipping (extension): {filepath}")


    return total_tokens, files_processed, files_skipped_ext, files_error


def main():
    parser = argparse.ArgumentParser(
        description="Simple Token Counter (whitespace-based). Counts tokens in a specified file or all allowed files within a folder."
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-f", "--file",
        metavar="FILEPATH",
        help="Path to a single text file to process."
    )
    group.add_argument(
        "-F", "--folder",
        metavar="FOLDERPATH",
        help="Path to a folder to process recursively. Only files with allowed extensions will be counted."
    )

    args = parser.parse_args()

    total_tokens = 0
    files_processed = 0
    files_skipped = 0
    files_error = 0
    target_path = ""

    print("--- Token Counter Initializing ---")

    if args.file:
        target_path = args.file
        print(f"Processing single file: {target_path}")
        if not os.path.isfile(target_path):
             print(f"Error: File not found: '{target_path}'", file=sys.stderr)
             sys.exit(1)
        token_count = count_tokens_in_file(args.file)
        if token_count is not None:
            total_tokens = token_count
            files_processed = 1
        else:
             files_error = 1 # Error message already printed by count_tokens_in_file

    elif args.folder:
        target_path = args.folder
        total_tokens, files_processed, files_skipped, files_error = process_folder(args.folder)

    # --- Generate Report ---
    print("\n--- Token Count Report ---")
    print(f"Target: {target_path}")
    if args.file:
        if files_processed == 1:
            print(f"Status: Success")
            print(f"Total Tokens: {total_tokens}")
        else:
            print(f"Status: Failed to process file")
    elif args.folder:
        print(f"Files Processed: {files_processed}")
        print(f"Files Skipped (unsupported extension): {files_skipped}")
        print(f"Files With Errors: {files_error}")
        print("-" * 20)
        print(f"Total Tokens Counted: {total_tokens}")

    print("--- End Report ---")

    if files_error > 0 and args.file:
        sys.exit(1) # Exit with error if the single file failed
    elif files_processed == 0 and files_error == 0 and args.folder:
        print("Note: No files with allowed extensions were found or processed.")


if __name__ == "__main__":
    main()
