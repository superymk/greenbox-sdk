// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef TEST_NEWLIB_FILE_H
#define TEST_NEWLIB_FILE_H

extern bool test_file_create_write(void);
extern bool test_file_create_write_longdata(void);
extern bool test_file_open_read(void);
extern bool test_file_open_read_longdata(void);
extern bool test_file_open_read_more_longdata(void);

#endif // TEST_NEWLIB_FILE_H