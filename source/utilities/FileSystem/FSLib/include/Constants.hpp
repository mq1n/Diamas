#pragma once
#include <cstdint>
#include <unordered_map>
#include <list>
#include <array>

#define FS_CREATEMAGIC(b0, b1, b2, b3) \
	(uint32_t(uint8_t(b0)) | (uint32_t(uint8_t(b1)) << 8) | \
	(uint32_t(uint8_t(b2)) << 16) | (uint32_t(uint8_t(b3)) << 24))

#define SHOW_FILE_NAMES
#define ENABLE_LAYER2_FILE_ENCRYPTION

namespace FileSystem
{
	static constexpr auto ARCHIVE_VERSION		= 5U;
	static constexpr auto ARCHIVE_KEY_LENGTH	= 64;
	static constexpr auto LAYER2_KEY_LENGTH		= 32;
	static constexpr auto ARCHIVE_MAGIC 		= FS_CREATEMAGIC('D', 'I', 'A', 'M');

	static constexpr auto FILE_HASH_MAGIC  = 3889535852L;
	static constexpr auto FILE_NAME_MAGIC  = 3586454764L;

	static const auto ENCRYPTED_RESOURCES_TYPES = std::vector <std::wstring>
	{
		L"gr2",
		L"sub"
	};

	static const auto DEFAULT_ARCHIVE_KEY = std::array <uint8_t, ARCHIVE_KEY_LENGTH> // TODO: XOR
	{
		// Key: 0 - 32
		0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,

		// IV: 32 - 64
		0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4
	};
	static const auto DEFAULT_LAYER2_KEY = std::array <uint8_t, LAYER2_KEY_LENGTH> // TODO: XOR
	{
		// Key: 0 - 16
		0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9,

		// IV: 16 - 32
		0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9, 0x6, 0x9
	};

	enum EArchiveFlags
	{
		ARCHIVE_FLAG_RAW  		= 1 << 0,
		ARCHIVE_FLAG_LZ4  		= 1 << 1,
		ARCHIVE_FLAG_LZO  		= 1 << 2,
		ARCHIVE_FLAG_ZLIB		= 1 << 3,
		ARCHIVE_FLAG_AES256		= 1 << 4,
		ARCHIVE_FLAG_TWOFISH 	= 1 << 5,
		ARCHIVE_FLAG_RC5 		= 1 << 6,
		ARCHIVE_FLAG_MAX		= ARCHIVE_FLAG_RC5 | ARCHIVE_FLAG_ZLIB
	};

	enum EFileFlags
	{
		FILE_FLAG_RAW  		= 1 << 0,
		FILE_FLAG_BLOWFISH 	= 1 << 1,
		FILE_FLAG_XTEA  	= 1 << 2
	};	

	enum EFileType : uint8_t
	{
		FILE_TYPE_NONE,
		FILE_TYPE_OUTPUT, // READ | WRITE (disk)
		FILE_TYPE_INPUT,  // WRITE ONLY (disk)
		FILE_TYPE_MAPPED, // READ ONLY (mapped)
		FILE_TYPE_MEMORY, // READ ONLY (memory)
		FILE_TYPE_MAX
	};

	enum EFileMode : uint8_t
	{
		FILEMODE_READ = (1 << 0),
		FILEMODE_WRITE = (1 << 1)
	};

	enum EProcessTypes : uint8_t
	{
		PROCESSTYPE_NONE,
		PROCESSTYPE_ENCRYPT,
		PROCESSTYPE_DECRPYT
	};

#pragma pack(push, 4)
	struct FSFileInformation // File header
	{
		// XH32 hash of the "normalized" filename
		uint32_t filenameHash; // TODO: x64 hash

		// XXH32 hash of the file's processed(runtime enc.[res./py.]) hash
		uint32_t fileHash; // TODO: x64 hash

		// XH32 hash of the file's raw hash
		uint32_t baseHash; // TODO: x64 hash

		// see EFileFlags
		uint32_t flags;

		// Offset of the file - relative to the archive's beginning.
		uint64_t offset;

		// Size of the unpacked file
		uint32_t rawSize;

		// Size of the file - on disk (compressed)
		uint32_t compressedSize;

		// Size of the file - on disk (compressed + crypted) / final size
		uint32_t cryptedSize;

#ifdef SHOW_FILE_NAMES
		wchar_t filename[300];
#endif
	};

	struct FSArchiveHeader // Archive header
	{
//		uint8_t __padding1[4];
		uint32_t magic;
		uint32_t version;
//		uint8_t __padding2[1];
		uint32_t fileInfoOffset;
//		uint8_t __padding3[3];
		uint32_t fileCount;
	};
#pragma pack(pop)
}
