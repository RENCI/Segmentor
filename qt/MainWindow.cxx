#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QPushButton>
#include <QToolBar>
#include <QShortcut>
#include <QPushButton>

#include <vtkGenericOpenGLRenderWindow.h>

#include "VisualizationContainer.h"
#include "RegionCollection.h"
#include "RegionTable.h"
#include "RegionMetadataIO.h"
#include "InteractionEnums.h"
#include "SliceView.h"
#include "VolumeView.h"

#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"

// Constructor
MainWindow::MainWindow() {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	// Default directory keys
	defaultImageDirectoryKey = "default_image_directory";
	defaultSegmentationDirectoryKey = "default_segmentation_directory";

	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create visualization container
	visualizationContainer = new VisualizationContainer(qvtkWidgetLeft->GetInteractor(), qvtkWidgetRight->GetInteractor(), this);

	// Create tool bar
	CreateToolBar();

	// Create region table
	regionTable = new RegionTable();
	regionTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	regionTableContainer->layout()->addWidget(regionTable);

	QObject::connect(regionTable, &RegionTable::regionDone, this, &MainWindow::on_regionDone);
	QObject::connect(regionTable, &RegionTable::removeRegion, this, &MainWindow::on_removeRegion);
	QObject::connect(regionTable, &RegionTable::highlightRegion, this, &MainWindow::on_highlightRegion);
	QObject::connect(regionTable, &RegionTable::selectRegion, this, &MainWindow::on_selectRegion);

	// Slice up and down
	QAction* sliceUpAction = new QAction("+", this);
	sliceUpAction->setShortcut(QKeySequence(Qt::Key_Up));
	sliceUpAction->setToolTip("Move slice up (up arrow)");
	QObject::connect(sliceUpAction, &QAction::triggered, this, &MainWindow::on_sliceUp);

	sliceUpButton->setDefaultAction(sliceUpAction);

	QAction* sliceDownAction = new QAction("-", this);
	sliceDownAction->setShortcut(QKeySequence(Qt::Key_Down));
	sliceDownAction->setToolTip("Move slice down (down arrow)");
	QObject::connect(sliceDownAction, &QAction::triggered, this, &MainWindow::on_sliceDown);

	sliceDownButton->setDefaultAction(sliceDownAction);

	// Window/level range
	windowSpinBox->setMinimum(0);
	windowSpinBox->setMaximum(9999);
	windowSpinBox->setSingleStep(100);
	windowSpinBox->setDecimals(1);

	levelSpinBox->setMinimum(-9999);
	levelSpinBox->setMaximum(9999);
	levelSpinBox->setSingleStep(100);
	levelSpinBox->setDecimals(1);

	// Overlay opacity
	QShortcut* overlayUp = new QShortcut(QKeySequence(Qt::Key_Right), this);
	QShortcut* overlayDown = new QShortcut(QKeySequence(Qt::Key_Left), this);

	QObject::connect(overlayUp, &QShortcut::activated, this, &MainWindow::on_overlayUp);
	QObject::connect(overlayDown, &QShortcut::activated, this, &MainWindow::on_overlayDown);

	overlaySpinBox->valueChanged(overlaySpinBox->value());

	// Neighbor opacity
	QShortcut* neighborUp = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
	QShortcut* neighborDown = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);

	QObject::connect(neighborUp, &QShortcut::activated, this, &MainWindow::on_neighborUp);
	QObject::connect(neighborDown, &QShortcut::activated, this, &MainWindow::on_neighborDown);

	neighborSpinBox->valueChanged(neighborSpinBox->value());

	qApp->installEventFilter(this);
}

MainWindow::~MainWindow() {
	// Clean up
	delete visualizationContainer;

	qApp->exit();
}

void MainWindow::updateRegions(RegionCollection* regions) {
	regionTable->update(regions);
}

void MainWindow::updateRegion(Region* region) {
	regionTable->update(region);
}

void MainWindow::selectRegion(unsigned short label) {
	regionTable->selectRegionLabel(label);
}

void MainWindow::setWindowLevel(double window, double level) {
	windowSpinBox->setValue(window);
	levelSpinBox->setValue(level);
}

void MainWindow::setSlicePosition(double x, double y, double z) {
	QString s = "Slice Position: (" + 
		QString::number(x, 'f', 1) + ", " + 
		QString::number(y, 'f', 1) + ", " + 
		QString::number(z, 'f' , 1) + ")";

	slicePositionLabel->setText(s);
}

void MainWindow::on_actionOpen_Image_File_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		GetDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultImageDirectoryKey, fileName);

	// Load data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageFile(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open file.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Image_Stack_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		GetDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultImageDirectoryKey, fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Check for files
	if (fileInfoList.length() == 0) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");
		errorMessage.setInformativeText("No files present.");
		errorMessage.exec();
		return;
	}

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageStack(fileNames);

	if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Segmentation_File_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Load segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenSegmentationFile(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open file.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		case VisualizationContainer::NoImageData:
			errorMessage.setInformativeText("Load image data first.");
			errorMessage.exec();
			break;

		case VisualizationContainer::VolumeMismatch:
			errorMessage.setInformativeText("Segmentation data volume does not match loaded image data volume.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Segmentation_Stack_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Check for files
	if (fileInfoList.length() == 0) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");
		errorMessage.setInformativeText("No files present.");
		errorMessage.exec();
		return;
	}

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenSegmentationStack(fileNames);

	if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		case VisualizationContainer::NoImageData:
			errorMessage.setInformativeText("Load image data first.");
			errorMessage.exec();
			break;

		case VisualizationContainer::VolumeMismatch:
			errorMessage.setInformativeText("Segmentation data volume does not match loaded image data volume.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSave_Segmentation_Data_triggered() {
	// Save segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveSegmentationData();

	if (errorCode == VisualizationContainer::NoFileName) {
		on_actionSave_Segmentation_Data_As_triggered();
	}
	else if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not save data.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSave_Segmentation_Data_As_triggered() {
	// Open a file dialog to save the file
	QString fileName = QFileDialog::getSaveFileName(this,
		"Save Segmentation Data",
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Save segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveSegmentationData(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not save data.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSegment_Volume_triggered() {
	visualizationContainer->SegmentVolume();
}

void MainWindow::on_actionExit_triggered() {
	// Exit Qt
	qApp->exit();
}

void MainWindow::on_actionNavigation() {
	visualizationContainer->SetInteractionMode(NavigationMode);	
}

void MainWindow::on_actionEdit() {
	visualizationContainer->SetInteractionMode(EditMode);
}

void MainWindow::on_actionOverlay(bool checked) {
	visualizationContainer->GetSliceView()->ShowLabelSlice(checked);
}

void MainWindow::on_actionVoxels(bool checked) {
	visualizationContainer->GetSliceView()->ShowVoxelOutlines(checked);
}

void MainWindow::on_actionOutline(bool checked) {
	visualizationContainer->GetSliceView()->ShowRegionOutlines(checked);
}

void MainWindow::on_actionRescaleFull(bool checked) {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	sliceView->RescaleFull();

	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
}

void MainWindow::on_actionRescalePartial(bool checked) {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	sliceView->RescalePartial();

	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
}

void MainWindow::on_actionSmoothNormals(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothShading(checked);
}

void MainWindow::on_actionSmoothSurfaces(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothSurfaces(checked);
}

void MainWindow::on_actionShowPlane(bool checked) {
	visualizationContainer->GetVolumeView()->SetShowPlane(checked);
}

void MainWindow::on_actionDilateRegion(bool) {
	visualizationContainer->DilateCurrentRegion();
}

void MainWindow::on_actionErodeRegion(bool) {
	visualizationContainer->ErodeCurrentRegion();
}

void MainWindow::on_sliceUp() {
	visualizationContainer->SliceUp();
}

void MainWindow::on_sliceDown() {
	visualizationContainer->SliceDown();
}

void MainWindow::on_windowSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetWindow(value);
}

void MainWindow::on_levelSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetLevel(value);
}

void MainWindow::on_overlaySpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetOverlayOpacity(value);
}

void MainWindow::on_overlayUp() {
	overlaySpinBox->stepUp();
}

void MainWindow::on_overlayDown() {
	overlaySpinBox->stepDown();
}

void MainWindow::on_neighborSpinBox_valueChanged(double value) {
	visualizationContainer->GetVolumeView()->SetNeighborOpacity(value);
}

void MainWindow::on_neighborUp() {
	neighborSpinBox->stepUp();
}

void MainWindow::on_neighborDown() {
	neighborSpinBox->stepDown();
}

void MainWindow::on_regionDone(int label, bool done) {
	visualizationContainer->SetRegionDone((unsigned short)label, done);
}

void MainWindow::on_removeRegion(int label) {
	visualizationContainer->RemoveRegion((unsigned short)label);
}

void MainWindow::on_highlightRegion(int label) {
	visualizationContainer->HighlightRegion((unsigned short)label);
}

void MainWindow::on_selectRegion(int label) {
	visualizationContainer->SelectRegion((unsigned short)label);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::Enter) {
		if (obj == qvtkWidgetLeft) {
			//std::cout << "LEFT" << std::endl;

			// XXX: How to grab focus so keypresses work without clicking first?
		}
		else if (obj == qvtkWidgetRight) {
			//std::cout << "RIGHT" << std::endl;

			// XXX: How to grab focus so keypresses work without clicking first?
		}
	}

	return QObject::eventFilter(obj, event);
}

QString MainWindow::GetDefaultDirectory(QString key) {
	QSettings settings;

	return settings.value(key).toString();
}

void MainWindow::SetDefaultDirectory(QString key, QString fileName) {
	QFileInfo fileInfo(fileName);

	QSettings settings;
	settings.setValue(key, fileInfo.absoluteDir().absolutePath());
}

void MainWindow::CreateToolBar() {
	// Create tool bar
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Vertical);

	// Interaction toggle
	QActionGroup* interactionModeGroup = new QActionGroup(this);
	interactionModeGroup->setExclusive(true);

	QAction* actionNavigation = new QAction("N", interactionModeGroup);
	actionNavigation->setToolTip("Navigation mode (space bar)");
	actionNavigation->setCheckable(true);
	actionNavigation->setChecked(visualizationContainer->GetInteractionMode() == NavigationMode);

	QAction* actionEdit = new QAction("E", interactionModeGroup);
	actionEdit->setToolTip("Edit mode (space bar)");
	actionEdit->setCheckable(true);

	// Filter mode toggle
	// XXX: With Qt 5.14, should be able to set exclusion policy to avoid extra logic below
	//QActionGroup* filterModeGroup = new QActionGroup(this);
	//interactionModeGroup->setExclusive(false);

	QAction* actionFilterPlane = new QAction(QIcon(":/icons/icon_filter_plane.png"), "Filter to plane (p)", this);
	actionFilterPlane->setShortcut(QKeySequence("p"));
	actionFilterPlane->setCheckable(true);
	actionFilterPlane->setChecked(visualizationContainer->GetFilterMode() == FilterPlane);

	QAction* actionFilterNeighbors = new QAction(QIcon(":/icons/icon_filter_neighbors.png"), "Filter neighbors (k)", this);
	actionFilterNeighbors->setShortcut(QKeySequence("k"));
	actionFilterNeighbors->setCheckable(true);
	actionFilterNeighbors->setChecked(visualizationContainer->GetFilterMode() == FilterNeighbors);

	QAction* actionFilterRegion = new QAction(QIcon(":/icons/icon_filter_region.png"), "Filter region: l", this);
	actionFilterRegion->setShortcut(QKeySequence("l"));
	actionFilterRegion->setCheckable(true);
	actionFilterRegion->setChecked(visualizationContainer->GetFilterMode() == FilterRegion);

	// Add widgets to tool bar
	toolBar->addWidget(CreateLabel("Mode"));
	toolBar->addAction(actionNavigation);
	toolBar->addAction(actionEdit);
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("2D"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_overlay.png", "Show overlay (q)", "q", visualizationContainer->GetSliceView()->GetShowLabelSlice(), &MainWindow::on_actionOverlay));
	toolBar->addAction(CreateActionIcon(":/icons/icon_voxels.png", "Show voxels (w)", "w", visualizationContainer->GetSliceView()->GetShowVoxelOutlines(), &MainWindow::on_actionVoxels));
	toolBar->addAction(CreateActionIcon(":/icons/icon_outline.png", "Show outlines (e)", "e", visualizationContainer->GetSliceView()->GetShowRegionOutlines(), &MainWindow::on_actionOutline));
	toolBar->addAction(CreateActionIcon(":/icons/icon_rescale_full.png", "Rescale full (=)", "=", &MainWindow::on_actionRescaleFull));
	toolBar->addAction(CreateActionIcon(":/icons/icon_rescale_partial.png", "Rescale partial (-)", "-", &MainWindow::on_actionRescalePartial));
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("3D"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_smooth_normals.png", "Smooth normals (n)", "n", visualizationContainer->GetVolumeView()->GetSmoothShading(), &MainWindow::on_actionSmoothNormals));
	toolBar->addAction(CreateActionIcon(":/icons/icon_smooth_surface.png", "Smooth surfaces (s)", "s", visualizationContainer->GetVolumeView()->GetSmoothSurfaces(), &MainWindow::on_actionSmoothSurfaces));
	toolBar->addAction(CreateActionIcon(":/icons/icon_plane.png", "Show plane (o)", "o", visualizationContainer->GetVolumeView()->GetShowPlane(), &MainWindow::on_actionShowPlane));
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("Filter"));
	toolBar->addAction(actionFilterPlane);
	toolBar->addAction(actionFilterNeighbors);
	toolBar->addAction(actionFilterRegion);
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("Edit"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_dilate.png", "Dilate region (c)", "c", &MainWindow::on_actionDilateRegion));
	toolBar->addAction(CreateActionIcon(":/icons/icon_erode.png", "Erode region (v)", "v",&MainWindow::on_actionErodeRegion));

	// Need extra logic for interaction mode
	QObject::connect(actionNavigation, &QAction::triggered, this, &MainWindow::on_actionNavigation);
	QObject::connect(actionEdit, &QAction::triggered, this, &MainWindow::on_actionEdit);
	QObject::connect(new QShortcut(QKeySequence(32), this), &QShortcut::activated, [actionNavigation, actionEdit]() {
		if (actionEdit->isChecked()) {
			actionNavigation->toggle();
			emit(actionNavigation->triggered(true));
		}
		else {
			actionEdit->toggle();
			emit(actionEdit->triggered(true));
		}
	});

	// Need extra logic for filter mode
	QObject::connect(actionFilterPlane, &QAction::triggered, [actionFilterPlane, actionFilterNeighbors, actionFilterRegion, this]() {
		actionFilterNeighbors->setChecked(false);
		actionFilterRegion->setChecked(false);

		this->visualizationContainer->SetFilterMode(actionFilterPlane->isChecked() ? FilterPlane : FilterNone);
	});
	QObject::connect(actionFilterNeighbors, &QAction::triggered, [actionFilterPlane, actionFilterNeighbors, actionFilterRegion, this]() {
		actionFilterPlane->setChecked(false);
		actionFilterRegion->setChecked(false);

		this->visualizationContainer->SetFilterMode(actionFilterNeighbors->isChecked() ? FilterNeighbors : FilterNone);
	});
	QObject::connect(actionFilterRegion, &QAction::triggered, [actionFilterPlane, actionFilterNeighbors, actionFilterRegion, this]() {
		actionFilterPlane->setChecked(false);
		actionFilterNeighbors->setChecked(false);

		this->visualizationContainer->SetFilterMode(actionFilterRegion->isChecked() ? FilterRegion : FilterNone);
	});

	toolBarWidget->layout()->addWidget(toolBar);
}

QAction* MainWindow::CreateActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)(bool)) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(false);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::CreateActionIcon(const QString& fileName, const QString& text, const QString& shortcut, bool checked, void (MainWindow::*slot)(bool)) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(true);
	action->setChecked(checked);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QLabel* MainWindow::CreateLabel(const QString& text, int topMargin, int bottomMargin) {
	QString style = QStringLiteral("color:#999;margin-top:%1px;margin-bottom:%2px").arg(topMargin).arg(bottomMargin);

	QLabel* label = new QLabel(text);
	label->setAlignment(Qt::AlignCenter);
	label->setStyleSheet(style);

	return label;
}