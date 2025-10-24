// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#include "utils.h"

char *url_alloc(void)
{
    void *url = NULL;

    url = smalloc(URL_MAX_SIZE);
    if (!url)
        return NULL;

    return (char *)url;
}

void url_free(char *url)
{
    if (url)
    {
        sfree(url, URL_MAX_SIZE);
    }
}