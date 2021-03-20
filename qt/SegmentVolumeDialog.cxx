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

	thresholdSpinBox->setMinimum(range[0]);
	thresholdSpinBox->setMaximum(range[1]);
	thresholdSpinBox->setSingleStep(step);
	thresholdSpinBox->setValue(otsuThreshold);

	thresholdSlider->setMinimum(0);
	thresholdSlider->setMaximum(99);
	thresholdSlider->setSingleStep(1);
	thresholdSlider->setPageStep(10);
	thresholdSlider->setValue(otsuThreshold / step);

	QObject::connect(thresholdSlider, &QSlider::valueChanged, [this, range](int value) {
		double v = ((double)value / thresholdSlider->maximum()) * (range[1] - range[0]) + range[0];

		thresholdSpinBox->blockSignals(true);
		thresholdSpinBox->setValue(v);
		thresholdSpinBox->blockSignals(false);
	});

	QObject::connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, range](int value) {
		int v = (value - range[0]) / (range[1] - range[0]) * thresholdSlider->maximum();

		thresholdSlider->blockSignals(true);
		thresholdSlider->setValue(v);
		thresholdSlider->blockSignals(false);
	});

	// Smoothing
	int maxSmoothing = 5;

	smoothingSlider->setMaximum(maxSmoothing);
	smoothingSpinBox->setMaximum(maxSmoothing);

	QObject::connect(smoothingSlider, &QSlider::valueChanged, smoothingSpinBox, &QSpinBox::setValue);
	QObject::connect(smoothingSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), smoothingSlider, &QSlider::setValue);

	// Open/close
	int maxOpenClose = 10;

	openCloseSlider->setMaximum(maxOpenClose);
	openCloseSpinBox->setMaximum(maxOpenClose);

	QObject::connect(openCloseSlider, &QSlider::valueChanged, openCloseSpinBox, &QSpinBox::setValue);
	QObject::connect(openCloseSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), openCloseSlider, &QSlider::setValue);

	// Perform initial segmentation
	SegmentVolume();
}

SegmentVolumeDialog::~SegmentVolumeDialog() {
}

void SegmentVolumeDialog::on_updateButton_clicked() {
	SegmentVolume();
}

void SegmentVolumeDialog::SegmentVolume() {
	visualizationContainer->SegmentVolume(thresholdSpinBox->value(), smoothingSpinBox->value(), openCloseSpinBox->value());
}