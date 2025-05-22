#include "../include/fs.h"
#include "../include/libc/string.h"
#include "../include/libc/stdio.h"
#include "../include/libc/stdlib.h"
#include "../include/serial.h"

// Global filesystem instance
static filesystem_t fs;

// Custom strtok implementation
char* fs_strtok(char* str, const char* delim) {
    static char* last_token = NULL;
    char* token_start;
    char* token_end;
    
    // If str is NULL, use the last position
    if (str == NULL) {
        if (last_token == NULL) {
            return NULL;
        }
        str = last_token;
    }
    
    // Skip leading delimiters
    while (*str && strchr(delim, *str)) {
        str++;
    }
    
    // If we reached the end, return NULL
    if (*str == '\0') {
        last_token = NULL;
        return NULL;
    }
    
    // This is the start of the token
    token_start = str;
    
    // Find the end of the token
    token_end = str;
    while (*token_end && !strchr(delim, *token_end)) {
        token_end++;
    }
    
    // If we found a delimiter, replace it with null and set last_token to the next character
    if (*token_end) {
        *token_end = '\0';
        last_token = token_end + 1;
    } else {
        // End of string
        last_token = NULL;
    }
    
    return token_start;
}

// Initialize the memory-based file system
void fs_init() {
    debug_println("Initializing filesystem...");
    
    // Clear all structures
    memset(&fs, 0, sizeof(filesystem_t));
    
    // Create root directory
    strcpy(fs.directories[0].name, "/");
    fs.directories[0].parent_dir = 0; // Root is its own parent
    fs.dir_count = 1;
    
    fs.initialized = 1;
    debug_println("Filesystem initialized successfully");
}

// Helper function to split a path into parts
void fs_parse_path(const char* path, char* parts[], int* part_count) {
    char path_copy[FS_MAX_PATH_LEN];
    strncpy(path_copy, path, FS_MAX_PATH_LEN);
    
    char* token = fs_strtok(path_copy, "/");
    *part_count = 0;
    
    while (token != NULL && *part_count < FS_MAX_PATH_LEN) {
        parts[*part_count] = token;
        (*part_count)++;
        token = fs_strtok(NULL, "/");
    }
}

// Find the parent directory index given a path
int fs_find_parent_dir(const char* path) {
    char* parts[FS_MAX_PATH_LEN];
    int part_count = 0;
    int current_dir = 0; // Start with root directory
    
    // Handle empty path or root path
    if (path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        return current_dir;
    }
    
    // Make a copy of the path to tokenize
    char path_copy[FS_MAX_PATH_LEN];
    strncpy(path_copy, path, FS_MAX_PATH_LEN);
    path_copy[FS_MAX_PATH_LEN - 1] = '\0';
    
    // Parse the path
    fs_parse_path(path_copy, parts, &part_count);
    
    // Find the parent directory by traversing the path
    // We stop at part_count - 1 because the last part is the name of the file/dir we're looking for
    for (int i = 0; i < part_count - 1; i++) {
        int found = 0;
        
        // Search for the directory in the current directory
        for (uint32_t j = 0; j < fs.directories[current_dir].file_count; j++) {
            uint32_t file_idx = fs.directories[current_dir].files[j];
            
            // Check if this entry is a directory and matches the name
            if (fs.files[file_idx].type == FS_TYPE_DIRECTORY && 
                strcmp(fs.files[file_idx].name, parts[i]) == 0) {
                
                // Find the actual directory index for this entry
                for (uint32_t k = 0; k < fs.dir_count; k++) {
                    if (strcmp(fs.directories[k].name, parts[i]) == 0 && 
                        fs.directories[k].parent_dir == current_dir) {
                        current_dir = k;
                        found = 1;
                        break;
                    }
                }
                
                if (found) break;
            }
        }
        
        if (!found) {
            return -1; // Directory not found
        }
    }
    
    return current_dir;
}

// Extract filename from a path
void fs_get_filename(const char* path, char* filename) {
    char* parts[FS_MAX_PATH_LEN];
    int part_count = 0;
    
    // Parse the path
    fs_parse_path(path, parts, &part_count);
    
    // The last part is the filename
    if (part_count > 0) {
        strncpy(filename, parts[part_count - 1], FS_MAX_FILENAME_LEN);
        filename[FS_MAX_FILENAME_LEN - 1] = '\0';
    } else {
        filename[0] = '\0';
    }
}

// Find a file or directory by path
int fs_find(const char* path) {
    // Handle root directory
    if (path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        // Return the index of the root directory's entry (special case)
        return -2; // Special value for root directory
    }
    
    int parent_dir = fs_find_parent_dir(path);
    if (parent_dir < 0) {
        return -1; // Parent directory not found
    }
    
    // Get the filename/dirname from the path
    char name[FS_MAX_FILENAME_LEN];
    fs_get_filename(path, name);
    
    // Search for the file/directory in the parent directory
    for (uint32_t i = 0; i < fs.directories[parent_dir].file_count; i++) {
        uint32_t file_idx = fs.directories[parent_dir].files[i];
        
        if (strcmp(fs.files[file_idx].name, name) == 0) {
            return file_idx;
        }
    }
    
    return -1; // Not found
}

// Create a directory
int fs_mkdir(const char* path) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Check if directory already exists
    int existing = fs_find(path);
    if (existing >= 0) {
        debug_println("Directory already exists");
        return -1;
    }
    
    // Find parent directory
    int parent_dir = fs_find_parent_dir(path);
    if (parent_dir < 0) {
        debug_println("Parent directory not found");
        return -1;
    }
    
    // Check if we've reached the maximum number of directories
    if (fs.dir_count >= FS_MAX_DIRECTORIES) {
        debug_println("Maximum number of directories reached");
        return -1;
    }
    
    // Check if parent directory has reached maximum number of files
    if (fs.directories[parent_dir].file_count >= FS_MAX_FILES_PER_DIR) {
        debug_println("Parent directory is full");
        return -1;
    }
    
    // Get directory name from path
    char dir_name[FS_MAX_FILENAME_LEN];
    fs_get_filename(path, dir_name);
    
    // Create file entry for the directory
    uint32_t file_idx = fs.file_count++;
    strncpy(fs.files[file_idx].name, dir_name, FS_MAX_FILENAME_LEN);
    fs.files[file_idx].type = FS_TYPE_DIRECTORY;
    fs.files[file_idx].size = 0;
    fs.files[file_idx].parent_dir = parent_dir;
    fs.files[file_idx].creation_time = 0; // TODO: Implement a real timestamp
    
    // Add the file entry to the parent directory
    fs.directories[parent_dir].files[fs.directories[parent_dir].file_count++] = file_idx;
    
    // Create directory entry
    uint32_t dir_idx = fs.dir_count++;
    strncpy(fs.directories[dir_idx].name, dir_name, FS_MAX_FILENAME_LEN);
    fs.directories[dir_idx].parent_dir = parent_dir;
    
    debug_println("Directory created successfully");
    return 0;
}

// Create a file
int fs_create(const char* path, uint32_t size) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Check if file already exists
    int existing = fs_find(path);
    if (existing >= 0) {
        debug_println("File already exists");
        return -1;
    }
    
    // Find parent directory
    int parent_dir = fs_find_parent_dir(path);
    if (parent_dir < 0) {
        debug_println("Parent directory not found");
        return -1;
    }
    
    // Check if we've reached the maximum number of files
    if (fs.file_count >= FS_MAX_FILES) {
        debug_println("Maximum number of files reached");
        return -1;
    }
    
    // Check if parent directory has reached maximum number of files
    if (fs.directories[parent_dir].file_count >= FS_MAX_FILES_PER_DIR) {
        debug_println("Parent directory is full");
        return -1;
    }
    
    // Get filename from path
    char filename[FS_MAX_FILENAME_LEN];
    fs_get_filename(path, filename);
    
    // Allocate memory for file data
    void* data_ptr = NULL;
    if (size > 0) {
        data_ptr = malloc(size);
        if (data_ptr == NULL) {
            debug_println("Failed to allocate memory for file");
            return -1;
        }
    }
    
    // Create file entry
    uint32_t file_idx = fs.file_count++;
    strncpy(fs.files[file_idx].name, filename, FS_MAX_FILENAME_LEN);
    fs.files[file_idx].type = FS_TYPE_FILE;
    fs.files[file_idx].size = size;
    fs.files[file_idx].data_pointer = (uint32_t)data_ptr;
    fs.files[file_idx].parent_dir = parent_dir;
    fs.files[file_idx].creation_time = 0; // TODO: Implement a real timestamp
    
    // Add the file entry to the parent directory
    fs.directories[parent_dir].files[fs.directories[parent_dir].file_count++] = file_idx;
    
    debug_println("File created successfully");
    return 0;
}

// Delete a file or directory
int fs_delete(const char* path) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Cannot delete root directory
    if (path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        debug_println("Cannot delete root directory");
        return -1;
    }
    
    // Find the file/directory
    int file_idx = fs_find(path);
    if (file_idx < 0) {
        debug_println("File or directory not found");
        return -1;
    }
    
    // Get parent directory
    uint32_t parent_dir = fs.files[file_idx].parent_dir;
    
    // If it's a directory, make sure it's empty
    if (fs.files[file_idx].type == FS_TYPE_DIRECTORY) {
        // Find the directory entry
        uint32_t dir_idx = 0;
        int found = 0;
        
        for (uint32_t i = 0; i < fs.dir_count; i++) {
            if (strcmp(fs.directories[i].name, fs.files[file_idx].name) == 0 && 
                fs.directories[i].parent_dir == parent_dir) {
                dir_idx = i;
                found = 1;
                break;
            }
        }
        
        if (!found) {
            debug_println("Directory entry not found");
            return -1;
        }
        
        // Check if directory is empty
        if (fs.directories[dir_idx].file_count > 0) {
            debug_println("Cannot delete non-empty directory");
            return -1;
        }
        
        // Remove directory entry
        // We'll just mark it as unused by setting its name to an empty string
        fs.directories[dir_idx].name[0] = '\0';
    } else {
        // Free file data memory
        if (fs.files[file_idx].data_pointer != 0) {
            free((void*)fs.files[file_idx].data_pointer);
        }
    }
    
    // Find and remove the file from the parent directory's file list
    for (uint32_t i = 0; i < fs.directories[parent_dir].file_count; i++) {
        if (fs.directories[parent_dir].files[i] == file_idx) {
            // Shift remaining files to fill the gap
            for (uint32_t j = i; j < fs.directories[parent_dir].file_count - 1; j++) {
                fs.directories[parent_dir].files[j] = fs.directories[parent_dir].files[j + 1];
            }
            fs.directories[parent_dir].file_count--;
            break;
        }
    }
    
    // Mark file entry as unused
    fs.files[file_idx].name[0] = '\0';
    
    debug_println("File or directory deleted successfully");
    return 0;
}

// Write data to a file
int fs_write(const char* path, const void* data, uint32_t size) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Find the file
    int file_idx = fs_find(path);
    if (file_idx < 0) {
        debug_println("File not found");
        return -1;
    }
    
    // Check if it's a file
    if (fs.files[file_idx].type != FS_TYPE_FILE) {
        debug_println("Not a file");
        return -1;
    }
    
    // If the file already has data and the size is different, free the old data
    if (fs.files[file_idx].data_pointer != 0 && fs.files[file_idx].size != size) {
        free((void*)fs.files[file_idx].data_pointer);
        fs.files[file_idx].data_pointer = 0;
    }
    
    // Allocate new memory if needed
    if (fs.files[file_idx].data_pointer == 0) {
        void* data_ptr = malloc(size);
        if (data_ptr == NULL) {
            debug_println("Failed to allocate memory for file");
            return -1;
        }
        fs.files[file_idx].data_pointer = (uint32_t)data_ptr;
    }
    
    // Copy data to file
    memcpy((void*)fs.files[file_idx].data_pointer, data, size);
    fs.files[file_idx].size = size;
    
    debug_println("File written successfully");
    return 0;
}

// Read data from a file
int fs_read(const char* path, void* buffer, uint32_t size) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Find the file
    int file_idx = fs_find(path);
    if (file_idx < 0) {
        debug_println("File not found");
        return -1;
    }
    
    // Check if it's a file
    if (fs.files[file_idx].type != FS_TYPE_FILE) {
        debug_println("Not a file");
        return -1;
    }
    
    // Check if file has data
    if (fs.files[file_idx].data_pointer == 0) {
        debug_println("File has no data");
        return 0;
    }
    
    // Check size
    uint32_t bytes_to_read = size;
    if (bytes_to_read > fs.files[file_idx].size) {
        bytes_to_read = fs.files[file_idx].size;
    }
    
    // Copy data from file to buffer
    memcpy(buffer, (void*)fs.files[file_idx].data_pointer, bytes_to_read);
    
    return bytes_to_read;
}

// List contents of a directory
int fs_list_dir(const char* path, char* buffer, uint32_t buffer_size) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Handle root directory specially
    uint32_t dir_idx = 0;
    
    if (!(path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0'))) {
        // Find the directory
        int file_idx = fs_find(path);
        if (file_idx < 0) {
            debug_println("Directory not found");
            return -1;
        }
        
        // Check if it's a directory
        if (fs.files[file_idx].type != FS_TYPE_DIRECTORY) {
            debug_println("Not a directory");
            return -1;
        }
        
        // Find the directory entry
        int found = 0;
        for (uint32_t i = 0; i < fs.dir_count; i++) {
            if (strcmp(fs.directories[i].name, fs.files[file_idx].name) == 0 && 
                fs.directories[i].parent_dir == fs.files[file_idx].parent_dir) {
                dir_idx = i;
                found = 1;
                break;
            }
        }
        
        if (!found) {
            debug_println("Directory entry not found");
            return -1;
        }
    }
    
    // Clear the buffer
    buffer[0] = '\0';
    
    // List all files in the directory
    uint32_t offset = 0;
    for (uint32_t i = 0; i < fs.directories[dir_idx].file_count; i++) {
        uint32_t file_idx = fs.directories[dir_idx].files[i];
        
        // Get type string
        const char* type_str = (fs.files[file_idx].type == FS_TYPE_DIRECTORY) ? "DIR" : "FILE";
        
        // Format: "name (TYPE) size: xxx bytes\n"
        int written = snprintf(buffer + offset, buffer_size - offset, 
                              "%s (%s) size: %d bytes\n", 
                              fs.files[file_idx].name, 
                              type_str, 
                              fs.files[file_idx].size);
        
        if (written < 0 || (uint32_t)written >= buffer_size - offset) {
            // Buffer is full
            break;
        }
        
        offset += written;
    }
    
    return offset;
}

// Get information about a file or directory
int fs_stat(const char* path, fs_entry_t* info) {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return -1;
    }
    
    // Special case for root directory
    if (path == NULL || path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        strcpy(info->name, "/");
        info->type = FS_TYPE_DIRECTORY;
        info->size = 0;
        info->data_pointer = 0;
        info->parent_dir = 0;
        info->creation_time = 0;
        return 0;
    }
    
    // Find the file or directory
    int file_idx = fs_find(path);
    if (file_idx < 0) {
        debug_println("File or directory not found");
        return -1;
    }
    
    // Copy information
    memcpy(info, &fs.files[file_idx], sizeof(fs_entry_t));
    
    return 0;
}

// Print filesystem statistics
void fs_print_stats() {
    if (!fs.initialized) {
        debug_println("Filesystem not initialized");
        return;
    }
    
    debug_print("Filesystem statistics:\n");
    debug_print("  Directories: ");
    serial_write_dec(fs.dir_count);
    debug_print("/");
    serial_write_dec(FS_MAX_DIRECTORIES);
    debug_print("\n");
    
    debug_print("  Files: ");
    serial_write_dec(fs.file_count);
    debug_print("/");
    serial_write_dec(FS_MAX_FILES);
    debug_print("\n");
    
    // Calculate total size of all files
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < fs.file_count; i++) {
        if (fs.files[i].name[0] != '\0' && fs.files[i].type == FS_TYPE_FILE) {
            total_size += fs.files[i].size;
        }
    }
    
    debug_print("  Total file size: ");
    serial_write_dec(total_size);
    debug_print(" bytes\n");
} 