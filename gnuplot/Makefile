AR        = ar
CC        = gcc
LD        = gcc

CPPFLAGS  = -D_XOPEN_SOURCE
CFLAGS    = -Wall -Wextra -Werror -std=c99 -g -I include
LDFLAGS   = -lm -g
ARFLAGS   = rs

CFLAGS   +=-D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L

SRCDIR    = src
OBJDIR    = obj
DEPDIR    = .deps
PROG      = gnuplot_output

CFILES    = gnuplot_output.c gnuplot_main_tools.c\
			gnuplot_measure.c gnuplot_main.c \
			gnuplot_read.c gnuplot_systemC.c

OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(CFILES:.c=.o)))
DEP       = $(addprefix $(OBJDIR)/, $(DEPDIR))
DEPFILES  = $(addprefix $(DEP)/, $(notdir $(CFILES:%.c=%.d)))

ifndef $(NO_DEP)
DEP_CFLAGS = -MP -MMD -MF $(DEP)/$*.d
CFLAGS    += $(DEP_CFLAGS)
endif

quiet-command = $(if $(VERB),$1,$(if $(2),@echo $2 && $1, @$1))

all : $(OBJDIR) $(DEP) $(PROG)

.PHONY  : clean distclean clean_measure

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(call quiet-command, $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<, "  CC     $<")

ifndef $(NO_DEP)
-include $(DEPFILES)
endif

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

clean_measure:
	rm -f perf_measures/* ; rm -f gnuplot_command/* ; rm -f gnuplot_data/* ;

# Local Variables:
# mode: makefile
# fill-column: 80
# End:
