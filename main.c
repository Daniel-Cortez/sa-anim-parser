/*==============================================================================
	Animations parser for GTA:SA
			by Daniel_Cortez
		www.pro-pawn.ru
================================================================================
This code is licensed under the terms of zlib/libpng license.

Copyright (c) 2014-2017 Stanislav Gromov (Daniel_Cortez)

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software. Permission is granted to anyone to use this software for
any purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:
	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in
		a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.
================================================================================
Special thanks to steve-m for .ifp animation format research:
	http://gtaforums.com/topic/205309-sa-charanimation-format/
==============================================================================*/


// C
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#if !defined _MAX_PATH
	#if defined PATH_MAX
		#define _MAX_PATH PATH_MAX
	#elseif defined _POSIX_PATH_MAX
		#define _MAX_PATH _POSIX_PATH_MAX
	#else
		#error _MAX_PATH is not defined
	#endif
#endif

// POSIX
#include <dirent.h>

// logprintf
#include "logprintf.h"


#define	APP_VERSION_MAJOR	1
#define	APP_VERSION_MINOR	5


#define	ANIM_LIB_NAME_SIZE	23+1
#define	ANIM_NAME_SIZE		23+1
#define	BONE_NAME_SIZE		23+1

typedef struct
{
	char signature[4];
	uint32_t filesize;
	char name[ANIM_LIB_NAME_SIZE];
	uint32_t num_anims;
}
anim_file_info_t;

typedef struct
{
	char name[ANIM_NAME_SIZE];
	uint32_t num_bones;
	uint32_t data_size;
	uint32_t unknown_000;
}
anim_info_t;

typedef struct
{
	char name[BONE_NAME_SIZE];
	uint32_t frame_type;
	uint32_t num_frames;
	uint32_t id;
}
anim_bone_info_t;

typedef struct
{
	uint16_t quat_x, quat_y, quat_z, quat_w;
	uint16_t frame_time;
}
anim_frame_info3_t;

typedef struct
{
	uint16_t quat_x, quat_y, quat_z, quat_w;
	uint16_t frame_time;
	uint16_t pos_x, pos_y, pos_z;
}
anim_frame_info4_t;


int main()
{
	if (0 == logprintf_init("anim_parser.txt"))
		return EXIT_FAILURE;
	logprintf("AnimParser v%d.%d\n", APP_VERSION_MAJOR, APP_VERSION_MINOR);
	logprintf("(c) 2014-2017, Daniel_Cortez\n\n");
	//
	DIR *dir;
	{
		char working_dir[_MAX_PATH];
		getcwd(working_dir, sizeof(working_dir));
		dir = opendir(working_dir);
	}
	if (dir == NULL)
	{
		logprintf("ERROR: Coludn't open the directory\n");
		return EXIT_FAILURE;
	}
	struct dirent *dir_entry;
	unsigned long num_files = 0;
	unsigned long num_anims = 0;
	while ((dir_entry = readdir(dir)) != NULL)
	{
		if (strstr(dir_entry->d_name, ".ifp")-dir_entry->d_name == dir_entry->d_namlen-4)
		{
			// open the file
			FILE* inp_file;
			inp_file = fopen(dir_entry->d_name, "rb");
			if (inp_file == NULL)
			{
				logprintf("ERROR: Couldn't open file \"%s\", skipping...\n", dir_entry->d_name);
				continue;
			}
			anim_file_info_t anim_file_info;
			anim_info_t anim_info;
			anim_bone_info_t bone_info;
			anim_frame_info4_t frame_info;
			// check the file size
			fseek(inp_file, 0, SEEK_END);
			long filesize = (unsigned int)ftell(inp_file);
			if (filesize < sizeof(anim_file_info))
			{
	incorrect_file:
				logprintf("ERROR: Incorrect file \"%s\", skipping...\n", dir_entry->d_name);
				fclose(inp_file);
				continue;
			}
			// read file info (signature, size, internal name and animations count)
			// and check if it was read successfully
			fseek(inp_file, 0, SEEK_SET);
			unsigned int elements_read = fread(&anim_file_info, sizeof(anim_file_info), 1, inp_file);
			if (elements_read != 1)
				goto incorrect_file;
			if (strncmp(anim_file_info.signature, "ANP3", 4) != 0)
				goto incorrect_file;
			// check the file size [2]
			long estimated_filesize	= (long)anim_file_info.filesize
									+ sizeof(anim_file_info.signature)
									+ sizeof(anim_file_info.filesize);
			if(filesize < estimated_filesize)
				logprintf("Warning: Invalid file size (got: %lu, real: %lu)\n", estimated_filesize, filesize);
			// print file information and its animation list
			logprintf("\nLibrary: %s, %u animations\n", anim_file_info.name, anim_file_info.num_anims);
			for (unsigned int i = 0; i < anim_file_info.num_anims; ++i)
			{
				elements_read = fread(&anim_info, sizeof(anim_info), 1, inp_file);
				if (elements_read != 1)
				{
					logprintf("ERROR: Couldn't read animation data %03d, skipping...\n", i);
					fclose(inp_file);
					return 0;
				}
				++num_anims;
				uint16_t time = 0;
				for (unsigned int j = 0; j < anim_info.num_bones; ++j)
				{
					int frame_info_size;
					fread(&bone_info, sizeof(bone_info), 1, inp_file);
					frame_info_size = (bone_info.frame_type == 3)
						? sizeof(anim_frame_info3_t) : sizeof(anim_frame_info4_t);
					for (unsigned int k = 0; k < bone_info.num_frames; ++k)
					{
						fread(&frame_info, frame_info_size, 1, inp_file);
					}
					if (frame_info.frame_time > time)
						time = frame_info.frame_time;
				}
				logprintf("\tAnimation: %s, %u frames\n", anim_info.name, (unsigned)time);
			}
			// close the file and increase the processed file counter
			fclose(inp_file);
			++num_files;
		}
	}
	closedir(dir);
	if (num_files == 0)
		logprintf("Couldn't find any .ifp file in current directory!\n");
	else
		logprintf("\nFound %lu animations in %lu files\n", num_anims, num_files);
	logprintf_quit();
	printf("Press any key to exit\n");
	getchar();
	return EXIT_SUCCESS;
}
