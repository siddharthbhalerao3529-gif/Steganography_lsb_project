#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("Source file must be a .bmp\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    char *ext = strrchr(argv[3], '.');
    if (ext == NULL)
    {
        printf( "Error: Secret file must have an extension\n");
        return e_failure;
    }

    const char *valid_extensions[] = {
        ".txt", ".csv", ".c", ".cpp", ".xml", ".html",
        ".css", ".json", ".js", ".py", ".xls", ".doc", ".docx", NULL};

    int valid = 0;
    for (int i = 0; valid_extensions[i] != NULL; i++)
    {
        if (strcmp(ext, valid_extensions[i]) == 0)
        {
            valid = 1;
            break;
        }
    }

    if (!valid)
    {
        printf( "Error: Unsupported secret file extension\n");
        return e_failure;
    }

    encInfo->secret_fname = argv[3];

    encInfo->stego_image_fname = (argv[4] && strstr(argv[4], ".bmp")) ? argv[4] : "stego.bmp";
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (!encInfo->fptr_src_image)
    {
        perror("Failed to open source image");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (!encInfo->fptr_secret)
    {
        perror("Failed to open secret file");
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (!encInfo->fptr_stego_image)
    {
        perror("Failed to create stego image");
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        return e_failure;
    }
    return e_success;
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);
    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);
    return width * height * 3;
}

long get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Extract file extension from secret file name
    char *extn = strrchr(encInfo->secret_fname, '.');
    if (extn == NULL)
    {
        printf("Unable to determine file extension of secret file.\n");
        return e_failure;
    }

    size_t required = (strlen(MAGIC_STRING) * 8) +     // Magic string
                      (32) +                           // Magic string size
                      (strlen(MAGIC_STRING) * 8) +     // Magic string data
                      (32) +                           // Extn size
                      (strlen(extn) * 8) +             // Extn
                      (32) +                           // File size
                      (encInfo->size_secret_file * 8); // File data

    printf("Available: %u, Required: %zu\n", encInfo->image_capacity, required);
    return (encInfo->image_capacity >= required) ? e_success : e_failure;
}

Status copy_bmp_header(FILE *fptr_src, FILE *fptr_dest)
{
    char header[54];
    rewind(fptr_src);
    if (fread(header, 1, 54, fptr_src) != 54)
    {
        printf("Failed to read BMP header\n");
        return e_failure;
    }
    if (fwrite(header, 1, 54, fptr_dest) != 54)
    {
        printf("Failed to write BMP header\n");
        return e_failure;
    }
    return e_success;
}

Status encode_byte_tolsb(char data, char *image_buffer)
{
    if (!image_buffer)
        return e_failure;
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1);
    }
    return e_success;
}

Status encode_sizeof_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    char buffer[32] = {0};
    int l = strlen(magic_string);
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        printf("Failed to read for magic string size\n");
        return e_failure;
    }
    if (encode_sizetolsb(l, buffer) != e_success)
    {
        printf("Failed to encode magic string size\n");
        return e_failure;
    }
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        printf("Failed to write magic string size\n");
        return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int l = strlen(magic_string);
    for (int i = 0; i < l; i++)
    {
        if (fread(encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
        {
            printf("Failed to read for magic string\n");
            return e_failure;
        }
        if (encode_byte_tolsb(magic_string[i], encInfo->image_data) != e_success)
        {
            printf("Failed to encode magic string byte\n");
            return e_failure;
        }
        if (fwrite(encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            printf("Failed to write magic string\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_sizeof_secrete_extension_file(char *fileextnsize, EncodeInfo *encInfo)
{
    int l = strlen(fileextnsize);
    char buffer[32] = {0};
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        printf("the size of secret file ext can load to buffer \n");
        return e_failure;
    }
    if (encode_sizetolsb(l, buffer) != e_success)
    {
        printf("size of secret file extension not getting from buffer size to lsb\n");
        return e_failure;
    }
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char *extn = strrchr(encInfo->secret_fname, '.');
    if (!extn)
    {
        printf("No extension found in secret file\n");
        return e_failure;
    }
    strncpy(encInfo->extn_secret_file, extn, MAX_FILE_SUFFIX);

    for (int i = 0; i < strlen(extn); i++)
    {
        if (fread(encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
        {
            printf("Failed to read for extension\n");
            return e_failure;
        }
        if (encode_byte_tolsb(extn[i], encInfo->image_data) != e_success)
        {
            printf("Failed to encode extension\n");
            return e_failure;
        }
        if (fwrite(encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            printf("Failed to write extension\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_sizetolsb(long file_size, char *str)
{
    if (!str)
        return e_failure;
    for (int i = 0; i < 32; i++)
    {
        str[i] = (str[i] & 0xFE) | ((file_size >> i) & 1);
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char str[32] = {0};
    if (fread(str, 1, 32, encInfo->fptr_src_image) != 32)
    {
        printf("Failed to read for file size\n");
        return e_failure;
    }
    if (encode_sizetolsb(file_size, str) != e_success)
    {
        printf("Failed to encode file size\n");
        return e_failure;
    }
    if (fwrite(str, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        printf("Failed to write file size\n");
        return e_failure;
    }
    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src, FILE *fptr_stego)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8, fptr_src) != 8)
        {
            printf("Failed to read for secret data\n");
            return e_failure;
        }
        if (encode_byte_tolsb(data[i], image_buffer) != e_success)
        {
            printf("Failed to encode secret data\n");
            return e_failure;
        }
        if (fwrite(image_buffer, 1, 8, fptr_stego) != 8)
        {
            printf("Failed to write secret data\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char *buffer = malloc(encInfo->size_secret_file);
    if (!buffer)
    {
        printf("Memory allocation failed\n");
        return e_failure;
    }

    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    if (fread(buffer, 1, encInfo->size_secret_file, encInfo->fptr_secret) != encInfo->size_secret_file)
    {
        printf("Failed to read secret file\n");
        free(buffer);
        return e_failure;
    }

    Status status = encode_data_to_image(buffer, encInfo->size_secret_file,
                                         encInfo->fptr_src_image, encInfo->fptr_stego_image);
    free(buffer);
    return status;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024];
    int bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr_src)))
    {
        if (fwrite(buffer, 1, bytes, fptr_dest) != bytes)
        {
            printf("Failed to copy remaining data\n");
            return e_failure;
        }
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("Starting encoding process...\n");

    if (open_files(encInfo) != e_success)
    {
        printf("Failed to open files\n");
        return e_failure;
    }
    printf("successfully file are opened for encoding \n");

    printf("Enter magic string: ");
    if (!fgets(encInfo->magic_string, sizeof(encInfo->magic_string), stdin))
    {
        printf("Failed to read magic string\n");
        return e_failure;
    }
    encInfo->magic_string[strcspn(encInfo->magic_string, "\n")] = '\0';

    if (check_capacity(encInfo) != e_success)
    {
        printf("Insufficient capacity\n");
        return e_failure;
    }
    printf("successfully capacity checked \n");

    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("Failed to copy BMP header\n");
        return e_failure;
    }
    printf("successfully header copied to encoded bmp\n");

    if (encode_sizeof_magic_string(encInfo->magic_string, encInfo) != e_success)
    {
        printf("Failed to encode magic string size\n");
        return e_failure;
    }
    printf("successfully size of magic string encoded image\n");
    if (encode_magic_string(encInfo->magic_string, encInfo) != e_success)
    {
        printf("Failed to encode magic string\n");
        return e_failure;
    }
    printf("successfully magic string encoded image\n");

    char *extn = strrchr(encInfo->secret_fname, '.');
    if (extn == NULL)
    {
        printf("Failed to determine file extension\n");
        return e_failure;
    }
    int l = strlen(extn);
    if (encode_sizeof_secrete_extension_file(extn, encInfo) != e_success)
    {
        printf("failed to encode size into stego.bmp\n");
        return e_failure;
    }

    if (encode_secret_file_extn(extn, encInfo) != e_success)
    {
        printf("Failed to encode file extension\n");
        return e_failure;
    }

    printf("Successfully extension encoded to image \n");

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
    {
        printf("Failed to encode file size\n");
        return e_failure;
    }
    printf("successfuly secret file size encoded to image\n");

    if (encode_secret_file_data(encInfo) != e_success)
    {
        printf("Failed to encode secret data\n");
        return e_failure;
    }
    printf("successfully secret data encoded to image\n");

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("Failed to copy remaining data\n");
        return e_failure;
    }
    printf("all data endcoded to image\n");

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    // printf("Encoding completed successfully!\n");
    return e_success;
}
