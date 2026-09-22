#include "../../include/io/disk_manager.hpp"
#include "../../include/common/error.hpp"
#include "../../include/common/logger.hpp"

namespace                       fs = std::filesystem;

DiskManager::DiskManager(const TorrentFile& torrent, const std::string& download_dir)
    : download_dir_(download_dir), piece_length_(torrent.piece_length)
{
    if (!fs::exists(download_dir_))
        fs::create_directories(download_dir_);

    if (torrent.is_multi_file()) {
        for (const auto& file : torrent.files) {
            std::string joined_path = "";
            for (size_t i = 0; i < file.path.size(); ++i) {
                joined_path += file.path[i];
                if (i < file.path.size() - 1)
                    joined_path += "/";
            }
            file_map_.add_file(joined_path, file.length);
            LOG_I("K0_Disk", "Registered file: %s (%lld bytes)", joined_path.c_str(), (long long)file.length);
        }
    } else {
        file_map_.add_file(torrent.name, torrent.length);
        LOG_I("K0_Disk", "Single-file torrent. Registered: %s (%lld bytes)", torrent.name.c_str(), (long long)torrent.length);
    }

    LOG_I("K0_Disk", "Download dir: %s", fs::absolute(download_dir_).c_str());
}

DiskManager::~DiskManager() {
    open_files_.clear();
}

void DiskManager::ensure_directory_exists(const std::string& file_path) {
    fs::path path(file_path);
    if (path.has_parent_path())
        fs::create_directories(path.parent_path());
}

std::shared_ptr<std::fstream> DiskManager::get_or_open_file(const std::string& path) {
    auto it = open_files_.find(path);
    if (it != open_files_.end())
        return it->second;

    fs::path full_path = fs::path(download_dir_) / path;

    LOG_I("K0_Disk", "Opening file: %s", fs::absolute(full_path).c_str());

    if (!fs::exists(full_path)) {
        ensure_directory_exists(full_path.string());
        std::ofstream create_file(full_path, std::ios::binary | std::ios::out);
        if (!create_file)
            LOG_E("K0_Disk", "FAILED to create file: %s", fs::absolute(full_path).c_str());
        else
            LOG_I("K0_Disk", "Created file: %s", fs::absolute(full_path).c_str());
    }

    auto stream = std::make_shared<std::fstream>(
        full_path, std::ios::in | std::ios::out | std::ios::binary
    );

    if (!stream->is_open())
        LOG_E("K0_Disk", "FAILED to open file: %s", fs::absolute(full_path).c_str());
    else
        LOG_I("K0_Disk", "Opened file successfully: %s", fs::absolute(full_path).c_str());

    open_files_[path] = stream;
    return stream;
}

void DiskManager::write_block(uint32_t piece_index, uint32_t block_offset, const std::vector<uint8_t>& data) {
    uint64_t global_offset = (static_cast<uint64_t>(piece_index) * piece_length_) + block_offset;

    auto slices = file_map_.map_block(global_offset, data.size());

    LOG_I("K0_Disk", "write_block(piece=%u, offset=%u, size=%zu) -> global=%llu, slices=%zu",
          piece_index, block_offset, data.size(),
          (unsigned long long)global_offset, slices.size());

    if (slices.empty()) {
        LOG_E("K0_Disk", "FATAL: map_block returned 0 slices for piece=%u offset=%u! File map has no files registered.",
              piece_index, block_offset);
        return;
    }

    uint64_t data_offset = 0;
    for (const auto& slice : slices) {
        LOG_I("K0_Disk", "  -> writing %llu bytes to '%s' at file_offset=%llu",
              (unsigned long long)slice.size, slice.file_path.c_str(),
              (unsigned long long)slice.file_offset);

        auto stream = get_or_open_file(slice.file_path);
        stream->seekp(slice.file_offset);
        stream->write(reinterpret_cast<const char*>(data.data() + data_offset), slice.size);

        if (stream->fail())
            LOG_E("K0_Disk", "  -> WRITE FAILED to '%s'", slice.file_path.c_str());
        else
            LOG_I("K0_Disk", "  -> write OK");

        data_offset += slice.size;
    }
}

std::vector<uint8_t> DiskManager::read_block(uint32_t piece_index, uint32_t block_offset, uint32_t length) {
    uint64_t global_offset = (static_cast<uint64_t>(piece_index) * piece_length_) + block_offset;
    auto slices = file_map_.map_block(global_offset, length);

    std::vector<uint8_t> data(length);
    uint64_t data_offset = 0;

    for (const auto& slice : slices) {
        auto stream = get_or_open_file(slice.file_path);

        stream->seekg(slice.file_offset);
        stream->read(reinterpret_cast<char*>(data.data() + data_offset), slice.size);

        if (stream->fail())
            throw std::runtime_error("Failed to read from file: " + slice.file_path);

        data_offset += slice.size;
    }

    return data;
}