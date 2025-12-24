#!/usr/bin/env python3

import sys
import re

def parse_log(file_path):
    line_pattern = re.compile(r"^DebugStr.*?text:\s+(.*)")
    test_start_pattern = re.compile(r"^==\s+\w+\s+==\s*")

    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                match = line_pattern.search(line)
                if not match:
                    continue

                current_payload = match.group(1)
                if test_start_pattern.match(current_payload):
                    print("\n\n")
                    print(current_payload)
                    continue

                print(current_payload)

    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <logfile>")
    else:
        parse_log(sys.argv[1])
