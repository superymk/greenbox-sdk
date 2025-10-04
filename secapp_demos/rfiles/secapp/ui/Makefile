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
all: rfiles_ui
	$(LD) -r -o lib-ui.o ./rfiles_ui/lib-rfiles-ui.o

.PHONY: rfiles_ui
rfiles_ui: 
	@$(MAKE) -C ./rfiles_ui all

.PHONY: clean
clean:
	cd rfiles_ui && $(MAKE) -w clean
	$(RM) *.o