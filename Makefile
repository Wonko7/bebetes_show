# The Bebete Show Makefile...
# todo : chercher du cote de install (man install)
#
# MAINTENANCE :
#
# Garder les fichiers source dans le dossier src.
# rajouter les .c a compiler dans la variable FICHIERS
# (ou la remplacer par FICHIERS=src/*.c ... bof hein)
#
# et ajouter le .o dans la macro OBJS, et ajouter les
# cibles avec les dependances. (voir en bas)
#
# Tous fichiers externes (images, textures...) sont
# dans le dossier share/bebetes_show/
# donc faut y acceder par ../share/bebetes_show/
#
# install c'est de la merde... je prefere le faire
# a la main et puis c'est tout...
#
# pour obtenir les dependances, soit les rajouter a la
# main avec gcc -MM mais y'a plus simple. toutes les
# suprrimer et lancer ca  :
#  gcc -MM src/*.c 1>> Makefile
#${GCC} ${GTK_DEPS} ${GLEXT_DEPS} ${OBJS} -o ${EXE}
#	mv ${EXE} bin/${EXE}

ANSI=-ansi
WARNING=-W -Wall
GTK_DEPS=`pkg-config --cflags --libs gtk+-2.0`
GLEXT_DEPS=`pkg-config --libs gtkgl-2.0` `pkg-config --libs glu`
GTK_DEPS2=`pkg-config --cflags gtkgl-2.0`
DLIB=-rdynamic -ldl -lm
CLIB=-fPIC -shared -nostartfiles
GCC=gcc -O2

DIR_INSTALL=/home/wjc/work/bebetes_show/local
NOM_PROJ_INSTALL=bebetes_show

OBJS=src/stat.o src/main_gtk.o src/ogl.o src/loader.o src/textures.o src/divers.o src/taskbrowser.o src/tga.o src/fourmi.o src/remote_control.o src/list_fav.o src/map.o src/object.o src/propag.o src/loading.o src/discrimination.o src/map_editor.o src/bmp.o src/options.o src/iaction.o src/info.o
#OBJS=src/main_gtk.o src/ogl.o src/loader.o src/textures.o src/divers.o src/taskbrowser.o src/tga.o src/fourmi.o src/remote_control.o src/list_fav.o src/map.o src/object.o src/propag.o src/loading.o src/discrimination.o src/map_editor.o src/bmp.o src/png.o src/jpeg.o src/options.o src/iaction.o src/info.o

OBJS_GE=src/gtk+extra/gtkplot.o src/gtk+extra/gtkplotdata.o src/gtk+extra/gtkplotcanvas.o src/gtk+extra/gtkplotcanvasplot.o src/gtk+extra/gtkplotcanvastext.o src/gtk+extra/gtkplotcanvaspixmap.o src/gtk+extra/gtkplotcanvasellipse.o src/gtk+extra/gtkplotcanvasline.o src/gtk+extra/gtkplotcanvasrectangle.o src/gtk+extra/gtkpsfont.o src/gtk+extra/gtkplotgdk.o src/gtk+extra/gtkextra-marshal.o src/gtk+extra/gtkplotpolar.o src/gtk+extra/gtkplot3d.o src/gtk+extra/gtkplotpc.o src/gtk+extra/gtkplotarray.o src/gtk+extra/gtkextra.o src/gtk+extra/gtkplotps.o src/gtk+extra/gtkplotsurface.o src/gtk+extra/gtkplotdt.o src/gtk+extra/gtkplotbar.o src/gtk+extra/gtkplotprint.o


FICHIERS_BACKUP=bin doc lib share src Makefile bck.ml
B=bin
D=doc
S=share
L=lib
FICHIERS_UNINSTALL=${DIR_INSTALL}/bin/${NOM_PROJ_INSTALL} ${DIR_INSTALL}/share/${NOM_PROJ_INSTALL} ${DIR_INSTALL}/doc/${NOM_PROJ_INSTALL} ${DIR_INSTALL}/lib/lib_*_behavior.so

EXE=${NOM_PROJ_INSTALL}

WORD=

#BACKUP=`date +"bbs_%Y-%m-%d_%H-%M_\`fortune | cut -d" " -f3 | head -n 1\`.tar.bz2"`
FO=\`fortune | cut -d" " -f3 | head -n 1\`
FU1=\`sex | cut -d" " -f5\`
FU2=\`sex | cut -d" " -f10\`
FU3=\`sex | cut -d" " -f8\`
BACKUP=`date +"bbs_%Y-%m-%d_%H-%M_${FO}-${FO}-${FO}.tar.bz2"`
BACKUP-SEX=`date +"bbs_%Y-%m-%d_%H-%M_${FU1}-${FU2}-${FU3}.tar.bz2"`
BACKUP2=`date +"bbs_%Y-%m-%d_%H-%M.tar.bz2"`
TAR=tar cjf


.SUFFIXES: .o .c
.c.o:
	$(GCC) ${GTK_DEPS2} ${GLEXT_DEPS2} -c $< -o $@
ebbs: libs gtkextra libs ${OBJS}
#	${GCC} ${DLIB} ${GTK_DEPS} ${GLEXT_DEPS} ${OBJS} -ljpeg -o bin/${EXE}
	${GCC} ${OBJS} ${OBJS_GE} -o bin/${EXE} ${DLIB} ${GTK_DEPS} ${GLEXT_DEPS}
	@echo "       ,_      _,       "
	@echo "         '.__.'         "
	@echo "    '-,   (__)   ,-'    "
	@echo "      '._ .::. _.'      "
	@echo "        _'(^^)'_        "
	@echo "     _,\` \`>\/<\` \`,_     "
	@echo "    \`  ,-\` )( \`-,  \`    "
	@echo "       |  /==\  |       "
	@echo "     ,-'  |=-|  '-,     "
	@echo "          )-=(          "
	@echo "          \__/          "
	@echo "                        "
	@echo "      Bebetes_show      "
gtkextra:
	cd src/gtk+extra/ && make
ansi:
	make "GCC=gcc -ansi"
no-w:
	make "GCC=gcc"
gdb:
	cd src/gtk+extra/ && make "GCC=gcc -g"
	make "GCC=gcc -g ${WARNING}"
efence:
	make "GCC=gcc -g -lefence"
clean:
	-rm bin/* lib/* src/*[^ch] src/gtk+extra/*.o
clean_src:
	-rm src/*~ src/\#* src/*\#
clean_exe:
	-rm bin/* lib/* src/*.o
b:
	ocaml bck.ml "${FICHIERS_BACKUP}"
backup:
	-mkdir ${NOM_PROJ_INSTALL}
	cp -r ${FICHIERS_BACKUP} ${NOM_PROJ_INSTALL}
	${TAR} ${BACKUP} ${NOM_PROJ_INSTALL}
	rm -r ${NOM_PROJ_INSTALL}
backup-no-fortune:
	mkdir ${NOM_PROJ_INSTALL}
	cp -r ${FICHIERS_BACKUP} ${NOM_PROJ_INSTALL}
	${TAR} ${BACKUP2} ${NOM_PROJ_INSTALL}
	rm -r ${NOM_PROJ_INSTALL}
install:
	mkdir -p ${DIR_INSTALL}/bin/
	mkdir -p ${DIR_INSTALL}/doc/${NOM_PROJ_INSTALL}/
	mkdir -p ${DIR_INSTALL}/share/${NOM_PROJ_INSTALL}/
	mkdir -p ${DIR_INSTALL}/lib/
	cp -r ${B}/* ${DIR_INSTALL}/bin/
	cp -r ${D}/* ${DIR_INSTALL}/doc/
	cp -r ${S}/* ${DIR_INSTALL}/share/
	cp -r ${L}/* ${DIR_INSTALL}/lib/
	chown root:bin -R ${DIR_INSTALL}/doc/${NOM_PROJ_INSTALL}/ ${DIR_INSTALL}/share/${NOM_PROJ_INSTALL}/ ${DIR_INSTALL}/bin/${NOM_PROJ_INSTALL} ${DIR_INSTALL}/lib/lib_*_behavior.so
	chmod 755  ${DIR_INSTALL}/bin/${NOM_PROJ_INSTALL} ${DIR_INSTALL}/lib/lib_*_behavior.so
	chmod 755 -R ${DIR_INSTALL}/doc/${NOM_PROJ_INSTALL}/ ${DIR_INSTALL}/share/${NOM_PROJ_INSTALL}/
	chmod 755 -R ${DIR_INSTALL}/share/${NOM_PROJ_INSTALL}/save
uninstall:
	rm -r ${FICHIERS_UNINSTALL}
info:
	@echo "############ GTK :"
	@echo
	@echo "${GTK_DEPS}"
	@echo
	@echo "############ GtkGlExt :"
	@echo
	@echo "${GLEXT_DEPS}"
	@echo
	@echo "a  compiler  : ${OBJS}"
	@echo "a archiver  : ${FICHIERS_BACKUP}"
	@echo "a installer : ${B} ${D} ${S}"
	@echo "a supprimer : ${FICHIERS_UNINSTALL}"
	@echo "dossier d'install : ${DIR_INSTALL}"

deps:
	@echo "##################################" >> Makefile
	gcc -MM src/*.c 1>> Makefile
	@echo "Editer le Makefile a la main, rajouter src/"
	@echo "Plus gerer le cas a part de la lib"
discrimination.o: src/discrimination.c src/lib_bbs_behavior.h src/loader.h src/map.h
divers.o: src/divers.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/map.h
fourmi.o: src/fourmi.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/textures.h src/tga.h src/fourmi.h src/f.h src/map.h
list_fav.o: src/list_fav.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/f.h
loader.o: src/loader.c src/lib_bbs_behavior.h src/loader.h src/divers.h
loading.o: src/loading.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/taskbrowser.h
main_gtk.o: src/main_gtk.c src/lib_bbs_behavior.h src/f.h src/loader.h src/divers.h src/map.h src/map_editor.h src/textures.h   src/taskbrowser.h src/remote_control.h src/list_fav.h src/loading.h src/options.h
map.o: src/map.c src/lib_bbs_behavior.h src/loader.h src/map.h src/divers.h src/camera.h
map_editor.o: src/map_editor.c src/map_editor.h src/lib_bbs_behavior.h src/loader.h src/map.h src/divers.h
object.o: src/object.c src/lib_bbs_behavior.h src/loader.h src/divers.h
ogl.o: src/ogl.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/fourmi.h src/f.h src/loading.h src/map.h
propag.o: src/propag.c src/lib_bbs_behavior.h src/loader.h src/map.h src/propag.h
remote_control.o: src/remote_control.c src/lib_bbs_behavior.h src/loader.h src/divers.h src/remote_control.h src/f.h
taskbrowser.o: src/taskbrowser.c src/taskbrowser.h src/lib_bbs_behavior.h src/loader.h src/divers.h
textures.o: src/textures.c src/textures.h src/glbmp.h src/tga.h
jpeg.o: src/jpeg.c
	$(GCC) ${GTK_DEPS2} ${GLEXT_DEPS2} -ljpeg -c $< -o $@
options.o: src/options.c src/options.h
iaction.o: src/iaction.c src/iaction.h src/divers.h
info.o: src/info.c src/info.h src/divers.h src/camera.h src/loader.h src/lib_bbs_behavior.c
stat.o: src/stat.c src/stat.h


libs: lib/lib_bbs_behavior.so lib/lib_trafic_behavior.so lib/lib_berserkers_behavior.so# lib/lib_camel_behavior.so lib/lib_lab_behavior.so


lib/lib_bbs_behavior.so: src/lib_bbs_behavior.c src/lib_bbs_behavior.h src/divers.h
	${GCC} ${GTK_DEPS2} ${GLEXT_DEPS2} ${CLIB} -o lib/lib_bbs_behavior.so src/lib_bbs_behavior.c
lib/lib_camel_behavior.so: src/lib_camel_behavior.c src/lib_bbs_behavior.h src/divers.h
	${GCC} ${GTK_DEPS2} ${GLEXT_DEPS2} ${CLIB} -o lib/lib_camel_behavior.so src/lib_camel_behavior.c
lib/lib_lab_behavior.so: src/lib_lab_behavior.c src/lib_bbs_behavior.h src/divers.h
	${GCC} ${GTK_DEPS2} ${GLEXT_DEPS2} ${CLIB} -o lib/lib_lab_behavior.so src/lib_lab_behavior.c
lib/lib_trafic_behavior.so: src/lib_trafic_behavior.c src/lib_bbs_behavior.h src/divers.h
	${GCC} ${GTK_DEPS2} ${GLEXT_DEPS2} ${CLIB} -o lib/lib_trafic_behavior.so src/lib_trafic_behavior.c

lib/lib_berserkers_behavior.so: src/lib_berserkers_behavior.c src/lib_bbs_behavior.h src/divers.h
	${GCC} ${GTK_DEPS2} ${GLEXT_DEPS2} ${CLIB} -o lib/lib_berserkers_behavior.so src/lib_berserkers_behavior.c
