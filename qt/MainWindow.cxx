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

void MainWindow::on_actionSmoothNormals(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothShading(checked);
}

void MainWindow::on_actionSmoothSurfaces(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothSurfaces(checked);
}

void MainWindow::on_actionShowPlane(bool checked) {
	visualizationContainer->GetVolumeView()->SetShowPlane(checked);
}

void MainWindow::on_actionFilterPlane(bool checked) {
	visualizationContainer->GetVolumeView()->SetFilterPlane(checked);
}

void MainWindow::on_actionFilterRegion(bool checked) {
	visualizationContainer->GetSliceView()->SetFilterRegion(checked);
	visualizationContainer->GetVolumeView()->SetFilterRegion(checked);
}

void MainWindow::on_actionDilateRegion(bool) {
	visualizationContainer->DilateCurrentRegion();
}

void MainWindow::on_actionErodeRegion(bool) {
	visualizationContainer->ErodeCurrentRegion();
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
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Vertical);

	QActionGroup* interactionModeGroup = new QActionGroup(this);
	interactionModeGroup->setExclusive(true);

	QAction* actionNavigation = new QAction("N", interactionModeGroup);
	actionNavigation->setCheckable(true);
	actionNavigation->setChecked(visualizationContainer->GetInteractionMode() == NavigationMode);

	QAction* actionEdit = new QAction("E", interactionModeGroup);
	actionEdit->setCheckable(true);

	toolBar->addWidget(CreateLabel("Mode"));
	toolBar->addAction(actionNavigation);
	toolBar->addAction(actionEdit);
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("2D"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_overlay.svg", "Show overlay", "q", visualizationContainer->GetSliceView()->GetShowLabelSlice(), &MainWindow::on_actionOverlay));
	toolBar->addAction(CreateActionIcon(":/icons/icon_voxels.svg", "Show voxels", "w", visualizationContainer->GetSliceView()->GetShowVoxelOutlines(), &MainWindow::on_actionVoxels));
	toolBar->addAction(CreateActionIcon(":/icons/icon_outline.svg", "Show outlines", "e", visualizationContainer->GetSliceView()->GetShowRegionOutlines(), &MainWindow::on_actionOutline));
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("3D"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_smooth_normals.svg", "Smooth normals", "n", visualizationContainer->GetVolumeView()->GetSmoothShading(), &MainWindow::on_actionSmoothNormals));
	toolBar->addAction(CreateActionIcon(":/icons/icon_smooth_surface.svg", "Smooth surfaces", "s", visualizationContainer->GetVolumeView()->GetSmoothSurfaces(), &MainWindow::on_actionSmoothSurfaces));
	toolBar->addAction(CreateActionIcon(":/icons/icon_plane.svg", "Show plane", "o", visualizationContainer->GetVolumeView()->GetShowPlane(), &MainWindow::on_actionShowPlane));
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("Filter"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_filter_plane.svg", "Filter to plane", "p", visualizationContainer->GetVolumeView()->GetFilterPlane(), &MainWindow::on_actionFilterPlane));
	toolBar->addAction(CreateActionIcon(":/icons/icon_filter_region.svg", "Filter region", "l", visualizationContainer->GetVolumeView()->GetFilterRegion(), &MainWindow::on_actionFilterRegion));
	toolBar->addSeparator();
	toolBar->addWidget(CreateLabel("Edit"));
	toolBar->addAction(CreateActionIcon(":/icons/icon_dilate.svg", "Dilate region", "z", &MainWindow::on_actionDilateRegion));
	toolBar->addAction(CreateActionIcon(":/icons/icon_erode.svg", "Erode region", "x",&MainWindow::on_actionErodeRegion));

	// Need extra logic for modes
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