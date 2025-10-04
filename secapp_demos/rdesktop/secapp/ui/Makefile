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