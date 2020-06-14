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
    printf("    --out     : output file name other inputfile.coe is created.\n");
    exit(EXIT_FAILURE);
}

void printVersion(void){
    printf("Version is %d.%d\n", MAJOR_ISSUE, MINOR_ISSUE);
    exit(EXIT_FAILURE);
}

void writeByteToCOEFile(int * dataIndex, int * dataCounter, FILE * outputFile, int dataSize, int dataIn) {
    if (*dataIndex == 0) {
        if (*dataCounter == 0) {
            fprintf(outputFile, "\n");
        } else {
            fprintf(outputFile, ",\n");
        }
    }
    fprintf(outputFile, "%02x", dataIn);
    *dataIndex = *dataIndex + 1;
    if (*dataIndex >= dataSize) {
        *dataIndex = 0;
        *dataCounter = *dataCounter + 1;
    }
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
    //char dataIn;
    int dataIndex = 0;
    int dataCounter = 0;
    

#ifdef DEBUG    
    argc = 7;
    argv[1] = "--file";
    argv[2] = "Cyber Core (USA).pce";
    argv[3] = "--width";
    argv[4] = "8";
    argv[5] = "--depth";
    argv[6] = "393216";
    
    argv[7] = "--out";
    argv[8] = "out.coe";
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
                    printf("test\n");
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
    fputs("memory_initialization_radix=16;\nmemory_initialization_vector=",outputFile);    
    while (fileSize-- > 0) {
        writeByteToCOEFile(&dataIndex, &dataCounter, outputFile, dataSize, fgetc(inputFile));
    }
    // check dest file size and fill with zeros if needed to pad file
    while (dataCounter < depth) {
        writeByteToCOEFile(&dataIndex, &dataCounter, outputFile, dataSize, 0x00);
    }
    fputc('\n',outputFile);        
    printf("COE file created!\n\r");
    free(outputFilename);
    fclose(inputFile);
    fclose(outputFile);
       
    return (EXIT_SUCCESS);
}

