# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

.PHONY: all
all: 
	@$(ECHO) "Building the SecApp Testcases..."
	@$(MAKE) -C ./secapp all
	@$(MAKE) -C ./app all

.PHONY: app
app: 
	@$(MAKE) -C ./app all

.PHONY: clean
clean:
	cd app && $(MAKE) -w clean
	cd secapp && $(MAKE) -w clean
	$(RM) *.o