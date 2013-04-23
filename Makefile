AR        = ar
CC        = gcc
LD        = gcc

CPPFLAGS  = -D_XOPEN_SOURCE
CFLAGS    = -Wall -Wextra -Werror -std=c99 -g -I include
LDFLAGS   = -lm -g -lrt
ARFLAGS   = rs

CFLAGS   +=-D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L


#CFLAGS   +=-DXSIM_MAIN_HDEBUG -DXSIM_MSG_BOX_HDEBUG -DXSIM_MSG_HDEBUG 
#CFLAGS   +=-DXSIM_NODE_HDEBUG
#CFLAGS   +=-DXSIM_NODE_DEBUG
#CFLAGS   +=-DXSIM_LISTENER_DEBUG
#CFLAGS   +=-DXSIM_MSG_DEBUG
#CFLAGS   +=-DXSIM_IFACE_DEBUG
#CFLAGS   +=-DXSIM_SYNC_HDEBUG
#CFLAGS   +=-DXSIM_TIME_DEBUG
#CFLAGS   +=-DXSIM_TIME_HDEBUG
#CFLAGS   +=-DXSIM_PERF_DEBUG
#CFLAGS   +=-DXSIM_TIME_MODEL1_DEBUG
#CFLAGS   +=-DXSIM_TIME_MODEL_COMMON_DEBUG
#CFLAGS   +=-DXSIM_TIME_MODEL4_DEBUG
#CFLAGS   +=-DXSIM_TIME_MODEL4_HDEBUG
#CFLAGS   +=-DXSIM_TIME_DEBUG
#CFLAGS   +=-DXSIM_CENTRAL_LIST_DEBUG
#CFLAGS   +=-DXSIM_FIFO_LIST_DEBUG


SRCDIR    = src
OBJDIR    = obj
DEPDIR    = .deps
LIB       = libxsim.a

CFILES    = src/xsim.c src/xsim_node.c src/xsim_sync.c \
		src/xsim_msg_box.c src/xsim_iface.c src/xsim_msg.c \
		src/xsim_time.c \
		src/xsim_topology.c src/xsim_performance_evaluation.c \
		src/xsim_performance_output.c \
		src/xsim_time_model_common.c \
		src/xsim_performance_deltaT.c\
		src/xsim_listener.c \
		src/xsim_garbage_list.c \
		src/xsim_central_list.c \
		src/xsim_FIFO_list.c 
		#src/xsim_time_model4.c \
		src/xsim_time_model1.c \
		src/xsim_sc_time_model1.c

OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(CFILES:.c=.o)))
DEP       = $(addprefix $(OBJDIR)/, $(DEPDIR))
DEPFILES  = $(addprefix $(DEP)/, $(notdir $(CFILES:%.c=%.d)))

ifndef $(NO_DEP)
DEP_CFLAGS = -MP -MMD -MF $(DEP)/$*.d
CFLAGS    += $(DEP_CFLAGS)
endif

quiet-command = $(if $(VERB),$1,$(if $(2),@echo $2 && $1, @$1))

all : $(OBJDIR) $(DEP) $(LIB)

.PHONY  : clean distclean

$(OBJDIR)/%.o : $(SRCDIR)/%.c
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
