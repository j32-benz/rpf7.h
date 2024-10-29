#ifndef RPF7_H
#define RPF7_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

#pragma pack(push, 1)

typedef struct {
    uint32_t version;
    uint32_t entry_count;
    uint32_t names_data_size;
    uint32_t encryption_type;
} rpf7_rpf_header;

typedef struct {
    uint16_t name_offset;
    uint8_t  flags[3];
    uint8_t  offset[3];
    union {
        struct {
            uint32_t size;
        } file;
        struct {
            uint32_t first_entry_index;
            uint32_t entry_count;
        } directory;
    };
} rpf7_rpf_entry;

#pragma pack(pop)

typedef struct rpf7_file_node {
    char* name;
    uint32_t size;
    uint32_t offset;
    struct rpf7_file_node* next;
} rpf7_file_node;

typedef struct rpf7_directory_node {
    char* name;
    struct rpf7_directory_node* subdirs;
    struct rpf7_file_node* files;
    struct rpf7_directory_node* next;
} rpf7_directory_node;

static uint32_t
read_u24_le(const uint8_t bytes[3])
{
    return ((uint32_t)bytes[0]) |
        ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16);
}

static int
rpf7_is_directory(const rpf7_rpf_entry* entry)
{
    return (read_u24_le(entry->offset) == 0x7FFFFF);
}

static int
rpf7_read_header(const uint8_t* data, size_t data_size, rpf7_rpf_header* header, rpf7_rpf_entry** entries_out, char** names_out)
{
    if (data_size < sizeof(rpf7_rpf_header)) {
        fprintf(stderr, "Data is too small to contain an RPF7 header\n");
        return -1;
    }

    memcpy(header, data, sizeof(rpf7_rpf_header));

    if (header->version != 0x52504637) {
        fprintf(stderr, "Invalid RPF7 Version: expected 0x52504637, got 0x%08X\n", header->version);
        return -1;
    }

    size_t required_size = sizeof(rpf7_rpf_header) + header->entry_count * sizeof(rpf7_rpf_entry) + header->names_data_size;
    if (data_size < required_size) {
        fprintf(stderr, "Data is too small to contain the required entries and names\n");
        return -1;
    }

    rpf7_rpf_entry* entries = malloc(header->entry_count * sizeof(rpf7_rpf_entry));
    if (!entries) {
        perror("Failed to allocate memory for entries");
        return -1;
    }

    const uint8_t* entries_data = data + sizeof(rpf7_rpf_header);
    memcpy(entries, entries_data, header->entry_count * sizeof(rpf7_rpf_entry));

    char* names_buffer = malloc(header->names_data_size);
    if (!names_buffer) {
        perror("Failed to allocate memory for names");
        free(entries);
        return -1;
    }

    const uint8_t* names_data = entries_data + header->entry_count * sizeof(rpf7_rpf_entry);
    memcpy(names_buffer, names_data, header->names_data_size);

    *entries_out = entries;
    *names_out = names_buffer;

    return 0;
}

rpf7_directory_node*
rpf7_build_directory_tree(rpf7_rpf_entry* entries, char* names_buffer, uint32_t index)
{
    rpf7_rpf_entry* directory_entry = &entries[index];
    if (!rpf7_is_directory(directory_entry)) {
        return NULL;
    }

    rpf7_directory_node* directory = malloc(sizeof(rpf7_directory_node));
    if (!directory) return NULL;

    directory->name = strdup(names_buffer + directory_entry->name_offset);
    directory->subdirs = NULL;
    directory->files = NULL;
    directory->next = NULL;

    uint32_t first_entry_index = directory_entry->directory.first_entry_index;
    uint32_t entry_count = directory_entry->directory.entry_count;

    rpf7_directory_node* last_subdir = NULL;
    rpf7_file_node* last_file = NULL;

    for (uint32_t i = first_entry_index; i < first_entry_index + entry_count; i++) {
        rpf7_rpf_entry* entry = &entries[i];
        char* name = names_buffer + entry->name_offset;

        if (rpf7_is_directory(entry)) {
            rpf7_directory_node* subdir = rpf7_build_directory_tree(entries, names_buffer, i);
            if (!subdir) continue;

            if (last_subdir) {
                last_subdir->next = subdir;
            }
            else {
                directory->subdirs = subdir;
            }
            last_subdir = subdir;
        }
        else {
            rpf7_file_node* file = malloc(sizeof(rpf7_file_node));
            if (!file) continue;

            file->name = strdup(name);
            file->size = entry->file.size;
            file->offset = read_u24_le(entry->offset);
            file->next = NULL;

            if (last_file) {
                last_file->next = file;
            }
            else {
                directory->files = file;
            }
            last_file = file;
        }
    }

    return directory;
}

void
rpf7_free_directory_tree(rpf7_directory_node* dir)
{
    if (!dir) return;

    rpf7_directory_node* subdir = dir->subdirs;
    while (subdir) {
        rpf7_directory_node* next_subdir = subdir->next;
        rpf7_free_directory_tree(subdir);
        subdir = next_subdir;
    }

    rpf7_file_node* file = dir->files;
    while (file) {
        rpf7_file_node* next_file = file->next;
        free(file->name);
        free(file);
        file = next_file;
    }

    free(dir->name);
    free(dir);
}

void
rpf7_free_entries(rpf7_rpf_entry* entries)
{
    if (entries) {
        free(entries);
    }
}

void
rpf7_free_names(char* names)
{
    if (names) {
        free(names);
    }
}

#endif // RPF7_H
