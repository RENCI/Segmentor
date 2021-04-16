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

	// Voxel size callbacks
	QObject::connect(xSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);
	QObject::connect(ySizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);
	QObject::connect(zSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_voxelSizeSpinBox);
}

SettingsDialog::~SettingsDialog() {
}

void SettingsDialog::initializeSettings() {
	SliceView* sliceView = visualizationContainer->GetSliceView();
	VolumeView* volumeView = visualizationContainer->GetVolumeView();

	const double* voxelSize = visualizationContainer->GetVoxelSize();

	xSizeSpinBox->setValue(voxelSize[0]);
	ySizeSpinBox->setValue(voxelSize[1]);
	zSizeSpinBox->setValue(voxelSize[2]);

	// Window/level
	const double* range = visualizationContainer->GetDataRange();
	double step = (range[1] - range[0]) / 25;
	double max = std::numeric_limits<double>::max();

	windowSpinBox->setMinimum(-max);
	windowSpinBox->setMaximum(max);
	windowSpinBox->setSingleStep(step);
	windowSpinBox->setDecimals(1);
	windowSpinBox->setValue(sliceView->GetWindow());

	levelSpinBox->setMinimum(-max);
	levelSpinBox->setMaximum(max);
	levelSpinBox->setSingleStep(step);
	levelSpinBox->setDecimals(1);
	levelSpinBox->setValue(sliceView->GetLevel());

	// Overlay opacity
	overlaySpinBox->setValue(sliceView->GetOverlayOpacity());

	// Region opacity
	opacitySpinBox->setValue(volumeView->GetVisibleOpacity());

	// Brush radius
	brushRadiusSpinBox->setValue(visualizationContainer->GetBrushRadius());
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

void SettingsDialog::on_windowLevelChanged(double window, double level) {
	windowSpinBox->setValue(window);
	levelSpinBox->setValue(level);
}

void SettingsDialog::on_overlayChanged(double value) {
	overlaySpinBox->setValue(value);
}

void SettingsDialog::on_opacityChanged(double value) {
	opacitySpinBox->setValue(value);
}

void SettingsDialog::on_brushRadiusChanged(int value) {
	brushRadiusSpinBox->setValue(value);
}