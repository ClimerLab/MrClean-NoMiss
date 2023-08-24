# NoMiss
Extension of the MrClean program suite that specializing in creating clean data sets with no missing data

## Overview
NoMoss is an ensemble of three algorithms that clean a data matrix, along with a few helper programs. First, the _Orient_ program checks the dimension of the data matrix and creates a transposed version if the matrix contained more rows than columns. Next, the _AddRowGreedy_ and _RowColLP_ cleaning programs can be executed. If the user wishes to run the _ElementIP_ algorithm, the _CalcPairs_ helper function must be run first. After running the desired cleaning programs, the _WriteCleanedMatrix_ program checks the results of the various cleaning programs and writes a cleaned data matrix to file. 

## Compile Program
The user may need to update 4 parameters in the _Makefile_ to specify the version and location of CPLEX

SYSTEM     = <system_type>  
LIBFORMAT  = <format_of_cplex_library>  
CPLEXDIR   = <full_path_to_cplex_directory>  
CONCERTDIR = <full_path_to_concert_directory>  

Example:  
SYSTEM     = x86-64_linux  
LIBFORMAT  = static_pic  
CPLEXDIR   = /opt/ibm/ILOG/CPLEX_Studio221/cplex  
CONCERTDIR = /opt/ibm/ILOG/CPLEX_Studio221/concert  

To compile the program, navigate to the directory containing the download and type 'make' (no quotes). The following executables will be created: _CheckMatrixOrientation_, _addRowGreedy_, _rowColLP_, _calcPairs_, _elementIp_, and _writeCleanedMatrix_.



## Running NoMiss
Included in the repo is a shell script named _clean_data.sh_. If you wish to use the script, there are 4 variables that need to be set:
1.	ARR – array containing the data files to clean. All data files should have the same number of header rows, number of header columns, and NA_SYMBOL.
2.	NA_SYMBOL - the string which represents missing data in the data file. This value needs to be a string. Spaces or tabs will cause unexpected behavior.
3.	NUM_HEADER_ROWS - number of header rows for each file in ARR
4.	NUM_HEADER_COLS - number of header columns for each file in ARR
The shell script will handle updating the data file information if a transpose occurs and cleans up all temporary files created during the cleaning process.
If you prefer to execute the programs separately, all four inputs listed above are required for each program. Note that checkMatrixOrientation should be executed before any cleaning programs and calcPairs must be run before elementIp.
The calcPairs and elementI programs use Open MPI to distribute the work. Mpitun should be used to call these programs. The rankfile can be used to specify which the number of desired processes. A minimum of two processors (or threads) are required to run calcPairs and elementIp.
**NOTE**: Double check the number of header rows and columns. The program will likely run, without error, if an incorect number of headers rows or header columns is provided.
**NOTE**: CPLEX is required to run the two Integer Programs (IP). The user will be required to provide the directories for CPLEX in the Makefile.
## Configuration File
The configuration file allows the user to turn several features of the program on/off.  
PRINT_SUMMARY - determines if a summary is printed to the screen for each algorithm.  
WRITE_STATS - determines if the statistics are recorded to a file. Each algorithm has a seperate file.  
LARGE_MATRIX – determines the number of elements in an elementIp problem when the constraints will be reduced. 
The program expects a file named _config.cfg_ in the same directory as the executable and all flags above should be included. If a flag is missing, the program will exit with an error condition.

## Program Output
If PRINT_SUMMARY is set to true a summary of each executed cleaning program will be printed to the screen for each data file. If WRITE_STATS is set true, a CSV file will be created for each cleaning program. The file will contain the data file, run time, number of valid elements, number of rows, and number of columns resulting from the algorithm. From the executed cleaning algorithms, the solution with the most valid elements will be used to create a cleaned data matrix for each input file. The cleaned files will be written in the same directory as the origan data files and will be named < data_file>_cleaned.tsv
