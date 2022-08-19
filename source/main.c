#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>

#define GBA_ROM_ADDR 0x08000000

// This works fine on real hardware with all the GBA games I have but most likely will not work in emulators. (It didn't in MelonDS)
int detectCartSize()
{
	char* c = (char*)GBA_ROM_ADDR;

	for (int mbits = 1; mbits < 256; mbits *= 2)
	{
		c = (char*) GBA_ROM_ADDR + ((1024 * 128) * mbits) + 1;

		if (c[0] == 0x00)
		{
			if (c[1] == 0x01 && c[3] == 0x02 && c[5] == 0x03 && c[7] == 0x04 && c[9] == 0x05 && c[11] == 0x06 && c[13] == 0x07 && c[15] == 0x08)
			{
				return mbits;
			}
		}
	}

	return 1;
}

int main(int argc, char* argv[])
{
	char gameID[7];
	char gameID_old[6];
	char gameName[13];
	char path[256];

	gameID[6] = '\0';
	gameName[12] = '\0';

	int cartMbits = 1;

	sysSetBusOwners(true, true);	// Allow the ARM9 to read from carts.
	consoleDemoInit();				// Get a simple console to print stuff to.

	cartMbits = detectCartSize();

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

		if (strncmp(gameID, gameID_old, 6) != 0)
		{
			memcpy(gameID_old, gameID, 6);
			cartMbits = detectCartSize();
		}

		printf("\x1b[1;1H ----------------------------");
		printf("\x1b[2;1H  Current Game Pak in SLOT-2");
		printf("\x1b[3;1H   %s (ID: %s)", gameName, gameID);
		printf("\x1b[4;1H ----------------------------");
		printf("\x1b[6;1HCart size: %03d Mbit (%05d KB)", cartMbits, 128 * cartMbits);
		printf("\x1b[8;0H  Use LEFT/RIGHT to adjust the\n cart size.  If you're not sure\n    use the 256 Mbit option.");
		printf("\x1b[31;1m\x1b[12;0HAutomatically detected cart size");
		printf("\x1b[13;0H Don't set a manual cart size\nunless you get a corrupted dump!\x1b[39m");
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
