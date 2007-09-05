/***
 * data2csrc.c -- Read data and output c source to stdout
 *
 * Copyright (c) 2007 Thomas Perl <thp@perli.net>
 * Website: http://thpinfo.com/
 *
 * License: This code is in the public domain.
 ***/

#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv) {
    FILE *fp;
    int c, x = 0;

    if( argc != 3) {
        fprintf( stderr, "Usage: %s /path/to/infile.dat name_of_variable\n", argv[0]);
        exit(1);
    }

    fp = fopen( argv[1], "rb");
    if( fp == NULL) { fprintf( stderr, "Cannot open file: %s\n", argv[1]); exit(1); }
    printf( "const char %s[] = {", argv[2]);
    while( (c = fgetc( fp)) != EOF) {
        if( x > 0) putchar(',');
        if( x++ % 20 == 0) putchar('\n');
        printf( "%d", c);
    }
    printf( "\n};\n");
    fclose( fp);
    return 0;
}

