## 3Dgpl3 ##################################################
# Makefile for 3Dgpl's sample apps.                        #
#                                                          #
# Options:                                                 #
#  _PAINTER_ | _Z_BUFFER_         Hidden surface removal   #
#  _FIXED_ | _FLOAT_              Transform arithmetic.    #        
#                                                          #
# Use as:                                                  #
#  nmake -f windows.mk window     To compile various       #
#  nmake -f windows.mk polygon    sample applications.     #
#  nmake -f windows.mk bicubic                             #
#  nmake -f windows.mk surface                             #
#  nmake -f windows.mk group                               #
#  nmake -f windows.mk texture                             #
#  nmake -f windows.mk shade                               #
#  nmake -f windows.mk line                                #
#  nmake -f windows.mk tracer                              #
#                                                          #
#  nmake -f windows.mk all        To compile all examples. #
#                                                          #
#  nmake -f windows.mk clean      To clean up.             #
############################################################

OP = -D_Z_BUFFER_ -D_FLOAT_
CC = cl
LN = link
CF = /c $(OP) /Fo
LL = gdi32.lib user32.lib

############################################################

OBJP = 1.obj 2.obj 3.obj 4.obj 5.obj 6.obj 7.obj 8.obj 9.obj a.obj b.obj c.obj d.obj e.obj f.obj g.obj
OBJT = 1.obj 8.obj a.obj b.obj c.obj e.obj h.obj i.obj
HEADP = ../clipper/clipper.h ../data/data.h ../engine/engine.h ../graphics/graphics.h ../hardware/hardware.h ../light/light.h ../shape/shape.h ../trans/trans.h
HEADT = ../graphics/graphics.h ../data/data.h ../hardware/hardware.h ../trace/trace.h ../vector/vector.h

############################################################

all: window polygon bicubic surface group texture shade line tracer inter

../hardware/hardware.c : ../hardware/windows/hardware.c
	copy ..\hardware\windows\hardware.c ..\hardware\hardware.c
../hardware/hardware.h : ../hardware/windows/hardware.h
	copy ..\hardware\windows\hardware.h ..\hardware\hardware.h
	
window: 00.obj $(OBJP)
	$(LN) /out:window.exe 00.obj ?.obj $(LL)
polygon: 01.obj $(OBJP)
	$(LN) /out:polygon.exe 01.obj ?.obj $(LL)
bicubic: 02.obj $(OBJP)
	$(LN) /out:bicubic.exe 02.obj ?.obj $(LL)
surface: 03.obj $(OBJP)
	$(LN) /out:surface.exe 03.obj ?.obj $(LL)
group: 04.obj $(OBJP)
	$(LN) /out:group.exe 04.obj ?.obj $(LL)
texture: 05.obj $(OBJP)
	$(LN) /out:texture.exe 05.obj ?.obj $(LL)
shade: 06.obj $(OBJP)
	$(LN) /out:shade.exe 06.obj ?.obj $(LL)
line: 07.obj $(OBJP)
	$(LN) /out:line.exe 07.obj ?.obj $(LL)
tracer: 08.obj $(OBJT)
	$(LN) /out:tracer.exe 08.obj ?.obj $(LL)
inter: 09.obj $(OBJP)
	$(LN) /out:inter.exe 09.obj ?.obj $(LL)

00.obj: window.c $(HEADP)
	$(CC) $(CF)00.obj window.c
01.obj: polygon.c $(HEADP)
	$(CC) $(CF)01.obj polygon.c
02.obj: bicubic.c $(HEADP)
	$(CC) $(CF)02.obj bicubic.c
03.obj: surface.c $(HEADP)
	$(CC) $(CF)03.obj surface.c
04.obj: group.c $(HEADP)
	$(CC) $(CF)04.obj group.c
05.obj: texture.c $(HEADP)
	$(CC) $(CF)05.obj texture.c
06.obj: shade.c $(HEADP)
	$(CC) $(CF)06.obj shade.c
07.obj: line.c $(HEADP)
	$(CC) $(CF)07.obj line.c
08.obj: tracer.c $(HEADT)
	$(CC) $(CF)08.obj tracer.c
09.obj: inter.c $(HEADP)
	$(CC) $(CF)09.obj inter.c

1.obj: ../clipper/clipp-2d.c $(HEADP)
	$(CC) $(CF)1.obj ../clipper/clipp-2d.c
2.obj: ../clipper/clipp-3d.c $(HEADP)
	$(CC) $(CF)2.obj ../clipper/clipp-3d.c

3.obj: ../engine/eng-base.c $(HEADP)
	$(CC) $(CF)3.obj ../engine/eng-base.c
4.obj: ../engine/eng-poly.c $(HEADP)
	$(CC) $(CF)4.obj ../engine/eng-poly.c
5.obj: ../engine/eng-bcub.c $(HEADP)
	$(CC) $(CF)5.obj ../engine/eng-bcub.c
6.obj: ../engine/eng-surf.c $(HEADP)
	$(CC) $(CF)6.obj ../engine/eng-surf.c
7.obj: ../engine/eng-grup.c $(HEADP)
	$(CC) $(CF)7.obj ../engine/eng-grup.c

8.obj: ../graphics/grp-base.c $(HEADP)
	$(CC) $(CF)8.obj ../graphics/grp-base.c
9.obj: ../graphics/grp-poly.c $(HEADP)
	$(CC) $(CF)9.obj ../graphics/grp-poly.c
a.obj: ../graphics/grp-text.c $(HEADP)
	$(CC) $(CF)a.obj ../graphics/grp-text.c

b.obj: ../hardware/hardware.c $(HEADP) $(HEADT)
	$(CC) $(CF)b.obj ../hardware/hardware.c

c.obj: ../trans/trans-bs.c $(HEADP)
	$(CC) $(CF)c.obj ../trans/trans-bs.c
d.obj: ../trans/trans-ln.c $(HEADP)
	$(CC) $(CF)d.obj ../trans/trans-ln.c

e.obj: ../data/data.c $(HEADP) $(HEADT)
	$(CC) $(CF)e.obj ../data/data.c

f.obj: ../light/light.c $(HEADP)
	$(CC) $(CF)f.obj ../light/light.c

g.obj: ../shape/shape.c $(HEADP)
	$(CC) $(CF)g.obj ../shape/shape.c

h.obj: ../trace/trace.c $(HEADT)
	$(CC) $(CF)h.obj ../trace/trace.c

i.obj: ../vector/vector.c $(HEADT)
	$(CC) $(CF)i.obj ../vector/vector.c

clean:
	del ..\hardware\hardware.h
	del ..\hardware\hardware.c
	del *.obj
	del *.exe

############################################################
