# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# Apps related headers
APP_INCLUDE = -I$(SECOS_PUBLIC_HEADERS)

export APP_INCLUDE

.PHONY: all
ifeq ($(CONFIG_SOSLOADER_GUI), y)
all: sp_loader sos_loader sos_loader_server_win_wrapper sos_loader_gui_client
else
all: sp_loader sos_loader sos_loader_server_win_wrapper
endif

.PHONY: sos_loader
sos_loader:
	@$(MAKE) -C $(SECOS_LOADER_BASEDIR) all
	
.PHONY: sp_loader
sp_loader:
	@$(MAKE) -C $(SAP_LOADER_BASEDIR) all
	
.PHONY: sos_loader_gui_client
sos_loader_gui_client:
	@$(MAKE) -C ./sos_loader_gui_client all

.PHONY: sos_loader_server_win_wrapper
sos_loader_server_win_wrapper:
ifeq ($(CONFIG_OS_WINDOWS_MSYS2), y)
	@$(MAKE) -C ./sos_loader_server_win_wrapper clean
	@$(MAKE) -C ./sos_loader_server_win_wrapper all
endif

.PHONY: clean
clean:
	cd $(SAP_LOADER_BASEDIR) && $(MAKE) -w clean
	cd $(SECOS_LOADER_BASEDIR) && $(MAKE) -w clean
	cd sos_loader_server_win_wrapper && $(MAKE) -w clean
	cd sos_loader_gui_client && $(MAKE) -w clean
	#cd $(SECOS_SERVICES_LOADER_BASEDIR) && $(MAKE) -w clean