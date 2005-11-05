obj :=

#opt := -O3
opt := -g
inc_flags := -Isrc -Ilibs -I/usr/local/include

CXXFLAGS := $(opt) -ansi -pedantic -Wall -fPIC $(inc_flags) `./3dengfx-config --cflags`
CFLAGS := $(opt) -std=c89 -pedantic -Wall -fPIC $(inc_flags) `./3dengfx-config --cflags`

include src/3dengfx/Makefile-part
include src/gfx/Makefile-part
include src/n3dmath2/Makefile-part
include src/dsys/Makefile-part
include src/common/Makefile-part
include src/fxwt/Makefile-part
include src/sim/Makefile-part
include libs/lib3ds/Makefile-part

.PHONY: all
all: lib3dengfx.so.0.1.0 lib3dengfx.a

.PHONY: shared
shared: lib3dengfx.so.0.1.0

.PHONY: static
static: lib3dengfx.a

lib3dengfx.so.0.1.0: 3dengfx-config $(obj)
	$(CXX) -shared -Wl,-soname,lib3dengfx.so.0 -o $@ $(obj)
	@echo '---- shared library created successfully! ----'

lib3dengfx.a: 3dengfx-config $(obj)
	$(AR) rcs $@ $(obj)
	@echo '---- static library created successfully! ----'

-include $(obj:.o=.d)

%.d: %.cpp
	@set -e; rm -f $@; $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

%.d: %.c
	@set -e; rm -f $@; $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$


.PHONY: clean
clean:
	$(RM) $(obj) lib3dengfx.so.0.1.0 lib3dengfx.a

.PHONY: cleandep
cleandep:
	find . \( -name '*.d' -o -name '*.d.*' \) -exec $(RM) '{}' ';'

.PHONY: install
install:
	inst/inst

.PHONY: remove
remove:
	inst/inst -r
