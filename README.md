# Segmentor
Segmentation tool for 3D tissue cleared microscopy images of nuclei.

## Data Formats:

The following data formats are supported for loading image and segmentation data, and saving segmentation data:

1. NIfTI (Neuroimaging Informatics Technology Initiative)
2. VTI (VTK'S XML Image Data format)
3. Single multipage TIFF file
4. Stack of TIFFs

When loading data as a stack of TIFFs, it is assumed that all files in the directory belong to the stack, and that the file names are in alphabetical order (e.g. image_001.tif, image_002.tif, etc.).

## Image Data Loading:

To load image data in NIfTI, VTI, or multipage TIFF format select `File->Open Image File...` and choose the desired file.
To load image data in stack of TIFFs format, select `File->Open Image Stack...` and choose a single file from the directory in which the image stack is stored.

After loading data, the 2D slice view on the right will display the current slice.

## Creating an Initial Segmentation

To generate an initial segmentation, select `Analyze->Segment Volume...`. The segmentation method currently consists of an Otsu thresholding, followed by morphological opening and closing operations, followed by calculating the connected components.

Once the initial segmentation is generated, the 3D view on the left will display surfaces of the segmentations, and the 2D slice view will show 2D overlays of the segmented regions on the current slice.

## Segmentation Loading:

To segmentation data in NIfTI, VTI, or multipage TIFF format select `File->Open Segmentation File...` and choose the desired file.
To load segmentation data in stack of TIFFs format, select `File->Open Segmentation Stack...` and choose a single file from the directory in which the segmentation stack is stored.

## Segmentation Saving

To save the current segmentation select `File->Save Segmentation Data...`. The format will be based on the file extension  provided `(.nii, .vti, or .tif)`. 

## Controls:

Switch between **Navigation** and **Edit** modes with the space bar.

### Navigation mode:

* Rotate: `Left-click and drag`
* Translate: `Middle-click and drag`
* Zoom: `Right-click and drag`
* Spin: `Ctrl + left-click and drag`

### Edit mode:

* Paint: `Left-click or left-click and drag`
* Erase: `Right-click or right-click and drag`

### Both modes:

#### General 

* Select current label: `Middle-click or Ctrl + left-click` on segmentation region
* Move clipping plane: `Ctrl + right-click and drag`
* Zoom to point: `f`
****
* Relabel current region: `u`
* Merge with current region: `m`
* Split current region: `2-9` (for number of split regions)
****
* Dilate current region: `z'
* Erode current region: `x`
* Region grow: `g` (2D slice view only)

#### 2D Visualization:

* Window/level: `Shift + left-click and drag` (2D slice view only)
****
* Toggle voxel overlay: `q`
* Toggle voxel outline: `w`
* Toggle region outline: `e`
****
* Move slice up or down: `up arrow` or `down arrow`
* Adjust overlay opacity: `right arrow` or `left arrow`

#### 3D Visualization

* Toggle surface smoothing: `s`
* Toggle surface normals: `n`
* Toggle slice plane visualization: `o`
****
* Toggle slice plane filtering: `p`
* Toggle neighbor filtering: `k`
* Toggle region filtering: `l`
****
* Adjust neighbor opacity (in neighbor filter mode): `Shift + right arrow` or `Shift + left arrow`

## Region Table:

* Mouse-over cells in the *Id*, *Color*, or *Size* columns to highlight that region in the 3D view
* Click cells in the *Id*, *Color*, or *Size* columns to select that region and zoom in
* Regions edited by the user are indicated in the *Modified* column
* Click the check box in the *Done* column to indicate regions for which editing is complete, which will render them as grey in the 3D and 2D slice views
* Click the button in the *Remove* column to remove that region
