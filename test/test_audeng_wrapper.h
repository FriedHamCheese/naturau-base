/*
Copyright 2024 Pawikan Boonnaum

This file is part of naturau-base/ntrb.

naturau-base/ntrb is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

naturau-base/ntrb is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with naturau-base/ntrb. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef test_ntrb_audeng_wrapper_h
#define test_ntrb_audeng_wrapper_h

#include "audeng_wrapper.h"

#include <assert.h>

static void test_ntrb_frames_for_msec(){	
	assert(ntrb_frames_for_msec(96000) == 96.0f);
}

static void test_suite_ntrb_audeng_wrapper(){
	test_ntrb_frames_for_msec();
}

#endif