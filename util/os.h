#pragma once

#include <sys/stat.h>

/**
 * @brief Recursive directory creation
 * 
 * @param dir path of directory
 * @param mode permissions, 0 for default (RWX for owner, RX for others)
 * 
 * @return 0 if success, -1 otherwise (see errno)
 */
int mkdirs(char *dir, __mode_t mode);

/**
 * @brief Get parent dir
 * 
 * @param path
 * 
 * @return pointer to parent_dir string if success, NULL otherwise
 */
char* dirname(char* path);