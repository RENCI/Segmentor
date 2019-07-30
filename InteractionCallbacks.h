#ifndef InteractionCallbacks_H
#define InteractionCallbacks_H

class vtkObject;

void volumeCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
void sliceCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

#endif