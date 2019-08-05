LAME_DIR=$(CURDIR)/lame-svn/lame

WMAKEFILE=$(LAME_DIR)/Makefile
WDIST_DIR=$(LAME_DIR)/wdist
WLAME_SO=$(WDIST_DIR)/lib/libmp3lame.so

CMAKEFILE=$(LAME_DIR)/Makefile.native
CDIST_DIR=$(LAME_DIR)/cdist
CLAME_SO=$(CDIST_DIR)/lib/libmp3lame.so

all: lame_native.wasm


# # WASM backend doesn't support EMSCRIPTEN_KEEPALIVE, see:
# # https://github.com/kripken/emscripten/issues/6233
# # Output to bare .wasm doesn't work properly so need to create
# # intermediate files.
lame_native.wasm: $(WLAME_SO) lame_native_bindings.c
	emcc $^ \
	  -I$(WDIST_DIR)/include \
		-DNDEBUG \
		-Oz -Os --llvm-lto 3 \
		-s WASM=1 -s MODULARIZE=1 -s NODEJS_CATCH_EXIT=0 \
		-s "EXPORTED_FUNCTIONS=['_lamejs_init', '_lamejs_max_encode_samples', '_lamejs_encode', '_lamejs_flush', '_lamejs_free', '_lamejs_print_debug_info', '_lamejs_tag_frame']" \
		-o lame_native.js

$(WLAME_SO): $(WMAKEFILE)
	cd $(LAME_DIR) && \
	emmake make -j8 && \
	emmake make install

$(WMAKEFILE):
	cd $(LAME_DIR) && \
	emconfigure ./configure \
		CFLAGS="-DNDEBUG -Oz" \
		--prefix="$(WDIST_DIR)" \
		--host=x86-none-linux \
		--disable-static \
		\
		--disable-gtktest \
		--disable-analyzer-hooks \
		--disable-decoder \
		--disable-frontend

distclean: clean cdistclean
clean: cclean
	rm -f lame_native.js lame_native.wasm
cclean:
	cd $(LAME_DIR) && make clean
cdistclean:
	cd $(LAME_DIR) && make distclean


ctest: clame
	gcc main.c

clame: $(CLAME_SO)

$(CLAME_SO):
	cd $(LAME_DIR) && \
	./configure --prefix="$(CDIST_DIR)" && \
	make -j8 && \
	make install

