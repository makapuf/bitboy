# make tables

def mkmorton8(x):
	"make a morton number by interleaving zeros, ex : 0x11111111 -> 0x1010101010101010"
	z=0
	for i in range(8) :
	  z |= (x & 1 << i) << i ;
	return z

def rev8(i) : 
	'bit reverse byte i. also there is an instr for this on cortex'
	return (i * 0x0202020202 & 0x010884422010) % 1023;


print "uint16_t morton8={"
for i in range(256) : 
	if (i%16==0): print 
	print "0x%04x,"%mkmorton8(i),
print "\n};"

print 
print "uint8_t reversed8={"
for i in range(256) : 
	if (i%8==0): print
	print "0x%02x,"%rev8(i),
print '\n};'

