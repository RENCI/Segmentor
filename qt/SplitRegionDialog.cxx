#include "SplitRegionDialog.h"

#include "VisualizationContainer.h"
#include "SliceView.h"
#include "VolumeView.h"

// Constructor
SplitRegionDialog::SplitRegionDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
	: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	// Handle rejection :-(
	QObject::connect(this, &SplitRegionDialog::rejected, this, &SplitRegionDialog::on_reject);

	QObject::connect(numberSlider, &QSlider::valueChanged, numberSpinBox, &QSpinBox::setValue);
	QObject::connect(numberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), numberSlider, &QSlider::setValue);

	numberSpinBox->setFocus();

	visualizationContainer->PushTempHistory();
}

SplitRegionDialog::~SplitRegionDialog() {
}

void SplitRegionDialog::on_updateButton_clicked() {
	SplitRegion();
}

void SplitRegionDialog::SplitRegion() {
	visualizationContainer->PopTempHistory();

	visualizationContainer->SplitCurrentRegion(numberSpinBox->value());
}

void SplitRegionDialog::on_reject() {
	visualizationContainer->PopTempHistory();
}