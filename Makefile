GREEN := \033[32m
RED   := \033[31m
RESET := \033[0m

CXX         := c++
CXXFLAGS    := -std=c++17 -Wall -Wextra -I include
LDFLAGS     := -lssl -lcrypto

SRC_DIR     := srcs
TEST_DIR    := tests
BIN_DIR     := bin

NAME        := K0_Torrent

CRYPTO_SRC  := $(SRC_DIR)/crypto/sha1.cpp \
               $(SRC_DIR)/crypto/peer_id.cpp

TORRENT_SRC := $(SRC_DIR)/torrent/metainfo.cpp \
               $(SRC_DIR)/torrent/torrent_file.cpp

BENCODE_SRC := $(SRC_DIR)/bencode/bencode.cpp

NET_SRC     := $(SRC_DIR)/net/event_loop.cpp \
               $(SRC_DIR)/net/tcp_socket.cpp \
               $(SRC_DIR)/net/udp_socket.cpp

TRACKER_SRC := $(SRC_DIR)/tracker/http_tracker.cpp \
               $(SRC_DIR)/tracker/tracker.cpp \
               $(SRC_DIR)/tracker/tracker_manager.cpp \
               $(SRC_DIR)/tracker/udp_tracker.cpp

PEER_SRC    := $(SRC_DIR)/peer/handshake.cpp \
               $(SRC_DIR)/peer/message.cpp \
               $(SRC_DIR)/peer/peer.cpp \
               $(SRC_DIR)/peer/peer_connection.cpp

IO_SRC      := $(SRC_DIR)/io/file_map.cpp \
               $(SRC_DIR)/io/disk_manager.cpp

PIECE_SRC   := $(SRC_DIR)/piece/piece_picker.cpp \
               $(SRC_DIR)/piece/piece_manager.cpp

SESSION_SRC := $(SRC_DIR)/session/session.cpp

all: $(NAME)

help:
	@echo ""
	@echo "\033[1;37m  ██╗  ██╗ ██████╗      ████████╗ ██████╗ ██████╗ ██████╗ ███████╗███╗   ██╗████████╗\033[0m"
	@echo "\033[1;37m  ██║ ██╔╝██╔═████╗     ╚══██╔══╝██╔═══██╗██╔══██╗██╔══██╗██╔════╝████╗  ██║╚══██╔══╝\033[0m"
	@echo "\033[1;37m  █████╔╝ ██║██╔██║        ██║   ██║   ██║██████╔╝██████╔╝█████╗  ██╔██╗ ██║   ██║\033[0m"
	@echo "\033[1;37m  ██╔═██╗ ████╔╝██║        ██║   ██║   ██║██╔══██╗██╔══██╗██╔══╝  ██║╚██╗██║   ██║\033[0m"
	@echo "\033[1;37m  ██║  ██╗╚██████╔╝        ██║   ╚██████╔╝██║  ██║██║  ██║███████╗██║ ╚████║   ██║\033[0m"
	@echo "\033[1;37m  ╚═╝  ╚═╝ ╚═════╝         ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝\033[0m"
	@echo ""
	@echo "\033[90m  ─────────────────────────────────────────────────────────────────────────────────\033[0m"
	@echo "\033[1;37m  From-scratch C++ BitTorrent client  \033[0m\033[90m│\033[0m\033[32m  c++17\033[0m\033[90m  │\033[0m\033[32m  OpenSSL\033[0m\033[90m  │\033[0m\033[32m  POSIX\033[0m"
	@echo "\033[90m  ─────────────────────────────────────────────────────────────────────────────────\033[0m"
	@echo ""
	@echo "\033[1;37m  PRIMARY TARGETS\033[0m"
	@echo ""
	@echo "  \033[32mmake all\033[0m             \033[90m→\033[0m  Run all tests then build the client"
	@echo "  \033[32mmake $(NAME)\033[0m      \033[90m→\033[0m  Build the main BitTorrent client binary"
	@echo "  \033[32mmake check\033[0m           \033[90m→\033[0m  Run the full test suite"
	@echo "  \033[32mmake clean\033[0m           \033[90m→\033[0m  Remove compiled binaries \033[90m(bin/)\033[0m"
	@echo "  \033[32mmake help\033[0m            \033[90m→\033[0m  Show this message"
	@echo ""
	@echo "\033[1;37m  TEST TARGETS\033[0m"
	@echo ""
	@echo "  \033[32mmake test_sha1\033[0m       \033[90m→\033[0m  SHA-1 hash implementation"
	@echo "  \033[32mmake test_bencode\033[0m    \033[90m→\033[0m  Bencode parser and encoder"
	@echo "  \033[32mmake test_metainfo\033[0m   \033[90m→\033[0m  .torrent file parsing"
	@echo "  \033[32mmake test_tracker\033[0m    \033[90m→\033[0m  HTTP + UDP tracker protocol \033[90m(BEP 3/15)\033[0m"
	@echo "  \033[32mmake test_peer_wire\033[0m  \033[90m→\033[0m  Peer wire protocol \033[90m(BEP 3)\033[0m"
	@echo "  \033[32mmake test_async\033[0m      \033[90m→\033[0m  Async event loop"
	@echo "  \033[32mmake test_io\033[0m         \033[90m→\033[0m  Disk manager and file map"
	@echo "  \033[32mmake test_piece\033[0m      \033[90m→\033[0m  Piece picker and manager"
	@echo "  \033[32mmake test_session\033[0m    \033[90m→\033[0m  Full session orchestration"
	@echo ""
	@echo "\033[90m  ─────────────────────────────────────────────────────────────────────────────────\033[0m"
	@echo ""

$(NAME): $(BIN_DIR)
	@echo ""
	@echo "\033[90m ───────────────────────────────────────────────────────────────────────────────\033[0m"
	@echo "\033[1;37m                           Building $(NAME)\033[0m"
	@echo "\033[90m ───────────────────────────────────────────────────────────────────────────────\033[0m"
	@echo ""
	@$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp \
		$(SESSION_SRC) $(TRACKER_SRC) $(TORRENT_SRC) $(BENCODE_SRC) \
		$(PIECE_SRC) $(IO_SRC) $(PEER_SRC) $(NET_SRC) $(CRYPTO_SRC) \
		-o $(NAME) $(LDFLAGS)
	@echo "  \033[32m[OK]\033[0m  $(NAME) built successfully"
	@echo "  \033[90mRun with: ./$(NAME) <file.torrent>\033[0m"
	@echo ""

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# --------------------- Individual Tests ---------------------

test_sha1: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_sha1.cpp $(CRYPTO_SRC) -o $(BIN_DIR)/test_sha1 $(LDFLAGS)
	@./$(BIN_DIR)/test_sha1 > /tmp/test_sha1.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_sha1" || (echo "  $(RED)[FAIL]$(RESET) test_sha1"; cat /tmp/test_sha1.log; exit 1)

test_bencode: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_bencode.cpp $(BENCODE_SRC) -o $(BIN_DIR)/test_bencode
	@./$(BIN_DIR)/test_bencode > /tmp/test_bencode.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_bencode" || (echo "  $(RED)[FAIL]$(RESET) test_bencode"; cat /tmp/test_bencode.log; exit 1)

test_metainfo: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_metainfo.cpp $(BENCODE_SRC) $(CRYPTO_SRC) $(TORRENT_SRC) -o $(BIN_DIR)/test_metainfo $(LDFLAGS)
	@./$(BIN_DIR)/test_metainfo > /tmp/test_metainfo.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_metainfo" || (echo "  $(RED)[FAIL]$(RESET) test_metainfo"; cat /tmp/test_metainfo.log; exit 1)

test_tracker: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_tracker.cpp \
		$(TRACKER_SRC) $(NET_SRC) $(TORRENT_SRC) $(BENCODE_SRC) $(CRYPTO_SRC) \
		-o $(BIN_DIR)/test_tracker $(LDFLAGS)
	@./$(BIN_DIR)/test_tracker > /tmp/test_tracker.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_tracker" || (echo "  $(RED)[FAIL]$(RESET) test_tracker"; cat /tmp/test_tracker.log; exit 1)

test_peer_wire: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_peer_wire.cpp \
		$(PEER_SRC) $(CRYPTO_SRC) $(NET_SRC) \
		-o $(BIN_DIR)/test_peer_wire $(LDFLAGS)
	@./$(BIN_DIR)/test_peer_wire > /tmp/test_peer_wire.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_peer_wire" || (echo "  $(RED)[FAIL]$(RESET) test_peer_wire"; cat /tmp/test_peer_wire.log; exit 1)

test_async: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_async.cpp \
		$(PEER_SRC) $(CRYPTO_SRC) $(NET_SRC) \
		-o $(BIN_DIR)/test_async $(LDFLAGS)
	@./$(BIN_DIR)/test_async > /tmp/test_async.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_async" || (echo "  $(RED)[FAIL]$(RESET) test_async"; cat /tmp/test_async.log; exit 1)

test_io: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_io.cpp \
		$(IO_SRC) \
		-o $(BIN_DIR)/test_io $(LDFLAGS)
	@./$(BIN_DIR)/test_io > /tmp/test_io.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_io" || (echo "  $(RED)[FAIL]$(RESET) test_io"; cat /tmp/test_io.log; exit 1)

test_piece: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_piece_manager.cpp \
		$(PIECE_SRC) $(CRYPTO_SRC) $(PEER_SRC) $(NET_SRC) \
		-o $(BIN_DIR)/test_piece $(LDFLAGS)
	@./$(BIN_DIR)/test_piece > /tmp/test_piece.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_piece" || (echo "  $(RED)[FAIL]$(RESET) test_piece"; cat /tmp/test_piece.log; exit 1)

test_session: $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_session.cpp \
		$(SESSION_SRC) $(TRACKER_SRC) $(TORRENT_SRC) $(BENCODE_SRC) \
		$(PIECE_SRC) $(IO_SRC) $(PEER_SRC) $(NET_SRC) $(CRYPTO_SRC) \
		-o $(BIN_DIR)/test_session $(LDFLAGS)
	@./$(BIN_DIR)/test_session > /tmp/test_session.log 2>&1 && echo "  $(GREEN)[OK]$(RESET) test_session" || (echo "  $(RED)[FAIL]$(RESET) test_session"; cat /tmp/test_session.log; exit 1)

# ---------------------------------------------------------------------

clean:
	@rm -rf $(BIN_DIR) $(NAME)

check: test_sha1 test_bencode test_metainfo test_tracker test_peer_wire test_async test_io test_piece test_session

.PHONY: all help check clean test_sha1 test_bencode test_metainfo test_tracker test_peer_wire test_async test_io test_piece test_session