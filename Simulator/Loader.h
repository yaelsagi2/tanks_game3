#pragma once

#include "ArgsParser.h"
#include <dirent.h>

/**
 * @file Loader.h
 * @brief Declares functions for loading and executing shared object (.so) files.
 */

/**
 * @brief Checks if a filename has a .so extension.
 * @param filename The filename to check.
 * @return Non-zero if the extension is .so, zero otherwise.
 */
int HasSOExtension(const char *filename);

/**
 * @brief Loads shared objects from the directory specified in ParsedArgs and executes their run function.
 * @param args ParsedArgs structure containing mode and relevant directories.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int LoadSharedObjects(const ParsedArgs& args);

/**
 * @brief Loads and executes all .so files in the given directory.
 * @param dir Pointer to opened DIR structure.
 * @param so_dir Directory path as a string.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int LoadSOFilesFromDir(DIR* dir, const std::string& so_dir);

/**
 * @brief Function pointer type for the run function in shared objects.
 */
typedef void (*run_func_t)(void);



