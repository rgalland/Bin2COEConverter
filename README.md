# Bin2COEConverter
Small C program to convert a bin file to COE; a format used by Xilinx to load memory contents.
 
The program takes the following arguments:
  
    --help    : prints this message.    
    --version : prints the version.
    --file    : bin or hex file to be converted to coe file.
    --width   : data size is a multiple of 8 - set to 8 by default.
    --depth   : Must be greater or equal to bin file size / ( width / 8 ) or set to that value by default.
    --out     : output file name other inputfile.coe is created.
