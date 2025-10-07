# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

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