#include <stdint.h>
// defs for second sector = EEPROM
#define MAX_RECORDS 4096
#define FLASH_PAGE 0x08004000 
#define FLASH_SECTOR 1 

// erase block 16k tERASE16KB = 250ms typ si 32bits
// block rewrite ram -> flash : 16us / word soit pour 4000 words : 64ms 

typedef enum { E_OK, E_NOSPCn, E_NOTFOUND = -1} RES;

int nvrecord_used();
int nvrecord_read(uint16_t id); // 0-0xffff value or -1 if not found
RES nvrecord_write(uint16_t id, uint16_t value); // OK / NOSPC

/*defrag : 
 - ds le second stage bootloader (et donc run from ram) 
 - ds le first stage bootloader 
 	- mais stalls... cela dit serialisé dc OK - sinon copy to stack !
*/
void nvrecord_defrag(void *tmp_data); // tempo data space of 4*MAX_RECORDS (typ 16k).

// beware writing to flash data running from flash !