#include "SegmentVolumeDialog.h"

#include "VisualizationContainer.h"
#include "SliceView.h"
#include "VolumeView.h"

// Constructor
SegmentVolumeDialog::SegmentVolumeDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
	: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	VolumeView* volumeView = visualizationContainer->GetVolumeView();
	
	// Get data range
	const double* range = visualizationContainer->GetDataRange();
	double step = (range[1] - range[0]) / 100;

	// Get Otsu threshold
	otsuThreshold = visualizationContainer->GetOtsuThreshold();

	thresholdSlider->setMinimum(range[0]);
	thresholdSlider->setMaximum(range[1]);
	thresholdSlider->setSingleStep(step);
	thresholdSlider->setPageStep(step * 10);
	thresholdSlider->setValue(otsuThreshold);
}

SegmentVolumeDialog::~SegmentVolumeDialog() {
}

void SegmentVolumeDialog::on_updateButton_clicked() {
	visualizationContainer->SegmentVolume(thresholdSlider->value());
}