#include "../include/FileSystem.hpp"
#include "../include/FileSystemManager.hpp"
#include "../include/LogHelper.hpp"
#include "../include/Constants.hpp"
#include "../include/FileNameHelper.hpp"
#include "../include/DataBuffer.hpp"
#include "../include/Utils.hpp"

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <lzo/lzo2a.h>
#include <zlib/zlib.h>
#include <xxhash.h>
#include <filesystem>

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/twofish.h>
#include <cryptopp/rc5.h>
#include <cryptopp/tea.h>
#include <cryptopp/blowfish.h>

namespace FileSystem
{
	// Read & Process
	bool CArchive::Create(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key, FSFileDict& dict)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_archiveMutex);

		if (!m_file.Create(filename, FILEMODE_READ, false))
			return false;

		FSArchiveHeader hdr{};
		if (!m_file.Read(&hdr, sizeof(hdr)))
			return false;

		if (hdr.magic != ARCHIVE_MAGIC)
			return false;

		if (hdr.version != ARCHIVE_VERSION)
			return false;

		m_file.SetPosition(hdr.fileInfoOffset);

		for (uint32_t i = 0; i != hdr.fileCount; ++i)
		{
			FSFileInformation e{};
			if (!m_file.Read(&e, sizeof(e)))
				return false;

			dict.insert(std::make_pair(e.filenameHash, std::make_pair(this, e)));
		}

		m_key.resize(ARCHIVE_KEY_LENGTH);
		memcpy(&m_key[0], key.data(), ARCHIVE_KEY_LENGTH);
		return true;
	}

	bool CArchive::Get(const CFileName& filename, const FSFileInformation& entry, CFile& fp)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_archiveMutex);

		// DEBUG_LOG(LL_SYS, "%lu %ls %u", filename.GetHash(), entry.filename, entry.rawSize);

		m_file.SetPosition(entry.offset);

		std::vector <uint8_t> rawdata(entry.cryptedSize);
		auto readsize = m_file.Read(&rawdata[0], entry.cryptedSize);
		if (readsize != entry.cryptedSize)
		{
			FS_LOG(LL_ERR, "File: %ls Read size mismatch: %u-%u", filename.GetPathW().c_str(), readsize, entry.cryptedSize);
			return false;
		}

		DEBUG_LOG(LL_TRACE,
			"Target file: %ls(%lu) Data: %p(%u) Hash: %lu Flags: %u",
			filename.GetPathW().c_str(), filename.GetHash(), rawdata.data(), entry.cryptedSize, entry.fileHash, entry.flags
		);

		// CRYPTION
		auto decrypted = DataBuffer(entry.rawSize);
		if (entry.flags & ARCHIVE_FLAG_AES256)
		{
			std::vector <uint8_t> outBuffer(entry.cryptedSize);
			try
			{
				CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption dec(&m_key[0], 32, &m_key[32]);
				dec.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(rawdata.data()), entry.cryptedSize);
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on decryption: %s", exception.what());
				abort();
			}
			decrypted = DataBuffer(&outBuffer[0], outBuffer.size());
		}
		else if (entry.flags & ARCHIVE_FLAG_TWOFISH)
		{
			std::vector <uint8_t> outBuffer(entry.cryptedSize);
			try
			{
				CryptoPP::CTR_Mode<CryptoPP::Twofish>::Decryption dec(&m_key[0], 32, &m_key[32]);
				dec.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(rawdata.data()), entry.cryptedSize);
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on decryption: %s", exception.what());
				abort();
			}
			decrypted = DataBuffer(&outBuffer[0], outBuffer.size());
		}
		else if (entry.flags & ARCHIVE_FLAG_RC5)
		{
			std::vector <uint8_t> outBuffer(entry.cryptedSize);
			try
			{
				CryptoPP::CTR_Mode<CryptoPP::RC5>::Decryption dec(&m_key[0], 32, &m_key[32]);
				dec.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(rawdata.data()), entry.cryptedSize);
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on decryption: %s", exception.what());
				abort();
			}
			decrypted = DataBuffer(&outBuffer[0], outBuffer.size());
		}
		else
		{
			decrypted = DataBuffer(rawdata.data(), entry.cryptedSize);
		}

		auto decompressed = DataBuffer(entry.rawSize);
		if (entry.flags & ARCHIVE_FLAG_LZ4)
		{
			std::vector <uint8_t> decompresseddata(entry.rawSize);

			auto decompressedsize = LZ4_decompress_safe(
				decrypted.get_data(), reinterpret_cast<char*>(&decompresseddata[0]),
				decrypted.get_size(), decompresseddata.size()
			);
			if (decompressedsize != entry.rawSize)
			{
				FS_LOG(LL_ERR, "Decomperssed size mismatch: %d-%u", decompressedsize, entry.rawSize);
				return false;
			}
			decompressed = DataBuffer(&decompresseddata[0], entry.rawSize);
		}
		else if (entry.flags & ARCHIVE_FLAG_LZO)
		{
			std::vector <uint8_t> decompresseddata(entry.rawSize);

			lzo_uint decompressedsize = 0;
			auto ret = lzo2a_decompress(
				reinterpret_cast<uint8_t*>(decrypted.get_data()), decrypted.get_size(),
				reinterpret_cast<uint8_t*>(&decompresseddata[0]), &decompressedsize,
				nullptr
			);
			if (LZO_E_OK != ret || decompressedsize != entry.rawSize)
			{
				FS_LOG(LL_ERR, "Decompress failed: ret %d, decompressedSize %u/%u",
					ret, decompressedsize, entry.compressedSize);
				return false;
			}
			decompressed = DataBuffer(&decompresseddata[0], entry.rawSize);
		}
		else if (entry.flags & ARCHIVE_FLAG_ZLIB)
		{
			std::vector <uint8_t> decompresseddata(entry.rawSize);

			uLongf decompressedsize = decompresseddata.size();
			auto ret = uncompress(
				reinterpret_cast<Bytef*>(&decompresseddata[0]), &decompressedsize,
				reinterpret_cast<const Bytef*>(decrypted.get_data()), decrypted.get_size()
			);

			if (Z_OK != ret || decompressedsize != entry.rawSize)
			{
				FS_LOG(LL_ERR, "Decompress failed: ret %d, decompressedSize %u/%u",
					ret, decompressedsize, entry.compressedSize);
				return false;
			}
			decompressed = DataBuffer(&decompresseddata[0], entry.rawSize);
		}
		else
		{
			decompressed = decrypted;
		}

#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		if (IsLayer2File(filename))
		{
			auto decrypterBuffer = FileSystemManager::Instance().DecryptLayer2Protection(
				reinterpret_cast<const uint8_t*>(decompressed.get_data()), decompressed.get_size()
			);
			if (decrypterBuffer.empty())
			{
				FS_LOG(LL_ERR, "Layer2 decryption fail");
				return false;
			}
			auto currenthash = XXH32(decrypterBuffer.data(), decrypterBuffer.size(), FILE_HASH_MAGIC);
			if (currenthash != entry.baseHash)
			{
				FS_LOG(LL_ERR, "Layer2 Hash mismatch: %lu-%lu", currenthash, entry.baseHash);
				return false;
			}
		}
		else
#endif
		{
			if (decompressed.get_size() != entry.rawSize)
			{
				FS_LOG(LL_ERR, "Size mismatch: %lu-%lu", decompressed.get_size(), entry.rawSize);
				return false;
			}

			auto currenthash = XXH32(decompressed.get_data(), decompressed.get_size(), FILE_HASH_MAGIC);
			if (currenthash != entry.baseHash)
			{
				FS_LOG(LL_ERR, "Hash mismatch: %lu-%lu", currenthash, entry.baseHash);
				return false;
			}
		}

		if (!fp.Assign(filename, decompressed.get_data(), decompressed.get_size(), true))
		{
			FS_LOG(LL_ERR, "File: %lu(%ls) assign failed", entry.baseHash, filename.GetPathW().c_str());
			return false;
		}
		return true;
	}

	// Write
	CArchiveMaker::CArchiveMaker()
	{
		m_key.resize(ARCHIVE_KEY_LENGTH);
	}

	bool CArchiveMaker::Create(const CFileName& filename, const std::array <uint8_t, ARCHIVE_KEY_LENGTH>& key)
	{
		if (std::filesystem::exists(filename.GetPathW()))
			std::filesystem::remove(filename.GetPathW()); // remove old archive, if already exist

		if (!m_file.Create(filename, FILEMODE_WRITE, false))
			return false;

		m_file.SetPosition(sizeof(FSArchiveHeader));

		m_layer2Type = FileSystemManager::Instance().GetLayer2Type();
		memcpy(&m_key[0], key.data(), ARCHIVE_KEY_LENGTH);
		return true;
	}

	bool CArchiveMaker::Add(const CFileName& archivedPath, const CFileName& diskPath, uint32_t flags)
	{
		CFile src;
		if (!src.Create(diskPath, FILEMODE_READ, true))
			return false;

		auto baseHash = XXH32(src.GetCurrentSeekPoint(), src.GetSize(), FILE_HASH_MAGIC);
		auto data_buffer = DataBuffer(src.GetCurrentSeekPoint(), src.GetSize());

		auto processed = false;
#ifdef ENABLE_LAYER2_FILE_ENCRYPTION
		if (IsLayer2File(archivedPath))
		{
			DEBUG_LOG(LL_SYS, "Layer2 supported file detected for secure: %ls", archivedPath.GetPathW().c_str());

			if (m_layer2Type == FILE_FLAG_BLOWFISH)
			{
				try
				{
					CryptoPP::CTR_Mode<CryptoPP::Blowfish>::Encryption enc(&DEFAULT_LAYER2_KEY[0], 16, &DEFAULT_LAYER2_KEY[16]);
					enc.ProcessData(
						reinterpret_cast<uint8_t*>(data_buffer.get_data()),
						reinterpret_cast<const uint8_t*>(src.GetCurrentSeekPoint()),
						src.GetSize()
					);
				}
				catch (const CryptoPP::Exception& exception)
				{
					FS_LOG(LL_CRI, "Caught exception on encryption: %s", exception.what());
					abort();
				}

				processed = true;
			}
			else if (m_layer2Type == FILE_FLAG_XTEA)
			{
				try
				{
					CryptoPP::CTR_Mode<CryptoPP::XTEA>::Encryption enc(&DEFAULT_LAYER2_KEY[0], 16, &DEFAULT_LAYER2_KEY[16]);
					enc.ProcessData(
						reinterpret_cast<uint8_t*>(data_buffer.get_data()),
						reinterpret_cast<const uint8_t*>(src.GetCurrentSeekPoint()),
						src.GetSize()
					);
				}
				catch (const CryptoPP::Exception& exception)
				{
					FS_LOG(LL_CRI, "Caught exception on encryption: %s", exception.what());
					abort();
				}

				processed = true;
			}
			else if (m_layer2Type != FILE_FLAG_RAW)
			{
				DEBUG_LOG(LL_CRI, "Unknown layer2 encryption type: %u", m_layer2Type);
				abort();				
			}
		}
#endif
		if (!processed)
			data_buffer = DataBuffer(src.GetCurrentSeekPoint(), src.GetSize());	

		FSFileInformation entry{};
		entry.filenameHash = archivedPath.GetHash();
		entry.baseHash = baseHash;
		entry.flags = flags;
		entry.offset = m_file.GetPosition();
		entry.rawSize = src.GetSize();
#ifdef SHOW_FILE_NAMES
		wcscpy_s(entry.filename, archivedPath.GetPathW().c_str());
#endif

		DEBUG_LOG(LL_SYS,
			"Target file: %ls(%lu) Data: %p(%u) Hash: %lu Flags: %u Offset: %llu",
			archivedPath.GetPathW().c_str(), archivedPath.GetHash(),
			data_buffer.get_data(), data_buffer.get_size(), entry.baseHash, flags,
			entry.offset
		);

		const auto bound = LZ4_compressBound(entry.rawSize);
		DataBuffer finalData(bound);

		int32_t compressedsize = 0;
		int32_t cryptedsize = 0;
		if (entry.flags & ARCHIVE_FLAG_LZ4)
		{
			std::vector <uint8_t> compressed(bound);

			compressedsize = LZ4_compress_HC(
				reinterpret_cast<const char*>(data_buffer.get_data()), reinterpret_cast<char*>(&compressed[0]),
				entry.rawSize, bound, LZ4HC_CLEVEL_MAX
			);
			if (compressedsize >= bound || compressedsize == 0)
			{
				FS_LOG(LL_ERR, "Compression fail! File: %ls Raw: %u Compressed: %u Capacity: %u",
					diskPath.GetPathW().c_str(), entry.rawSize, compressedsize, bound);

				finalData = data_buffer;
				flags &= ~ARCHIVE_FLAG_LZ4;
			}
			else
			{
				finalData = DataBuffer(&compressed[0], compressedsize);
			}
		}
		else if (flags & ARCHIVE_FLAG_LZO)
		{
			auto lzoWorkMem = FileSystemManager::Instance().GetLzoWorkMem();
			assert(lzoWorkMem);

			std::vector <uint8_t> compressed(finalData.get_size());

			lzo_uint outlen = 0;
			auto ret = lzo2a_999_compress(
				(const uint8_t*)data_buffer.get_data(), entry.rawSize,
				reinterpret_cast<uint8_t*>(&compressed[0]), &outlen,
				lzoWorkMem
			);

			if (LZO_E_OK != ret)
			{
				FS_LOG(LL_ERR, "Compression fail! File: %ls Raw: %u Compressed: %u Ret: %d",
					diskPath.GetPathW().c_str(), entry.rawSize, outlen, ret);

				finalData = data_buffer;
				flags &= ~ARCHIVE_FLAG_LZO;
			}
			else
			{
				compressedsize = outlen;
				finalData = DataBuffer(&compressed[0], compressedsize);
			}
		}
		else if (flags & ARCHIVE_FLAG_ZLIB)
		{
			finalData = DataBuffer(compressBound(entry.rawSize));
			std::vector <uint8_t> compressed(finalData.get_size());

			uLongf outlen = finalData.get_size();
			auto ret = compress(
				reinterpret_cast<Bytef*>(&compressed[0]), &outlen,
				reinterpret_cast<const Bytef*>(data_buffer.get_data()), entry.rawSize
			);

			if (Z_OK != ret)
			{
				FS_LOG(LL_ERR, "Compression fail! File: %ls Raw: %u Compressed: %u Ret: %d",
					diskPath.GetPathW().c_str(), entry.rawSize, outlen, ret);

				finalData = data_buffer;
				flags &= ~ARCHIVE_FLAG_ZLIB;
			}
			else
			{
				compressedsize = outlen;
				finalData = DataBuffer(&compressed[0], compressedsize);
			}
		}
		if (compressedsize == 0)
		{
			FS_LOG(LL_ERR, "Compression fail! Raw data moved to compressed buffer");

			if (flags & ARCHIVE_FLAG_LZ4)
				flags &= ~ARCHIVE_FLAG_LZ4;
			else if (flags & ARCHIVE_FLAG_LZO)
				flags &= ~ARCHIVE_FLAG_LZO;
			else if (flags & ARCHIVE_FLAG_ZLIB)
				flags &= ~ARCHIVE_FLAG_ZLIB;

			finalData = data_buffer;
		}

		DEBUG_LOG(LL_SYS, "Compression completed! Data: %p Size: %u - %u",
			finalData.get_data(), finalData.get_size(), compressedsize);
		entry.compressedSize = compressedsize;

		if (flags & ARCHIVE_FLAG_AES256)
		{
			std::vector <uint8_t> outBuffer(finalData.get_size());

			try
			{
				CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption enc(&m_key[0], 32, &m_key[32]);
				enc.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(finalData.get_data()), finalData.get_size());
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on encryption: %s", exception.what());
				abort();
			}

			finalData = DataBuffer(&outBuffer[0], outBuffer.size());
			cryptedsize = outBuffer.size();
		}
		else if (flags & ARCHIVE_FLAG_TWOFISH)
		{
			std::vector <uint8_t> outBuffer(finalData.get_size());

			try
			{
				CryptoPP::CTR_Mode<CryptoPP::Twofish>::Encryption enc(&m_key[0], 32, &m_key[32]);
				enc.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(finalData.get_data()), finalData.get_size());
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on encryption: %s", exception.what());
				abort();
			}

			finalData = DataBuffer(&outBuffer[0], outBuffer.size());
			cryptedsize = outBuffer.size();
		}
		else if (flags & ARCHIVE_FLAG_RC5)
		{
			std::vector <uint8_t> outBuffer(finalData.get_size());

			try
			{
				CryptoPP::CTR_Mode<CryptoPP::RC5>::Encryption enc(&m_key[0], 32, &m_key[32]);
				enc.ProcessData(&outBuffer[0], reinterpret_cast<const uint8_t*>(finalData.get_data()), finalData.get_size());
			}
			catch (const CryptoPP::Exception& exception)
			{
				FS_LOG(LL_CRI, "Caught exception on encryption: %s", exception.what());
				abort();
			}

			finalData = DataBuffer(&outBuffer[0], outBuffer.size());
			cryptedsize = outBuffer.size();
		}
		else
		{
			cryptedsize = finalData.get_size(); // copy of compressed size
		}

		entry.cryptedSize = cryptedsize;
		entry.fileHash = XXH32(finalData.get_data(), finalData.get_size(), FILE_HASH_MAGIC);
		DEBUG_LOG(LL_SYS, "Encryption completed! Data: %p Size: %u - %u Hash: %lu",
			finalData.get_data(), finalData.get_size(), cryptedsize, entry.fileHash);

		if (!m_file.Write(&finalData[0], finalData.get_size()))
		{
			FS_LOG(LL_ERR, "Target file: %ls can NOT writed to archive!", archivedPath.GetPathW().c_str());
			return false;
		}

		m_files.emplace_back(entry);
		return true;
	}

	bool CArchiveMaker::Save()
	{
		FSArchiveHeader hdr;
		hdr.magic = ARCHIVE_MAGIC;
		hdr.version = ARCHIVE_VERSION;
		hdr.fileInfoOffset = m_file.GetPosition();
		hdr.fileCount = m_files.size();

		for (const auto& f : m_files)
			m_file.Write(&f, sizeof(f));

		m_file.SetPosition(0);
		m_file.Write(&hdr, sizeof(hdr));
		m_file.Close();
		return true;
	}
};
