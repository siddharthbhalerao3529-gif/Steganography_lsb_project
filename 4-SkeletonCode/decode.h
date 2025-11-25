#ifndef DECODE_H
#define DECODE_H

#include <string.h>
#include "types.h"  // Contains user defined types

/* Structure to store information required for
 * decoding secret file from stego Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

// Changed d_extn_secret_file from pointer to array to match usage
typedef struct _DecodeInfo
{
    // Stego image info
    char *d_stego_image_fname;
    FILE *d_fptr_stego_image;
    char stego_image_data[MAX_IMAGE_BUF_SIZE];

    // Output file info
    char output_file_fname[100];
    FILE *fptr_output;
    char extn_secret_file[MAX_FILE_SUFFIX];  // Changed from pointer to array
    
    // Decoded secret file info
    char *secret_fname;
    long size_secret_file;

    // Magic string for validation
    char magic_string[100];
} DecodeInfo;


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status open_filesd(DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_sizeof_magic_string(DecodeInfo *decInfo, int *magic_size);

Status decode_sizetolsb(int *size, char *str);

Status decode_magic_string( char *magic_string, DecodeInfo *decInfo, int magic_size);

Status decode_bytes_tolsb(char *magic_string, char *image_buffer, int size);

Status decode_extension_secretefile(char *str, DecodeInfo *decInfo , int extn);

Status decode_secretfile_size(DecodeInfo *decInfo, int *size);

Status store_data_into_outputFile(char *secret_data , FILE *file , int file_size);

Status decode_filesize_extension(DecodeInfo *decInfo , int *size);

#endif /* DECODE_H */