/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 *
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 *
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 *
 * Authors: Miao Yu
 *          Virgil Gligor
 */

#ifndef RFILES_PROTOCOLS_H
#define RFILES_PROTOCOLS_H

#include <sap_headers.h>
#include <palddk.h>

#ifndef __ASSEMBLY__

typedef struct {
    char *id;               /* backend id */
    char *name;             /* display name */
    char *modified_time;    /* "YYYY-MM-DD HH:MM" (or empty) */
    unsigned long long size;/* 0 for folders */
    int   is_folder;        /* non-zero if folder */
} gdrive_entry_t;

/// @brief Return structured children of a folder id ("root" ok).
/// If parent_id == "..", we simply go to "root" (simple UX); you can enhance this later with a parent stack.
/// @param parent_id 
/// @param out 
/// @param out_n 
/// @return 
extern int gdrive_list_children(const char *parent_id, gdrive_entry_t **out, size_t *out_n);

/// @brief Free entries returned by the <gdrive_list_children> function.
extern void gdrive_entries_free(gdrive_entry_t *arr, size_t n);

/// @brief Download a file or folder (recursive) to local_dir.
/// @param id 
/// @param local_dir 
/// @return 
extern int gdrive_download_to(const char *id, const char *local_dir);

/// @brief Upload one local file to a parent folder id.
/// @param local_path 
/// @param parent_id 
/// @return 
extern int gdrive_upload_one(const char *local_path, const char *parent_id);

/// @brief Resolve id -> name (useful for breadcrumbs).
/// @param id 
/// @param buf 
/// @param bufsz 
/// @return 
extern int gdrive_id_to_name(const char *id, char *buf, size_t bufsz);

/// @brief Delete a single Drive item (file or folder) by id.
/// Notes:
/// - Requires that the authorized account has permission to delete the item.
/// - With drive.file scope you can delete files your app created or that the user
///   explicitly opened/authorized for your app.
/// - For shared drives, we pass supportsAllDrives=true.
/// @param id 
/// @return 
extern int gdrive_delete_one(const char *id);

#endif // __ASSEMBLY__
#endif // RFILES_PROTOCOLS_H