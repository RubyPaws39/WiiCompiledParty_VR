"""Merge private relocatable AOT output into the developer build graph."""
import argparse
import hashlib
import json
import os
import shutil
import re
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

root = Path('R:/MarioParty9VR')
generated = root / 'private-game/generated-native-boot'
metadata = generated / 'base_translation_output.json'
document = json.loads(metadata.read_text())
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--add', nargs='+', help='Add named module folders while retaining current build coverage')
args = parser.parse_args()
modules = sorted((root / 'private-game/generated-rel-aot').glob('*/module.json'))
if args.add:
    wanted = set(args.add)
    modules = [p for p in modules if p.parent.name in wanted]
    assert {p.parent.name for p in modules} == wanted, 'Unknown module folder'
    replacing = {json.loads(p.read_text())['id'] for p in modules}
    document['functions'] = [f for f in document['functions']
        if f['entryPoint'] < 0xA0000000 or ((f['entryPoint'] - 0xA0000000) >> 21) not in replacing]
else:
    disc_names = {p.name.removesuffix('.lz') for p in (root / 'private-game/extracted/DATA/files').rglob('*.rel*')
        if p.name.endswith(('.rel', '.rel.lz'))}
    module_names = {json.loads(p.read_text())['name'] for p in modules}
    assert disc_names and module_names == disc_names, (
        f'Module inventory mismatch: missing {sorted(disc_names - module_names)}, extra {sorted(module_names - disc_names)}')
    document['functions'] = [f for f in document['functions'] if f['entryPoint'] < 0xA0000000]
for entry in document['functions']:
    content = (generated / 'functions' / entry['relativePath']).read_bytes()
    assert hashlib.sha256(content).hexdigest() == entry['sha256'], (
        f"Stale build metadata for {entry['entryPoint']:#x}; regenerate with --output-metadata")
known = {f['entryPoint'] for f in document['functions']}
assert modules, 'No private REL AOT modules generated'
def source_facts(source):
    content = source.read_bytes()
    target = generated / 'functions' / source.name
    if not target.exists() or target.read_bytes() != content:
        if target.exists():
            target.unlink()
        try:
            os.link(source, target)
        except OSError:
            shutil.copyfile(source, target)
    return hashlib.sha256(content).hexdigest(), len(content)

with ThreadPoolExecutor(max_workers=8) as readers:
    for manifest in modules:
        module = json.loads(manifest.read_text())
        print(f"Merging {manifest.parent.name}: {len(module['entries'])} functions", flush=True)
        sources = [manifest.parent / entry['relativePath'] for entry in module['entries']]
        for entry, source, (digest, size) in zip(module['entries'], sources, readers.map(source_facts, sources)):
            address = entry['entryPoint']
            assert address not in known, f'Duplicate native entry {address:#x}'
            known.add(address)
            # Flat paths preserve emitter checks; hard links avoid duplicate data.
            entry['relativePath'] = source.name
            entry['sha256'] = digest
            entry['size'] = size
            document['functions'].append(entry)
native = {int(value, 16) for value in re.findall(
    r'REGISTER_NATIVE_FUNCTION\(0x([0-9A-Fa-f]+)',
    (root / 'work/Wiicompiled_VR/ports/mp9/sdk_bindings.cpp').read_text())}
available = known | native
for entry in document['functions']:
    if entry['entryPoint'] < 0xA0000000:
        continue  # Existing DOL graph also lists SDK overrides and interrupt vectors.
    for target in entry.get('build', {}).get('directCallDependencies', []):
        assert target in available, f'Missing dependency {target:#x} from {entry["entryPoint"]:#x}'
document['functions'].sort(key=lambda f: f['entryPoint'])
metadata.write_text(json.dumps(document, indent=2))
print(f'Merged {len(modules)} private native REL modules, {len(known)} total entries.')
