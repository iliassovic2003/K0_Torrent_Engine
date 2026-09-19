## v0.5
    corrected the test_metadata file.


## v0.4

#### Added
- Build/test automation via `Makefile`:
  - `test_sha1`
  - `test_bencode`
  - `test_metainfo`
  - `clean`
- `.gitignore` entry for build artifacts (`/bin`).
- SHA1 test suite in `tests/test_sha1.cpp`:
  - Known SHA1 vectors
  - Raw digest vs hex consistency
  - Binary/null-byte input handling
  - Hex output format checks
- Metainfo parser test suite in `tests/test_metainfo.cpp`:
  - Single-file and multi-file torrent parsing
  - Piece and size validations
  - Error-path coverage

#### Fixed
- `sha1_hex()` iteration variable bug in `srcs/crypto/sha1.cpp` (`digest` -> `result`).
- Missing crypto-related includes in SHA1/PeerID headers for build correctness.

#### Changed
- Minor formatting/log output improvements in tests and parser code.


---
## v0.3

#### Added
- Torrent metainfo domain models:
  - `TorrentFile`
  - `FileInfo`
  - `MetainfoError`
- Torrent metadata parser (`parse_torrent`) in `srcs/torrent/metainfo.cpp`:
  - Reads and decodes `.torrent` files
  - Extracts tracker/comment/creator/date metadata
  - Supports single-file and multi-file torrents
  - Parses piece hashes into 20-byte SHA1 blocks
  - Computes `info_hash` from raw bencoded `info` dictionary bytes
- SHA1 utility module:
  - `sha1_raw(const std::string&)`
  - `sha1_hex(const std::string&)`
- Peer ID utility:
  - `generate_peer_id()`
- `TorrentFile` helpers:
  - `is_multi_file()`
  - `piece_count()`
  - `total_size()`

---
## v0.2

#### Added
- Full bencode decoding support:
  - Integers
  - Strings
  - Lists
  - Dictionaries
- `BencodeValue` variant type with typed accessors.
- Robust decode error handling for malformed/unterminated input.
- Comprehensive bencode test suite (`tests/test_bencode.cpp`):
  - Positive parsing scenarios
  - Nested structures
  - Offset tracking/re-entry behavior
  - Type mismatch and invalid input checks

---
## v0.1

### Added
- Initial repository scaffold.
- Base project structure and placeholder files under:
  - `docs/`
  - `include/` (bencode, crypto, dht, io, net, peer, piece, session, torrent, tracker)
  - `Makefile`