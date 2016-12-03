/*
 * SOFT310 Advanced Windows Programming
 * Coursework - Paint application
 *  
 * Shaun Bradley - Bsc Multimedia Computing yr 3
 * 9/12/02
 * 
 * Known issues:
 * Line drawing output gets 'bitty' when thickness increases, due to something in mouseMove()???
 * Load and Save functions NOT on right click popup menu.
 *
 */



// standard Qt header includes
#include <qapplication.h>
#include <qfiledialog.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qcursor.h>


// values for pen color selection
enum MenuIDs{
  COLOR_MENU_ID_BLACK,
  COLOR_MENU_ID_RED,
  COLOR_MENU_ID_BLUE,
  COLOR_MENU_ID_GREEN,
  COLOR_MENU_ID_YELLOW };

// values for pen size selection
enum SizeIDs{
  THICK_MENU_ID_SMALLEST,
  THICK_MENU_ID_SMALLER,
  THICK_MENU_ID_MEDIUM,
  THICK_MENU_ID_LARGER,
  THICK_MENU_ID_LARGEST };



/*
 * The Canvas class, this is the paint area, it knows
 * how to change pen color, thickness, drawing mode, and
 * also loads/saves/clears.
 *
 */

class Canvas : public QWidget
{
  Q_OBJECT    // needed as this class contains slots
  
public:
  Canvas();  // constructor/destructor
  ~Canvas();
  
public slots:
void setColor( QColor );   // set pen color
  void setThick ( int );   // set pen size
  void setMode( int );     // set draw mode
  void slotLoad( const char* );  // load 
  void slotSave( const char* );  // save
  
protected:
  // these functions are overridden from somewhere
  // up in the Qt event classes
  virtual void mousePressEvent( QMouseEvent* );   // called when mouse button is clicked
  virtual void mouseMoveEvent( QMouseEvent* );    // called when clicked _and dragging
  virtual void mouseReleaseEvent( QMouseEvent* ); // called when mouse button released
  virtual void paintEvent( QPaintEvent* );        // called when window needs repainting
  virtual void resizeEvent( QResizeEvent* );      // called when window is resized
  
private slots:
void slotClearArea();
  
private:
  // attribute data
  QPoint _last;      // records last xy mouse points
  QPoint _lastclick; // records last xy when mouse button released
  
  QColor _currentcolor;  // current pen color
  int _currentsize;      // current pen size
  int drawingmode;       // for the drawingmode

  int mousedown;         // to flag if mouse button is pressed
  
  // the QPainters
  QPainter windowpainter;   // current area
  QPainter bufferpainter;   // the off-screen buffer
  
  QPixmap _buffer;
  
  QPopupMenu* _popupmenu;  // our right click context menu
  
};



/* 
 * The PaintWindow class, this contains the menus etc,
 * that send signals to 'Canvas' available slots.
 *
 */

class PaintWindow : public QWidget
{
  Q_OBJECT  // needed as this class contains slots/signals
  
public:
  PaintWindow();  // constructor/destructor
  ~PaintWindow();
  
private slots:
void slotAbout();      // about the Paint app
  void slotAboutQt();  // about Qt3
  void slotColorMenu( int );   // the color selection menu
  void slotThickMenu( int );   // the thick selection menu 
  void slotModeMenu ( int );   // the drawing mode selection menu
  void slotLoad();
  void slotSave();
  
  signals:
  void colorChanged( QColor ); // signal for pen color changed
  void thickChanged( int );    // signal for pen thickness changed
  void modeChanged( int );     // signal for drawmode changed
  void load( const char* );    // load
  void save( const char* );    // save
  
protected:
  virtual void resizeEvent( QResizeEvent* );
  
private:
  // attribute data
  QMenuBar* _menubar;     // the menu bar
  QPopupMenu* _filemenu;  // file
  QPopupMenu* _colormenu; // submenu for color
  QPopupMenu* _thickmenu; // submenu for thickness
  QPopupMenu* _drawmenu;  // for drawing mode options {lines, circles, squares}
  QPopupMenu* _penmenu;   // for pen menu options { color, thickness submenus }
  QPopupMenu* _helpmenu;  // for about() dialogs
  
  QScrollView* _scrollview; // scrollable view
  Canvas* _Canvas;          // a 'Canvas' from above 'Canvas' class definition
  
};




// include the meta object compiler results - for object communication
#include "kern_paint.moc"


// Constructor
Canvas::Canvas()
{
  // initialise member variables
  // set default Pen up
  _currentcolor = black;
  _currentsize = THICK_MENU_ID_SMALLEST;
  drawingmode = 1; // line drawing mode
  
  // dont blank the window before repainting
  setBackgroundMode( NoBackground );
  
  // create a pop-up menu
  _popupmenu = new QPopupMenu();
  _popupmenu->insertItem( "&Clear", this, SLOT( slotClearArea() ) );
  // add load and save here if possible
  
}


// Destructor
Canvas::~Canvas()
{
  delete _popupmenu;  // destroy popup-menu
}



/*
 * This slot sets the current color for the Canvas. It will be
 * connected with the colorChanged( QColor ) signal from the 
 * ScribbleWindow.
 */
void Canvas::setColor( QColor new_color )
{
  _currentcolor = new_color;  // set color
}

/*
 * This slot sets the current thickness for the Canvas. Its 
 * connected with the thickChanged( int ) signal from the 
 * PaintWindow.
 */
void Canvas::setThick( int new_thickness )
{
  _currentsize = new_thickness; // set thick
}

/*
 * This slot sets the drawing mode
 */
void Canvas::setMode ( int new_mode )
{
  drawingmode = new_mode;
}


/*
 * This slot clears the drawing area by filling the off-screen buffer with
 * white and copying it over to the window.
 */

void Canvas::slotClearArea()
{
  // fill the off-screen buffer with plain white
  _buffer.fill ( white );
  
  // and copy it over to the window
  bitBlt( this, 0, 0, &_buffer );
}


/*
 * This method does the actual loading. It relies on QPixmap (and the
 * underlying I/O machinery) to determine the filetype.
 */

void Canvas::slotLoad( const char* filename )
{
  if( !_buffer.load( filename ) )
    QMessageBox::warning( 0, "Load error", "Could not load file" );
  
  repaint();  // refresh the window
}


/*
 * This method does the actual saving. We hard-code the file type as
 * BMP. Unix users might want to replace this with something like XPM.
 */

void Canvas::slotSave( const char* filename )
{
  if(!_buffer.save( filename, "BMP" ) ) 
    QMessageBox::warning( 0, "Save error", "Could not save file" );
}



/*
 * This virtual method is called whenever the user presses the
 * mouse over the window. It records the position of the mouse
 * at the time of the click.
 */

void Canvas::mousePressEvent( QMouseEvent* event )
{
  if( event->button() == RightButton )
    _popupmenu->exec( QCursor::pos() );
  else
    {
      mousedown = 1;             // flag mouse button pressed
      _last = event->pos();      // get the co-ords from the event
      _lastclick = event->pos(); // buffer the last click, for circle and square dragging
    }
}


/*
 * This virtual method is called whenever the user moves the mouse
 * while the mouse button is pressed. I've added a flag for the mouse button
 * as, after loading/saving it would 'stick' down and draw regardless.. 
 */

void Canvas::mouseMoveEvent( QMouseEvent* event )
{
  
  // check if mouse button is held down, should be as we are in here
  if(mousedown) {
    
    // set up pen with color, thickness
    QPen myPen ( _currentcolor, _currentsize );
    
    // start painting
    windowpainter.begin( this ); // paint onto window
    bufferpainter.begin( &_buffer ); // paint into buffer
    
    // fiddling with brush for filled stuff
    //windowpainter.setBrush( Qt::SolidPattern );
    //windowpainter.drawEllipse( 90, 300, 60, 60 );
    //windowpainter.setBrush( Qt::DiagCrossPattern );        // black diagonal cross pattern
    //windowpainter.drawEllipse( 90, 210, 80, 80 );          // test
    //windowpainter.setBrush( Qt::CrossPattern );            // black cross fill pattern
    //windowpainter.drawEllipse( 310, 210, 80, 80 );         // test
    // hmm bother with fill styles?
    
    // setup the pen from menu values
    windowpainter.setPen(myPen);
    bufferpainter.setPen(myPen);
    // end
    
    // bother making constants?
    switch (drawingmode) 
      {
      case 1: 
	{ // lines
	  windowpainter.drawLine( _last, event->pos() );
	  //bufferpainter.drawLine( _last, event->pos() ); // no need, does on mouseUP
	  break;
	}
      case 2:
	{ // squares
	  bitBlt( this, 0, 0, &_buffer );
	  windowpainter.drawRect(_lastclick.x(), _lastclick.y(), 
				 event->x() - _lastclick.x(), event->y() - _lastclick.y() );
	  break;
	}
      case 3:
	{
	  // circles
	  bitBlt( this, 0, 0, &_buffer );
	  windowpainter.drawEllipse(_lastclick.x(), _lastclick.y(), 
				    event->x() - _lastclick.x(), event->y() - _lastclick.y() );
	  break;
	}
      case 4:
	{
	  // filled squares
	  bitBlt( this, 0, 0, &_buffer );
	  windowpainter.setBrush(_currentcolor); // set fill color
	  windowpainter.drawRect(_lastclick.x(), _lastclick.y(),
				 event->x() - _lastclick.x(), event->y() - _lastclick.y() );
	  break;
	}
      case 5:
	{
	  // filled circles
	  bitBlt( this, 0, 0, &_buffer );
	  windowpainter.setBrush(_currentcolor); // set fill color.. works
	  windowpainter.drawEllipse(_lastclick.x(), _lastclick.y(),
				    event->x() - _lastclick.x(), event->y() - _lastclick.y() );
	  break;
	}
      };
    
    
    // finish painting
    windowpainter.end();
    bufferpainter.end();
    
    // catch the last mouse x,y position
    _last = event->pos();

  }  // end if(mousedown)

}
  

// this virtual method is called whenever the user lets go of the mouse
void Canvas::mouseReleaseEvent( QMouseEvent* event )
{
  mousedown = 0;                  // flag that mouse button is released
  bitBlt( &_buffer, 0, 0, this);  // finish drawing, copy buffer into real window
}


/*
 * This virtual method is called whenever the widget needs
 * painting, such as when it has been obscured and then revealed again.
 */

void Canvas::paintEvent( QPaintEvent* event )
{
  // copy the image from the buffer pixmap into the window
  bitBlt( this, 0, 0, &_buffer );
}


/*
 * This virtual method is called whenever the window is resized. We
 * use it to make sure the off-screen buffer is always the same
 * size as the window.
 * To retain the original drawing, it is first copied
 * to a temporary buffer. After the main buffer has been resized and
 * filled with white, the image is copied from the temporary buffer to
 * the main buffer.
 */

void Canvas::resizeEvent( QResizeEvent* event )
{
  QPixmap save( _buffer );
  _buffer.resize( event->size() );
  _buffer.fill( white ); //fill with white
  bitBlt( &_buffer, 0, 0, &save );
}


PaintWindow::PaintWindow()
{
  /* The next few lines build the menu bar. We first create the menus
   * one by one, then add them to the menu bar. */

  _filemenu = new QPopupMenu;  // create a file menu
  _filemenu->insertItem( "&Load", this, SLOT( slotLoad() ) );
  _filemenu->insertItem( "&Save", this, SLOT( slotSave() ) );
  _filemenu->insertItem( "&Quit", qApp, SLOT( quit() ) );
  
  _drawmenu = new QPopupMenu;            // draw mode menu
  _drawmenu->insertTearOffHandle();      // tear off menu capability
  _drawmenu->insertItem( "Lines", 1);
  _drawmenu->insertItem( "Squares", 2);
  _drawmenu->insertItem( "Circles", 3);
  _drawmenu->insertItem( "Filled Squares", 4);
  _drawmenu->insertItem( "Filled Circles", 5);
  QObject::connect( _drawmenu, SIGNAL( activated( int ) ),
		    this, SLOT( slotModeMenu( int ) ) );
  
  _colormenu = new QPopupMenu;         // create a color menu
  _colormenu->insertTearOffHandle();   // tear off menu
  _colormenu->insertItem( "B&lack", COLOR_MENU_ID_BLACK );
  _colormenu->insertItem( "&Red", COLOR_MENU_ID_RED );
  _colormenu->insertItem( "&Blue", COLOR_MENU_ID_BLUE );
  _colormenu->insertItem( "&Green", COLOR_MENU_ID_GREEN );
  _colormenu->insertItem( "&Yellow", COLOR_MENU_ID_YELLOW );
  QObject::connect( _colormenu, SIGNAL( activated( int ) ),
		    this, SLOT( slotColorMenu( int ) ) );
  
  _helpmenu = new QPopupMenu;    // create a help menu
  _helpmenu->insertItem( "&About scribble 3", this, SLOT( slotAbout() ) );
  _helpmenu->insertItem( "About &Qt", this, SLOT( slotAboutQt() ) );
  
  _thickmenu = new QPopupMenu; // the THICK menu
  _thickmenu->insertTearOffHandle();
  _thickmenu->insertItem( "Thinnest", THICK_MENU_ID_SMALLEST );
  _thickmenu->insertItem( "Thinner", THICK_MENU_ID_SMALLER );
  _thickmenu->insertItem( "Medium", THICK_MENU_ID_MEDIUM );
  _thickmenu->insertItem( "Thicker", THICK_MENU_ID_LARGER );
  _thickmenu->insertItem( "Thickest", THICK_MENU_ID_LARGEST );
  QObject::connect( _thickmenu, SIGNAL( activated( int ) ),
		    this, SLOT( slotThickMenu( int ) ) ); // connect slot
  
  _penmenu = new QPopupMenu;           // pen options menu
  _penmenu->insertTearOffHandle();     // tear off menu
  _penmenu->insertItem( "Pen colour", _colormenu);    // color submenu
  _penmenu->insertItem( "Pen size", _thickmenu);      // size submenu
  
  _helpmenu = new QPopupMenu;  // create a help menu
  _helpmenu->insertItem( "&About scribble", this, SLOT( slotAbout() ) );
  _helpmenu->insertItem( "About &Qt", this, SLOT( slotAboutQt() ) );
  
  _menubar = new QMenuBar( this );  // create a menu bar
  _menubar->insertItem( "&File", _filemenu );
  _menubar->insertItem( "&Draw mode", _drawmenu );
  _menubar->insertItem( "&Pen options", _penmenu );
  _menubar->insertSeparator();
  _menubar->insertItem( "&Help", _helpmenu );
  // add all my menus to the menuBar
  
  /* We create a QScrollView and a Canvas. The Canvas will
   * be managed by the scroll view. */
  _scrollview = new QScrollView( this );
  _scrollview->setGeometry( 0, _menubar->height(), width(), height() - _menubar->height() );
  _Canvas = new Canvas();                   // construct new canvas
  _Canvas->setGeometry( 0, 0, 1000, 1000 ); // set its size
  _scrollview->addChild( _Canvas );         // add the Canvas as a child to scrollview
  
  // connect object slots and signals
  QObject::connect( this, SIGNAL( colorChanged( QColor ) ),
		    _Canvas, SLOT( setColor( QColor ) ) );
  QObject::connect( this, SIGNAL( thickChanged( int ) ),
		    _Canvas, SLOT( setThick ( int ) ) );
  QObject::connect( this, SIGNAL( modeChanged( int ) ),
		    _Canvas, SLOT( setMode( int ) ) );
  QObject::connect( this, SIGNAL( save( const char* ) ),
		    _Canvas, SLOT( slotSave( const char* ) ) );
  QObject::connect( this, SIGNAL( load( const char* ) ),
		    _Canvas, SLOT( slotLoad( const char* ) ) );
  
}


// Constructor
PaintWindow::~PaintWindow()
{
}


void PaintWindow::resizeEvent( QResizeEvent* event )
{
  /* When the whole window is resized, we have to rearrange the geometry
   * in the PaintWindow as well. Note that the Canvas does not need
   * to be changed.
   */
  _scrollview->setGeometry( 0, _menubar->height(), width(), height() - _menubar->height() );
  
}


void PaintWindow::slotAbout()
{
  
  QMessageBox::information( this, "About Kern_Paint", 
			    "This is the SOFT310 Qt Paint Coursework\n"
			    "Copyright 2002 by Shaun Bradley."
			    );
  
}


// display about information
void PaintWindow::slotAboutQt()
{
  QMessageBox::aboutQt( this, "About Qt" );
}



// this emits a signal for the pen color menu changing
void PaintWindow::slotColorMenu( int item )
{
  
  switch( item )
    {
    case COLOR_MENU_ID_BLACK:
      emit colorChanged( black );
      break;
    case COLOR_MENU_ID_RED:
      emit colorChanged( red );
      break;
    case COLOR_MENU_ID_BLUE:
      emit colorChanged( blue );
      break;
    case COLOR_MENU_ID_GREEN:
      emit colorChanged( green );
      break;
    case COLOR_MENU_ID_YELLOW:
      emit colorChanged( yellow );
      break;
    }
  
}


// ok, this emits the thickChanged signal for pen size
void PaintWindow::slotThickMenu( int item )
{
  switch ( item )
    {
    case THICK_MENU_ID_SMALLEST:
      emit thickChanged( 1 );
      break;
    case THICK_MENU_ID_SMALLER:
      emit thickChanged( 3 );
      break;
    case THICK_MENU_ID_MEDIUM:
      emit thickChanged( 5 );
      break;
    case THICK_MENU_ID_LARGER:
      emit thickChanged( 7 );
      break;
    case THICK_MENU_ID_LARGEST:
      emit thickChanged( 9 );
      break;
    }
}



// this emits modeChanged(int) for the drawing mode
void PaintWindow::slotModeMenu( int mode)
{
  switch (mode) {
  case 1:  
    emit modeChanged( 1 );  // lines
    break;
  case 2:
    emit modeChanged( 2 );  // squares
    break;
  case 3:
    emit modeChanged( 3 );  // circles
    break;
  case 4:
    emit modeChanged( 4 );  // filled squares
    break;
  case 5:
    emit modeChanged( 5 );  // filled circles
    break;
  }
  
}

/*
 * This is the slot for the menu item File/Load. It opens a
 * QFileDialog to ask the user for the filename, then emits a save()
 * signal with the filename as a parameter.
 */
void PaintWindow::slotLoad()
{
  /* Open a file dialog for loading. The default directory is the
   * current directory, thie filter *.bmp.
   */
  QString filename = QFileDialog::getOpenFileName( ".", "*.bmp", this );
  if( !filename.isEmpty() )
    emit load( filename );
  
}

/*
 * This is the save equivalent to slotLoad(). Again, we just ask for a
 * filename and emit a signal.
 */
void PaintWindow::slotSave()
{
  /* Open a file dialog for saving. The default directory is the 
   * current directory, the filter *.bmp.
   */
  QString filename = QFileDialog::getSaveFileName( ".", "*.bmp", this );
  if( !filename.isEmpty() )
    emit save( filename );
  
}



int main( int argc, char* argv[] )
{
  
  QApplication myapp( argc, argv );
  
  PaintWindow* Paint = new PaintWindow();
  Paint->setGeometry( 50, 500, 400, 400 );
  
  myapp.setMainWidget( Paint );  // set paint to main
  Paint->show();
  
  return myapp.exec();  // application exit code
  
}

