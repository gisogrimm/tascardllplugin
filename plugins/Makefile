 # Makefile for TASCAR modules

# general purpose and actor modules:
#PLUGINS += tascar_rotate

# receiver modules:
#PLUGINS += tascarreceiver_example

# source modules:
#PLUGINS += tascarsource_example

# audio processing plugins:
PLUGINS += tascar_ap_udpsend tascar_ap_udpreceive

# list of test files
TESTFILES = $(shell find ./test/ -name "*.tsc")

all: $(BUILD_DIR) modules

# include compiler configuration to ensure ABI compatibility:
include /usr/share/tascar/config.mk

# include TASCAR specific make rules:
include /usr/share/tascar/rules.mk

EXTERNALS = libxml++-2.6

LDLIBS += `pkg-config --libs $(EXTERNALS)`
CXXFLAGS += `pkg-config --cflags $(EXTERNALS)`

LDLIBS += -ltascar -lovserver
CXXFLAGS += -Llibov/build -Ilibov/src

# build targets:
BUILDPLUGINS = $(patsubst %,$(BUILD_DIR)/%.so,$(PLUGINS))

# install targets:
INSTPLUGINS = $(patsubst %,$(PREFIX)/lib/%.so,$(PLUGINS))

OBJECTS = $(BUILD_DIR)/netaudio.o

modules: $(BUILDPLUGINS)

$(BUILDPLUGINS): libovserver

install: $(INSTPLUGINS)

uninstall:
	rm -f $(INSTMOD)

clean:
	rm -Rf *~ src/*~ $(BUILD_DIR)
	find -name "*.jvalid" -exec rm -f \{\} \;
	find -name "*.validated" -exec rm -f \{\} \;
	rm -Rf googlemock googletest $(BUILD_DIR)
	rm -Rf libov/build

$(PREFIX)/lib/%: $(BUILD_DIR)/%
	cp $< $@

$(BUILD_DIR)/%.o: src/%.cc $(wildcard src/*.h)
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.so: src/%.cc $(wildcard src/*.h) $(OBJECTS)
	$(CXX) -shared -fpic -o $@ $< $(OBJECTS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

%.validated: %.tsc $(BUILDPLUGINS)
	LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:$(PWD)/$(BUILD_DIR) tascar_validatetsc $< || (echo "$(<):1:";false)
	echo ok > $@

%.jvalid: %.tsc $(BUILDPLUGINS)
	LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:$(PWD)/$(BUILD_DIR) tascar_cli -v $<  || (echo "$(<):1:";false)
	echo ok > $@

test: $(TESTFILES:.tsc=.validated)

testjack:
	jackd -d dummy -C 64 -P 64 -r 44100 -p 1024 &
	sleep 1
	$(MAKE) $(TESTFILES:.tsc=.jvalid) || (killall jackd;false)
	killall jackd

gtest:
	$(MAKE) googlemock

unit-tests: gtest

googlemock: $(BUILD_DIR)/googlemock.is_installed

$(BUILD_DIR)/googlemock.is_installed: googletest/WORKSPACE \
	$(BUILD_DIR)/lib/.directory $(BUILD_DIR)/include/.directory
	echo $(CXXFLAGS)
	cd googletest/googlemock/make && $(MAKE)
	cp googletest/googlemock/make/gmock_main.a $(BUILD_DIR)/lib/libgmock_main.a
	cp -a googletest/googletest/include/gtest $(BUILD_DIR)/include/
	cp -a googletest/googlemock/include/gmock $(BUILD_DIR)/include/
	touch $@

googletest/WORKSPACE:
	git clone https://github.com/google/googletest
	(cd googletest && git checkout 31eb5e9b873af4b509be2f77616113007fa0de9d)

$(BUILD_DIR)/unit-test-runner: $(OBJECTS) $(BUILD_DIR)/.directory $(unit_tests_test_files) $(patsubst %_unit_tests.cpp, %.cpp , $(unit_tests_test_files))
	if test -n "$(unit_tests_test_files)"; then $(CXX) $(CXXFLAGS) -I$(BUILD_DIR)/include -L$(BUILD_DIR)/lib -o $@ $(wordlist 2, $(words $^), $^) $(LDFLAGS) $(LDLIBS) $(OBJECTS) -lgmock_main -lpthread; fi

.PHONY: doc

doc:
	cd doc && doxygen doxygen.cfg

libovserver:
	$(MAKE) -C libov build libovserver


# Local Variables:
# compile-command: "make"
# End:
