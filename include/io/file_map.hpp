#pragma once

# include <string>
# include <vector>
# include <cstdint>
# include <algorithm>

struct FileEntry {
    std::string     path;            // e.g., "Downloads/Ubuntu/ubuntu.iso"
    uint64_t        length;          // Size of this specific file
    uint64_t        global_offset;   // Where this file starts in the giant BitTorrent byte stream
};

struct FileSlice {
    std::string     file_path;
    uint64_t        file_offset;     // Where to start writing in THIS file
    uint64_t        size;            // How many bytes to write to THIS file
};

class FileMap {
public:
    FileMap() = default;

    void                    add_file(const std::string& path, uint64_t length);

    std::vector<FileSlice>  map_block(uint64_t global_offset, uint64_t length) const;

    uint64_t                total_size() const {
        return total_size_;
    }

private:
    std::vector<FileEntry>  files_;
    uint64_t                total_size_ = 0;
};