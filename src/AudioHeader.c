/**
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

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