
CFLAGSW_GCC = -Wall -Wextra -Wno-missing-field-initializers \
    -Wno-parentheses -Wno-missing-braces \
    -Wmissing-prototypes -Wfloat-equal \
    -Wwrite-strings -Wpointer-arith -Wcast-align \
    -Wnull-dereference \
    -Werror=multichar -Werror=sizeof-pointer-memaccess -Werror=return-type \
    -fstrict-aliasing
CXXFLAGSW_GCC = -Wall -Wextra -Wno-missing-field-initializers \
    -Wno-parentheses -Wno-missing-braces \
    -Wno-unused-local-typedefs \
    -Wfloat-equal \
    -Wpointer-arith -Wcast-align \
    -Wnull-dereference \
    -Wnon-virtual-dtor -Wmissing-declarations \
    -Werror=multichar -Werror=sizeof-pointer-memaccess -Werror=return-type \
    -Werror=delete-non-virtual-dtor \
    -fstrict-aliasing

#CXXFLAGSO = -Og -fsanitize=address -g
#CXXFLAGSO = -O2
CXXFLAGSO = -g

#CFLAGSO = -g -fsanitize=address
CFLAGSO = -g
#CFLAGSO = -O3 -march=native

#LDFLAGS = -g -fsanitize=address

CXXFLAGS += $(CXXFLAGSW_GCC)
CXXFLAGS += $(CXXFLAGSO)

CFLAGS += $(CFLAGSW_GCC) $(CFLAGSO)

CPPFLAGS = -Ifastmod/include

PYBIND11_CPPFLAGS := $(shell python3 -m pybind11 --includes)
PY_EXT_SUFFIX     := $(shell python3-config --extension-suffix)


.PHONY: all
all: divide$(PY_EXT_SUFFIX) fastmod$(PY_EXT_SUFFIX)


divide$(PY_EXT_SUFFIX): pydivide.cc
	$(CXX) $(PYBIND11_CPPFLAGS) $(CXXFLAGS) -shared -fPIC $< -o $@

TEMP += divide$(PY_EXT_SUFFIX)

fastmod$(PY_EXT_SUFFIX): pyfastmod.cc
	$(CXX) $(PYBIND11_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -shared -fPIC $< -o $@

TEMP += fastmod$(PY_EXT_SUFFIX)

test_hash_table: test_hash_table.o phash_table.o instrument.o

TEMP += test_hash_table test_hash_table.o phash_table.o instrument.o


testxx: testxx.o phash_table.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

TEMP += testxx testxx.o phash_table.o

.PHONY: clean
clean:
	rm -rf $(TEMP)
