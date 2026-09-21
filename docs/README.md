# K0_Torrent

K0_Torrent is a C++ BitTorrent client project.

The project currently provides the foundation for:

- Bencode decoding
- Torrent metadata parsing
- SHA1 hashing
- Peer ID generation
- Tracker communication
- Peer-wire communication
- Piece management
- Disk and file management
- DHT and Kademlia support
- Network event handling
- Torrent session management

## Project Structure

```text
include/     <- Public header files
srcs/        <- C++ source files
tests/       <- Unit and integration tests
docs/        <- Project documentation
Makefile     <- Build and test commands
```

### Main Modules

| Module | Description |
|---|---|
| `bencode` | Decodes torrent bencoded data |
| `crypto` | SHA1 hashing and peer ID generation |
| `torrent` | Torrent metadata and `.torrent` file handling |
| `tracker` | HTTP and UDP tracker support |
| `peer` | Peer connections and peer-wire messages |
| `piece` | Piece selection, blocks, and piece management |
| `io` | Disk and file management |
| `net` | TCP, UDP, and event-loop support |
| `dht` | DHT and Kademlia functionality |
| `session` | Torrent session management |
| `common` | Shared types, logging, and error handling |

## Building

From the repository root:

```bash
make
```

The build output is generated in the `bin/` directory.

## Running Tests

Run the complete test suite with:

```bash
make test
```

Individual test targets include:

```bash
make test_bencode
make test_sha1
make test_metainfo
```

Additional tests cover:

- Peer-wire messages
- Piece management
- Tracker communication


## Example Usage

A torrent metadata file can be loaded and parsed using the torrent metainfo
module.

The project also includes support for decoding bencoded values and calculating
the SHA1 `info_hash` used by BitTorrent peers and trackers.


## Documentation

Project documentation is available in the `docs/` directory:

- `docs/README.md`
- `docs/CHANGELOGS.md`

## License

License information will be added when the project license is finalized.