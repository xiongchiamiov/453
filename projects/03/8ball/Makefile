# Makefile for terminal driver (TTY)

.include <bsd.own.mk>

PROG=	8ball
SRCS=	8ball.c

DPADD+=	${LIBDRIVER} ${LIBSYS} ${LIBTIMERS}
LDADD+=	-ldriver -lsys -ltimers

MAN=

BINDIR?= /usr/sbin
INSTALLFLAGS+=	-S 16k


# Needs kernel/const.h, etc
CPPFLAGS+=	-I${MINIXSRCDIR}

.include <bsd.prog.mk>
.include <bsd.subdir.mk>
