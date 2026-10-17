"""Validate explicit SDK entries; prepare native DOL build metadata."""
import json
import re
from pathlib import Path

ROOT = Path('R:/MarioParty9VR')
fork = ROOT / 'work/Wiicompiled_VR'
bindings = (fork / 'ports/mp9/sdk_bindings.cpp').read_text()
symbols = (ROOT / 'reference-mp9/config/SSQE01/symbols.txt').read_text()
expected = {
    'Mp9OSDisableInterrupts': 'OSDisableInterrupts',
    'Mp9OSEnableInterrupts': 'OSEnableInterrupts',
    'Mp9OSRestoreInterrupts': 'OSRestoreInterrupts',
    'OS__GetTime_HLE': 'OSGetTime',
    'OS____GetSystemTime_801aad7c': '__OSGetSystemTime',
    'OS__Report_801a25d0': 'OSReport', 'OSFatal_HLE_801a4ec4': 'OSFatal',
    'DCInvalidateRange_801a1600': 'DCInvalidateRange', 'DCFlushRange_801a162c': 'DCFlushRange',
    'DCFlushRangeNoSync_801a168c': 'DCFlushRangeNoSync', 'DCZeroRange_HLE_801a16e4': 'DCZeroRange',
    'Mp9OSInterruptInit': '__OSInterruptInit',
    'Mp9EXIInit': 'EXIInit', 'Mp9EXISelect': 'EXISelect', 'Mp9EXIDeselect': 'EXIDeselect',
    'Mp9EXILock': 'EXILock', 'Mp9EXIUnlock': 'EXIUnlock', 'Mp9EXIImm': 'EXIImm',
    'Mp9EXIDma': 'EXIDma', 'Mp9EXISync': 'EXISync', 'Mp9EXIGetID': 'EXIGetID',
    'VIGetHostTvFormat': 'VIGetTvFormat', 'Mp9SIInit': 'SIInit',
    'Mp9SISetSamplingRate': 'SISetSamplingRate', 'Mp9SIGetType': 'SIGetType',
    'Mp9OSInitAudioSystem': '__OSInitAudioSystem',
    'Mp9OSMaskInterrupts': '__OSMaskInterrupts', 'Mp9OSUnmaskInterrupts': '__OSUnmaskInterrupts',
    'Mp9OSInitMemoryProtection': '__OSInitMemoryProtection',
    'Mp9IPCCltInit': 'IPCCltInit', 'NAND_IOS_Open_HLE': 'IOS_Open',
    'IOS_OpenAsync_HLE': 'IOS_OpenAsync', 'IOS_IoctlAsync_80194158': 'IOS_IoctlAsync',
    'NAND_IOS_Ioctlv_HLE': 'IOS_Ioctlv',
    'IOS_CloseAsync_HLE': 'fn_801B9490',
    'IOS_ReadAsync_HLE': 'fn_801B9600',
    'NAND_IOS_Read_HLE': 'fn_801B9700',
    'IOS_WriteAsync_HLE': 'fn_801B9810',
    'NAND_IOS_Write_HLE': 'fn_801B9910',
    'IOS_SeekAsync_HLE': 'fn_801B9A20',
    'NAND_IOS_Seek_HLE': 'fn_801B9B00',
    'NAND_IOS_Ioctl_HLE': 'fn_801B9D30',
    'IOS_IoctlvAsync_HLE': 'fn_801B9FA0',
    'VIInit_HLE_801b94a4': 'fn_8018DF90',
    'VISetPreRetraceCallback_HLE_801b90f4': 'fn_8018DBD0',
    'VISetPostRetraceCallback_HLE_801b9138': 'fn_8018DC20',
    'VIWaitForRetrace_HLE_801b99ec': 'fn_8018E4E0',
    'VIConfigure_HLE_801b9f6c': 'fn_8018EA70',
    'VIFlush_HLE_801ba9a4': 'fn_8018F4A0',
    'VISetNextFrameBuffer_HLE_801baab8': 'fn_8018F5C0',
    'Mp9VIGetCurrentFrameBuffer': 'fn_8018F630',
    'VISetBlack_HLE_801bab2c': 'fn_8018F640',
    'VIGetRetraceCount_HLE_801baba4': 'fn_8018F6C0',
    'VIGetNextField_HLE_801babac': 'fn_8018F6D0',
    'VIGetCurrentLine_HLE_801bac48': 'fn_8018F770',
    'Mp9VIGetScanMode': 'fn_8018F860',
    'VIGetDTVStatus_HLE_801bad38': 'fn_8018F8C0',
    'GX__SetCPUFifo_8016c94c': 'GXSetCPUFifo',
    'Mp9OSGetResetCode': 'OSGetResetCode',
    'DVDInit_8015EA1C': 'DVDInit',
    'Mp9DVDInquiryAsync': 'DVDInquiryAsync',
    'DVDReadPrio_8015E834': 'fn_8019D810',
    'DVD__ReadAsyncPrio_HLE_8015e74c': 'fn_8019D720',
    'Mp9GXInit': 'GXInit',
    'GX__InitTexObj_801707f8': 'fn_80198480',
    'GX__InitTexObjCI_80170a04': 'fn_80198690',
    'Mp9GXLoadTexObjPreLoaded': 'fn_80198AF0',
    'GX__LoadTexObj_80170f2c': 'fn_80198C60',
    'GX__InitTlutObj_80170f80': 'fn_80198CC0',
    'GX__LoadTlut_80170fa8': 'fn_80198CF0',
    'GX__InitTexObjLOD_80170a4c': 'fn_801986E0',
    'GX__SetGPFifo_8016cb2c': 'GXSetGPFifo',
    '__GX__PEInit_8016ee14': '__GXPEInit',
    'GX__DrawDone_8016eab0': 'fn_80196560',
    'Mp9GXCopyDisp': 'fn_80197830',
    'GX__SetTexCopySrc_8016f478': 'fn_80197040',
    'GX__SetTexCopyDst_8016f4dc': 'fn_801970B0',
    'GX__CopyTex_8016fd74': 'fn_80197970',
    'SelectThread_801a9c08': 'SelectThread',
    'OSCreateThread_HLE_801a9e84': 'fn_801869E0',
    'OSExitThread_HLE_801aa0f0': 'fn_80186C50',
    'OSCancelThread_HLE_801aa1d4': 'OSCancelThread',
    'OSJoinThread_HLE_801aa3ac': 'fn_80186F20',
    'OSResumeThread_HLE_801aa58c': 'fn_80187070',
    'OSSuspendThread_HLE_801aa6a8': 'fn_80187310',
    'OSSleepThread_HLE_801aa9b8': 'OSSleepThread',
    'OSWakeupThread_HLE_801aaaa4': 'fn_80187590',
    'Mp9WUDInit': 'fn_801D0CC0',
    'Mp9WUDGetStatus': 'fn_801D1030',
    'Mp9WPADInitSub': 'fn_801BE8B0',
    'Mp9KPADReadEx': 'fn_801D6F80',
    'WPADProbe_HLE': 'fn_801C0370',
    'AIInit_801240b0': 'fn_801A5FC0',
    'AIRegisterDMACallback_80123f88': 'fn_801A5E80',
    'AIInitDMA_80123fcc': 'fn_801A5ED0',
    'AIStartDMA_80124048': 'fn_801A5F50',
    'AIGetDMABytesLeft_8012405c': 'fn_801A5F70',
    'AIGetDMAStartAddr_8012406c': 'fn_801A5F80',
    'AIGetDMALength_80124084': 'fn_801A5FA0',
    'AICheckInit_80124094': 'fn_801A5FB0',
    'DSPInit_8015d444': 'fn_801B1810',
    'DSPCheckInit_8015d504': 'fn_801B18D0',
    'DSPAddTask_8015d50c': 'fn_801B18E0',
    'DSPAssertTask_8015d57c': 'fn_801B1950',
    'DSPCheckMailToDSP_8015d3fc': 'fn_801B17B0',
    'DSPCheckMailFromDSP_8015d40c': 'fn_801B17C0',
    'DSPReadMailFromDSP_8015d41c': 'fn_801B17D0',
    'DSPSendMailToDSP_8015d430': 'fn_801B17F0',
    '__AXOutInitDSP_801269bc': 'fn_801A80C0',
    # Verified unnamed wrapper builds IPC command 2 with FD from r3, no callback.
    'NAND_IOS_Close_HLE': 'fn_801B9550',
}
registrations = re.findall(r'REGISTER_NATIVE_FUNCTION\((0x[0-9A-Fa-f]+),\s*(\w+)\)', bindings)
if len(registrations) != len(expected):
    raise ValueError('Unexpected SDK binding count')
for address, name in registrations:
    symbol = expected[name]
    match = re.search(r'^' + re.escape(symbol) + r' = \.text:(0x[0-9A-Fa-f]+);', symbols, re.M)
    if not match or int(match[1], 16) != int(address, 16):
        raise ValueError(f'SDK address mismatch: {symbol} at {address}')
data_symbols = {
    'lbl_8026EBC8': 0x8026EBC8, 'lbl_8026EBD0': 0x8026EBD0,
    'lbl_8026EBD4': 0x8026EBD4, 'lbl_8026EBD8': 0x8026EBD8,
    'lbl_8026EBDC': 0x8026EBDC, 'lbl_8026EBE0': 0x8026EBE0,
    'lbl_8026EBE4': 0x8026EBE4,
    'lbl_8026EA40': 0x8026EA40, 'lbl_8026EA44': 0x8026EA44,
    'lbl_8026EA74': 0x8026EA74, 'lbl_8026EA78': 0x8026EA78,
    'lbl_802F5250': 0x802F5250, 'lbl_802F6260': 0x802F6260,
    'lbl_802FB408': 0x802FB408,
    'OSExceptionTable': 0x8026E6A4, 'InterruptHandlerTable': 0x8026E718,
    'AlarmQueue': 0x8026E6C8, 'Ecb': 0x802CD8B0, 'Scb': 0x802CCA80,
    'Packet': 0x802CD970, 'Type': 0x8025B6E0, 'Si': 0x8025B6C8,
    'SamplingRate': 0x8026E7F0,
    'DefaultThread': 0x802CCAD8, 'lbl_802CCDF0': 0x802CCDF0,
    'lbl_802CCEF0': 0x802CCEF0, 'SwitchThreadCallback': 0x8026DE00,
    'Reschedule': 0x8026E740, 'RunQueueHint': 0x8026E744, 'RunQueueBits': 0x8026E748,
    '__OSRebootParams': 0x802CC6C0,
    'WaitingQueue': 0x802D3330, '__DVDThreadQueue': 0x8026E930,
    'DVDInitialized': 0x8026E99C,
    'GXTexRegionAddrTable': 0x8025C470,
    '__GXData': 0x80270220, 'FifoObj': 0x802CDD20,
    'lbl_802CDDA0': 0x802CDDA0, 'lbl_8026E910': 0x8026E910,
    'lbl_8026E918': 0x8026E918,
    'CurrTvMode': 0x8026E868, 'lbl_802CDC80': 0x802CDC80,
    'lbl_8026E7F8': 0x8026E7F8,
    'lbl_8026E804': 0x8026E804,
    'lbl_8026E860': 0x8026E860,
    'lbl_8026E864': 0x8026E864,
    'lbl_8026E874': 0x8026E874,
    'lbl_8026E878': 0x8026E878,
    'lbl_8026E880': 0x8026E880,
    'lbl_8026E8A4': 0x8026E8A4,

}
for name, address in data_symbols.items():
    entries = re.findall(r'^' + re.escape(name) + r' = [^:]+:(0x[0-9A-Fa-f]+);', symbols, re.M)
    if not any(int(entry, 16) == address for entry in entries):
        raise ValueError(f'SDK global mismatch: {name} at {address:#x}')
manifest = json.loads((ROOT / 'work/native-probe/mp9-strap.yml').read_text())
manifest['project']['id'] = 'mp9-ssqe01-native-boot-probe'
manifest['project']['display_name'] = 'Mario Party 9 native boot probe'
manifest['inputs'].pop('rel')
manifest['translation']['entry_points'] = ['0x80004050']
manifest['translation']['function_map'] = {'path': 'work/native-probe/functions.map'}
manifest['runtime']['native_registration_root'] = 'work/Wiicompiled_VR/ports/mp9'
manifest['output']['root'] = 'private-game/generated-native-boot'
(ROOT / 'work/native-probe/mp9-native-boot.yml').write_text(json.dumps(manifest, indent=2))
print(f'Validated {len(registrations)} SDK entries and {len(data_symbols)} data globals against SSQE01 symbols.')
