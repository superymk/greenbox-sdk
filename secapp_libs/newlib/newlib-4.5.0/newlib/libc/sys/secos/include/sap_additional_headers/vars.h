// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SECAPP_VAR_H
#define SECAPP_VAR_H

#ifndef __ASSEMBLY__

// Secure foreach
// we should use it as much as possible for every for statement
#define FOREACH_S(var_name, m1, m2, initial, step) for(var_name = (initial); (var_name < (m1)) && (var_name < (m2)); var_name += (step))

#endif // __ASSEMBLY__
#endif // SECAPP_VAR_H