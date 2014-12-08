#include "libc.h"

int main(int argc, char** argv) {
	if(argc < 2){
		puts(argv[0]); puts(": no file specified\n");
		return -1;
	}
	int fd = open(argv[1]);
	if (fd < 0) {
		puts(argv[0]); puts(": ");
		puts(argv[1]); puts(": No such file or directory\n");
		return 0;
	}

	int chunkID = 0;
	readFully(fd,&chunkID,4);

	if (chunkID != 0x46464952) {
		puts("error: not a RIFF file\n");
		return 0;
	}
	int chunkSize = 0;
	readFully(fd, &chunkSize, 4);

	int waveID = 0;
	readFully(fd, &waveID, 4);
	if (waveID != 0x45564157) {
		puts("error: not WAVE file\n");
		return 0;
	}

	/* SubChunk 1: fmt */

	int subchunkID = 0;
	// read subchunk 1 ID
	readFully(fd, &subchunkID, 4);
	if (subchunkID != 0x20746d66) {
		puts("first subchunk not fmt\n");
	}
	int buff = 0;
	// read subchunk1 size
	readFully(fd, &buff, 4);
	int subchunkSize = buff;
	// read format tag
	buff = 0;
	readFully(fd, &buff, 2);
	puts("format: "); putdec(buff); puts("\n");
	// read num channels
	buff = 0;
	readFully(fd, &buff, 2);
	// read sample rate
	readFully(fd, &buff, 4);
	// read byte rate
	readFully(fd, &buff, 4);
	// read block align
	buff = 0;
	readFully(fd, &buff, 2);
	// read bits per sample
	buff = 0;
	readFully(fd, &buff, 2);

	if (subchunkSize > 16) {
		// read size of the extension
		readFully(fd, &buff, 2);
	}
	if (subchunkSize > 18) {
		// read number of valid bits per sample
		readFully(fd, &buff, 2);
		// read channel position mask
		readFully(fd, &buff, 4);
		char subFormat[16];
		readFully(fd, subFormat, 16);
		// for some reason this was needed to even it out?
		readFully(fd, &buff, 2);
	}

	/* SubChunk 2 data */

	buff = 0;
	// read subchunk 2 ID
	readFully(fd, &subchunkID, 4);
	if (subchunkID != 0x61746164) {
		puts("second subchunk not data\n");
		puthex(subchunkID);
		puts("\n");
	}
	// read size of data chunk
	readFully(fd, &buff, 4);
	subchunkSize = buff;

	return 0;
}
