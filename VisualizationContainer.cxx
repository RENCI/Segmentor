#include "VisualizationContainer.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageThreshold.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include <vtkInteractorStyleSlice.h>
#include <vtkInteractorStyleVolume.h>

#include "InteractionCallbacks.h"
#include "SegmentorMath.h"
#include "SlicePipeline.h"
#include "VolumePipeline.h"

VisualizationContainer::VisualizationContainer(vtkRenderWindowInteractor* volumeInteractor, vtkRenderWindowInteractor* sliceInteractor) {
	data = nullptr;
	labels = nullptr;
	currentLabel = 0;

	// Create rendering pipelines
	volumePipeline = new VolumePipeline(volumeInteractor);
	slicePipeline = new SlicePipeline(sliceInteractor);

	// Callbacks

	// Camera
	vtkSmartPointer <vtkCallbackCommand> volumeCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeCameraCallback->SetCallback(InteractionCallbacks::VolumeCameraChange);
	volumeCameraCallback->SetClientData(slicePipeline->GetRenderer());
	volumePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, volumeCameraCallback);

	vtkSmartPointer <vtkCallbackCommand> sliceCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceCameraCallback->SetCallback(InteractionCallbacks::SliceCameraChange);
	sliceCameraCallback->SetClientData(volumePipeline->GetRenderer());
	slicePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, sliceCameraCallback);

	// Char
	vtkSmartPointer<vtkCallbackCommand> onCharCallback = vtkSmartPointer<vtkCallbackCommand>::New(); 
	onCharCallback->SetCallback(InteractionCallbacks::OnChar);
	onCharCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);
	sliceInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);

	// Label select
	vtkSmartPointer<vtkCallbackCommand> volumeSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeSelectLabelCallback->SetCallback(InteractionCallbacks::VolumeSelectLabel);
	volumeSelectLabelCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::SelectLabelEvent, volumeSelectLabelCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceSelectLabelCallback->SetCallback(InteractionCallbacks::SliceSelectLabel);
	sliceSelectLabelCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::SelectLabelEvent, sliceSelectLabelCallback);

	// Paint
	vtkSmartPointer<vtkCallbackCommand> volumePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumePaintCallback->SetCallback(InteractionCallbacks::VolumePaint);
	volumePaintCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::PaintEvent, volumePaintCallback);

	vtkSmartPointer<vtkCallbackCommand> slicePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	slicePaintCallback->SetCallback(InteractionCallbacks::SlicePaint);
	slicePaintCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::PaintEvent, slicePaintCallback);

	// Erase
	vtkSmartPointer<vtkCallbackCommand> volumeEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeEraseCallback->SetCallback(InteractionCallbacks::VolumeErase);
	volumeEraseCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::EraseEvent, volumeEraseCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceEraseCallback->SetCallback(InteractionCallbacks::SliceErase);
	sliceEraseCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::EraseEvent, sliceEraseCallback);

	// Mouse move
	vtkSmartPointer<vtkCallbackCommand> volumeMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeMouseMoveCallback->SetCallback(InteractionCallbacks::VolumeMouseMove);
	volumeMouseMoveCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::MouseMoveEvent, volumeMouseMoveCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceMouseMoveCallback->SetCallback(InteractionCallbacks::SliceMouseMove);
	sliceMouseMoveCallback->SetClientData(this);
	sliceInteractor->AddObserver(vtkCommand::MouseMoveEvent, sliceMouseMoveCallback);
}

VisualizationContainer::~VisualizationContainer() {
}

bool VisualizationContainer::OpenImageFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		data = reader->GetOutput();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		data = reader->GetOutput();
	}
	else {
		return false;
	}

	slicePipeline->SetImageData(data);

	return true;
}

bool VisualizationContainer::OpenImageStack(const std::vector<std::string>& fileNames) {
	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);
		reader->Update();

		data = reader->GetOutput();
	}
	else {
		return false;
	}

	slicePipeline->SetImageData(data);

	return true;
}

bool VisualizationContainer::OpenSegmentationFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		labels = reader->GetOutput();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		labels = reader->GetOutput();
	}
	else {
		return false;
	}

	volumePipeline->SetSegmentationData(labels);
	slicePipeline->SetSegmentationData(labels);

	return true;
}

bool VisualizationContainer::OpenSegmentationStack(const std::vector<std::string>& fileNames) {
	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);
		reader->Update();

		labels = reader->GetOutput();
	}
	else {
		return false;
	}

	volumePipeline->SetSegmentationData(labels);
	slicePipeline->SetSegmentationData(labels);

	return true;
}

bool VisualizationContainer::SaveSegmentationData(const std::string& fileName) {
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
	else if (extension == "tif" || extension == "tiff") {
		std::string prefix = fileName.substr(0, fileName.find_last_of("."));

		std::cout << prefix << std::endl;

		vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
		writer->SetFilePrefix(prefix.c_str());
		writer->SetFilePattern("%s_%04d.tif");
		writer->SetFileDimensionality(2);
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else {
		return false;
	}
	
	return true;
}

void VisualizationContainer::SegmentVolume() {
	if (!data) return;

	// Get Otsu threshold
	double value = SegmentorMath::OtsuThreshold(data);

	// Filter
	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
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
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetInputConnection(openClose->GetOutputPort());
	connectivity->Update();

	// Set label output
	labels = connectivity->GetOutput();

	volumePipeline->SetSegmentationData(labels);
	slicePipeline->SetSegmentationData(labels);
}

void VisualizationContainer::PickLabel(int x, int y, int z) {
	SetCurrentLabel(static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z))[0]);
}

void VisualizationContainer::Paint(int x, int y, int z) {
	SetLabel(x, y, z, currentLabel);
}

void VisualizationContainer::Erase(int x, int y, int z) {	
	SetLabel(x, y, z, 0);
}

void VisualizationContainer::PickPointLabel(double x, double y, double z) {
	// Get the label
	vtkIdType id = labels->FindPoint(x, y, z);
	SetCurrentLabel(static_cast<unsigned short*>(labels->GetScalarPointer())[id]);
}

void VisualizationContainer::PaintPoint(double x, double y, double z) {
	SetPointLabel(x, y, z, currentLabel);
}

void VisualizationContainer::ErasePoint(double x, double y, double z) {
	SetPointLabel(x, y, z, 0);
}

void VisualizationContainer::SetCurrentLabel(unsigned short label) {
	if (label == 0) return;

	currentLabel = label;

	volumePipeline->SetLabel(currentLabel);
	slicePipeline->SetLabel(currentLabel);
}

void VisualizationContainer::Render() {
	volumePipeline->Render();
	slicePipeline->Render();
}

VolumePipeline* VisualizationContainer::GetVolumePipeline() {
	return volumePipeline;
}

SlicePipeline* VisualizationContainer::GetSlicePipeline() {
	return slicePipeline;
}

void VisualizationContainer::SetLabel(int x, int y, int z, unsigned short label) {
	unsigned short* p = static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z));
	p[0] = label;
	labels->Modified();
}

void VisualizationContainer::SetPointLabel(double x, double y, double z, unsigned short label) {
	vtkIdType id = labels->FindPoint(x, y, z);
	unsigned short* p = static_cast<unsigned short*>(labels->GetScalarPointer());
	p[id] = label;
	labels->Modified();
}