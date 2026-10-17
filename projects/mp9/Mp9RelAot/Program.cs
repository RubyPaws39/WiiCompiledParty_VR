using System.Buffers.Binary;
using System.Text.Json;
using System.Text.RegularExpressions;
using Translator.Core.Ir;
using Translator.Core.Lifting;
using Translator.Core.Loading;
using Translator.Core.Parsing.Dol;
using Translator.Core.Parsing.Rel;
using Translator.Core.Translation;

if (args.Length != 2) throw new ArgumentException("Usage: Mp9RelAot ROOT MODULE.rel");
var root = Path.GetFullPath(args[0]);
if (Path.GetFileName(args[1]) != args[1] || !args[1].EndsWith(".rel"))
    throw new ArgumentException("Plain REL filename required");
var path = Path.Combine(root, "private-game/decoded-modules", args[1]);
var raw = File.ReadAllBytes(path);
uint Word(int offset) => BinaryPrimitives.ReadUInt32BigEndian(raw.AsSpan(offset, 4));
var id = Word(0);
if (id is 0 or > 115) throw new InvalidDataException("Unexpected MP9 module id");
const uint linkBase = 0x80400000;
var dispatchBase = 0xA0000000u + (id << 21);
var rel = RelFile.Load(path);
if (raw.Length >= 0x200000) throw new InvalidDataException("REL exceeds canonical dispatch slot");
var image = new ProgramImageBuilder().Build(
    DolFile.Load(Path.Combine(root, "private-game/extracted/DATA/sys/main.dol")),
    rel.BuildImage(linkBase));
var translator = new FunctionTranslator(image);
bool IsCode(uint address) => rel.Sections.Any(s => s.Executable &&
    address >= linkBase + s.FileOffset && address < linkBase + s.FileOffset + s.Size);
var seeds = new SortedSet<uint>();
for (int i = 0; i < 3; ++i) {
    var section = raw[0x30 + i];
    if (section != 0) seeds.Add(linkBase + rel.Sections[section].FileOffset + Word(0x34 + i * 4));
}
// Seed every linked code pointer and every direct call target in executable
// sections, not only the module prolog's reachable graph.
var immediates = new Dictionary<uint, (int Section, uint Site)>();
for (uint p = Word(0x28), end = p + Word(0x2C); p < end; p += 8) {
    var importId = Word((int)p);
    if (importId != 0 && importId != id) throw new InvalidDataException("Cross-module import unsupported");
    uint cursor = Word((int)p + 4), site = 0;
    int section = 0;
    while (true) {
        var delta = BinaryPrimitives.ReadUInt16BigEndian(raw.AsSpan((int)cursor, 2));
        var type = raw[cursor + 2]; var targetSection = raw[cursor + 3];
        var addend = Word((int)cursor + 4); cursor += 8;
        if (type == 203) break;
        if (type == 202) { section = targetSection; site = 0; continue; }
        site += delta;
        if (type is 4 or 6 && rel.Sections[section].Executable) {
            if ((site & 3) != 2) throw new InvalidDataException("Unaligned immediate relocation");
            immediates.Add(linkBase + rel.Sections[section].FileOffset + site - 2,
                (section, site - 2));
        }
        if (type is 1 or 4 or 6 && importId == id && rel.Sections[targetSection].Executable)
            seeds.Add(linkBase + rel.Sections[targetSection].FileOffset + addend);
    }
}
foreach (var section in rel.Sections.Where(s => s.Executable)) {
    for (uint offset = 0; offset + 4 <= section.Size; offset += 4) {
        var pc = linkBase + section.FileOffset + offset;
        var word = BinaryPrimitives.ReadUInt32BigEndian(image.Memory.AsSpan(image.GetOffset(pc), 4));
        if ((word >> 26) == 18 && (word & 1) != 0) {
            var displacement = (int)(word & 0x03FFFFFC);
            if ((displacement & 0x02000000) != 0) displacement |= unchecked((int)0xFC000000);
            var target = (word & 2) != 0 ? (uint)displacement : unchecked(pc + (uint)displacement);
            if (IsCode(target)) seeds.Add(target);
        }
    }
}
if (seeds.Any(a => !IsCode(a))) throw new InvalidDataException("Entry outside executable section");

IReadOnlyList<IrInstruction> Rewrite(LiftedInstruction instruction) {
    if (!immediates.TryGetValue(instruction.Origin.Address, out var relocation)) return instruction.Ir;
    var pc = instruction.Origin.Address;
    var word = BinaryPrimitives.ReadUInt32BigEndian(image.Memory.AsSpan(image.GetOffset(pc), 4));
    var opcode = word >> 26;
    if (opcode is not (13 or 14 or 15 or 32 or 33 or 34 or 36 or 37 or 38 or 40 or 41 or 42 or 44 or 48 or 50 or 52 or 54))
        throw new InvalidDataException($"Unsupported relocated opcode {opcode} at {pc:X8}");
    var oldImmediate = opcode == 15 ? word << 16 : unchecked((uint)(int)(short)word);
    var name = $"addr_mp9_imm_{pc:X8}_loc";
    var result = new List<IrInstruction> {
        new IrCall(name, opcode == 15 ? "Mp9RelHighImmediate" : "Mp9RelSignedImmediate",
            new[] { IrValue.Imm(id), IrValue.Imm(relocation.Section), IrValue.Imm(relocation.Site) })
    };
    int replaced = 0;
    IrValue Value(IrValue value) {
        if (value.Kind == "const" && unchecked((uint)value.Constant!.Value) == oldImmediate) {
            ++replaced; return IrValue.Register(name);
        }
        return value;
    }
    IrAddress Address(IrAddress address) {
        if (unchecked((uint)address.Offset) != oldImmediate) return address;
        var temp = name + "_address_loc";
        result.Add(new IrBinary(temp, IrValue.Register(address.Base), IrValue.Register(name), "add"));
        ++replaced; return new IrAddress(temp, 0);
    }
    foreach (var ir in instruction.Ir) result.Add(ir switch {
        IrAssign a => a with { Value = Value(a.Value) },
        IrBinary b => b with { Left = Value(b.Left), Right = Value(b.Right) },
        IrLoad l => l with { Address = Address(l.Address) },
        IrStore s => s with { Address = Address(s.Address) },
        IrCall c => c with { Arguments = c.Arguments.Select(Value).ToArray() },
        _ => ir
    });
    if (replaced == 0) throw new InvalidDataException($"Relocation operand not found at {pc:X8}");
    return result;
}

var output = Path.Combine(root, "private-game/generated-rel-aot", Path.GetFileNameWithoutExtension(args[1]));
Directory.CreateDirectory(output);
var entries = new List<object>();
var pending = seeds.ToList();
for (int index = 0; index < pending.Count; ++index) {
    var entry = pending[index];
    var translated = translator.Translate(entry, new TranslationOptions(
        EnableLeafInlining: false, KnownFunctionEntryPoints: seeds,
        RewriteLiftedInstructions: instructions =>
            instructions.Select(i => i with { Ir = Rewrite(i) }).ToArray()));
    var code = translated.CxxCode;
    File.WriteAllLines(Path.Combine(output, $"func_{dispatchBase + entry - linkBase:X8}.asm.txt"),
        translated.Instructions.Select(i => $"{i.Address:X8} {i.Mnemonic} " +
            string.Join(", ", i.Operands?.Select(o => o.ToOperandString()) ?? Array.Empty<string>())));
    // LR is an actual guest address even though native dispatch uses stable
    // module keys. Resolve it from the linked section, never a fixed heap base.
    code = Regex.Replace(code, @"ctx->lr = 0x([0-9A-Fa-f]{8})u;", match => {
        var address = Convert.ToUInt32(match.Groups[1].Value, 16);
        var section = rel.Sections.FirstOrDefault(s => s.Executable &&
            address >= linkBase + s.FileOffset && address <= linkBase + s.FileOffset + s.Size);
        return section == null ? match.Value :
            $"ctx->lr = Mp9RelAddress({id}u, {section.Index}u, {address - linkBase - section.FileOffset}u);";
    });
    code = Regex.Replace(code, @"switch \(([^\r\n]+)\) \{([\s\S]*?)default:", match => {
        var body = match.Groups[2].Value;
        if (!Regex.Matches(body, @"case 0x([0-9A-F]{8})u:")
            .Any(m => IsCode(Convert.ToUInt32(m.Groups[1].Value, 16)))) return match.Value;
        body = Regex.Replace(body, @"case 0x([0-9A-F]{8})u:", item => {
            var address = Convert.ToUInt32(item.Groups[1].Value, 16);
            return IsCode(address) ? $"case 0x{dispatchBase + address - linkBase:X8}u:" : item.Value;
        });
        return $"switch (Mp9CanonicalRelTarget({match.Groups[1].Value})) {{{body}default:";
    });
    // Only symbolic dispatch/label spellings are rebased. Runtime operands
    // above were lowered before constant folding; no guessed literal patches.
    code = Regex.Replace(code, @"(?<=func_)[0-9A-F]{8}|(?<=loc_)[0-9A-F]{8}|(?<=0x)[0-9A-F]{8}(?=u>\(ctx\))", match => {
        var address = Convert.ToUInt32(match.Value, 16);
        return IsCode(address) ? (dispatchBase + address - linkBase).ToString("X8") : match.Value;
    });
    // Conservative ABI contract prevents callers from treating relocatable
    // module entry points as ordinary leaf/state-free functions.
    code = "#include \"mp9_rel_context.h\"\n" + code;
    var addressKey = dispatchBase + entry - linkBase;
    var filename = $"func_{addressKey:X8}.cpp";
    File.WriteAllText(Path.Combine(output, filename), code);
    var dependencies = Regex.Matches(code, @"InvokeDirectCpu<0x([0-9A-F]{8})u>")
        .Select(m => Convert.ToUInt32(m.Groups[1].Value, 16)).Distinct().ToArray();
    foreach (var dependency in dependencies) {
        if (dependency < dispatchBase || dependency >= dispatchBase + 0x200000) continue;
        var originalEntry = linkBase + dependency - dispatchBase;
        if (!IsCode(originalEntry)) throw new InvalidDataException("Native call outside REL executable sections");
        if (seeds.Add(originalEntry)) pending.Add(originalEntry);
    }
    entries.Add(new { entryPoint = addressKey, relativePath = filename,
        build = new { symbol = $"func_{addressKey:X8}", preservesNonvolatileFprs = false,
            nonvolatileFprWriteMask = 0xFFFFC000u,
            directCallDependencies = dependencies,
            estimatedIrInstructions = translated.Metrics.IrInstructionCount },
        localLabelAddresses = Array.Empty<uint>(), interiorToOtherTranslation = false });
}
File.WriteAllText(Path.Combine(output, "module.json"), JsonSerializer.Serialize(new {
    id, name = args[1], dispatchBase, entries
}, new JsonSerializerOptions { WriteIndented = true }));
Console.WriteLine($"REL AOT {args[1]}: {entries.Count} native entries; {immediates.Count} relocated instruction operands");
