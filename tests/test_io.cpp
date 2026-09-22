#include "../include/io/file_map.hpp"
#include <iostream>
#include <cassert>

static void test_file_map_boundaries() {
    FileMap map;
    
    map.add_file("episode1.mp4", 100);
    map.add_file("episode2.mp4", 200);

    assert(map.total_size() == 300);

    auto slices1 = map.map_block(10, 50);
    assert(slices1.size() == 1);
    assert(slices1[0].file_path == "episode1.mp4");
    assert(slices1[0].file_offset == 10);
    assert(slices1[0].size == 50);

    auto slices2 = map.map_block(90, 20);
    assert(slices2.size() == 2);
    
    assert(slices2[0].file_path == "episode1.mp4");
    assert(slices2[0].file_offset == 90);
    assert(slices2[0].size == 10);
    
    assert(slices2[1].file_path == "episode2.mp4");
    assert(slices2[1].file_offset == 0);
    assert(slices2[1].size == 10);

    std::cout << "  [PASS] FileMap boundary crossing calculation\n";
}

int main() {
    std::cout << "=== IO Layer tests ===\n";
    test_file_map_boundaries();
    std::cout << "All IO tests passed.\n";
    return 0;
}