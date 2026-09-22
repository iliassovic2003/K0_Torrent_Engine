#pragma once

# include <cstdio>

# define LOG_D(tag, fmt, ...) std::printf("[DEBUG] [%s] " fmt "\n", tag, ##__VA_ARGS__)
# define LOG_I(tag, fmt, ...) std::printf("[INFO]  [%s] " fmt "\n", tag, ##__VA_ARGS__)
# define LOG_W(tag, fmt, ...) std::printf("[WARN]  [%s] " fmt "\n", tag, ##__VA_ARGS__)
# define LOG_E(tag, fmt, ...) std::fprintf(stderr, "[ERROR] [%s] " fmt "\n", tag, ##__VA_ARGS__)