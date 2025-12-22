#!/usr/bin/env python3

import sys
import re

def parse_log(file_path):
    # Extracts everything after 'text:'
    # Group 1: The full payload
    line_pattern = re.compile(r"^DebugStr.*?text:\s+(.*)")

    # Detects if the payload starts with an index (number followed by space)
    # Group 1: The rest of the string (the actual register data)
    index_pattern = re.compile(r"^\d+\s+(.*)")

    last_key = None      # The data used for comparison (no index)
    last_payload = None  # The full string to print (includes index)
    dup_count = 0

    def flush():
        """Helper to print the buffered line and the duplication count."""
        nonlocal last_payload, dup_count
        if last_payload is not None:
            print(last_payload)
            if dup_count > 0:
                print(f"  ... <{dup_count}> identical entries")

    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                match = line_pattern.search(line)

                # Skip lines that don't match the DebugStr format
                if not match:
                    continue

                current_payload = match.group(1)

                # Determine the comparison key.
                # If there is a leading index, strip it.
                # Otherwise, use the whole payload.
                index_match = index_pattern.match(current_payload)
                if index_match:
                    current_key = index_match.group(1)
                else:
                    current_key = current_payload

                # Compare with the previous line
                if current_key == last_key:
                    dup_count += 1
                else:
                    # New content found: print previous batch and reset
                    flush()
                    last_key = current_key
                    last_payload = current_payload
                    dup_count = 0

            # Print the final remaining buffer after file ends
            flush()

    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <logfile>")
    else:
        parse_log(sys.argv[1])
