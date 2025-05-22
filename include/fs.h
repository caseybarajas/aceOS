#ifndef FS_H
#define FS_H

#include "libc/stdint.h"

// Maximum filename length
#define FS_MAX_FILENAME_LEN 32

// Maximum path length
#define FS_MAX_PATH_LEN 256

// Maximum number of files in a directory
#define FS_MAX_FILES_PER_DIR 64

// Maximum number of directories in the filesystem
#define FS_MAX_DIRECTORIES 32

// Maximum number of files in the filesystem
#define FS_MAX_FILES 128

// File types
typedef enum {
    FS_TYPE_FILE = 0,
    FS_TYPE_DIRECTORY = 1
} fs_entry_type_t;

// File attributes
typedef struct {
    uint8_t read_only : 1;
    uint8_t hidden : 1;
    uint8_t system : 1;
    uint8_t reserved : 5;
} fs_attributes_t;

// File entry structure
typedef struct {
    char name[FS_MAX_FILENAME_LEN];
    fs_entry_type_t type;
    fs_attributes_t attributes;
    uint32_t size;           // Size in bytes
    uint32_t data_pointer;   // Pointer to data (memory address)
    uint32_t parent_dir;     // Index of parent directory in directory table
    uint32_t creation_time;  // Simple timestamp
} fs_entry_t;

// Directory entry structure
typedef struct {
    char name[FS_MAX_FILENAME_LEN];
    uint32_t parent_dir;     // Index of parent directory (for traversal upwards)
    uint32_t file_count;     // Number of files in this directory
    uint32_t files[FS_MAX_FILES_PER_DIR]; // Indices of files in this directory
} fs_directory_t;

// Main filesystem structure
typedef struct {
    uint32_t initialized;
    uint32_t dir_count;
    uint32_t file_count;
    fs_directory_t directories[FS_MAX_DIRECTORIES];
    fs_entry_t files[FS_MAX_FILES];
} filesystem_t;

// Initialize the filesystem
void fs_init();

// Create a directory
int fs_mkdir(const char* path);

// Create a file
int fs_create(const char* path, uint32_t size);

// Delete a file or directory
int fs_delete(const char* path);

// Write data to a file
int fs_write(const char* path, const void* data, uint32_t size);

// Read data from a file
int fs_read(const char* path, void* buffer, uint32_t size);

// List contents of a directory
int fs_list_dir(const char* path, char* buffer, uint32_t buffer_size);

// Get information about a file or directory
int fs_stat(const char* path, fs_entry_t* info);

// Print filesystem statistics
void fs_print_stats();

// Find a file or directory by path
int fs_find(const char* path);

// Parse a path into components
void fs_parse_path(const char* path, char* parts[], int* part_count);

#endif // FS_H 