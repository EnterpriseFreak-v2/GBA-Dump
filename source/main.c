#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>

#define GBA_ROM_ADDR 0x08000000

int main(int argc, char* argv[])
{
	char gameID[7];
	char gameName[13];
	char path[256];

	gameID[6] = '\0';
	gameName[12] = '\0';

	int cartMbits = 1;

	sysSetBusOwners(true, true);	// Allow the ARM9 to read from carts.
	consoleDemoInit();				// Get a simple console to print stuff to.

	if (!fatInitDefault())
	{
		printf("Failed to mount SD card!");	
		for(;;)
			swiWaitForVBlank();
	}

	for(;;)
	{
		scanKeys();
		u32 keys = keysDown();

		memcpy(gameName, (void*)GBA_ROM_ADDR + 0xA0, 12);
		memcpy(gameID, (void*)GBA_ROM_ADDR + 0xAC, 6);

		printf("\x1b[1;1H ----------------------------");
		printf("\x1b[2;1H  Current Game Pak in SLOT-2");
		printf("\x1b[3;1H   %s (ID: %s)", gameName, gameID);
		printf("\x1b[4;1H ----------------------------");
		printf("\x1b[6;1HCart size: %03d Mbit (%05d KB)", cartMbits, 128 * cartMbits);
		printf("\x1b[8;0H  Use LEFT/RIGHT to adjust the\n cart size.  If you're not sure\n    use the 256 Mbit option.");
		printf("\x1b[18;0H Press START to  dump the game.");

		if (keys & KEY_LEFT && cartMbits > 1)
			cartMbits /= 2;

		if (keys & KEY_RIGHT && cartMbits < 256)
			cartMbits *= 2;

		if (keys & KEY_START)
		{
			consoleClear();
			printf("\x1b[12;0H         PLEASE WAIT...");
			sprintf(path, "fat:/%s (AGB-P-%s).gba", gameName, gameID);

			FILE* rom = fopen(path, "wb");

			for (int i = 0; i < cartMbits; i++)
			{
				fwrite((void*)GBA_ROM_ADDR + ((1024*128)*i), 1024*128, 1, rom);
				printf("\x1b[13;0H     %03d Mbit / %03d Mbit", i, cartMbits);
			}

			fclose(rom);
			consoleClear();
			printf("\x1b[12;0H         Dump complete.");

			for (int i = 0; i < 240; i++)
			{
				swiWaitForVBlank();
			}
		}

		swiWaitForVBlank();
	}

	return 0;
}
