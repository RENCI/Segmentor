# Segmentor
Segmentation tool for 3D tissue cleared microscopy images of nuclei.

## Data Formats:

The following data formats are supported for loading image and segmentation data, and saving segmentation data:

1. VTI (VTK'S XML Image Data format)
2. NIfTI (Neuroimaging Informatics Technology Initiative)
3. Stack of TIFFs

When loading data as a stack of TIFFs, it is assumed that all files in the directory belong to the stack, and that the file names are in alphabetical order (e.g. image_001.tif, image_002.tif, etc.).

## Image Data Loading:

To load image data in VTI or NIfTI format select `File->Open Image File...` and choose the desired file.
To load image data in stack of TIFFs format, select `File->Open Image Stack...` and choose a single file from the directory in which the image stack is stored.

After loading data, the 2D slice view on the right will display the current slice.

## Creating an Initial Segmentation

To generate an initial segmentation, select `Analyze->Segment Volume...`. The segmentation method currently consists of an Otsu thresholding, followed by morphological opening and closing operations, followed by calculating the connected components.

Once the initial segmentation is generated, the 3D view on the left will display surfaces of the segmentations, and the 2D slice view will show 2D overlays of the segmented regions on the current slice.

## Segmentation Loading:

To segmentation data in VTI or NIfTI format select `File->Open Segmentation File...` and choose the desired file.
To load segmentation data in stack of TIFFs format, select `File->Open Segmentation Stack...` and choose a single file from the directory in which the segmentation stack is stored.

## Segmentation Saving

To save the current segmentation select `File->Save Segmentation Data...`. The format will be based on the file extension  provided `(.vti, .nii, or .tif)`. 

## Controls:

