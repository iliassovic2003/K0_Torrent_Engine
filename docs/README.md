# K0 Torrent Engine

K0 Torrent Engine is a C++17 BitTorrent client developed from scratch. It currently supports torrent metainfo parsing, tracker communication, peer-wire messaging, asynchronous networking, piece verification, and disk management.

The project has been validated through an end-to-end download using a `.torrent` file. It is currently an **early-beta implementation** intended for development, testing, and controlled use. Production hardening, DHT support, and performance optimization remain part of the development roadmap.

## Features

- Bencode decoding
  - Integers
  - Strings
  - Lists
  - Dictionaries
  - Nested values
  - Offset-aware decoding and validation

- Torrent metainfo parsing
  - Single-file torrents
  - Multi-file torrents
  - Tracker metadata
  - Piece length and piece hashes
  - SHA1 `info_hash` calculation

- Cryptography and identity
  - SHA1 hashing
  - Raw and hexadecimal digest generation
  - BitTorrent peer ID generation

- Tracker communication
  - HTTP tracker support
  - UDP tracker support
  - Compact peer-list parsing
  - Dictionary-based peer-list parsing
  - Multi-tracker tiers
  - Tracker failover and retry handling

- Peer-wire communication
  - BitTorrent handshake exchange
  - Peer message framing and parsing
  - Keep-alive messages
  - Choke and unchoke handling
  - Interested and not-interested messages
  - Bitfield and `Have` messages
  - Piece requests and piece messages

- Asynchronous networking
  - Non-blocking TCP connections
  - Event-loop based socket handling
  - Buffered reads and writes
  - Peer connection callbacks

- Piece and disk management
  - Piece selection
  - Block request tracking
  - SHA1 piece verification
  - Multi-file offset mapping
  - Disk reads and writes
  - Download directory creation

- Torrent sessions
  - Tracker peer discovery
  - Peer connection management
  - Piece request coordination
  - Graceful shutdown handling

## Project Status

The current implementation is capable of downloading and verifying torrent data end to end.

It should be considered:

> **Functional early beta — not yet production-ready.**

Before a production release, the project requires broader testing and hardening for:

- Interrupted and resumable downloads
- Peer reconnection
- Tracker and network failure recovery
- Corrupted or malicious input
- Disk-full and permission errors
- Large torrents and long-running sessions
- Resource and connection limits
- Concurrent peer performance
- IPv6 support
- Security and input validation

## Requirements

- A C++17-compatible compiler
- GNU Make
- OpenSSL development libraries
- POSIX-compatible networking APIs

On Debian or Ubuntu-based systems:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

## Build

Clone the repository and enter its root directory:

```bash
git clone https://github.com/iliassovic2003/K0_Torrent_Engine.git
cd K0_Torrent_Engine
```

Build the client:

```bash
make
```

The resulting executable is:

```text
K0_Torrent
```

You can also build it explicitly:

```bash
make K0_Torrent
```

## Usage

Run the client with a torrent metainfo file:

```bash
./K0_Torrent path/to/file.torrent
```

Downloaded files are stored in the `downloads/` directory.

The client currently performs the following session flow:

1. Loads and parses the `.torrent` file.
2. Calculates the torrent information hash.
3. Generates a peer ID.
4. Contacts configured trackers.
5. Discovers peers.
6. Establishes peer-wire connections.
7. Requests available piece blocks.
8. Verifies completed pieces using SHA1.
9. Writes verified data to disk.

## Testing

Run the complete test suite:

```bash
make check
```

Available test targets include:

```bash
make test_sha1
make test_bencode
make test_metainfo
make test_tracker
make test_peer_wire
make test_async
make test_io
make test_piece
make test_session
```

Build and run the full validation workflow:

```bash
make all
```

Display the available Make targets:

```bash
make help
```

Remove compiled binaries:

```bash
make clean
```

## Repository Structure

```text
.
├── include/                Public C++ headers
│   ├── bencode/            Bencode data model and decoder
│   ├── common/             Shared types, errors, and logging
│   ├── crypto/             SHA1 and peer ID utilities
│   ├── dht/                DHT-related interfaces and future work
│   ├── io/                 File mapping and disk management
│   ├── net/                TCP, UDP, and event-loop components
│   ├── peer/               Peer state and peer-wire protocol
│   ├── piece/              Piece selection and block management
│   ├── session/            Torrent session orchestration
│   ├── torrent/            Torrent metadata models
│   └── tracker/            HTTP, UDP, and tracker management
├── srcs/                   C++ implementations
├── tests/                  Unit and integration tests
├── docs/                   Project documentation
├── Makefile                Build and test targets
└── downloads/              Runtime download directory
```

## Architecture

The client is organized into several cooperating layers:

```text
Session
  ├── TrackerManager
  │     ├── HTTP tracker
  │     └── UDP tracker
  ├── PeerConnection
  │     ├── TCP socket
  │     └── Event loop
  ├── PieceManager
  │     └── PiecePicker
  └── DiskManager
        └── FileMap
```

The `Session` coordinates the torrent lifecycle. Trackers provide peer addresses, peer connections handle the BitTorrent wire protocol, the piece manager tracks requests and verifies data, and the disk manager maps torrent offsets to files.

## Roadmap

### DHT and Kademlia

- Implement Mainline DHT peer discovery
- Add UDP-based DHT messages
- Add Kademlia node and routing-table management
- Implement `ping`
- Implement `find_node`
- Implement `get_peers`
- Implement `announce_peer`
- Use DHT discovery when trackers are unavailable

### Performance Optimization

- Improve piece and block scheduling
- Tune request pipelining
- Reduce unnecessary network-buffer copies
- Optimize disk write batching
- Improve file-map lookup performance
- Profile CPU, memory, network, and disk usage
- Add benchmarks for hashing, networking, piece management, and IO
- Improve behavior with many simultaneous peer connections

### Reliability

- Add resume support
- Add peer reconnection
- Improve timeout and retry policies
- Add stronger corruption recovery
- Handle disk-full and permission failures
- Expand stress and integration testing
- Add support for larger and long-running torrents

## Documentation

Project documentation is available in:

- [`CHANGELOGS.md`](CHANGELOGS.md)

## License

see [`LICENSE`](../LICENSE)