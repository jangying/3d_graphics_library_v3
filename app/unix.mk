## 3Dgpl3 ##################################################
# Makefile for 3Dgpl's sample apps.                        #
#                                                          #
# Options:                                                 #
#  _PAINTER_ | _Z_BUFFER_         Hidden surface removal   #
#  _FIXED_ | _FLOAT_              Transform arithmetic.    #        
#                                                          #
# Use as:                                                  #
#  make -f unix.mk window         To compile various       #
#  make -f unix.mk polygon        sample applications.     #
#  make -f unix.mk bicubic                                 #
#  make -f unix.mk surface                                 #
#  make -f unix.mk group                                   #
#  make -f unix.mk texture                                 #
#  make -f unix.mk shade                                   #
#  make -f unix.mk line                                    #
#  make -f unix.mk tracer                                  #
#                                                          #
#  make -f unix.mk all            Compile all examples.    #     
#                                                          #
#  make -f unix.mk clean          To clean up.             #
############################################################

OP = -D_Z_BUFFER_ -D_FLOAT_
CC = gcc
LN = gcc
CF = -Wall -c -g $(OP) -o
LF = -g -L/usr/X11/lib
LL = -lX11 -lm

############################################################

OBJP = 1.o 2.o 3.o 4.o 5.o 6.o 7.o 8.o 9.o a.o b.o c.o d.o e.o f.o g.o
OBJT = 1.o 8.o a.o b.o c.o e.o h.o i.o
HEADP = ../clipper/clipper.h ../data/data.h ../engine/engine.h ../graphics/graphics.h ../hardware/hardware.h ../light/light.h ../shape/shape.h ../trans/trans.h
HEADT = ../graphics/graphics.h ../data/data.h ../hardware/hardware.h ../trace/trace.h ../vector/vector.h

############################################################

all: window polygon bicubic surface group texture shade line tracer inter

../hardware/hardware.c : ../hardware/unix/hardware.c
	cp ../hardware/unix/hardware.c ../hardware/hardware.c
../hardware/hardware.h : ../hardware/unix/hardware.h
	cp ../hardware/unix/hardware.h ../hardware/hardware.h
	
window: 00.o $(OBJP)
	$(LN) $(LF) -o window 00.o $(OBJP) $(LL)
polygon: 01.o $(OBJP)
	$(LN) $(LF) -o polygon 01.o $(OBJP) $(LL)
bicubic: 02.o $(OBJP)
	$(LN) $(LF) -o bicubic 02.o $(OBJP) $(LL)
surface: 03.o $(OBJP)
	$(LN) $(LF) -o surface 03.o $(OBJP) $(LL)
group: 04.o $(OBJP)
	$(LN) $(LF) -o group 04.o $(OBJP) $(LL)
texture: 05.o $(OBJP)
	$(LN) $(LF) -o texture 05.o $(OBJP) $(LL)
shade: 06.o $(OBJP)
	$(LN) $(LF) -o shade 06.o $(OBJP) $(LL)
line: 07.o $(OBJP)
	$(LN) $(LF) -o line 07.o $(OBJP) $(LL)
tracer: 08.o $(OBJT)
	$(LN) $(LF) -o tracer 08.o $(OBJT) $(LL)
inter: 09.o $(OBJP)
	$(LN) $(LF) -o inter 09.o $(OBJP) $(LL)

00.o: window.c $(HEADP)
	$(CC) $(CF)00.o window.c
01.o: polygon.c $(HEADP)
	$(CC) $(CF)01.o polygon.c
02.o: bicubic.c $(HEADP)
	$(CC) $(CF)02.o bicubic.c
03.o: surface.c $(HEADP)
	$(CC) $(CF)03.o surface.c
04.o: group.c $(HEADP)
	$(CC) $(CF)04.o group.c
05.o: texture.c $(HEADP)
	$(CC) $(CF)05.o texture.c
06.o: shade.c $(HEADP)
	$(CC) $(CF)06.o shade.c
07.o: line.c $(HEADT)
	$(CC) $(CF)07.o line.c
08.o: tracer.c $(HEADT)
	$(CC) $(CF)08.o tracer.c
09.o: inter.c $(HEADP)
	$(CC) $(CF)09.o inter.c

1.o: ../clipper/clipp-2d.c $(HEADP)
	$(CC) $(CF)1.o ../clipper/clipp-2d.c
2.o: ../clipper/clipp-3d.c $(HEADP)
	$(CC) $(CF)2.o ../clipper/clipp-3d.c

3.o: ../engine/eng-base.c $(HEADP)
	$(CC) $(CF)3.o ../engine/eng-base.c
4.o: ../engine/eng-poly.c $(HEADP)
	$(CC) $(CF)4.o ../engine/eng-poly.c
5.o: ../engine/eng-bcub.c $(HEADP)
	$(CC) $(CF)5.o ../engine/eng-bcub.c
6.o: ../engine/eng-surf.c $(HEADP)
	$(CC) $(CF)6.o ../engine/eng-surf.c
7.o: ../engine/eng-grup.c $(HEADP)
	$(CC) $(CF)7.o ../engine/eng-grup.c

8.o: ../graphics/grp-base.c $(HEADP)
	$(CC) $(CF)8.o ../graphics/grp-base.c
9.o: ../graphics/grp-poly.c $(HEADP)
	$(CC) $(CF)9.o ../graphics/grp-poly.c
a.o: ../graphics/grp-text.c $(HEADP)
	$(CC) $(CF)a.o ../graphics/grp-text.c

b.o: ../hardware/hardware.c $(HEADP) $(HEADT)
	$(CC) $(CF)b.o ../hardware/hardware.c

c.o: ../trans/trans-bs.c $(HEADP)
	$(CC) $(CF)c.o ../trans/trans-bs.c
d.o: ../trans/trans-ln.c $(HEADP)
	$(CC) $(CF)d.o ../trans/trans-ln.c

e.o: ../data/data.c $(HEADP) $(HEADT)
	$(CC) $(CF)e.o ../data/data.c

f.o: ../light/light.c $(HEADP)
	$(CC) $(CF)f.o ../light/light.c

g.o: ../shape/shape.c $(HEADP)
	$(CC) $(CF)g.o ../shape/shape.c

h.o: ../trace/trace.c $(HEADT)
	$(CC) $(CF)h.o ../trace/trace.c

i.o: ../vector/vector.c $(HEADT)
	$(CC) $(CF)i.o ../vector/vector.c

clean:
	rm ../hardware/hardware.h
	rm ../hardware/hardware.c
	rm *.o
	rm window polygon bicubic surface group texture shade line tracer inter

############################################################
