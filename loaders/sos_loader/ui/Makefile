# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

CPP_SOURCES = $(wildcard *.cpp)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.c, %.o, $(CPP_SOURCES))



APP_INCLUDE += -I../include

.PHONY: all
ifeq ($(CONFIG_SOSLOADER_GUI), y)
# all: gui
all: server
else
all: console
endif
	
.PHONY: console
console:
	@$(MAKE) -C ./console
	
.PHONY: gui
gui:
	@$(MAKE) -C ./gui

.PHONY: server
server:
	@$(MAKE) -C ./server


.PHONY: clean
clean:
	cd console && $(MAKE) -w clean
	cd server && $(MAKE) -w clean
	$(RM) *.o