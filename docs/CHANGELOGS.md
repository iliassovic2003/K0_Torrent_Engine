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


────────────────────────────────────────────────────────────────
  v0.6.0 => v0.7.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Tracker protocol support
      - Added HTTP tracker support based on BEP 3
      - Added UDP tracker support based on BEP 15
      - Added tracker announce requests
      - Added tracker response parsing
      - Added compact peer list parsing
      - Added dictionary-based peer list parsing
      - Added tracker URL encoding for info hashes and peer IDs

  + Multi-tracker management
      - Added `TrackerManager`
      - Added announce-list tier support
      - Added tracker failover between tiers
      - Added tracker retry handling
      - Added tracker announce interval tracking
      - Added minimum announce interval support
      - Added exponential backoff after tracker failures
      - Added support for started, completed, stopped, and regular
        tracker events

  + Networking foundations
      - Added common network types:
          `InfoHash`
          `PeerId`
          `PeerAddress`
      - Added common error types:
          `TrackerError`
          `NetworkError`
          `ParseError`
      - Added lightweight logging macros
      - Added initial network source integration in the `Makefile`

  + Tracker test coverage
      - Added `tests/test_tracker.cpp`
      - Added URL encoding tests
      - Added compact peer decoding tests
      - Added UDP connect packet layout tests
      - Added tracker manager construction tests

  · Project organization
      - Removed the previous `k0` namespace wrappers
      - Standardized tracker and common type declarations
      - Corrected tracker logging tags


────────────────────────────────────────────────────────────────
  v0.7.0 => v0.8.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + BitTorrent peer wire protocol
      - Added BitTorrent handshake serialization
      - Added BitTorrent handshake deserialization
      - Added protocol string and length validation
      - Added info-hash validation during handshakes
      - Added peer ID exchange support

  + Peer message framing
      - Added peer message identifiers:
          Choke
          Unchoke
          Interested
          NotInterested
          Have
          Bitfield
          Request
          Piece
          Cancel
      - Added length-prefixed peer message serialization
      - Added keep-alive message support
      - Added fragmented TCP message parsing
      - Added peer message payload extraction

  + Peer state and bitfields
      - Added the `Peer` class
      - Added peer bitfield storage
      - Added piece availability checks
      - Added piece availability updates through `Have` messages
      - Added peer choking and interest state tracking

  + Peer connection layer
      - Added the initial `PeerConnection` implementation
      - Added TCP connection management
      - Added handshake exchange handling
      - Added incoming peer message processing
      - Added outgoing peer message support

  + Peer wire test coverage
      - Added `tests/test_peer_wire.cpp`
      - Added handshake serialization tests
      - Added handshake parsing tests
      - Added message framing tests
      - Added TCP fragmentation tests
      - Added peer bitfield tracking tests

  + Build system updates
      - Added `test_peer_wire` to the `Makefile`
      - Added peer source files to the test build commands
      - Corrected tracker logging tag names


────────────────────────────────────────────────────────────────
  v0.8.0 => v0.8.1                                      [PATCH]
────────────────────────────────────────────────────────────────
  / Peer and test compilation fixes
      - Added a default constructor to `Handshake`
      - Added a default `PeerMessage` constructor
      - Added the missing `<cstddef>` include to:
          `include/peer/peer.hpp`
      - Fixed a missing semicolon in:
          `tests/test_peer_wire.cpp`

  · Improved default object initialization
      - Ensured peer wire structures can be safely default-constructed
      - Preserved the default `Choke` message state for `PeerMessage`


────────────────────────────────────────────────────────────────
  v0.8.1 => v0.9.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Asynchronous networking
      - Added the event loop implementation
      - Added non-blocking TCP socket support
      - Added asynchronous peer connection handling
      - Added connection event registration and removal
      - Added readable and writable socket event handling
      - Added buffered outgoing peer messages
      - Added buffered incoming peer messages
      - Added asynchronous handshake processing

  + Disk and file management
      - Added `FileMap`
      - Added global torrent-offset to file-offset mapping
      - Added cross-file block mapping
      - Added `DiskManager`
      - Added block read support
      - Added block write support
      - Added automatic download directory creation
      - Added nested directory creation for multi-file torrents
      - Added open-file stream caching

  + Piece management
      - Added `Block`
      - Added block state tracking:
          Missing
          Requested
          Downloaded
      - Added `PieceManager`
      - Added active piece tracking
      - Added piece request generation
      - Added block processing
      - Added SHA1 piece verification
      - Added completed-piece tracking
      - Added `PiecePicker`
      - Added peer-aware piece selection

  + Improved peer connection architecture
      - Integrated `PeerConnection` with `EventLoop`
      - Added message handler callbacks
      - Added asynchronous read and write handlers
      - Added connection state tracking
      - Added non-blocking socket error handling
      - Added graceful handling of partial sends and receives

  + New test suites
      - Added `tests/test_async.cpp`
      - Added asynchronous handshake tests
      - Added `tests/test_io.cpp`
      - Added file-map boundary tests
      - Added `tests/test_piece_manager.cpp`
      - Added piece request generation tests
      - Added block processing tests
      - Added SHA1 piece verification tests

  + Build automation
      - Added `test_async`
      - Added `test_io`
      - Added `test_piece`
      - Added network, IO, and piece source groups to the `Makefile`


────────────────────────────────────────────────────────────────
  v0.9.0 => v1.0.0                                      [MAJOR]
────────────────────────────────────────────────────────────────
  + Full torrent session orchestration
      - Added the `Session` class
      - Added torrent initialization from a `.torrent` file
      - Added peer ID generation during session startup
      - Added tracker manager initialization
      - Added disk manager initialization
      - Added piece manager initialization
      - Added tracker peer discovery
      - Added peer connection creation
      - Added peer connection lifecycle management
      - Added session start and stop operations

  + Peer download coordination
      - Added handling for choke and unchoke messages
      - Added handling for interested and not-interested messages
      - Added handling for `Have` messages
      - Added handling for bitfield messages
      - Added automatic `Interested` message generation
      - Added request pipelining for peer blocks
      - Added piece message processing
      - Added downloaded block forwarding to the piece manager
      - Added completed piece writes through the disk manager

  + Command-line torrent client
      - Added `srcs/main.cpp`
      - Added torrent-file command-line argument handling
      - Added executable target:
          `K0_Torrent`
      - Added SIGINT handling for graceful shutdown
      - Added fatal error reporting
      - Added client termination logging

  + Session integration tests
      - Added `tests/test_session.cpp`
      - Added dummy torrent generation
      - Added session initialization tests
      - Added tracker connection failure handling tests
      - Added clean session shutdown validation

  + Build system redesign
      - Added the main `K0_Torrent` executable target
      - Added the `check` target for the complete test suite
      - Added the `test_session` target
      - Added colored build and test output
      - Added a Makefile help screen
      - Added failure log capture for individual tests
      - Updated `clean` to remove the main executable
      - Added the main client source files to the build


────────────────────────────────────────────────────────────────
  v1.0.0 => v1.1.0                                      [MINOR]
────────────────────────────────────────────────────────────────
  + Piece scheduling improvements
      - Added tracking for active pieces currently in flight
      - Added prevention of duplicate active piece selection
      - Added support for requesting additional blocks from active pieces
      - Added a limit on the number of simultaneously active pieces
      - Added piece picker support for skipped piece indices
      - Added cleanup of completed active pieces
      - Improved handling of final, shorter pieces
      - Added 64-bit torrent-size and offset calculations

  + Disk manager improvements
      - Added correct single-file torrent registration
      - Preserved multi-file torrent path registration
      - Added detailed file registration logging
      - Added download directory logging
      - Added file open and creation logging
      - Added block-to-file mapping diagnostics
      - Added write success and failure diagnostics
      - Improved disk write error reporting

  + Peer connection reliability
      - Added non-blocking TCP connection setup
      - Added `EINPROGRESS` handling
      - Improved asynchronous connection startup
      - Added peer address information to connection errors
      - Added clean handling of peer disconnects
      - Added EOF and socket read error handling
      - Improved handshake success logging

  + Session data flow fixes
      - Changed downloaded block writes to occur at the correct
        piece and block offset
      - Improved bitfield handling
      - Improved `Interested` message creation
      - Added safer peer message switch scoping
      - Improved request generation after receiving piece blocks

  + Runtime cleanup
      - Added immediate process termination after SIGINT shutdown
      - Standardized download directory handling
      - Improved logging consistency across networking, disk, and
        piece-management modules


════════════════════════════════════════════════════════════════
                        CURRENT RELEASE
════════════════════════════════════════════════════════════════

  BUILD STATUS:     ✅ END-TO-END TORRENT DOWNLOAD VERIFIED
  VERSION:          v1.1.0
  RELEASE DATE:     September 22, 2026
  MATURITY:         FUNCTIONAL BETA
  FOCUS:            Torrent session orchestration, peer downloading,
                    asynchronous networking, piece scheduling,
                    disk management, and runtime reliability

  VALIDATION:
      - Successfully downloaded file data using a `.torrent` file
      - Successfully contacted trackers and discovered peers
      - Successfully established peer connections
      - Successfully exchanged BitTorrent wire messages
      - Successfully requested and received piece blocks
      - Successfully verified downloaded pieces using SHA1
      - Successfully wrote verified data to disk

  RELEASE POSITIONING:
      - Suitable for development, testing, and controlled usage
      - Not yet classified as production-ready
      - Production hardening remains in progress


────────────────────────────────────────────────────────────────
                        IMPROVEMENT WINDOW
────────────────────────────────────────────────────────────────

  + DHT support
      - Add distributed peer discovery through the Mainline DHT
      - Implement UDP-based DHT communication
      - Add routing-table management
      - Add Kademlia node and transaction handling
      - Implement `ping`, `find_node`, `get_peers`, and `announce_peer`
      - Support compact node and peer representations
      - Use DHT discovery when trackers are unavailable
      - Add IPv4 and future IPv6 DHT support

  + Performance optimization
      - Optimize piece and block scheduling
      - Improve peer request pipelining
      - Tune the number of active pieces and outstanding requests
      - Reduce unnecessary memory copies in network buffers
      - Improve disk write batching and sequential write behavior
      - Optimize file-map lookups for large multi-file torrents
      - Improve event-loop efficiency under many peer connections
      - Add connection and tracker concurrency limits
      - Measure download throughput, CPU usage, memory usage, and disk usage
      - Add profiling benchmarks for networking, hashing, piece management,
        and disk operations

  + Reliability and production hardening
      - Add resume support for interrupted downloads
      - Add corruption recovery and piece re-request handling
      - Add tracker reannounce and peer reconnection tests
      - Improve timeout and retry policies
      - Handle disk-full and permission errors more safely
      - Validate torrent metadata and network input more strictly
      - Add large-file and long-running session tests
      - Add broader integration and stress-test coverage

════════════════════════════════════════════════════════════════
  Legend:  + Added          · Changed             / Fixed
════════════════════════════════════════════════════════════════
```
