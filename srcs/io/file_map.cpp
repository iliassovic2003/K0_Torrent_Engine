#include "../../include/io/file_map.hpp"

void FileMap::add_file(const std::string& path, uint64_t length) {
    files_.push_back({path, length, total_size_});
    total_size_ += length;
}

std::vector<FileSlice> FileMap::map_block(uint64_t global_offset, uint64_t length) const {
    std::vector<FileSlice> slices;
    uint64_t bytes_left = length;
    uint64_t current_offset = global_offset;

    for (const auto& file : files_) {
        if (bytes_left == 0)
            break;

        uint64_t file_start = file.global_offset;
        uint64_t file_end = file.global_offset + file.length;

        if (current_offset >= file_start && current_offset < file_end) {
            uint64_t offset_in_file = current_offset - file_start;
            
            uint64_t bytes_in_this_file = std::min(bytes_left, file.length - offset_in_file);

            slices.push_back({
                file.path,
                offset_in_file,
                bytes_in_this_file
            });

            current_offset += bytes_in_this_file;
            bytes_left -= bytes_in_this_file;
        }
    }
    return slices;
}