
/*
EEprom in flash

16ko = 4096 slots (bien)
1 slot = ID 16b data 16b id = block ID
+ c'est bas (grand id) + c nouveau
*/

#include "eeprom.h"
#include "stm32f4xx.h"

// flash record
typedef struct {
	uint16_t id, value;
} NVRecord;


NVRecord *flash_data = (NVRecord*)(FLASH_PAGE);


inline void flash_wait()
{
	while (FLASH->CR & FLASH_BSY);
}

void flash_optkey()
{
	// unlock optkey sequence 
	FLASH->OPTKEYR=0x08192A3B; // OPTKEY1
	FLASH->OPTKEYR=0x4C5D6E7F; // OPTKEY2
}

void flash_key()
{
	// unlock sequence 
	FLASH->KEYR=0x45670123; // KEY1
	FLASH->KEYR=0xCDEF89AB; // KEY2
}


void flash_lock()
{
	FLASH->CR|= FLASH_LOCK;
	FLASH->OPTCR|= FLASH_LOCK;
}


void flash_erase()
{
	flash_wait();
	flash_key();

	FLASH->CR |= FLASH_SER;
	FLASH->CR |= FLASH_SNB*FLASH_SECTOR;
	FLASH->CR |= FLASH_STRT;

	flash_lock();
	flash_wait();
}


void flash_rw()
{
	flash_wait();

	flash_key();
	// set width = 32bits
	FLASH->CR &= ~FLASH_PSIZE*3; // clear 2 bits
	FLASH->CR |= FLASH_PSIZE*2; // set 10 = x32
	FLASH->CR |= FLASH_STRT;

	flash_optkey();	
	FLASH->OPTCR |= nWRP*FLASH_SECTOR; // allow sector 2 for writing 
	FLASH->OPTCR |= OPTSTRT; // start 

	flash_lock();

	flash_wait();
}


void flash_ro()
{
	flash_wait();

	// prevent sector from writing  (clear nonwrite protect)
	flash_optkey();
	FLASH->OPTCR &= nWRP*FLASH_SECTOR; 
	FLASH->OPTCR |= OPTSTRT; // start 
	
	flash_lock();
	flash_wait();
}


int nvrecord_read(uint16_t id) // -1 if not found
{
	// partir de la fin, remonter tt que ID pas trouve
	for (int i=MAX_RECORDS ;i>=0;i--)
	{
		if (flash_data[i].id==id) 
			return flash_data[i].value
	}
	return E_NOTFOUND;
}

int nvrecord_used()
{
	int i=MAX_RECORDS;
	while (i>=0)
	{	
		if (flash_data[i].id != 0xffff) 
			return i;
		i--;
	}
	return 0; // all free
}

int nvrecord_write(uint16_t id, uint16_t value)
{
	// full ?
	if (flash_data[MAX_RECORDS-1].id != 0xffff) return E_NOSPC;

	// Set flash writable

	int i=MAX_RECORDS-2;
	while (i>=0 && flash_data[i+1] == 0xffff ) i--; 
	// case empty : i=-1 here
	
	flash_rw();
	flash_data[i+1] = (NVRecord){.id = id, .value=value}; // WRITE to flash
	flash_ro();

	return E_OK;
}


void nvrecord_defrag(NVRecord *tmp_data) 
/*
 * NVRecord : tmp space able to store MAX_RECORDS data
 */
{
	int dst_i=0; dst_nb=0;
	uint16_t tmp; // tmp record

	// defrag to ram
	for (int src_i=MAX_RECORDS;src_i>=0;src_i--)
	{
		tmp = flash_data[src_i];
		if ( tmp.id == 0xffff) continue; // skip empty 
		// test if this id is already known, if so skip
		for (dst_i=0;dst_i<dst_nb;dst_i++)
			if (tmp_data[dst_i].id==tmp.id) 
				break

		if (dst_i==dst_nb) // not found : add
			tmp_data[dst_nb++] = tmp;
	}

	// erase block
	flash_erase(FLASh_BLOCK);

	// write block
	flash_rw();
	for (dst_i=0;dst_i<dst_nb;dst_i++)
		flash_data[i]=tmp_data[dst_i];
	flash_ro();

}
/*
run from ram : 
https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Flat.aspx?RootFolder=/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/Running%20a%20particular%20function%20from%20RAM&FolderCTID=0x01200200770978C69A1141439FE559EB459D7580009C4E14902C3CDE46A77F0FFD06506F5B&currentviews=238


encode to flash => 0b1111111111111111 peut etre ecrit 16 fois en decrementant sans erase (monotone) . quelles données ?  

*/
