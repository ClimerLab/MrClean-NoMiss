SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#---------------------------------------------------------------------------------------------------
#
# Set CPLEXDIR and CONCERTDIR to the directories where CPLEX and CONCERT are installed.
#
#---------------------------------------------------------------------------------------------------

CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio221/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio221/concert

#---------------------------------------------------------------------------------------------------
# Compiler selection
#---------------------------------------------------------------------------------------------------

MPICXX	= /usr/local/bin/mpicxx
CXX			= g++

#---------------------------------------------------------------------------------------------------
# Directories
#---------------------------------------------------------------------------------------------------

OBJDIR = build
SRCDIR = src

#---------------------------------------------------------------------------------------------------
# Executables
#---------------------------------------------------------------------------------------------------

EXE = rowColLp addRowGreedy calcPairs elementIp

#---------------------------------------------------------------------------------------------------
# Object files
#---------------------------------------------------------------------------------------------------

COMMON_OBJ = DataContainer.o Timer.o ConfigParser.o NoMissSummary.o
ROWCOL_OBJ = $(COMMON_OBJ) RowColLpSolver.o RowColLpWrapper.o
GREEDY_OBJ = $(COMMON_OBJ) AddRowGreedy.o AddRowGreedyWrapper.o
CALCPAIRS_OBJ = DataContainer.o Timer.o CalcPairsWrapper.o CalcPairsController.o CalcPairsWorker.o Parallel.o
ELEMENT_OBJ = $(COMMON_OBJ) ElementSolver.o ElementWrapper.o Pairs.o AddRowGreedy.o ElementIpSolver.o

#---------------------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------------------

CXXFLAGS = -O3 -Wall -fPIC -fexceptions -DIL_STD -std=c++11 -fno-strict-aliasing

#---------------------------------------------------------------------------------------------------
# Link options and libraries
#---------------------------------------------------------------------------------------------------

CPLEXLIBDIR    = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR  = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

CXXLNDIRS      = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR)
CXXLNFLAGS     = -lconcert -lilocplex -lcplex
CXXPFLAGS			 = -lm -lpthread -ldl

#---------------------------------------------------------------------------------------------------
# Includes
#---------------------------------------------------------------------------------------------------

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

INCLUDES = -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)

#---------------------------------------------------------------------------------------------------
all: CXXFLAGS += -DNDEBUG
all: $(EXE)

debug: CXXFLAGS += -g
debug: $(EXE)

elementIp: $(addprefix $(OBJDIR)/, ElementWrapper.o)
	$(CXX) $(CXXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(ELEMENT_OBJ)) $(CXXLNFLAGS)

$(OBJDIR)/ElementWrapper.o:	$(addprefix $(SRCDIR)/, ElementWrapper.cpp ) \
														$(addprefix $(OBJDIR)/, ElementSolver.o) \
														$(addprefix $(OBJDIR)/, $(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/ElementSolver.o:	$(addprefix $(SRCDIR)/, ElementSolver.cpp ElementSolver.h) \
														$(addprefix $(OBJDIR)/, DataContainer.o Timer.o Pairs.o) \
														$(addprefix $(OBJDIR)/, AddRowGreedy.o ElementIpSolver.o) \
														$(addprefix $(SRCDIR)/, Utils.h)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/Pairs.o: $(addprefix $(SRCDIR)/, Pairs.cpp Pairs.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ElementIpSolver.o:	$(addprefix $(SRCDIR)/, ElementIpSolver.cpp ElementIpSolver.h) \
															$(addprefix $(OBJDIR)/, DataContainer.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

calcPairs: $(addprefix $(OBJDIR)/, CalcPairsWrapper.o)
	$(MPICXX)  $(CXXPFLAGS) -o $@  $(addprefix $(OBJDIR)/, $(CALCPAIRS_OBJ))

$(OBJDIR)/CalcPairsWrapper.o:	$(addprefix $(SRCDIR)/, CalcPairsWrapper.cpp ) \
															$(addprefix $(OBJDIR)/, DataContainer.o Timer.o Parallel.o) \
															$(addprefix $(OBJDIR)/, CalcPairsController.o CalcPairsWorker.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CalcPairsController.o:	$(addprefix $(SRCDIR)/, CalcPairsController.cpp CalcPairsController.h) \
																	$(addprefix $(OBJDIR)/, DataContainer.o Parallel.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CalcPairsWorker.o:	$(addprefix $(SRCDIR)/, CalcPairsWorker.cpp CalcPairsWorker.h) \
															$(addprefix $(OBJDIR)/, DataContainer.o Parallel.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

addRowGreedy: $(addprefix $(OBJDIR)/, AddRowGreedyWrapper.o)
	$(CXX) $(CXXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(GREEDY_OBJ)) $(CXXLNFLAGS)

$(OBJDIR)/AddRowGreedyWrapper.o:	$(addprefix $(SRCDIR)/, AddRowGreedyWrapper.cpp ) \
																	$(addprefix $(OBJDIR)/, AddRowGreedy.o) \
																	$(addprefix $(OBJDIR)/, $(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

rowColLp: $(addprefix $(OBJDIR)/, RowColLpWrapper.o)
	$(CXX) $(CXXLNDIRS) -o $@  $(addprefix $(OBJDIR)/, $(ROWCOL_OBJ)) $(CXXLNFLAGS)

$(OBJDIR)/RowColLpWrapper.o:	$(addprefix $(SRCDIR)/, RowColLpWrapper.cpp ) \
															$(addprefix $(OBJDIR)/, RowColLpSolver.o) \
															$(addprefix $(OBJDIR)/, $(COMMON_OBJ))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/AddRowGreedy.o:	$(addprefix $(SRCDIR)/, AddRowGreedy.cpp AddRowGreedy.h) \
													$(addprefix $(OBJDIR)/, DataContainer.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/RowColLpSolver.o:	$(addprefix $(SRCDIR)/, RowColLpSolver.cpp RowColLpSolver.h) \
														$(addprefix $(OBJDIR)/, DataContainer.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/NoMissSummary.o: $(addprefix $(SRCDIR)/, NoMissSummary.cpp NoMissSummary.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/DataContainer.o: $(addprefix $(SRCDIR)/, DataContainer.cpp DataContainer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Timer.o: $(addprefix $(SRCDIR)/, Timer.cpp Timer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/ConfigParser.o: $(addprefix $(SRCDIR)/, ConfigParser.cpp ConfigParser.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Parallel.o: $(addprefix $(SRCDIR)/, Parallel.cpp Parallel.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#---------------------------------------------------------------------------------------------------
.PHONY: clean
clean:
	/bin/rm -f $(OBJDIR)/*.o
#---------------------------------------------------------------------------------------------------
