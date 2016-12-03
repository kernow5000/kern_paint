# Qt project Makefile
#
# Shaun Bradley 11/11/02


# project name, change this to suit
PRJ = kern_paint

SRC = $(PRJ).c++
BIN = $(PRJ)
MOC = $(PRJ).moc

QTDIR = /usr/lib/qt
QTMOC = /usr/bin/moc

all:
	    	
		$(QTMOC) -o $(MOC) $(SRC)
		c++ -I$(QTDIR)/include -L$(QTDIR)/lib -lqt-mt $(SRC) -o $(BIN)

clean:
      	
		rm -Rf *~ $(BIN) $(MOC)
backup:
		
		tar cvf $(PRJ).tar *[^*.tar.gz]
		gzip $(PRJ).tar
		mv $(PRJ).tar.gz $(HOME)	
