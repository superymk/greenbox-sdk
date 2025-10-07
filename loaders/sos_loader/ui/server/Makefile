# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

CPP_SOURCES = $(wildcard *.cpp)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.cpp, %.o, $(CPP_SOURCES))

APP_INCLUDE += -I../../include
# APP_INCLUDE += -I./rpclib/include

# Remove "-Werror" because 3rd party libraries may have compile warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

.PHONY: all
all: $(OBJECTS)
	$(LD) -r -o ../ui.o $(OBJECTS)
	
%.o: %.S Makefile FORCE
	$(CXX) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.cpp Makefile FORCE
	$(CXX) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<


.PHONY: clean
clean:
	$(RMDIR) ./rpclib/build
	$(RM) *.o

# Define a phony target that always runs
.PHONY: FORCE
FORCE: