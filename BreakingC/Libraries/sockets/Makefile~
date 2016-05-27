# Javier Abellán. 20 Junio 2000
#
# Makefile para la construcción de la mini-librería de sockets.
#

# Objetivo del Makefile, la librería
all : libChSocket.a

# Ficheros objeto que van dentro de la minilibrería.
OBJETOS=\
	Socket_Servidor.o\
	Socket_Cliente.o\
	Socket.o

# Establecemos una dependencia, la librería depende de los ficheros objeto.
libChSocket.a : libChSocket.a($(OBJETOS))
