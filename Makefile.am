bin_PROGRAMS = oscbridge
oscbridge_SOURCES = main.c
AM_LDFLAGS = -llo

if DEBUG
	AM_CFLAGS = -g3 -O0 -DDEBUG
	AM_CXXFLAGS = -g3 -O0
else
	AM_CFLAGS = -O2
	AM_CXXFLAGS = -O2
endif
