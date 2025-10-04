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