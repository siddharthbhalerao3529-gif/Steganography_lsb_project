#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Source file must be a .bmp\n");
        return e_failure;
    }
    decInfo->d_stego_image_fname = argv[2];

    // decInfo->secret_fname = (argv[3] && strstr(argv[3], ".txt")) ? argv[3] : "data.txt";

    if (argv[3])
    {
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = NULL; // Let do_decoding assign default based on decoded extension
    }

    return e_success;
}

Status open_filesd(DecodeInfo *decInfo)
{
    decInfo->d_fptr_stego_image = fopen(decInfo->d_stego_image_fname, "r");
    if (!decInfo->d_fptr_stego_image)
    {
        perror("Failed to open stego image\n");
        return e_failure;
    }
    decInfo->fptr_output = NULL;
    return e_success;
}

Status decode_sizetolsb(int *size, char *str)
{

    for (int i = 0; i < 32; i++)
    {
        *size |= ((str[i] & 1) << i);
    }
    return e_success;
}

Status decode_sizeof_magic_string(DecodeInfo *decInfo, int *magic_size)
{
    char str[32] = {0};

    if (fread(str, 1, 32, decInfo->d_fptr_stego_image) != 32)
    {
        printf("Failed to read for magic string size\n");
        return e_failure;
    }

    if (decode_sizetolsb(magic_size, str) != e_success)
    {
        printf("Failed to decode magic string size\n");
        return e_failure;
    }

    return e_success;
}

Status decode_bytes_tolsb(char *magic_string, char *image_buffer, int size)
{
    for (int i = 0; i < size; i++)
    {
        magic_string[i] = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            // Properly extract each bit from the image buffer
            magic_string[i] |= ((image_buffer[i * 8 + bit] & 1) << bit);
        }
    }
    magic_string[size] = '\0'; // Null-terminate the string
    return e_success;
}

Status decode_magic_string(char *magic_string, DecodeInfo *decInfo, int size)
{
    char *image_buffer = malloc(size * 8);
    if (!image_buffer)
    {
        printf("Memory allocation failed\n");
        return e_failure;
    }

    if (fread(image_buffer, 1, size * 8, decInfo->d_fptr_stego_image) != size * 8)
    {
        printf("Failed to read magic string data\n");
        free(image_buffer);
        return e_failure;
    }

    if (decode_bytes_tolsb(magic_string, image_buffer, size) != e_success)
    {
        printf("Failed to decode magic string\n");
        free(image_buffer);
        return e_failure;
    }

    free(image_buffer);
    return e_success;
}

Status decode_filesize_extension(DecodeInfo *decInfo, int *size)
{
    char str[32] = {0};
    if (fread(str, 1, 32, decInfo->d_fptr_stego_image) != 32)
    {
        printf("file size not decoding from stego image\n");
        return e_failure;
    }
    if (decode_sizetolsb(size, str) != e_success)
    {
        printf("file size not decoding from string \n");
        return e_failure;
    }
    return e_success;
}
Status decode_extension_secretefile(char *str, DecodeInfo *decInfo, int extn_size)
{
    // Validate input parameters
    if (!str || !decInfo || extn_size <= 0 || extn_size > MAX_FILE_SUFFIX)
    {
        printf("ERROR: Invalid parameters for extension decoding\n");
        return e_failure;
    }

    // Read encoded extension data from stego image
    char image_buffer[MAX_FILE_SUFFIX * 8] = {0};
    if (fread(image_buffer, 1, extn_size * 8, decInfo->d_fptr_stego_image) != extn_size * 8)
    {
        printf("ERROR: Failed to read extension data from stego image\n");
        return e_failure;
    }

    // Decode each character from LSBs
    if (decode_bytes_tolsb(str, image_buffer, extn_size) != e_success)
    {
        printf("Failed to load extension \n");
        return e_failure;
    }

    // Verify the decoded extension is valid
    if (str[0] != '.')
    {
        printf("ERROR: Invalid file extension format (must start with '.'), got '%s'\n", str);
        return e_failure;
    }

    printf("INFO: Successfully decoded extension: %s\n", str);
    return e_success;
}

Status decode_secretfile_size(DecodeInfo *decInfo, int *size)
{
    char str[32];
    if (fread(str, 1, 32, decInfo->d_fptr_stego_image) != 32)
    {
        printf("failed to load from stegao image \n");
        return e_failure;
    }
    if (decode_sizetolsb(size, str) != e_success)
    {
        printf("failed to load size of secret file from image \n");
        return e_failure;
    }
    printf("the secret file size from image is %d \n", *size);
    return e_success;
}

Status decode_secretefile_data(char *secret_data, DecodeInfo *decInfo, int file_size)
{
    char *image_buffer = malloc(file_size * 8);
    if (!image_buffer)
    {
        printf("image buffer not allocated \n");
        return e_failure;
    }

    if (fread(image_buffer, 1, file_size * 8, decInfo->d_fptr_stego_image) != 8 * file_size)
    {
        printf("failed to data load from image \n");
        return e_failure;
    }
    if (decode_bytes_tolsb(secret_data, image_buffer, file_size) != e_success)
    {
        printf("Failed to get data from image\n");
        return e_failure;
    }

    //  printf("the data found - %s\n", secret_data);
    free(image_buffer);
    return e_success;
}

Status store_data_into_outputFile(char *secret_data, FILE *file, int file_size)
{
    fwrite(secret_data, 1, file_size, file);
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    // 1. Open files
    if (open_filesd(decInfo) != e_success)
    {
        printf("Failed to open files\n");
        return e_failure;
    }
    printf("Image successfully opened for encoding \n");

    // 2. Skip BMP header (54 bytes)
    fseek(decInfo->d_fptr_stego_image, 54, SEEK_SET);

    printf("Enter the magic string for decode \n");
    scanf(" %[^\n]", decInfo->magic_string);

    // 3. Decode magic string size (32 bits)
    int magic_size = 0;
    if (decode_sizeof_magic_string(decInfo, &magic_size) != e_success)
    {
        printf("Failed to decode magic string size\n");
        return e_failure;
    }
    //   printf("Magic string size: %d\n", magic_size);
    printf("successfully decode size of magic string \n");

    // 4. Decode magic string data
    char decoded_magic[100] = {0};
    if (decode_magic_string(decoded_magic, decInfo, magic_size) != e_success)
    {
        printf("Failed to decode magic string\n");
        return e_failure;
    }
    //  printf("Decoded magic string: %s\n", decoded_magic);

    printf("successfully decoded magic string \n");

    // 5. Verify magic string matches expected value
    if (strcmp(decoded_magic, decInfo->magic_string) != 0)
    {
        // printf("Magic string mismatch! Expected: %s, Got: %s\n",
        //        decInfo->magic_string, decoded_magic);
        printf("magic string is mismatched \n");
        return e_failure;
    }
    printf("Successfully complile with image string\n");

    int file_ext_size = 0;
    if (decode_filesize_extension(decInfo, &file_ext_size) != e_success)
    {
        printf("file size extension not decoded \n");
        return e_failure;
    }
    printf("Successfully file size extension decoded\n");
    // 7. Decode secret file extension  (8)
    char str[9] = {0}; // Buffer to store encoded extension (4 chars * 8 bits + null terminator)

    if (decode_extension_secretefile(str, decInfo, file_ext_size) != e_success)
    {
        printf("Failed to decode secret file extension: %s\n", str);
        return e_failure;
    }
    if (!decInfo->secret_fname)
    {
        if (strlen(str) > 1)
        {
            int total_size = strlen("data")+strlen(str)+1;
            decInfo->secret_fname = malloc(total_size);
            if(!decInfo->secret_fname)
            {
                printf("memmory not allocated for secret name file\n");
                return e_failure;
            }

            strcpy(decInfo->secret_fname, "data");
            strcat(decInfo->secret_fname, str);
        }
        else
        {
            printf("not extension found \n");
            return e_failure;
        }
    }

    decInfo->fptr_output = fopen(decInfo->secret_fname, "w");
    if (!decInfo->fptr_output)
    {
        perror("Failed to create output file");
        fclose(decInfo->d_fptr_stego_image);
        return e_failure;
    }

    printf("successfully decoded extension from image\n");

    // 8. Decode secret file size
    int file_size = 0;
    if (decode_secretfile_size(decInfo, &file_size) != e_success)
    {
        printf("Failed to secretfile size\n");
        return e_failure;
    }
    printf("successfully decode size found\n");
    // printf("file size %d\n",file_size);
    char *secret_data = malloc(file_size);
    if (!secret_data)
    {
        printf("Memory allocation failed\n");
        return e_failure;
    }

    if (decode_secretefile_data(secret_data, decInfo, file_size) != e_success)
    {
        printf("failed to load  data from these image \n");
        return e_failure;
    }
    printf("successfully data load from image \n");

    if (store_data_into_outputFile(secret_data, decInfo->fptr_output, file_size) != e_success)
    {
        printf("the data not to output file\n");
        return e_failure;
    }
    fclose(decInfo->d_fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("successfully image text copied as it to file\n");
    return e_success;
}