/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: regis
 *
 * Created on May 15, 2020, 7:49 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

//#define DEBUG

#define MAJOR_ISSUE 1
#define MINOR_ISSUE 0

#define DATA_WIDTH 8

void printHelpMessage(void){
    printf("BIN to COE file converter\n");
    printf("Help message to run this program.\n");
    printf("  The program takes the following arguments:\n");
    printf("    --help    : prints this message.\n");
    printf("    --version : prints the version.\n");
    printf("    --file    : bin or hex file to be converted to coe file.\n");
    printf("    --width   : data size is a multiple of 8 - set to 8 by default.\n");
    printf("    --depth   : Must be greater or equal to bin file size / ( width / 8 ) or set to that value by default.\n");
    printf("    --big     : bin file is big endian instead of little endian by default; only applies to width > 8.\n");
    printf("    --out     : output file name other inputfile.coe is created.\n");
    exit(EXIT_FAILURE);
}

void printVersion(void){
    printf("Version is %d.%d\n", MAJOR_ISSUE, MINOR_ISSUE);
    exit(EXIT_FAILURE);
}

/*
 * 
 */
int main(int argc, char** argv) {

    FILE* inputFile;
    FILE* outputFile;
    int width = 0;
    int depth = 0;
    char * inputFilename = NULL;
    char * outputFilename = NULL;
    int fileSize = 0;
    int dataSize;
    int charCnt;
    int dataIn; 
    char dataInString[3];
    char * dataInFullString;
    int dataIndex = 0;
    int dataCounter = 0;
    int big = 0;
    

#ifdef DEBUG    
    /*
    argc = 7;   // will ignore --out args
    argv[1] = "--file";
    argv[2] = "Cyber Core (USA).pce";
    argv[3] = "--width";
    argv[4] = "8";
    argv[5] = "--depth";
    argv[6] = "393216";
    argv[7] = "--out";
    argv[8] = "out.coe";
    
    argc = 9;
    argv[1] = "--file";
    argv[2] = "Iria_144p.data.bin";
    argv[3] = "--width";
    argv[4] = "16";
    argv[5] = "--depth";
    argv[6] = "23040";      //  "23040" for width = 16    
    argv[7] = "--out";
    argv[8] = "out16.coe";
    */
    argc = 10;
    argv[1] = "--file";
    argv[2] = "MC68000_test_all_opcodes.bin";
    argv[3] = "--width";
    argv[4] = "16";
    argv[5] = "--depth";
    argv[6] = "32768";          
    argv[7] = "--big";      // file is big endian 
    argv[8] = "--out";
    argv[9] = "out16_bige.coe";
    
#endif
    
    if (argc > 1) {
        for (int I = 0; I < argc; I++) {
            if (strcmp(argv[I],"--help" ) == 0) {
                printHelpMessage();             
            } else if (strcmp(argv[I],"--version" ) == 0) {
                printVersion();             
            } else if (strcmp(argv[I],"--file" ) == 0) {
                I++;
                if (I < argc){
                    inputFilename = argv[I];
                }
            } else if (strcmp(argv[I],"--width" ) == 0) {
                I++;
                if (I < argc){
                    width =  atoi(argv[I]);
                }                
            } else if (strcmp(argv[I],"--depth" ) == 0) {
                I++;
                if (I < argc){
                    depth =  atoi(argv[I]);
                }                
            } else if (strcmp(argv[I],"--out" ) == 0) {
                I++;
                if (I < argc){
                    outputFilename = malloc(strlen(argv[I]));
                    strcpy(outputFilename, argv[I]);
                }                
            } else if (strcmp(argv[I],"--big" ) == 0) {
                I++;
                big = 1;
            }
        }
    } else {
        printHelpMessage();
    }
    
    printf("BIN to COE file converter\n\r");
    // check arguments
    // start with data width
    if (width % DATA_WIDTH != 0) {
        printf("Error: Width is not a multiple of 8!\n\r");
        exit(EXIT_FAILURE);
    }
    if (width == 0) {
        printf("Data width is set to default value i.e. 8.\n\r");
        width = DATA_WIDTH;
    }
    dataSize = width / DATA_WIDTH;
    // then check input file path 
    if (inputFilename == NULL) {
        printf("Error: The file path was not defined.\n\r");
        exit(EXIT_FAILURE);
    }
    inputFile = fopen(inputFilename, "r");
    if (inputFile == NULL) {
        printf("Error: The file cannot be opened. Check path.\n\r");
        exit(EXIT_FAILURE);
    }
    
    fseek(inputFile, 0, SEEK_END);
    fileSize = ftell(inputFile);
    if (depth < fileSize/dataSize) {
        printf("Error: Destination COE file is not large enough.\n\r");
        printf("Depth must be set to %d or greater.\n\r", fileSize/dataSize);
        exit(EXIT_FAILURE);
    }
    if (outputFilename == NULL) {
        printf("No output file specified. COE file will be named %s.coe.\n\r", inputFilename);
        outputFilename = malloc(strlen(inputFilename) + 5); 
        strcpy(outputFilename, inputFilename);        
        strcat(outputFilename, ".coe\0");
    }
    outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL) {
        printf("Error: The output file cannot be opened. Check path.\n\r");
        exit(EXIT_FAILURE);
    }
    // go back to file start
    fseek(inputFile, 0, SEEK_SET);
    // first write 2 top lines of COE file. Always use radix 16
    fputs("memory_initialization_radix=16;\nmemory_initialization_vector=\n",outputFile);    
    // allocate memory to read values from binary as char strings based on data size, 2 chars per byte + 1 char for \0 
    charCnt = dataSize * 2;    
    dataInFullString = malloc(charCnt + 1);
    dataInFullString[charCnt] = 0x00;  // end of string char at the end of array
    
    // parse each byte in binary file
    while (fileSize > 0) {
        
        for (int I = dataSize; I > 0; I--) {
            // read next char in binary file and save to string array
            dataIn = fgetc(inputFile);
            snprintf(dataInString, 3, "%02x", dataIn);
            if (big == 1) {
                memcpy(&dataInFullString[charCnt - (I * 2)], dataInString, 2); 
            } else {
                memcpy(&dataInFullString[(I * 2) - 2], dataInString, 2); 
            }
            fileSize--;
        }
        fputs(dataInFullString, outputFile);
        dataCounter++;
        if (dataCounter < depth) {
            fputs(",\n", outputFile);
        }   
    }
    
    // if depth not yet reached, pad file with 0's
    while (dataCounter < depth) {
        fputc('0', outputFile);   // no need to pad with extra zeros
        dataCounter++;
        if (dataCounter < depth) {
            fputs(",\n", outputFile);
        }
    }
    
    printf("COE file named %s created!\n\r", outputFilename);
    free(outputFilename);
    free(dataInFullString);
    fclose(inputFile);
    fclose(outputFile);
       
    return (EXIT_SUCCESS);
}

