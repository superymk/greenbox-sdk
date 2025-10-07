# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

PAL_INCLUDE += -I$(LIBGUI_BASEDIR)

.PHONY: all
all: other_ui rdesktop_ui ui_control
	$(LD) -r -o lib-ui.o ./other_ui/other-ui.o ./rdesktop_ui/lib-rdesktop-ui.o ./ui_control/lib-ui-control.o

.PHONY: other_ui
other_ui: 
	@$(MAKE) -C ./other_ui all

.PHONY: rdesktop_ui
rdesktop_ui: 
	@$(MAKE) -C ./rdesktop_ui all

.PHONY: ui_control
ui_control: 
	@$(MAKE) -C ./ui_control all

.PHONY: clean
clean:
	cd other_ui && $(MAKE) -w clean
	cd rdesktop_ui && $(MAKE) -w clean
	cd ui_control && $(MAKE) -w clean
	$(RM) *.o