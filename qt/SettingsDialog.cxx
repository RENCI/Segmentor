#include "SettingsDialog.h"

#include "VisualizationContainer.h"
#include "SliceView.h"
#include "VolumeView.h"

// Constructor
SettingsDialog::SettingsDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);	
	
	SliceView* sliceView = visualizationContainer->GetSliceView();
	VolumeView* volumeView = visualizationContainer->GetVolumeView();

	// Handle rejection :-(
	QObject::connect(this, &SettingsDialog::rejected, this, &SettingsDialog::on_reject);

	// Voxel size callbacks
	QObject::connect(xSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);
	QObject::connect(ySizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);
	QObject::connect(zSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);

	const double* voxelSize = visualizationContainer->GetVoxelSize();

	xSize = voxelSize[0];
	ySize = voxelSize[1];
	zSize = voxelSize[2];

	xSizeSpinBox->setValue(xSize);
	ySizeSpinBox->setValue(ySize);
	zSizeSpinBox->setValue(zSize);

	// Window/level
	const double* range = visualizationContainer->GetDataRange();

	double step = (range[1] - range[0]) / 25;

	window = sliceView->GetWindow();
	level = sliceView->GetLevel();

	double max = std::numeric_limits<double>::max();

	windowSpinBox->setMinimum(-max);
	windowSpinBox->setMaximum(max);
	windowSpinBox->setSingleStep(step);
	windowSpinBox->setDecimals(1);
	windowSpinBox->setValue(window);

	levelSpinBox->setMinimum(-max);
	levelSpinBox->setMaximum(max);
	levelSpinBox->setSingleStep(step);
	levelSpinBox->setDecimals(1);
	levelSpinBox->setValue(level);

	// Overlay opacity
	overlayOpacity = sliceView->GetOverlayOpacity();
	overlaySpinBox->setValue(overlayOpacity);

	// Region opacity
	regionOpacity = volumeView->GetVisibleOpacity();
	opacitySpinBox->setValue(regionOpacity);

	// Brush radius
	brushRadius = visualizationContainer->GetBrushRadius();
	brushRadiusSpinBox->setValue(brushRadius);
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::on_windowSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetWindow(value);
}

void SettingsDialog::on_levelSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetLevel(value);
}

void SettingsDialog::on_overlaySpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetOverlayOpacity(value);
}

void SettingsDialog::on_overlayUp() {
	overlaySpinBox->stepUp();
}

void SettingsDialog::on_overlayDown() {
	overlaySpinBox->stepDown();
}

void SettingsDialog::on_opacitySpinBox_valueChanged(double value) {
	visualizationContainer->SetVisibleOpacity(value);
}

void SettingsDialog::on_opacityUp() {
	opacitySpinBox->stepUp();
}

void SettingsDialog::on_opacityDown() {
	opacitySpinBox->stepDown();
}

void SettingsDialog::on_voxelSizeSpinBox() {
	visualizationContainer->SetVoxelSize(
		xSizeSpinBox->value(),
		ySizeSpinBox->value(),
		zSizeSpinBox->value()
	);
}

void SettingsDialog::on_brushRadiusSpinBox_valueChanged(int value) {
	visualizationContainer->SetBrushRadius(value);
}

void SettingsDialog::on_brushRadiusUp() {
	brushRadiusSpinBox->stepUp();
}

void SettingsDialog::on_brushRadiusDown() {
	brushRadiusSpinBox->stepDown();
}

void SettingsDialog::on_reject() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	visualizationContainer->SetVoxelSize(xSize, ySize, zSize);
	sliceView->SetWindow(window);
	sliceView->SetLevel(level);
	sliceView->SetOverlayOpacity(overlayOpacity);
	visualizationContainer->SetVisibleOpacity(regionOpacity);
	visualizationContainer->SetBrushRadius(brushRadius);
}