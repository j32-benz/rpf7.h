# RPF7 Library

## Overview
The **RPF7** library is a simple C-based utility for reading and handling RPF7 archive files. It provides functions to read headers, parse file entries, and build a directory tree structure for managing directories and files within an RPF7 archive.

## Features
- Parse RPF7 archive headers and entries.
- Build a hierarchical directory tree from file and directory entries.
- Memory management functions for efficient allocation and deallocation.

## API Functions
- **`rpf7_read_header`**: Reads the RPF7 header, entries, and names data from a binary buffer.
- **`rpf7_build_directory_tree`**: Constructs a directory tree based on parsed entries.
- **`rpf7_free_directory_tree`**: Frees the memory used by the directory tree.
- **`rpf7_free_entries`**: Frees the memory allocated for file entries.
- **`rpf7_free_names`**: Frees the memory allocated for name data.

## Structures
- **`rpf7_rpf_header`**: Contains metadata about the RPF7 archive.
- **`rpf7_rpf_entry`**: Represents either a file or a directory entry in the archive.
- **`rpf7_directory_node`**: Represents a directory with pointers to subdirectories and files.
- **`rpf7_file_node`**: Represents a file with its name, size, and offset.

## To Do
- **File extraction**: Implement functionality to extract file contents based on the file's offset and size in the archive.
- **NG encryption/decryption**: Add support for handling encryption types specified in the header (NG encryption), including decryption of encrypted file contents.

## Notes
- Ensure the provided data buffer is large enough to contain the full archive before parsing.
- Directory and file entries are organized as linked lists within the directory tree.

## License
This library is released under the MIT License.
