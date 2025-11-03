#!/usr/bin/env python3
import os
import sys

print()
print()

print("______________________ hello.py ______________________\n")

# Print env vars
for k in sorted(os.environ.keys()):
	print(f"{k}={os.environ[k]}")
	print()

# Read request body from stdin
cl = os.environ.get("CONTENT_LENGTH", "0")
try:
	n = int(cl)
except ValueError:
	n = 0

body_bytes = sys.stdin.buffer.read(n) if n > 0 else b""
print("______________________ BODY (raw) ______________________")
print(body_bytes.decode("utf-8", errors="replace"))

# Optional: parse URL-encoded forms
ct = os.environ.get("CONTENT_TYPE", "")
if ct.startswith("application/x-www-form-urlencoded"):
	from urllib.parse import parse_qs
	print("\n______________________ BODY (form parsed) ______________________")
	print(parse_qs(body_bytes.decode("utf-8", errors="replace")))
