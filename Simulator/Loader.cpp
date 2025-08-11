#include "ArgsParser.h"
#include "Loader.h"
#include <iostream>
#include <fstream>
#include <dirent.h> // For directory operations
#include <dlfcn.h>  // For dynamic loading of shared 
#include <iostream> // For input/output operations
#include <cstring>

int HasSOExtension(const char *filename) {
    // This function checks if a filename has a .so extension.
    const char *ext = strrchr(filename, '.'); // Find the last occurrence of '.'
    return ext && strcmp(ext, ".so") == 0; // Check if the extension is .so
}

int LoadSharedObjects(const ParsedArgs& args) {
    // This function loads shared objects based on the parsed arguments and returns an exit code.
    // Determine the directory to load shared objects from based on the mode.
    std::string so_dir;
    if (args.mode == ParsedArgs::Mode::Comparative)
        so_dir = args.game_managers_folder;
    else if (args.mode == ParsedArgs::Mode::Competition)
        so_dir = args.algorithms_folder;
    else {
        std::cerr << "Unsupported mode for loading shared objects." << std::endl;
        return EXIT_FAILURE;
    }
    DIR *dir = opendir(so_dir.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << so_dir << std::endl;
        return EXIT_FAILURE;
    }
    int result = LoadSOFilesFromDir(dir, so_dir);
    closedir(dir);
    return result;
}

int LoadSOFilesFromDir(DIR* dir, const std::string& so_dir) {
    // This function iterates through the directory and loads each .so file.
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG && entry->d_type != DT_LNK)
            continue; // Skip non-regular files
        if (!HasSOExtension(entry->d_name)) // Skip files without .so extension
            continue;
        std::string so_path = so_dir + "/" + entry->d_name; // Construct the full path to the shared object file
        void *handle = dlopen(so_path.c_str(), RTLD_LAZY);  // Load the shared object file
        if (!handle) {
            std::cerr << "dlopen failed for " << so_path << ": " << dlerror() << std::endl;
            continue;
        }
        run_func_t run_func = (run_func_t)dlsym(handle, "run"); // Get the address of the "run" function
        // TODO: should not use dlsym!
        char *error = dlerror();
        if (error != NULL) {
            std::cerr << "dlsym failed for " << so_path << ": " << error << std::endl;
            dlclose(handle);
            continue;
        }
        run_func(); // Call the run function from the shared object
        dlclose(handle); // Close the shared object handle
    }
    return EXIT_SUCCESS;
}
