all:
	$(MAKE) -C plugins all gtest unit-tests

clean:
	$(MAKE) -C plugins clean

clangformat:
	clang-format-9 -i $(wildcard plugins/src/*.cc) $(wildcard plugins/src/*.h)
