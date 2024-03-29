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

	// Flip axis callbacks
	QObject::connect(flipXButton, &QPushButton::clicked, this, [this]() { on_flipAxisButton(0); });
	QObject::connect(flipYButton, &QPushButton::clicked, this, [this]() { on_flipAxisButton(1); });
	QObject::connect(flipZButton, &QPushButton::clicked, this, [this]() { on_flipAxisButton(2); });
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

	// Neighbor radius
	neighborRadiusSpinBox->setValue(visualizationContainer->GetNeighborRadius());

	// Dot size
	dotSizeSpinBox->setValue(sliceView->GetDotSize());
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

void SettingsDialog::on_neighborRadiusSpinBox_valueChanged(double value) {
	visualizationContainer->SetNeighborRadius(value);
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

void SettingsDialog::on_enableDotAnnotationCheckBox_stateChanged(int state) {
	emit enableDotAnnotationChanged(state != 0);
}

void SettingsDialog::on_dotSizeSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetDotSize(value);
}

void SettingsDialog::on_flipAxisButton(int axis) {
	visualizationContainer->FlipAxis(axis);
}