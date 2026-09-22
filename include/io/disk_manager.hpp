#pragma once

# include "../torrent/metainfo.hpp"
# include "file_map.hpp"

# include <vector>
# include <string>
# include <fstream>
# include <filesystem>
# include <iostream>
# include <unordered_map>

class DiskManager {
public:
    DiskManager(const TorrentFile& TorrentFile, const std::string& download_dir);
    ~DiskManager();

    void                    write_block(uint32_t piece_index, uint32_t block_offset, const std::vector<uint8_t>& data);
    std::vector<uint8_t>    read_block(uint32_t piece_index, uint32_t block_offset, uint32_t length);

private:
    FileMap         file_map_;
    std::string     download_dir_;
    uint64_t        piece_length_;

    std::unordered_map<std::string, std::shared_ptr<std::fstream>> open_files_;

    std::shared_ptr<std::fstream>   get_or_open_file(const std::string& path);
    void                            ensure_directory_exists(const std::string& file_path);
};