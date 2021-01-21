all:
	$(MAKE) -C plugins

clangformat:
	clang-format-9 -i $(wildcard plugins/src/*.cc) $(wildcard plugins/src/*.h)
