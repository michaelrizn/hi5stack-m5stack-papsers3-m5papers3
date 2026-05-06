EMCC = emcc
SRC_DIR = ../src
GAMES_DIR = $(SRC_DIR)/games
OUT_DIR = games

EMCC_FLAGS = -O2 \
	-s WASM=1 \
	-s EXPORTED_FUNCTIONS="['_malloc', '_free']" \
	-s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME="'Module'" \
	-s ENVIRONMENT='web' \
	-I$(SRC_DIR) \
	-I. \
	-DUI_H="ui_stub.h" \
	-std=c++17 \
	--bind

GAMES = minesweeper sudoku test

.PHONY: all clean $(GAMES)

all: $(GAMES)

$(GAMES): %: $(OUT_DIR)/%/index.html

$(OUT_DIR)/%/index.html: $(GAMES_DIR)/%/game.cpp $(OUT_DIR)/%/game_adapter.cpp ui_stub.cpp
	@mkdir -p $(OUT_DIR)/$*
	$(EMCC) $(EMCC_FLAGS) \
		$(OUT_DIR)/$*/game_adapter.cpp \
		$(GAMES_DIR)/$*/game.cpp \
		ui_stub.cpp \
		-o $(OUT_DIR)/$*/game.js
	cp game_template.html $(OUT_DIR)/$*/index.html
	sed -i '' 's/{{GAME_NAME}}/$*/g' $(OUT_DIR)/$*/index.html
	sed -i '' 's/{{GAME_TITLE}}/$(shell echo $* | sed 's/.*/\u&/')/' $(OUT_DIR)/$*/index.html
	sed -i '' 's/{{NAMESPACE}}/games_$*/g' $(OUT_DIR)/$*/index.html

$(OUT_DIR)/%/game_adapter.cpp: game_adapter_template.cpp
	@mkdir -p $(OUT_DIR)/$*
	cp game_adapter_template.cpp $(OUT_DIR)/$*/game_adapter.cpp
	sed -i '' 's/{{NAMESPACE}}/games_$*/g' $(OUT_DIR)/$*/game_adapter.cpp

clean:
	rm -rf $(OUT_DIR)

setup:
	@mkdir -p $(OUT_DIR)
	@for game in $(GAMES); do \
		mkdir -p $(OUT_DIR)/$$game; \
	done

help:
	@echo "Available targets:"
	@echo "  all       - Build all games"
	@echo "  clean     - Remove build artifacts"
	@echo "  setup     - Create directory structure"
	@echo "  <game>    - Build specific game ($(GAMES))"
	@echo ""
	@echo "Games: $(GAMES)"