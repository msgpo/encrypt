.PHONY: encrypt gui all gui-all install install-all clean distclean uninstall

LIB_MAKE      = 
LIB_INSTALL   = 
LIB_CLEAN     = 
LIB_UNINSTALL = 

PO_MAKE       = 
PO_INSTALL    = 
PO_CLEAN      = 
PO_UNINSTALL  = 

OPTIONS := -o encrypt -std=c99 -Wall -Wextra -O2 -pipe -ldl -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -I ./ ${OS_OPTS}
GUIOPTS := `pkg-config --cflags --libs gtk+-2.0` -lpthread -D_BUILD_GUI_
COMMON  := common/common.c common/list.c src/encrypt.c

encrypt:
# build the main executible
	 @gcc $(OPTIONS) $(COMMON)
	-@echo "compiled \`src/encrypt.c common/common.c common/list.c' --> \`encrypt'"

gui:
# build the gui package
	 @gcc $(OPTIONS) $(COMMON) $(GUIOPTS) src/callbacks.c src/interface.c src/support.c
	-@echo "compiled \`src/encrypt.c common/common.c src/callbacks.c src/interface.c src/support.c --> encrypt'"

-include lib/*.mk
-include po/*.mk
all: encrypt $(LIB_MAKE) $(PO_MAKE)
gui-all: gui $(LIB_MAKE) $(PO_MAKE)

install:
# install the main executible, then softlink to it from /usr/bin
	 @install -c -m 755 -s -D -T encrypt $(PREFIX)/usr/lib/encrypt/encrypt
	 @ln -fs /usr/lib/encrypt/encrypt $(PREFIX)/usr/bin/
	-@echo "installed \`encrypt' --> \`$(PREFIX)/usr/bin/encrypt'"
# install the icon/pixmap
	 @install -c -m 644 -D -T pixmap/encrypt.xpm $(PREFIX)/usr/lib/encrypt/pixmap/encrypt.xpm
	-@echo "installed \`encrypt.xpm' --> \`$(PREFIX)/usr/lib/encrypt/pixmap/encrypt.xpm'" 
	 @install -c -m 644 -D -T pixmap/albinoloverats.xpm $(PREFIX)/usr/lib/encrypt/pixmap/albinoloverats.xpm
	-@echo "installed \`albinoloverats.xpm' --> \`$(PREFIX)/usr/lib/encrypt/pixmap/albinoloverats.xpm'" 
# ditto, but this time for the man page
	 @install -c -m 644 -D -T doc/encrypt.1a.gz $(PREFIX)/usr/lib/encrypt/doc/encrypt.1a.gz
	 @ln -fs /usr/lib/encrypt/doc/encrypt.1a.gz $(PREFIX)/usr/man/man1/
	-@echo "installed \`doc/encrypt.1a.gz' --> \`$(PREFIX)/usr/man/man1/encrypt.1a.gz'"
# finally the desktop file
	 @install -c -m 644 -D -T encrypt.desktop $(PREFIX)/usr/lib/encrypt/encrypt.desktop
	 @ln -fs /usr/lib/encrypt/encrypt.desktop $(PREFIX)/usr/share/applications/
	-@echo "installed \`encrypt.desktop' --> \`$(PREFIX)/usr/share/applications/encrypt.desktop'"
install-all: install $(LIB_INSTALL) $(PO_INSTALL)

clean:
	-@rm -fv encrypt
distclean: clean $(LIB_CLEAN) $(PO_CLEAN)

uninstall: $(LIB_UNINSTALL) $(PO_CLEAN)
	 @rm -fv  $(PREFIX)/usr/share/applications/encrypt.desktop
	 @rm -fv  $(PREFIX)/usr/man/man1/encrypt.1a.gz
	 @rm -fv  $(PREFIX)/usr/lib/encrypt/pixmap/encrypt.xpm
	 @rm -frv $(PREFIX)/usr/lib/encrypt/pixmap
	 @rm -fv  $(PREFIX)/usr/bin/encrypt
	 @rm -frv $(PREFIX)/usr/lib/encrypt
