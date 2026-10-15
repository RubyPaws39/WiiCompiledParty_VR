"""Lower the owned MP9 DOL's four custom task save/load callers.

No game instructions or assets are embedded here. Generated input stays private.
Run after translate-recursive and before emit-build-shards.
"""
import re
import json
import hashlib
import sys
from pathlib import Path

SAVE = '    InvokeDirectCpu<0x8020C6D8u>(ctx);'
LOAD = '    InvokeDirectCpu<0x8020C7DCu>(ctx);'
EXPECTED = {
    '80071160': ('80071200', '80071290'),
    '800712F0': ('80071320',),
    '80071590': ('800715AC',),
    '800715E0': ('80071600',),
}


def lower(text, resumes):
    if '#include "mp9_task_context.h"' in text:
        return text
    assert text.count(SAVE) == len(resumes), 'Unexpected MP9 save sites'
    assert text.count(LOAD) == 1, 'Unexpected MP9 restore sites'
    # Every resident architectural register must reflect the restored context,
    # even where ordinary ABI analysis would only reload volatile return regs.
    residents = re.findall(
        r'^    (?:uint32_t|PPC_FPR) (\w+) = (ctx->(?:gpr\[\d+\]|fpr\[\d+\]|cr|xer|ctr));$',
        text, re.M)
    assert residents
    reloads = '\n'.join(f'    {name} = {source};' for name, source in residents)
    text = text.replace('#include "abi_bridge.h"',
                        '#include "abi_bridge.h"\n#include "mp9_task_context.h"')
    cursor = 0
    for resume in resumes:
        call = text.index(SAVE, cursor)
        # Confirm original save has the expected LR before adding its resume
        # label. Fail closed if translation output or function boundaries drift.
        previous = text.rfind('ctx->lr = ', 0, call)
        assert text[previous:].startswith(f'ctx->lr = 0x{resume}u;')
        text = text[:call] + text[call:].replace(
            SAVE, '    Mp9RememberTaskContext(ctx);\n'
            '    InvokeDirectCpu<0x8020C6D8u>(ctx);\n'
            f'mp9_resume_{resume}:', 1)
        cursor = text.index(f'mp9_resume_{resume}:', call) + len(f'mp9_resume_{resume}:')
    dispatch = '\n'.join(f'    case 0x{pc}u: goto mp9_resume_{pc};' for pc in resumes)
    text = text.replace(LOAD,
        '    Mp9RestoreTaskContext(ctx);\n' + reloads + '\n'
        '    switch (ctx->lr) {\n' + dispatch + '\n'
        '    default: throw std::runtime_error("MP9 task resumed at unexpected LR: " + std::to_string(ctx->lr));\n'
        '    }')
    return text


def main(root):
    folder = root / 'private-game/generated-native-boot/functions'
    callers = {p.stem.removeprefix('func_') for p in folder.glob('*.cpp')
               if p.stem not in ('func_8020C6D8', 'func_8020C7DC')
               and ('0x8020C6D8u>(ctx)' in p.read_text() or
                    '0x8020C7DCu>(ctx)' in p.read_text())}
    assert callers == set(EXPECTED), f'Unrecognized task callers: {callers}'
    for address, resumes in EXPECTED.items():
        path = folder / f'func_{address}.cpp'
        text = lower(path.read_text(), resumes)
        assert lower(text, resumes) == text, 'Lowering must be idempotent'
        assert LOAD not in text
        assert text.count('Mp9RememberTaskContext(ctx);') == len(resumes)
        for pc in resumes:
            assert f'mp9_resume_{pc}:' in text
            assert f'case 0x{pc}u: goto mp9_resume_{pc};' in text
        path.write_text(text)
    metadata_path = root / 'private-game/generated-native-boot/base_translation_output.json'
    if metadata_path.exists():
        document = json.loads(metadata_path.read_text())
        records = {entry['entryPoint']: entry for entry in document['functions']}
        for address in EXPECTED:
            content = (folder / f'func_{address}.cpp').read_bytes()
            record = records[int(address, 16)]
            record['size'] = len(content)
            record['sha256'] = hashlib.sha256(content).hexdigest()
        metadata_path.write_text(json.dumps(document, indent=2))
    print('Validated/lowered four MP9 task callers, five save continuations.')


if __name__ == '__main__':
    main(Path(sys.argv[1]) if len(sys.argv) > 1 else Path('R:/MarioParty9VR'))
