#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>

DataPipeline::DataPipeline() {
	data = nullptr;
	labels = nullptr;
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenImageFile(const std::string& fileName) {
	double zScale = 2.0;

	// Load data
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();

	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return false;
	}

	// Match output dims and resolution
	info->SetOutputSpacing(1.0, 1.0, zScale);

	// Resample in z
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->SetInputConnection(info->GetOutputPort()); 
	resample->SetAxisMagnificationFactor(2, zScale);
	resample->Update();

	// Set data output
	data = resample->GetOutput();

	return true;
}

bool DataPipeline::OpenImageStack(const std::vector<std::string>& fileNames) {
	return false;
}

bool DataPipeline::OpenSegmentationFile(const std::string& fileName) {
	//double zScale = 2.0;

	// Load data
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	reader->SetFileName(fileName.c_str());

	// Match output dims and resolution
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetInputConnection(reader->GetOutputPort());
	//info->SetOutputSpacing(1.0, 1.0, zScale);
	info->Update();

/*
	// Resample in z
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->SetInputConnection(info->GetOutputPort());
	resample->SetAxisMagnificationFactor(2, zScale);
	resample->Update();
*/

	// Set label output
	labels = info->GetOutput();

	return true;
}

bool DataPipeline::OpenSegmentationStack(const std::vector<std::string>& fileNames) {
	return false;
}

bool DataPipeline::SaveSegmentationData(const std::string& fileName) {
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "tif") {

	}
	else {
		return false;
	}
	
	return true;
}

void DataPipeline::SegmentVolume() {
	if (!data) return;

	double value = OtsuThreshold(data);

	// Filter
//	double minValue = data->GetScalarRange()[0];
//	double maxValue = data->GetScalarRange()[1];

	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
//	threshold->ThresholdByUpper(minValue + (maxValue - minValue) * 0.15);
	threshold->ThresholdByUpper(value);
	threshold->SetInValue(255);
	threshold->SetOutValue(0);
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetOutputScalarTypeToUnsignedChar();
	threshold->SetInputDataObject(data);

	vtkSmartPointer<vtkImageOpenClose3D> openClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
	openClose->SetKernelSize(10, 10, 10);
	openClose->SetOpenValue(0);
	openClose->SetCloseValue(255);
	openClose->SetInputConnection(threshold->GetOutputPort());

	// Generate labels
	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(255, 255);
	connectivity->SetLabelScalarTypeToInt();
	connectivity->SetInputConnection(openClose->GetOutputPort());
	connectivity->Update();

	// Set label output
	labels = connectivity->GetOutput();
}

vtkImageData* DataPipeline::GetData() {
	return data;
}

vtkImageData* DataPipeline::GetLabels() {
	return labels;
}

double DataPipeline::OtsuThreshold(vtkImageData* image) {
	// Get image dimensions and extent
	int dims[3];
	image->GetDimensions(dims);

	int extent[6];
	image->GetExtent(extent);
	
	// Initialize values
	double minValue = image->GetScalarRange()[0];
	double maxValue = image->GetScalarRange()[1];
	double mean = 0.0;
	double sigma = 0.0;
	const int count = dims[0] * dims[1] * dims[2];

	// Bin values
	std::vector<int> histogram(256, 0);
	std::vector<int> values(count, 0);

	int index = 0;
	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				double value = image->GetScalarComponentAsDouble(i, j, k, 0);
				values[index++] = (value - minValue) / (maxValue - minValue) * (histogram.size() - 1);
			}
		}
	}

	// Compute histogram and mean
	for (int i = 0; i < values.size(); i++) {
		int v = values[i];
		histogram[v]++;
		mean += v;
	}
	
	mean /= count;

	std::cout << "Mean: " << mean << std::endl;

	// Calculate sigma
	for (int i = 0; i < histogram.size(); i++) {
		double v = i - mean;

		sigma += v * v * histogram[i] / count;
	}

	std::cout << "Sigma: " << sigma << std::endl;

	// Calculate threshold bin
	int bin = 0;
	double criterion = 0.0;

	for (int i = 0; i < histogram.size(); i++) {
		double c = OtsuCriterion(image, histogram, values, sigma, count, i);
		if (c > criterion) {
			criterion = c;
			bin = i;
		}
	}

	std::cout << "Bin: " << bin << std::endl;

	double threshold = minValue + (maxValue - minValue) * bin / histogram.size();

	std::cout << "Threshold: " << threshold << std::endl;

	return threshold;
}

double DataPipeline::OtsuCriterion(vtkImageData* image, const std::vector<int>& histogram, const std::vector<int>& values, double sigma, int count, int bin) {
	// Initialize values
	double meanA = 0.0;
	double wA = 0.0;
	int countA = 0;

	double meanB = 0.0;
	double wB = 0.0;
	int countB = 0;

	// Get class means
	for (int i = 0; i < values.size(); i++) {
		int v = values[i];
		if (v < bin) {
			meanA += v;
			countA++;
		}
		else {
			meanB += v;
			countB++;
		}
	}

	meanA /= countA;
	wA = (double)countA / count;

	meanB /= countB;
	wB = (double)countB / count;

	// Get the variance for each class
	double sigmaA = 0.0;
	double sigmaB = 0.0;

	for (int i = 0; i < bin; i++) {
		double v = i - meanA;

		sigmaA += v * v * histogram[i] / countA;
	}

	for (int i = bin; i < histogram.size(); i++) {
		double v = i - meanB;

		sigmaB += v * v * histogram[i] / countB;
	}

	// Get the inter-class variance
	double meanAB = meanA - meanB;
	double sigmaAB = wA * wB * meanAB * meanAB;
	double criterion = sigmaAB / sigma;

	return criterion;
}