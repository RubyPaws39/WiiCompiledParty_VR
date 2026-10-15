using System.Buffers.Binary;
using Translator.Core.Ir;
using Translator.Core.Loading;
using Translator.Core.Translation;
using Xunit;

namespace Translator.Tests;

public class RelocationOperandRewriteTests
{
    [Fact]
    public void RewriteRunsBeforeConstantFoldingAndRetainsDynamicOperand()
    {
        var memory = new byte[12];
        BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(0), 0x3C608040); // lis r3,0x8040
        BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(4), 0x38630100); // addi r3,r3,0x100
        BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(8), 0x4E800020); // blr
        var range = AddressRange.FromStartAndSize(MemoryLayout.RamBase, 12);
        var image = new ProgramImage(memory, range, range, default, "synthetic-relocation");
        var observed = new List<uint>();
        var result = new FunctionTranslator(image).Translate(MemoryLayout.RamBase,
            TranslationOptions.Default with {
                RewriteLiftedInstructions = instructions => instructions.Select(instruction => {
                    observed.Add(instruction.Origin.Address);
                    return instruction.Origin.Address != MemoryLayout.RamBase ? instruction :
                        instruction with { Ir = new IrInstruction[] {
                            new IrCall("r3", "ReadSyntheticRelocationHigh", new[] { IrValue.Imm(1) })
                        }};
                }).ToArray()
            });
        Assert.Equal(new[] { MemoryLayout.RamBase, MemoryLayout.RamBase + 4, MemoryLayout.RamBase + 8 }, observed);
        Assert.Contains("ReadSyntheticRelocationHigh(1)", result.CxxCode);
        Assert.DoesNotContain("r3 = 0x80400100u", result.CxxCode);
    }
}
