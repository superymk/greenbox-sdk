# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

CPP_SOURCES = $(wildcard *.cpp)
AS_SOURCES = $(wildcard *.S)
OBJECTS = $(patsubst %.S, %.o, $(AS_SOURCES))
OBJECTS += $(patsubst %.cpp, %.o, $(CPP_SOURCES))

APP_INCLUDE += -I$(SECOS_LOADER_BASEDIR)/include -I$(SECBASE_PUBLIC_HEADERS) -I$(SECAGENT_PUBLIC_HEADERS) -I$(SECOS_PUBLIC_HEADERS) -I$(APP_HEADERS)
APP_INCLUDE += -I$(SECOS_LOADER_BASEDIR)/ui/server/rpclib

SL_LFLAGS = -pthread

# Additional SL_LFLAGS for wxWidgets
ifeq ($(CONFIG_SOSLOADER_GUI), y)
	ifeq ($(CONFIG_OS_LINUX), y)
		WX_CPP_FLAGS := $(shell echo `wx-config --cxxflags`)
		WX_LIBS_FLAGS := $(shell echo `wx-config --libs`)
	endif
	SL_LFLAGS += $(WX_LIBS_FLAGS)
endif

# Additional SL_LFLAGS for LibUSB
ifeq ($(CONFIG_SOSLOADER_GUI), y)
	SL_LFLAGS += $(shell echo `pkg-config libusb-1.0 --libs`)

	ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
		SL_LFLAGS += -L/ucrt64/lib
	endif
endif

# .PHONY: all
# ifeq ($(CONFIG_SOSLOADER_GUI), y)
# all: UI cpp_link
# else
# all: UI gcc_link
# endif
	
# .PHONY: gcc_link
# gcc_link:
# 	$(CC) -o $(DIST_SOSLOADER_GUI_PATH)/sos_loader_client $(OBJECTS) lib-gui.o $(SL_LFLAGS)
# 	@$(CP) -f ./sos_conf.json $(DIST_SOS_PATH)/
	
# .PHONY: cpp_link
# cpp_link:
# ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
# 	# $(CXX) -o $(DIST_SOSLOADER_GUI_PATH)/sos_loader_client $(OBJECTS) lib-gui.o $(SL_LFLAGS)
# 	# 
# 	@$(CP) -f ./sos_conf.json $(DIST_SOS_PATH)/
# else
# 	$(CXX) -o $(DIST_SOSLOADER_GUI_PATH)/sos_loader_client $(OBJECTS) lib-gui.o $(SL_LFLAGS)
# 	@$(CP) -f ./sos_conf.json $(DIST_SOS_PATH)/
# endif

.PHONY: all
ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
all: 
else
all: UI_linux cpp_link_linux
endif

.PHONY: UI_linux
UI_linux:
ifeq ($(CONFIG_OS_LINUX), y)
	@$(MAKE) -C ./linux
endif

.PHONY: cpp_link_linux
cpp_link_linux:
	$(CXX) -o $(DIST_SOSLOADER_GUI_PATH)/sos_loader_client $(OBJECTS) lib-gui.o $(SL_LFLAGS)
	$(STRIP) $(DIST_SOSLOADER_GUI_PATH)/sos_loader_client
	$(CXX) -o $(DIST_PRIV_SOSLOADER_GUI_PATH)/sos_loader_client $(OBJECTS) lib-gui.o $(SL_LFLAGS)


%.o: %.S Makefile FORCE
	$(CXX) -c $(APP_INCLUDE) $(CFLAGS) -D__ASSEMBLY__ -o $@ $<
%.o: %.cpp Makefile FORCE
	$(CXX) -c $(APP_INCLUDE) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	cd cJSON && $(MAKE) -w clean
	cd loader && $(MAKE) -w clean
	cd linux && $(MAKE) -w clean
	$(RM) *.o sos_loader_client

# Define a phony target that always runs
.PHONY: FORCE
FORCE: