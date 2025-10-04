# /*
#  * -------------------------------------------------------------------------------------
#  * Carnegie Mellon University
#  * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
#  * 
#  * This software is the proprietary information of Carnegie Mellon University.
#  * Use is subject to license terms.
#  * 
#  * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
#  * written consent of Carnegie Mellon University. is strictly prohibited.
#  * -------------------------------------------------------------------------------------
#  * 
#  * Authors: Miao Yu
#  *          Virgil Gligor
#  */

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