"""Drive normal Wii input and verify each menu header using fresh game captures."""
import subprocess,time,sys
from pathlib import Path
from PIL import Image,ImageChops,ImageStat,ImageFilter
root=Path('R:/MarioParty9VR/work');driver=root/'Drive-Mp9DebugInput.py'
references={
 'title':('alarm-scope-before-start.bmp',(300,365,685,405)),
 'main':('menu-fixed-reference.bmp',(260,42,700,95)),
 'solo':('solo-fixed-reference.bmp',(350,40,620,105)),
 'characters':('characters-fixed-reference.bmp',(290,42,700,88)),
 'setup':('setup-fixed-reference.bmp',(345,40,625,85)),
}
def send(buttons=0,x=0,y=0,capture=None):
    args=[sys.executable,str(driver),'--buttons',hex(buttons),'--x',str(x),'--y',str(y),'--press-ms','220']
    if capture:args+=['--capture','--output',str(capture)]
    subprocess.run(args,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    if buttons:time.sleep(.35)
def wait_for(name,timeout=60):
    ref,box=references[name];expected=Image.open(root/ref).convert('RGB').crop(box)
    deadline=time.monotonic()+timeout
    while True:
        path=root/'replay-current.bmp';send(capture=path)
        actual=Image.open(path).convert('RGB').crop(box)
        error=sum(ImageStat.Stat(ImageChops.difference(actual,expected)).mean)/3
        edge=lambda im:im.convert('L').filter(ImageFilter.FIND_EDGES).point(lambda v:255 if v>80 else 0)
        edge_error=ImageStat.Stat(ImageChops.difference(edge(actual),edge(expected))).mean[0]/255
        if error<9 or edge_error<.08:
            (root/f'replay-{name}-verified.bmp').write_bytes(path.read_bytes())
            print(f'Verified {name}; header difference {error:.2f}, edge mismatch {edge_error:.3f}',flush=True);return
        if time.monotonic()>deadline:raise TimeoutError(f'Expected {name}; latest capture {path}, difference {error:.2f}')
        time.sleep(1)
try:
    if '--from-characters' not in sys.argv:
        if '--from-main' not in sys.argv:
            wait_for('title')
            send(0xc00);time.sleep(2)
            send(0x800);time.sleep(1);send(0x800)
        wait_for('main')
        send(0x800,-.6,.3);wait_for('solo')
        time.sleep(2);send(capture=root/'replay-solo-visible.bmp')
        send(0x800,.73,.72)
    wait_for('characters')
    time.sleep(2);send(capture=root/'replay-characters-visible.bmp')
    if '--characters' in sys.argv:
        print('Verified character-selection screen; leaving game there for inspection.',flush=True)
        sys.exit(0)
    send(0x800,-.71,-.38);time.sleep(.5);send(0x800,.73,.72);wait_for('setup')
    send(0x800,.73,.72);time.sleep(5)
    send(capture=root/'replay-after-start-fresh.bmp')
    print('Verified route through Toad Road setup. Start sent; inspect fresh after-start capture before gameplay claims.',flush=True)
finally:
    subprocess.run([sys.executable,str(driver),'--disable'],check=True)
