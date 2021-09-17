CC ?= gcc
LIBS = -lncurses -lmenu

SRC = main.c sokoban.c

.PHONY=all clean

OBJDIR := objdir
OBJS := $(addprefix $(OBJDIR)/,$(SRC:.c=.o))

all: sokoban

$(OBJDIR)/%.o: %.c
	$(CC) -c -o $@ $<

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

sokoban: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -rf $(OBJDIR)
	rm -f sokoban
