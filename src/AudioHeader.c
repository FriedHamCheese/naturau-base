#include "AudioHeader.h"

#include <stdio.h>

void ntrb_AudioHeader_print(const ntrb_AudioHeader header, FILE* const printstream){
	fprintf(printstream, "ntrb_AudioHeader contents from ntrb_AudioHeader_print():\n");
		
	fprintf(printstream, "AudioFormat: %lu\n", header.AudioFormat);
	fprintf(printstream, "SampleRate: %u\n", header.SampleRate);
	fprintf(printstream, "NumChannels: %hu\n", header.NumChannels);
	fprintf(printstream, "BitsPerSample: %hu\n", header.BitsPerSample);
	
	fprintf(printstream, "\n");	
	fflush(printstream);
}