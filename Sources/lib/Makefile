NAME= libcanna.a
CFLAGS =-g  -D__HAIKU__ -isystem../  -iquote../canna -iquote../rk
INSTALLDIR = ../..
OBJDIR = ../../object/libcanna

SRCS= \
bunsetsu.c \
bushu.c \
chikuji.c \
commondata.c \
defaultmap.c \
ebind.c \
empty.c \
henkan.c \
hex.c \
ichiran.c \
jishu.c \
jrbind.c \
kctrl.c \
keydef.c \
kigo.c \
lisp.c \
mode.c \
multi.c \
obind.c \
onoff.c \
parse.c \
RKkana.c \
RKroma.c \
romaji.c \
uiutil.c \
uldefine.c \
uldelete.c \
ulhinshi.c \
ulkigo.c \
ulmount.c \
util.c \
yesno.c

OBJS= \
$(OBJDIR)/bunsetsu.o \
$(OBJDIR)/bushu.o \
$(OBJDIR)/chikuji.o \
$(OBJDIR)/commondata.o \
$(OBJDIR)/defaultmap.o \
$(OBJDIR)/ebind.o \
$(OBJDIR)/empty.o \
$(OBJDIR)/henkan.o \
$(OBJDIR)/hex.o \
$(OBJDIR)/ichiran.o \
$(OBJDIR)/jishu.o \
$(OBJDIR)/jrbind.o \
$(OBJDIR)/kctrl.o \
$(OBJDIR)/keydef.o \
$(OBJDIR)/kigo.o \
$(OBJDIR)/lisp.o \
$(OBJDIR)/mode.o \
$(OBJDIR)/multi.o \
$(OBJDIR)/obind.o \
$(OBJDIR)/onoff.o \
$(OBJDIR)/parse.o \
$(OBJDIR)/RKkana.o \
$(OBJDIR)/RKroma.o \
$(OBJDIR)/romaji.o \
$(OBJDIR)/uiutil.o \
$(OBJDIR)/uldefine.o \
$(OBJDIR)/uldelete.o \
$(OBJDIR)/ulhinshi.o \
$(OBJDIR)/ulkigo.o \
$(OBJDIR)/ulmount.o \
$(OBJDIR)/util.o \
$(OBJDIR)/yesno.o

$(OBJDIR)/%.o: ./%.c
	-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

all: libcanna.a

libcanna.a: $(OBJS)
	$(AR) rcs $(INSTALLDIR)/libcanna.a $(OBJS)

