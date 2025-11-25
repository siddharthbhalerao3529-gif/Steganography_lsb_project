LSB Steganography in C with Password Protection
Project Overview

This project implements Least Significant Bit (LSB) steganography in C to hide and extract secret data inside BMP images. It allows users to securely encode and decode messages or files, optionally protected with a password. Hidden data can be saved in any user-preferred file format.

Features

Hide text or binary data inside BMP images using LSB technique.

Extract hidden data from encoded BMP images.

Password-protected encoding and decoding for enhanced security.

Save the extracted data as a file with a user-defined name and extension.

Simple command-line interface for easy usage.

How It Works

Encoding Process:

The secret data is converted into binary format.

Each bit of the data is embedded into the least significant bit of the BMP image pixels.

If a password is provided, the message is encrypted before embedding.

The modified BMP image is saved as the encoded file.

Decoding Process:

Reads the LSBs of the encoded BMP image to reconstruct the hidden data.

If a password was used, decrypts the message.

Saves the extracted data as a file with a user-chosen name and format.

Supported Media

Input: BMP images

Hidden Data: Any binary/text file

Output: Any file type as specified by the user

Requirements

C compiler (GCC recommended)

Standard C libraries (stdio.h, stdlib.h, string.h, etc.)
