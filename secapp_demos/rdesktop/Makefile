# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

.PHONY: all
all: 
	@$(ECHO) "Building the Remote Desktop Demo..."
	@$(MAKE) -C ./secapp all
	@$(MAKE) -C ./app all

.PHONY: app
app: 
	@$(MAKE) -C ./app all

.PHONY: first_build
first_build:
	@$(MAKE) -C ./secapp first_build

.PHONY: first_build_clean
first_build_clean:
	@$(MAKE) -C ./secapp first_build_clean

.PHONY: clean
clean:
	cd app && $(MAKE) -w clean
	cd secapp && $(MAKE) -w clean
	$(RM) *.o