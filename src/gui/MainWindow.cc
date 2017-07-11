#include "MainWindow.hh"

#include "DataSetModel.hh"
#include "PersistenceDiagramView.hh"

#include <aleph/persistenceDiagrams/io/Raw.hh>

#include <QAction>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMimeData>
#include <QMenuBar>
#include <QStatusBar>

#include <limits>

namespace aleph
{

namespace gui
{

MainWindow::MainWindow()
  : _mdiArea( new QMdiArea( this ) )
  , _dataSetView( new QTreeView( this ) )
  , _dataSetModel( new DataSetModel( this ) )
{
  _dataSetView->setModel( _dataSetModel );

  this->createMenus();
  this->createStatusBar();
  this->createToolBars();

  // Needs to be created later on because they modify the menus of the
  // main window.
  this->createDockWidgets();

  // Permits drag & drop events to be handled. I am using this mainly
  // for the quick loading of data sets.
  this->setAcceptDrops( true );

  // MDI area ----------------------------------------------------------

  _mdiArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  _mdiArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded ),

  this->setCentralWidget( _mdiArea );
}

void MainWindow::createDockWidgets()
{
  {
    QDockWidget* dockWidget = new QDockWidget( tr("Data sets"), this );
    dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    dockWidget->setWidget( _dataSetView );

    this->addDockWidget( Qt::LeftDockWidgetArea, dockWidget );

    _showMenu->addAction( dockWidget->toggleViewAction() );
  }

  //
  // FIXME: remove after debugging
  //

  _dataSetModel->add( tr("Iris_dimension_1.txt"), QVariant::fromValue( aleph::io::load<DataType>( "/home/brieck/Projects/Aleph/tests/persistenceDiagrams/Iris_dimension_1.txt" ) ) );
}

void MainWindow::createMenus()
{
  auto menuBar  = this->menuBar();
  auto fileMenu = menuBar->addMenu( tr("&File") );
  _showMenu     = menuBar->addMenu( tr("&Show") );

  // "Load" menu -------------------------------------------------------

  auto loadMenu = fileMenu->addMenu( tr("Load") );

  QAction* loadPersistenceDiagram
    = new QAction( "Persistence diagram", loadMenu );

  loadMenu->addAction( loadPersistenceDiagram );

  this->connect( loadPersistenceDiagram, &QAction::triggered, this, &MainWindow::loadPersistenceDiagram );
}

void MainWindow::createStatusBar()
{
  this->statusBar()->showMessage( tr("Welcome!"), 2000 );
}

void MainWindow::createToolBars()
{
}

void MainWindow::loadPersistenceDiagram()
{
  auto fileName = QFileDialog::getOpenFileName( this );
  if( !fileName.isEmpty() )
  {
    _persistenceDiagram = aleph::io::load<DataType>( fileName.toStdString() );

    this->statusBar()->showMessage(
      QString( "Loaded persistence diagram with %1 entries" ).arg( _persistenceDiagram.size() )
    );

    auto pd        = new PersistenceDiagramView( _persistenceDiagram );
    auto subWindow = _mdiArea->addSubWindow( pd );

    this->connect( pd, SIGNAL( clicked( QPointF ) ),
                   this, SLOT( handlePersistenceDiagramClick( QPointF ) ) );

    subWindow->resize( 300, 300 );
    subWindow->show();
  }
}

void MainWindow::handlePersistenceDiagramClick( const QPointF& point )
{
  this->statusBar()->showMessage(
    QString( "Selected point: (%1,%2)" ).arg( point.x() ).arg( point.y() )
  );
}

void MainWindow::dragEnterEvent( QDragEnterEvent* event )
{
  if( event->mimeData() && event->mimeData()->hasText() )
  {
    event->setDropAction( Qt::CopyAction );
    event->accept();
  }
  else
    event->ignore();
}

void MainWindow::dropEvent( QDropEvent* event )
{
  auto mimeData = event->mimeData();

  if( mimeData->hasUrls() )
  {
    auto urls = mimeData->urls();

    // Ignore an event if multiple URLs are attached to it. We may only handle
    // a single file.
    if( urls.size() > 1 )
    {
      event->ignore();
      return;
    }

    QString file = urls.first().toLocalFile();

    this->statusBar()->showMessage( "File: " + file );

    // TODO:
    //  - Check data format (if possible)
    //  - Load it
    //  - Add it to the model
  }
  else
    event->ignore();

}

} // namespace gui

} // namespace aleph
