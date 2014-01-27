/*
** Dingoo Native SIMulator registration
** NOTE if GetFileType(), etc. exists the .app file no longer works
*/

#include <stdlib.h>
#include <string.h>

/* file extension name */
int GetFileType(char* pname) {
	if(pname != NULL)
	{
		 /*
		 ** Emulator ROM extensions
		 ** (use "EXT|EXT|EXT" for several file-type associations, not more than five)
		 ** Must NOT include "."/period/dot/full-stop
		 ** Must be a maximum of 3 characters for each extension, e.g. I've tried "GBZ|GZ|GBCZ" (with "GBCZ") and it didn't work.
		 ** TODO check Vectrex emu as it uses VECX without problem (but only the one file extension)
		 */
		strcpy(pname, "GB|GBC|GBZ|GZ");
	}
	return 0;
}

/* to get default path */
int GetDefaultPath(char* path) {
	if(path != NULL)
		strcpy(path, "A:\\GAME\\");
	return 0;
}

/* module description, optional */
int GetModuleName(char* name, int code_page) {
	if((name != NULL) && (code_page == 0)) /* ansi */
		strcpy(name, "gnuboy_dingoo.sim");  /* emulator executable name, has to be a *.sim name */
	return 0;
}
