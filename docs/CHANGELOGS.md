```text
════════════════════════════════════════════════════════════════
                    K0_TORRENT — CHANGELOG
════════════════════════════════════════════════════════════════

────────────────────────────────────────────────────────────────
  v0.0.0 => v0.1.0                                      [MAJOR]
────────────────────────────────────────────────────────────────
  + Initial project scaffold
      - Created the initial repository structure
      - Added documentation directory:
          docs/
      - Added header directory structure:
          include/bencode/
          include/crypto/
          include/dht/
          include/io/
          include/net/
          include/peer/
          include/piece/
          include/session/
          include/torrent/
          include/tracker/
      - Added the initial `Makefile`
      - Added placeholder files for future torrent-client modules
      - Established the foundation for bencode decoding,
        torrent metadata parsing, peer communication, and tracking


────────────────────────────────────────────────────────────────
  v0.1.0 => v0.2.0                                      [MAJOR]
────────────────────────────────────────────────────────────────
  + Full bencode decoding support
      - Added integer decoding
      - Added string decoding
      - Added list decoding
      - Added dictionary decoding
      - Added nested bencode value support

  + Bencode data model
      - Added the `BencodeValue` variant type
      - Added typed accessors for decoded values
      - Added support for checking and retrieving stored value types

  + Decoder error handling
      - Added robust handling for malformed input
      - Added unterminated value detection
      - Added invalid input validation
      - Added type mismatch error handling
      - Added offset tracking during decoding
      - Added decoder re-entry support from tracked offsets

  + Bencode test coverage
      - Added `tests/test_bencode.cpp`
      - Added positive parsing tests
      - Added nested structure tests
      - Added offset tracking and re-entry tests
      - Added invalid input tests
      - Added type mismatch tests


────────────────────────────────────────────────────────────────
  v0.2.0 => v0.3.0                                      [MAJOR]
────────────────────────────────────────────────────────────────
  + Torrent metainfo domain models
      - Added `TorrentFile`
      - Added `FileInfo`
      - Added `MetainfoError`
      - Added support for representing single-file torrents
      - Added support for representing multi-file torrents

  + Torrent metadata parser
      - Added `parse_torrent` in:
          `srcs/torrent/metainfo.cpp`
      - Added `.torrent` file reading and decoding
      - Added tracker metadata extraction
      - Added comment metadata extraction
      - Added creator metadata extraction
      - Added creation-date metadata extraction
      - Added single-file torrent parsing
      - Added multi-file torrent parsing
      - Added piece hash parsing into 20-byte SHA1 blocks
      - Added `info_hash` calculation from the raw bencoded
        `info` dictionary bytes

  + SHA1 utility module
      - Added `sha1_raw(const std::string&)`
      - Added `sha1_hex(const std::string&)`
      - Added raw SHA1 digest generation
      - Added hexadecimal SHA1 digest generation

  + Peer ID utility
      - Added `generate_peer_id()`
      - Added initial peer identity generation support

  + TorrentFile helper methods
      - Added `is_multi_file()`
      - Added `piece_count()`
      - Added `total_size()`


────────────────────────────────────────────────────────────────
  v0.3.0 => v0.4.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Build and test automation
      - Expanded the `Makefile` with:
          `test_sha1`
          `test_bencode`
          `test_metainfo`
          `clean`

  + SHA1 test suite
      - Added `tests/test_sha1.cpp`
      - Added known SHA1 test vectors
      - Added raw digest versus hexadecimal digest consistency tests
      - Added binary input handling tests
      - Added null-byte input tests
      - Added hexadecimal output format validation

  + Metainfo parser test suite
      - Added `tests/test_metainfo.cpp`
      - Added single-file torrent parsing tests
      - Added multi-file torrent parsing tests
      - Added piece validation tests
      - Added total-size validation tests
      - Added parser error-path tests

  + Repository hygiene
      - Added `/bin` to `.gitignore`
      - Added protection against committing build artifacts

  / SHA1 implementation fixes
      - Fixed the `sha1_hex()` iteration variable bug in:
          `srcs/crypto/sha1.cpp`
      - Replaced the incorrect `digest` variable with `result`

  / Header and build fixes
      - Added missing crypto-related includes
      - Fixed SHA1 header compilation dependencies
      - Fixed PeerID header compilation dependencies

  · Test and parser cleanup
      - Improved test formatting
      - Improved test log output
      - Applied minor parser readability improvements


────────────────────────────────────────────────────────────────
  v0.4.0 => v0.5.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Test metadata maintenance
      - Corrected the test metadata file
      - Synchronized test metadata with the current test structure
      - Updated repository testing information for future development


────────────────────────────────────────────────────────────────
  v0.5.0 => v0.6.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Test configuration correction
      - Corrected the test metadata file
      - Refined the metadata used by the repository test workflow
      - Ensured the test configuration reflects the current project state


════════════════════════════════════════════════════════════════
                        CURRENT RELEASE
════════════════════════════════════════════════════════════════

  BUILD STATUS:     ✅ BENCODE + TORRENT METAINFO + SHA1 TESTS
  VERSION:          v0.6
  RELEASE DATE:     September 21, 2026
  FOCUS:            Test metadata correction and continued validation
                    of the bencode, SHA1, and torrent metainfo modules

════════════════════════════════════════════════════════════════
  Legend:  + Added          · Changed             / Fixed
════════════════════════════════════════════════════════════════
```
