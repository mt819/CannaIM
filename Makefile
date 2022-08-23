CFLAGS =-g  -isystem../canna/  -iquote../canna/ -isystem./
RSRCS=kanbe_x86.rsrc
LIBS= be tracker textencoding canna rk stdc++
INSTALLDIR = ./
OBJDIR = ./object
SRCDIR = ./kanbe_source

SRCS= \
$(SRCDIR)/FEPView.cpp \
$(SRCDIR)/PrefDlg.cpp \
$(SRCDIR)/kanBeApp.cpp \
$(SRCDIR)/kanbe_main.cpp

OBJS= \
$(OBJDIR)/FEPView.o \
$(OBJDIR)/PrefDlg.o \
$(OBJDIR)/kanBeApp.o \
$(OBJDIR)/kanbe_main.o

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

all: kanbe

libcanna:
	make -C Sources/lib
	make -C Sources/rk

kanbe: $(OBJS)
	make -C Sources/lib
	make -C Sources/rk
	$(CC) $(OBJS) -l$(LIBS) -o $(INSTALLDIR)kanbe -c $<

