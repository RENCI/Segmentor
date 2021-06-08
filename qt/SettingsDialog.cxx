#include "SettingsDialog.h"

#include "RegionTable.h"
#include "VisualizationContainer.h"
#include "SliceView.h"
#include "VolumeView.h"

// Constructor
SettingsDialog::SettingsDialog(QWidget* parent, VisualizationContainer* visualizationContainer, RegionTable* regionTable)
: QDialog(parent), visualizationContainer(visualizationContainer), regionTable(regionTable) {
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
	overlayOpacitySpinBox->setValue(sliceView->GetOverlayOpacity());

	// Surface opacity
	surfaceOpacitySpinBox->setValue(volumeView->GetVisibleOpacity());
}

void SettingsDialog::on_windowSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetWindow(value);
}

void SettingsDialog::on_levelSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetLevel(value);
}

void SettingsDialog::on_overlayOpacitySpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetOverlayOpacity(value);
}

void SettingsDialog::on_overlayOpacityUp() {
	overlayOpacitySpinBox->stepUp();
}

void SettingsDialog::on_overlayOpacityDown() {
	overlayOpacitySpinBox->stepDown();
}

void SettingsDialog::on_surfaceOpacitySpinBox_valueChanged(double value) {
	visualizationContainer->SetVisibleOpacity(value);
}

void SettingsDialog::on_surfaceOpacityUp() {
	surfaceOpacitySpinBox->stepUp();
}

void SettingsDialog::on_surfaceOpacityDown() {
	surfaceOpacitySpinBox->stepDown();
}

void SettingsDialog::on_voxelSizeSpinBox() {
	visualizationContainer->SetVoxelSize(
		xSizeSpinBox->value(),
		ySizeSpinBox->value(),
		zSizeSpinBox->value()
	);
}

void SettingsDialog::on_windowLevelChanged(double window, double level) {
	windowSpinBox->setValue(window);
	levelSpinBox->setValue(level);
}

void SettingsDialog::on_overlayOpacityChanged(double value) {
	overlayOpacitySpinBox->setValue(value);
}

void SettingsDialog::on_surfaceOpacityChanged(double value) {
	surfaceOpacitySpinBox->setValue(value);
}

void SettingsDialog::on_gradientOpacityCheckBox_stateChanged(int state) {
	visualizationContainer->GetVolumeView()->SetVolumeRenderingGradientOpacity(state != 0);
}

void SettingsDialog::on_autoAdjustSamplingCheckBox_stateChanged(int state) {
	visualizationContainer->GetVolumeView()->SetVolumeRenderingAutoAdjustSampling(state != 0);
}

void SettingsDialog::on_showFeedbackCheckBox_stateChanged(int state) {
	regionTable->setShowFeedbackColumns(state != 0);
}