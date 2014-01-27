#! /usr/bin/python
# simple object export for jumper game

# Those MUST have only one tileset from the given file.
# ---------------------------------------------
from xml.etree import ElementTree as ET
from PIL import Image
import sys

TILESIZE=8

if len(sys.argv)<3 : 
    print >>sys.stderr, "Error : usage mk_tileset outputfilename tile1.tmx tile2.tmx ... "

outputfile=sys.argv[1]
tile_files=sys.argv[2:]

file_c = open(outputfile+'.c','w')
file_h = open(outputfile+'.h','w')

def mk_bitbox_word(r,g,b,a) :
    "real bitbox words : A000BBBBGGGGRRRR"
    if a<128 : return 0
    return 1<<12 | int((7+15*b)//255)<<8|int((7+15*g)//255)<<4 | ((7+15*r)//255)


print >>file_h, "#include <stdint.h>"
print >>file_c, '#include "%s.h"'%outputfile

# tileset : unique, FIXED,  must be kept as is (no optimization is done)  
def mk_img(imgsrc) :
    img =Image.open(imgsrc).convert('RGBA')
    imgw,imgh = img.size
    tw,th = TILESIZE,TILESIZE
    print >>file_h, 'extern const uint16_t tile_data[%d*%d][%d*%d];'%(imgw/tw,imgh/th,tw,th)
    print >>file_c, 'const uint16_t tile_data[%d*%d][%d*%d] = {'%(imgw/tw,imgh/th,tw,th)
    for lig in range(imgh/th) :
        for col in range(imgw/tw) :
            tile=img.crop((col*tw,lig*th,col*tw+tw,lig*th+th))
            data=[mk_bitbox_word(r,g,b,a) for r,g,b,a in tile.getdata()]
            print >>file_c, '{ // tile %d'%(col+lig*(imgw/tw))
            for i in range(th) : 
                print >>file_c, '    '+','.join("0x%04x"%x for x in data[i*tw:i*tw+tw])+','
            print >>file_c,'},'
    print >>file_c, '};\n'


# ------ output layers in all files 
for f in tile_files : 
    root=ET.parse(f).getroot()
    for im in root.findall('tileset/image') : 
        imgsrc=im.get('source')
        mk_img(imgsrc)

    # check only one tileset
    ts_l=root.findall('tileset')
    assert len(ts_l)==1
    ts=ts_l[0]
    assert int(ts.get('firstgid'))==1
    assert int(ts.get('tilewidth'))==int(ts.get('tileheight'))==TILESIZE
    

    for layer in root.findall('layer') :
        assert int(root.get('tilewidth'))==TILESIZE
        data_w = int(layer.get('width'))
        data_h = int(layer.get('height'))
        if layer.get('name').startswith('__') : continue

        print >>file_h, "#define tilemap_%s_w %d"%(layer.get('name'),data_w)
        print >>file_h, "#define tilemap_%s_h %d"%(layer.get('name'),data_h)
        layerdata = layer.find('data')
        assert layerdata.get('encoding')=='csv','must decode CSV - Edition/Preferences/General '
        basedata = [int(x.strip()) for x in layerdata.text.replace('\n','').split(',')]

        # now write data
        print >>file_h,"extern const uint8_t tilemap_%s[%d];"%(layer.get('name'), len(basedata))
        print >>file_c,"const uint8_t tilemap_%s[%d] = {"%(layer.get('name'), len(basedata))
        for i in range(0,len(basedata),data_w) : 
            print >>file_c, '    '+','.join('0x%04x'%(basedata[i+j]-1) for j in range(data_w))+',' # 1-based->0-based !
        print >>file_c, '};'

    for objectgroup in root.findall('objectgroup') : 
        n=objectgroup.get('name')
        pos = [(int(o.get('x')),int(o.get('y'))) for o in objectgroup.findall('object')]

        print >>file_h,"#define nb_%s %d"%(n,len(pos))
        print >>file_h,"extern const uint16_t %s_pos[nb_%s][2];"%(n,n)
        print >>file_c,"const uint16_t %s_pos[nb_%s][2] = {"%(n,n)
        for c in sorted(pos,key=lambda o:o[1],reverse=True) : 
            print >>file_c,"    {%d,%d},"%c
        print >>file_c,"};"
    print >>file_h
    print >>file_c
