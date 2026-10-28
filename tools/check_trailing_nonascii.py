"""Lists (and optionally fixes) source lines that end with a non-ASCII character.

MSVC on a CP949 locale reads BOM-less files as CP949; a trailing UTF-8 byte right before the
newline pairs with the newline as a double-byte character and the next line is swallowed into
the comment. Fix: make every such line end with ASCII (a period for comments).
"""
import os, sys

root = sys.argv[1]
fix = len(sys.argv) > 2 and sys.argv[2] == '--fix'
exts = ('.cpp', '.h', '.hpp', '.inl', '.fx', '.rc', '.def')
skip = (os.path.join('core', 'fxjit'),)
found = 0
for dirpath, _, files in os.walk(root):
    rel = os.path.relpath(dirpath, root)
    if any(rel.startswith(s) for s in skip):
        continue
    for f in files:
        if not f.endswith(exts):
            continue
        p = os.path.join(dirpath, f)
        raw = open(p, 'rb').read()
        if raw.startswith(b'\xef\xbb\xbf'):
            continue   # BOM: MSVC reads it as UTF-8, no problem
        lines = raw.split(b'\n')
        changed = False
        for i, line in enumerate(lines):
            body = line.rstrip(b'\r')
            if body and body[-1] >= 0x80:
                found += 1
                tail = body[-60:].decode('utf-8', 'replace').encode('ascii', 'backslashreplace').decode('ascii')
                print('%s:%d: %s' % (os.path.relpath(p, root), i + 1, tail))
                if fix:
                    stripped = body.lstrip()
                    if b'//' in body or stripped.startswith(b'*') or stripped.startswith(b'/*'):
                        lines[i] = body + b'.' + (b'\r' if line.endswith(b'\r') else b'')
                        changed = True
        if changed:
            open(p, 'wb').write(b'\n'.join(lines))
print('lines found:', found)
