#include "AudioHeader.h"

#include <stdio.h>

void print_ntrb_AudioHeader(const ntrb_AudioHeader header, FILE* const printstream){
	fprintf(printstream, "ntrb_AudioHeader contents from print_ntrb_AudioHeader():\n");
		
	fprintf(printstream, "AudioFormat: %lu\n", header.AudioFormat);
	fprintf(printstream, "SampleRate: %u\n", header.SampleRate);
	fprintf(printstream, "NumChannels: %hu\n", header.NumChannels);
	fprintf(printstream, "BitsPerSample: %hu\n", header.BitsPerSample);
	
	fprintf(printstream, "\n");	
	fflush(printstream);
}