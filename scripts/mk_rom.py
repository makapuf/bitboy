#!/usr/bin/python
'''
make an object from a list of ROMs, with : name, custom palette ; cut it for loader also
'''
# TODO : compress par map

ROW=32
import sys
names = []

print "// embedded gb roms"
print "#include <stdint.h>"

totsize = 0

for nb, fn in enumerate(sys.argv[1:]) : 
	data=open(fn).read() # all in mem
	
	# remove end of file if enough are the same # XXX do it by 16k rfontiers !
	if all(data[x]==data[-1] for x in range(-16,-1)) : 
		print '// striped from',len(data),'to',
		data=data.rstrip(data[-1])
		print len(data)
	else : 
		print '// size :',len(data)

	totsize += len(data)
	name = data[0x134:0x134+16].upper()
	
	for x in 15,14 : 
		if (ord(name[x]) & 0x80) : name=name[:-1]
	names.append(name.strip('\0'))

	if ord(data[0x0143]) in (0x80, 0xc0) : 
		print "// WARNING : This is a GBC ROM ! "

	print '// file :',fn
	print '// name :',names[-1]
	print 'const uint8_t ROM_%d[%d] = {'%(nb,len(data))
	
	for i in range(len(data)/ROW) : 
		print '    '+''.join('0x%02x,'%ord(x) for x in data[i*ROW:i*ROW+ROW] );

	print '};'
print '// total data : %d'%totsize
# icon ?



# custom palettes (from http://tcrf.net/CGB_Bootstrap_ROM)
palettes = {
	'SUPER MARIOLAND' : dict (
		BG=  (0xB5B5FF,0xFFFF94,0xAD5A42,0x000000),
		OBJ0=(0x000000,0xFFFFFF,0xFF8484,0x943A3A),
		OBJ1=(0x000000,0xFFFFFF,0xFF8484,0x943A3A)
		),
	'QIX' : dict (
		BG=(	0xFFFFFF,	0xFFFF00, 	0xFF0000, 	0x000000),
		OBJ0=(	0xFFFFFF,	0xFFFF00, 	0xFF0000, 	0x000000),
		OBJ1=(	0xFFFFFF,	0x5ABDFF, 	0xFF0000, 	0x0000FF), 
	),
	'DR.MARIO' : dict(
		BG=(	0xFFFFFF, 	0x63A5FF, 	0x0000FF, 	0x000000),
		OBJ0=(	0xFFFFFF, 	0x63A5FF, 	0x0000FF, 	0x000000),
		OBJ1=(	0xFFFFFF, 	0xFF8484, 	0x943A3A, 	0x000000), 
		)
}

p = (0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C)
defp = dict(BG=p,OBJ0=p,OBJ1=p)

print "const uint32_t game_palettes[%s][3][4] = {"%len(names)
for n in names : 
	p = palettes.get(n,defp)
	print "    { // %s - %s"%(n,"std palette" if p == defp else "custom palette")
	for x in 'BG','OBJ0','OBJ1' :
		print "        {%s},"%(','.join('0x%08X'%x for x in p[x]))
	print "    },"
print "\n};"

print "const char * const game_names[%d] = {"%len(names),",".join('\n    "%s"'%s for s in names),"\n};"
print "const uint8_t * const game_roms[16] = {"
for nb in range(16) : 
	if nb<len(names) : 
		print '  ROM_%s,'%nb
	else : 
		print '    0,'
print '};'
#extern uint16_t *game_icons; // 16 max