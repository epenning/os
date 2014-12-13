#include "libc.h"
#include "stdint.h"

#pragma pack(push)
#pragma pack(2)	
struct {
	int16_t format;
	int16_t numChannels;
	int32_t sampleRate;
	int32_t byteRate;
	int16_t blockAlign;
	int16_t bitsPerSample;
} formatData;
#pragma pack(pop)

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
	int subchunkSize = 0;
	// read subchunk1 size
	readFully(fd, &subchunkSize, 4);
	puts("format chunk size: "); putdec(subchunkSize); puts("\n");

	readFully(fd, &formatData, 16);
	if(subchunkSize > 16){
		char temp[subchunkSize];
		readFully(fd, temp, subchunkSize - 16);
	}
	puts("format: 0x"); puthex(formatData.format); puts("\n");
	puts("numChannels: "); putdec(formatData.numChannels); puts("\n");
	puts("sampleRate: "); putdec(formatData.sampleRate); puts("\n");
	puts("byteRate: "); putdec(formatData.byteRate); puts("\n");
	puts("blockAlign: "); putdec(formatData.blockAlign); puts("\n");
	puts("bitsPerSample: "); putdec(formatData.bitsPerSample); puts("\n");

	// stuff other than 8-bit PCM not implemented yet

	int buff = 0;

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
	readFully(fd, &subchunkSize, 4);
	puts("data chunk size: "); putdec(subchunkSize); puts("\n");
	unsigned char byte = 0;
	int bytestoplay = subchunkSize;
	// play the specified number of bytes
	if (argc > 2) {
		bytestoplay = atoi(argv[2]);
		if (bytestoplay > subchunkSize)
			bytestoplay = subchunkSize;
	}
	for(int i = 0; i < bytestoplay; i++){
		//TODO: control sample rate

		//play a sample
		//note that this is hard-coded for 8-bit mono PCM
		readFully(fd, &byte, 1);
		puts("playing "); puthex(byte); puts("\n");
		play(byte);
		wait(formatData.sampleRate);
	}

	return 0;
}
