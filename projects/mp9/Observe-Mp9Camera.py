"""Add read-only camera provenance to inspected private SDK translations.

Run after task lowering/REL merging, before shard emission. Original guest
calculations remain intact; only native observation scopes are inserted.
"""
import hashlib
import json
from pathlib import Path
import sys

FUNCTIONS = {
    '80192510': ('53e33aa8c0312a166b32c07f0ca791111d6e995f1bf1baf4fee2447d503d1c2b',
                 'LookAt', 'ctx->gpr[3], ctx->gpr[4], ctx->gpr[5], ctx->gpr[6]'),
    '80191D30': ('3371549db62f7a3cf67d0d5afdf30b98c4179024a7f5333e801aa36283f73d4a',
                 'Concat', 'ctx->gpr[5], ctx->gpr[3], ctx->gpr[4], 0'),
    '80191E00': ('0d2aad470432ca6a54a26b2786d81f725e8098b315063230340bbfd02b5e8860',
                 'ConcatArray', 'ctx->gpr[5], ctx->gpr[3], ctx->gpr[4], ctx->gpr[6]'),
}

def observe(text, address, kind, args):
    marker = f'    Mp9CameraObservationScope mp9CameraScope(Mp9CameraObservationScope::Kind::{kind}, {args}, ctx->lr);'
    if marker in text:
        return text
    start = f'extern "C" void func_{address}(CpuContext* MKW_RESTRICT ctx)\n{{'
    assert text.count(start) == 1, f'Unexpected SDK body at {address}'
    return ('#include "mp9_camera.h"\n' + text).replace(start, start + '\n' + marker, 1)

def main(root):
    generated = root / 'private-game/generated-native-boot'
    metadata = generated / 'base_translation_output.json'
    doc = json.loads(metadata.read_text())
    records = {r['entryPoint']: r for r in doc['functions']}
    for address, (expected, kind, args) in FUNCTIONS.items():
        path = generated / 'functions' / f'func_{address}.cpp'
        content = path.read_bytes()
        text = content.decode('utf-8').replace('\r\n', '\n')
        if 'Mp9CameraObservationScope mp9CameraScope' in text:
            marker = f'    Mp9CameraObservationScope mp9CameraScope(Mp9CameraObservationScope::Kind::{kind}, {args}, ctx->lr);\n'
            assert text.count(marker) == 1 and text.count('#include "mp9_camera.h"\n') == 1
            original = text.replace(marker, '', 1).replace('#include "mp9_camera.h"\n', '', 1)
            hashes = {hashlib.sha256(s.encode()).hexdigest() for s in (original, original.replace('\n', '\r\n'))}
            assert expected in hashes, f'Modified observed SDK source at {address}'
        else:
            assert hashlib.sha256(content).hexdigest() == expected, f'Uninspected SDK source at {address}'
        rewritten = observe(text, address, kind, args)
        assert observe(rewritten, address, kind, args) == rewritten
        # Break staging hard links rather than changing another translation's source.
        temporary = path.with_suffix('.camera.tmp')
        temporary.write_text(rewritten, encoding='utf-8', newline='\n')
        temporary.replace(path)
        record = records[int(address, 16)]
        record['size'] = path.stat().st_size
        record['sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
    metadata.write_text(json.dumps(doc, indent=2), encoding='utf-8')
    print('Observed original LookAt, Concat and ConcatArray bodies; guest math unchanged. '
          'Run emit-build-shards before building: shards contain copied bodies, not includes.')

if __name__ == '__main__':
    main(Path(sys.argv[1]) if len(sys.argv) > 1 else Path('R:/MarioParty9VR'))
