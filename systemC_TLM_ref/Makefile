AR        = ar
CC        = g++
LD        = g++

# configuration for Sarrazin laptop
#TARGET_ARCH = linux
#SYSTEMC	 	= /home/sarrazin/ENSIMAG/PFE/SystemC/systemc-2.2.0

# configuration for pomahaka
#TARGET_ARCH = linux64
#SYSTEMC	 = /opt/systemc-2.2.0

# configuration for Nil
TARGET_ARCH = linux64
SYSTEMC	    = /opt/systemc-2.3
#/altamaha/softs/opt/systemc-2.2.0


CPPFLAGS  = 
CFLAGS    = -Wall -Wextra -g -I ./include -I$(SYSTEMC)/include 
#CFLAGS	 += -Werror
LDFLAGS   = -lm -lrt -lpthread -g -L$(SYSTEMC)/lib-$(TARGET_ARCH) -lsystemc -lbfd -liberty 
ARFLAGS   = rs

CFLAGS   +=-D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L
#CFLAGS   +=-DDEBUG_SYSTEMC

SRCDIR    = src
OBJDIR    = obj
DEPDIR    = .deps
PROG      = simulation_systemC_ref

#CPPFILES  = xsim_node_systemC.cpp
CPPFILES  = $(notdir $(wildcard $(SRCDIR)/*.cpp))

OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(CPPFILES:.cpp=.o)))
DEP       = $(addprefix $(OBJDIR)/, $(DEPDIR))
DEPFILES  = $(addprefix $(DEP)/, $(notdir $(CPPFILES:%.cpp=%.d)))

COMMA = ,

ifndef $(NO_DEP)
DEP_CFLAGS = -MP -MMD -MF $(DEP)/$*.d
CFLAGS    += $(DEP_CFLAGS)
endif

#VERB = 1
quiet-command = $(if $(VERB),$1,$(if $(2),@echo $2 && $1, @$1))

all : $(OBJDIR) $(DEP) $(PROG)

.PHONY  : clean distclean

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(call quiet-command, $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<, "  CC     $<")

ifndef $(NO_DEP)
-include $(DEPFILES)
endif

$(LIB): $(OBJECTS)
	$(call quiet-command, $(AR) $(ARFLAGS) $@ $^, "  AR     $@")


$(PROG) : $(OBJECTS)
	$(call quiet-command, $(LD) $^ $(LDFLAGS) -o $@, "  LD     $@")
 

$(OBJDIR):
	$(call quiet-command, mkdir -p $(OBJDIR),)

$(DEP):
	$(call quiet-command, mkdir -p $(DEP),)

clean    :
	$(call quiet-command, rm -f $(OBJECTS) $(PROG) *~, "  CLEAN    ")

distclean: clean
	$(call quiet-command, rm -fr $(OBJDIR), "  DISTCLEAN    ")

# Local Variables:
# mode: makefile
# fill-column: 80
# End:
