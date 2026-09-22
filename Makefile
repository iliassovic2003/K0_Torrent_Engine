CXX      := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -I include
LDFLAGS  := -lssl -lcrypto

SRC_DIR  := srcs
TEST_DIR := tests
BIN_DIR  := bin


CRYPTO_SRC 	:= $(SRC_DIR)/crypto/sha1.cpp \
              	$(SRC_DIR)/crypto/peer_id.cpp

TORRENT_SRC := $(SRC_DIR)/torrent/metainfo.cpp \
               	$(SRC_DIR)/torrent/torrent_file.cpp

BENCODE_SRC := $(SRC_DIR)/bencode/bencode.cpp

NET_SRC 	:= $(SRC_DIR)/net/event_loop.cpp \
           		$(SRC_DIR)/net/tcp_socket.cpp \
           		$(SRC_DIR)/net/udp_socket.cpp

TRACKER_SRC := $(SRC_DIR)/tracker/http_tracker.cpp \
               	$(SRC_DIR)/tracker/tracker.cpp \
               	$(SRC_DIR)/tracker/tracker_manager.cpp \
               	$(SRC_DIR)/tracker/udp_tracker.cpp

PEER_SRC := 	$(SRC_DIR)/peer/handshake.cpp \
            	$(SRC_DIR)/peer/message.cpp \
            	$(SRC_DIR)/peer/peer.cpp \
            	$(SRC_DIR)/peer/peer_connection.cpp

all: test_sha1 test_bencode test_metainfo test_tracker test_peer_wire

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

test_sha1: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_sha1.cpp $(CRYPTO_SRC) -o $(BIN_DIR)/test_sha1 $(LDFLAGS)
	@./$(BIN_DIR)/test_sha1

test_bencode: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_bencode.cpp $(BENCODE_SRC) -o $(BIN_DIR)/test_bencode
	@./$(BIN_DIR)/test_bencode

test_metainfo: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_metainfo.cpp $(BENCODE_SRC) $(CRYPTO_SRC) $(TORRENT_SRC) -o $(BIN_DIR)/test_metainfo $(LDFLAGS)
	@./$(BIN_DIR)/test_metainfo

test_tracker: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_tracker.cpp \
		$(TRACKER_SRC) $(NET_SRC) $(TORRENT_SRC) $(BENCODE_SRC) $(CRYPTO_SRC) \
		-o $(BIN_DIR)/test_tracker $(LDFLAGS)
	@./$(BIN_DIR)/test_tracker

test_peer_wire: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_peer_wire.cpp \
		$(PEER_SRC) $(CRYPTO_SRC) \
		-o $(BIN_DIR)/test_peer_wire $(LDFLAGS)
	@./$(BIN_DIR)/test_peer_wire

clean:
	@rm -rf $(BIN_DIR)

.PHONY: all clean test_sha1 test_bencode test_metainfo test_tracker test_peer_wire