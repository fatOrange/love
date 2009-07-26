/**
* Copyright (c) 2006-2009 LOVE Development Team
* 
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

#ifndef LOVE_FILESYSTEM_PHYSFS_FILE_DATA_H
#define LOVE_FILESYSTEM_PHYSFS_FILE_DATA_H

// LOVE
#include <filesystem/FileData.h>

namespace love
{	
namespace filesystem
{
namespace physfs
{
	class FileData : public love::filesystem::FileData
	{
	private:

		// The actual data.
		char * data;

		// Size of the data.
		int size;

		// The filename used for error purposes.
		std::string filename;

		// The extension (without dot). Used to identify file type.
		std::string extension;

	public:

		FileData(int size, const std::string & filename);

		virtual ~FileData();

		// Implements Data.
		void * getData() const;
		int getSize() const;

		const std::string & getFilename() const;
		const std::string & getExtension() const;

	}; // FileData

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILE_DATA_H