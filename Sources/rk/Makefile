NAME= librk.a
CFLAGS =-g  -D__HAIKU__ -isystem../  -iquote../canna -iquote../rk
INSTALLDIR = ../..
OBJDIR = ../../object/librk


SRCS= \
bits.c \
bun.c \
context.c \
dd.c \
dic.c \
dicsw.c \
fq.c \
kana.c \
ncache.c \
ngram.c \
nword.c \
permdic.c \
tempdic.c \
util.c

OBJS= \
$(OBJDIR)/bits.o \
$(OBJDIR)/bun.o \
$(OBJDIR)/context.o \
$(OBJDIR)/dd.o \
$(OBJDIR)/dic.o \
$(OBJDIR)/dicsw.o \
$(OBJDIR)/fq.o \
$(OBJDIR)/kana.o \
$(OBJDIR)/ncache.o \
$(OBJDIR)/ngram.o \
$(OBJDIR)/nword.o \
$(OBJDIR)/permdic.o \
$(OBJDIR)/tempdic.o \
$(OBJDIR)/util.o

$(OBJDIR)/%.o: ./%.c
	-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

librk.a: $(OBJS)
	$(AR) rcs $(INSTALLDIR)/librk.a $(OBJS)

