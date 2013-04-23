AR        = ar
CC        = gcc
LD        = gcc

CPPFLAGS  = 
CFLAGS    = -Wall -Wextra -Werror -std=c99 -g -I ../include -I ./include
LDFLAGS   = -lm -lrt -lpthread -L.. -lxsim -g
ARFLAGS   = rs

CFLAGS   +=-D_GNU_SOURCE #-D_POSIX_C_SOURCE=199309L
#CFLAGS   +=-DXSIM_MODEL4_DEBUG
CFLAGS   +=-DXSIM_MODEL_DEBUG
#CFLAGS   +=-DXSIM_MODEL_HDEBUG

SRCDIR    = src
OBJDIR    = obj
DEPDIR    = .deps
PROG      = test_poisson
			#test_model 

CFILES    = test_model.c poisson_law_generator.c test_poisson.c

OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(CFILES:.c=.o)))
DEP       = $(addprefix $(OBJDIR)/, $(DEPDIR))
DEPFILES  = $(addprefix $(DEP)/, $(notdir $(CFILES:%.c=%.d)))

ifndef $(NO_DEP)
DEP_CFLAGS = -MP -MMD -MF $(DEP)/$*.d
CFLAGS    += $(DEP_CFLAGS)
endif

quiet-command = $(if $(VERB),$1,$(if $(2),@echo $2 && $1, @$1))

all : $(OBJDIR) $(DEP) $(PROG)

.PHONY  : clean distclean

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(call quiet-command, $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<, "  CC     $<")

ifndef $(NO_DEP)
-include $(DEPFILES)
endif

$(LIB): $(OBJECTS)
	$(call quiet-command, $(AR) $(ARFLAGS) $@ $^, "  AR     $@")


# $(PROG) : $(OBJECTS)
# 	$(call quiet-command, $(LD) $^ $(LDFLAGS) -o $@, "  LD     $@")
# 

test_model: $(OBJDIR)/test_model.o 
	$(call quiet-command, $(LD) $^ $(LDFLAGS) -o $@, "  LD     $@")

test_poisson: $(OBJDIR)/test_poisson.o $(OBJDIR)/poisson_law_generator.o
	$(call quiet-command, $(LD) $^ $(LDFLAGS) -o $@, "  LD     $@")

$(OBJDIR):
	$(call quiet-command, mkdir -p $(OBJDIR),)

$(DEP):
	$(call quiet-command, mkdir -p $(DEP),)

clean    :
	$(call quiet-command, rm -f $(OBJECTS) $(PROG) *~, "  CLEAN    ")

distclean: clean
	$(call quiet-command, rm -fr $(OBJDIR), "  DISTCLEAN    ")

xsim_lib :
	make -C ..
	make clean
	make $(PROG)

# Local Variables:
# mode: makefile
# fill-column: 80
# End:
