"""Opt-in private developer controller through the game's normal KPAD sample path."""
import argparse, subprocess, time
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--buttons',type=lambda s:int(s,0),default=0)
p.add_argument('--x',type=float,default=0)
p.add_argument('--y',type=float,default=0)
p.add_argument('--press-ms',type=int,default=250)
p.add_argument('--capture',action='store_true')
p.add_argument('--disable',action='store_true')
p.add_argument('--output',default='R:/MarioParty9VR/work/dev-input-frame.bmp')
a=p.parse_args()
assert 0<=a.buttons<=65535 and -1<=a.x<=1 and -1<=a.y<=1 and 0<=a.press_ms<=2000
adb='R:/SonicRidersQuest/sdk/platform-tools/adb.exe'
def run(*args,**kw):
    kw.setdefault('check',True)
    kw.setdefault('timeout',15)
    return subprocess.run([adb,*args],**kw)
if a.disable:
    run('shell','run-as','org.marioparty9.quest','rm','-f','files/debug-tools/wii-input.txt')
else:
    if a.capture:
        run('shell','run-as','org.marioparty9.quest','rm','-f','files/debug-tools/game-frame.bmp')
    seq=time.time_ns()//1000000
    command=f'{seq} {a.buttons} {a.x} {a.y} {a.press_ms} {int(a.capture)}\n'
    local=Path('R:/MarioParty9VR/work/current-debug-input.txt');local.write_text(command,encoding='ascii')
    run('push',str(local),'/data/local/tmp/mp9-debug-input.txt',stdout=subprocess.DEVNULL)
    run('shell','run-as','org.marioparty9.quest','mkdir','-p','files/debug-tools')
    run('shell','run-as','org.marioparty9.quest','cp','/data/local/tmp/mp9-debug-input.txt','files/debug-tools/wii-input.txt')
    if a.capture:
        deadline=time.monotonic()+8
        while True:
            time.sleep(0.25)
            result=run('exec-out','run-as','org.marioparty9.quest','cat','files/debug-tools/game-frame.bmp',
                       stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,check=False)
            data=result.stdout
            if data[:2]==b'BM' and len(data)>=54 and int.from_bytes(data[2:6],'little')==len(data):break
            if time.monotonic()>=deadline:raise TimeoutError('Game did not acknowledge fresh capture; guest may be stalled')
        Path(a.output).write_bytes(data)
        print(a.output)
    else: print(command.strip())
