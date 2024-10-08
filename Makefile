SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#---------------------------------------------------------------------------------------------------
#
# Set CPLEXDIR and CONCERTDIR to the directories where CPLEX and CONCERT are installed.
#
#---------------------------------------------------------------------------------------------------

#CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio221/cplex
#CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio221/concert

CPLEXDIR      = /cluster/software/cplex-studio/cplex-studio-12.7.0/cplex
CONCERTDIR    = /cluster/software/cplex-studio/cplex-studio-12.7.0/concert

#---------------------------------------------------------------------------------------------------
# Set MPIDIR to the directories where OPEN MPI is installed
#---------------------------------------------------------------------------------------------------
MPIDIR = /cluster/spack-2022/opt/spack/linux-centos7-x86_64/gcc-9.3.0/openmpi-4.1.1-udg7sdl3kjslokkcsrmuzz5kn6krohpa

#---------------------------------------------------------------------------------------------------
# Compiler selection
#---------------------------------------------------------------------------------------------------

MPICXX	= mpicxx
CXX			= g++

#---------------------------------------------------------------------------------------------------
# Directories
#---------------------------------------------------------------------------------------------------

OBJDIR = build
SRCDIR = src

#---------------------------------------------------------------------------------------------------
# Executables
#---------------------------------------------------------------------------------------------------

EXE = rowColLp calcPairs elementIp writeCleanedMatrix CheckMatrixOrientation

#---------------------------------------------------------------------------------------------------
# Object files
#---------------------------------------------------------------------------------------------------

COMMON_OBJ = BinContainer.o Timer.o ConfigParser.o NoMissSummary.o
ROWCOL_OBJ = $(COMMON_OBJ) RowColLpSolver.o RowColLpWrapper.o
CALCPAIRS_OBJ = BinContainer.o Timer.o CalcPairsWrapper.o CalcPairsController.o \
								CalcPairsWorker.o Parallel.o CalcPairsCore.o
ELEMENT_OBJ = $(COMMON_OBJ) ElementIpSolver.o ElementWrapper.o Pairs.o CleanSolution.o \
							ElementSolverController.o ElementSolverWorker.o Parallel.o
CLEAN_OBJ = WriteCleanedMatrix.o BinContainer.o NoMissSummary.o
ORIENT_OBJ = CheckMatrixOrientation.o BinContainer.o

#---------------------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------------------

CXXFLAGS = -O3 -Wall -fPIC -fexceptions -DIL_STD -std=c++11 -fno-strict-aliasing

#---------------------------------------------------------------------------------------------------
# Link options and libraries
#---------------------------------------------------------------------------------------------------

CPLEXLIBDIR    = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR  = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
MPILIBDIR      = $(MPIDIR)/lib/**

CPLEXLNDIRS	   = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CPLEXLNFLAGS	 = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

MPILNDIRS      = -L$(MPILIBDIR)
MPILNFLAGS     = -lmpi

ALLLNDIR       = $(CPLEXLNDIRS) $(MPILNDIRS)
ALLLNFLAGS     = $(CPLEXLNFLAGS) $(MPILNFLAGS)
#---------------------------------------------------------------------------------------------------
# Includes
#---------------------------------------------------------------------------------------------------

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
MPIINCDIR     = $(MPIDIR)/include

CPLEXINCLUDES = -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)
MPIINCLUDES   = -I$(MPIINCDIR)

#---------------------------------------------------------------------------------------------------
all: CXXFLAGS += -DNDEBUG
all: $(EXE)

debug: CXXFLAGS += -g
debug: $(EXE)

CheckMatrixOrientation: $(addprefix $(OBJDIR)/, CheckMatrixOrientation.o)
	$(CXX) $(CXXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(ORIENT_OBJ)) $(CXXLNFLAGS)

$(OBJDIR)/CheckMatrixOrientation.o:	$(addprefix $(SRCDIR)/, CheckMatrixOrientation.cpp ) \
																		$(addprefix $(OBJDIR)/, BinContainer.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

writeCleanedMatrix: $(addprefix $(OBJDIR)/, WriteCleanedMatrix.o)
	$(CXX) $(CXXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(CLEAN_OBJ)) $(CXXLNFLAGS)

$(OBJDIR)/WriteCleanedMatrix.o:	$(addprefix $(SRCDIR)/, WriteCleanedMatrix.cpp ) \
																$(addprefix $(OBJDIR)/, BinContainer.o NoMissSummary.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

elementIp: $(addprefix $(OBJDIR)/, ElementWrapper.o)
	$(MPICXX) $(ALLLNDIR) -o $@ $(addprefix $(OBJDIR)/, $(ELEMENT_OBJ)) $(ALLLNFLAGS)

$(OBJDIR)/ElementWrapper.o:	$(addprefix $(SRCDIR)/, ElementWrapper.cpp ) \
				$(addprefix $(OBJDIR)/, ElementSolverController.o ElementSolverWorker.o) \
				$(addprefix $(OBJDIR)/, Parallel.o) \
				$(addprefix $(OBJDIR)/, $(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ElementSolverController.o:	$(addprefix $(SRCDIR)/, ElementSolverController.cpp ElementSolverController.h) \
          $(addprefix $(OBJDIR)/, BinContainer.o Pairs.o) \
					$(addprefix $(OBJDIR)/, Parallel.o CleanSolution.o) \
					$(addprefix $(SRCDIR)/, Utils.h )
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ElementSolverWorker.o:	$(addprefix $(SRCDIR)/, ElementSolverWorker.cpp ElementSolverWorker.h) \
					$(addprefix $(OBJDIR)/, BinContainer.o Pairs.o) \
					$(addprefix $(OBJDIR)/, ElementIpSolver.o Parallel.o) \
					$(addprefix $(SRCDIR)/, Utils.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Pairs.o: $(addprefix $(SRCDIR)/, Pairs.cpp Pairs.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ElementIpSolver.o:	$(addprefix $(SRCDIR)/, ElementIpSolver.cpp ElementIpSolver.h) \
				$(addprefix $(OBJDIR)/, BinContainer.o)
	$(CXX) $(CXXFLAGS) $(CPLEXINCLUDES) -c -o $@ $<

calcPairs: $(addprefix $(OBJDIR)/, CalcPairsWrapper.o)
	$(MPICXX) $(MPILNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(CALCPAIRS_OBJ)) $(MPILNFLAGS)

$(OBJDIR)/CalcPairsWrapper.o:	$(addprefix $(SRCDIR)/, CalcPairsWrapper.cpp ) \
				$(addprefix $(OBJDIR)/, BinContainer.o Timer.o Parallel.o) \
				$(addprefix $(OBJDIR)/, CalcPairsController.o CalcPairsWorker.o CalcPairsCore.o) 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CalcPairsController.o:	$(addprefix $(SRCDIR)/, CalcPairsController.cpp CalcPairsController.h) \
					$(addprefix $(OBJDIR)/, BinContainer.o Parallel.o CalcPairsCore.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CalcPairsWorker.o:	$(addprefix $(SRCDIR)/, CalcPairsWorker.cpp CalcPairsWorker.h) \
				$(addprefix $(OBJDIR)/, BinContainer.o Parallel.o CalcPairsCore.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CalcPairsCore.o:	$(addprefix $(SRCDIR)/, CalcPairsCore.cpp CalcPairsCore.h) \
				$(addprefix $(OBJDIR)/, BinContainer.o Parallel.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

rowColLp: $(addprefix $(OBJDIR)/, RowColLpWrapper.o)
	$(CXX) $(CPLEXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(ROWCOL_OBJ)) $(CPLEXLNFLAGS)

$(OBJDIR)/RowColLpWrapper.o:	$(addprefix $(SRCDIR)/, RowColLpWrapper.cpp ) \
				$(addprefix $(OBJDIR)/, RowColLpSolver.o) \
				$(addprefix $(OBJDIR)/, $(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/RowColLpSolver.o:	$(addprefix $(SRCDIR)/, RowColLpSolver.cpp RowColLpSolver.h) \
				$(addprefix $(OBJDIR)/, BinContainer.o)
	$(CXX) $(CXXFLAGS) $(CPLEXINCLUDES) -c -o $@ $<

$(OBJDIR)/NoMissSummary.o: $(addprefix $(SRCDIR)/, NoMissSummary.cpp NoMissSummary.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/BinContainer.o: $(addprefix $(SRCDIR)/, BinContainer.cpp BinContainer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Timer.o: $(addprefix $(SRCDIR)/, Timer.cpp Timer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ConfigParser.o: $(addprefix $(SRCDIR)/, ConfigParser.cpp ConfigParser.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Parallel.o: $(addprefix $(SRCDIR)/, Parallel.cpp Parallel.h)
	$(MPICXX) $(CXXFLAGS) $(MPIINCLUDES) -c -o $@ $<

$(OBJDIR)/CleanSolution.o: $(addprefix $(SRCDIR)/, CleanSolution.cpp CleanSolution.h)
	$(CXX) $(CXXFLAGS) $(MPIINCLUDES) -c -o $@ $<

#---------------------------------------------------------------------------------------------------
.PHONY: clean
clean:
	/bin/rm -f $(OBJDIR)/*.o
#---------------------------------------------------------------------------------------------------
