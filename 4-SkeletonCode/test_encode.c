/*
name - siddharth bhalerao
date - 5-7-25
description - A C-based project that hides and extracts secret data inside BMP images using LSB steganography,
             supports password-protected encoding/decoding, and allows the hidden data to be saved as user-preferred file types
               */
#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    if (argc < 3)//
    {
        printf("argument not correct ");
        return e_failure;
    }
    // OperationType check_operation_type(argv);
    if (check_operation_type(argv) == e_encode)
    {
        printf("you have chossed endcoding\n");
        // return e_success;
    }
    else if ((check_operation_type(argv)) == e_decode)
    {
        printf("you have chossed decoding\n");
        // return e_success;
    }
    else
    {
        printf("pass valid argument \n");
        return e_failure;
    }

    if (strcmp(argv[1], "-e") == 0)
    {
      //  printf("you have chossed endcoding\n");
        EncodeInfo encInfo;

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Successfully reading and validate \n");
            printf("---------------<start encoding>----------------\n");
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding completed successfully\n");
            }
            else
            {
                printf("Encoding failed \n");
            }
        }
        else
        {
            printf("failed to read and validate\n");
        }
        // return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
       // printf("you have chossed decoding\n");

        DecodeInfo decInfo;
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Successfully reading and validate \n");
            printf("---------------<start decoding>----------------\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding completed successfully\n");
            }
            else
            {
                printf("Decoding failed \n");
            }
        }
        else
        {
            printf("decoding read validation failed  \n");
        }
        // return e_decode;
    }
    else
    {
        printf("invalid ");
    }
    // return e_success;
}
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}
