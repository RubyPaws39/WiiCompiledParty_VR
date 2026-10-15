"""Merge private relocatable AOT output into the developer build graph."""
import hashlib
import json
import re
from pathlib import Path

root = Path('R:/MarioParty9VR')
generated = root / 'private-game/generated-native-boot'
metadata = generated / 'base_translation_output.json'
document = json.loads(metadata.read_text())
document['functions'] = [f for f in document['functions'] if f['entryPoint'] < 0xA0000000]
for entry in document['functions']:
    content = (generated / 'functions' / entry['relativePath']).read_bytes()
    assert hashlib.sha256(content).hexdigest() == entry['sha256'], (
        f"Stale build metadata for {entry['entryPoint']:#x}; regenerate with --output-metadata")
known = {f['entryPoint'] for f in document['functions']}
modules = sorted((root / 'private-game/generated-rel-aot').glob('*/module.json'))
assert modules, 'No private REL AOT modules generated'
for manifest in modules:
    module = json.loads(manifest.read_text())
    for entry in module['entries']:
        address = entry['entryPoint']
        assert address not in known, f'Duplicate native entry {address:#x}'
        known.add(address)
        source = manifest.parent / entry['relativePath']
        target = generated / 'functions' / entry['relativePath']
        content = source.read_bytes()
        if not target.exists() or target.read_bytes() != content:
            target.write_bytes(content)
        entry['sha256'] = hashlib.sha256(content).hexdigest()
        entry['size'] = len(content)
        document['functions'].append(entry)
native = {int(value, 16) for value in re.findall(
    r'REGISTER_NATIVE_FUNCTION\(0x([0-9A-Fa-f]+)',
    (root / 'work/Wiicompiled_VR/ports/mp9/sdk_bindings.cpp').read_text())}
for entry in document['functions']:
    if entry['entryPoint'] < 0xA0000000:
        continue  # Existing DOL graph also lists SDK overrides and interrupt vectors.
    for target in entry.get('build', {}).get('directCallDependencies', []):
        assert target in known | native, f'Missing dependency {target:#x} from {entry["entryPoint"]:#x}'
document['functions'].sort(key=lambda f: f['entryPoint'])
metadata.write_text(json.dumps(document, indent=2))
print(f'Merged {len(modules)} private native REL modules, {len(known)} total entries.')
