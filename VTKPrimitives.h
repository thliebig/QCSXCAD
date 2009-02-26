/*!
\class VTKPrimitives
\author Thorsten Liebig
\version 2
\date 7.8.2007
\bug Methode AddRotationalPoly produces false 3D alignment
\todo Methode AddDisc data structure has to be revised 
\\Überarbeitet für Layer-Prinzip
*/

#ifndef _VTKPRIMITIVES_H
#define _VTKPRIMITIVES_H

class vtkRenderer;
class vtkActorCollection;

#define PI 3.141592654


class VTKPrimitives
{
public:
	/// Constructor needs a vtkRenderer for adding primitives
	/*! \param *Renderer necessary for construction is a valid vtkRenderer \sa VTKVisualization */
	VTKPrimitives(vtkRenderer *Renderer);
	/// Deconstructor
	~VTKPrimitives();
	/// Add a Cube to scene
	/*! \param *dCoords Set points as room diagonal (Xmin,Ymin,Zmin,Xmax,Ymax,Zmax) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)  */
	void AddCube(double *dCoords, double *dRGB, double dOpacity);	//complete
	/// Add a discrete object (many cubes) to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn. \param uiQtyCoords Set quantity of Points X,Y,Z. \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) */
	void AddDisc(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity);
	/// Add a closed polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param dExtrusionVector Give extrusion vector (X,Y,Z) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddClosedPoly(double *dCoords, unsigned int uiQtyCoords, double *dExtrusionVector, double *dRGB, double dOpacity);//complete
	/// Add a linear polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param LineWidth Set line width \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddLinePoly(double *dCoords, unsigned int uiQtyCoords,unsigned int LineWidth, double *dRGB, double dOpacity);//complete
	/// Add a cylinder to scene
	/*!  \param *dCenter Set Base Point in 3D space \param *dExtrusionVector Set height (absolute value) and direction \param fRadius Set Radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddCylinder(double *dCenter, double *dExtrusionVector, float fRadius, double *dRGB, double dOpacity, int iResolution); //complete
	/// Add a Sphere to scene
	/*! \param *dCenter Set Center Point in 3D space \param fRadius Set Radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddSphere(double *dCenter, float fRadius, double *dRGB, double dOpacity, int iResolution);	//complete
	/// Add an arrow to scene
	/*! \param *dStart Set start point \param *dEnd Set end point \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddArrow(double *dStart, double *dEnd, double *dRGB, double dOpacity, int iResolution=6);
	/// Add a text-label to scene
	/*! \param *cText Set text for the label \param *dCoords Set point of label origin \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param dscale Scale the size of the label*/
	void AddLabel(char *cText, double *dCoords, double *dRGB, double dOpacity, double dscale=1.0);
	/// Add a rotated polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param *fRotAxis Set rotation axis (X1,X2,Y1,Y2,Z1,Z2) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddRotationalPoly(double *dCoords, unsigned int uiQtyCoords, double *fRotAxis, double *dRGB, double dOpacity, int iResolution);//complete
	/// Add a rotated circle (torus) to scene
	/*! \param dPoint Set point in 3D space \param fRadius Set radius of the torus \param *fRotAxis Set rotation axis (X1,X2,Y1,Y2,Z1,Z2) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddRotationalSolid(double *dPoint, double fRadius, double *fRotAxis, double *dRGB, double dOpacity, int iResolution); //complete
	/// Add a surface (triangles) to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for triangles of surface. \param uiQtyCoords Set Quantity of Points X,Y,Z \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddSurface(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity);//complete
	/// Add a STL-object to scene
	/*! \param *Filename Set filename of STL object \param *dCenter Set point of origin for STL object \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddSTLObject(char *Filename, double *dCenter, double *dRGB, double dOpacity);//complete
	
	void SetOpacity2All(double opacity);

protected:
	/// unusable constructor, only for inheritance 
	VTKPrimitives();
	/// vtkRenderer, has to be set by constructor
	vtkRenderer *ren;
	/// Collection of all Actors generated in this class so far
	vtkActorCollection *ActorColl;
	/// Calculate angle between 2 vectors
	double VectorAngel(double dV1_1, double dV1_2, double dV1_3, double dV2_1, double dV2_2, double dV2_3);
	/// Calculate distance between a point and an axis
	/*! \param *dpoint Set point \param *dstart Set footpoint of axis \param *dvector Set direction of axis \param *dFootpoint Returns the point on axis, that is nearest to the input point \return Methode returns distance */
	double DistancePointLine(double *dpoint,double *dstart,double *dvector, double *dFootpoint);
	/// Calculate distance between one point and another 
	/*! \param *dpoint1 Set first point \param *dpoint2 Set second point \return Methode returns distance */
	double DistancePointPoint(double *dpoint1, double *dpoint2);
};

#endif
