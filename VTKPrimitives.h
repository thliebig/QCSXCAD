/*
*	Copyright (C) 2005,2006,2007,2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
\class VTKPrimitives
\author Thorsten Liebig
\version 2
\date 7.8.2007
\bug Methode AddRotationalPoly produces false 3D alignment
\todo Methode AddDisc data structure has to be revised 
*/

#ifndef _VTKPRIMITIVES_H
#define _VTKPRIMITIVES_H

class vtkRenderer;
class vtkPolyData;
class vtkActorCollection;
class vtkAppendPolyData;
class vtkActor;
class vtkAlgorithmOutput;

#define _USE_MATH_DEFINES

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
	/*! \param *dCoords Set points as room diagonal (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)  */
	void AddCube(double *dCoords, double *dRGB, double dOpacity, const double* tf_matrix=0);	//complete
	void AddCube(const double *dStart, const double *dStop, double *dRGB, double dOpacity, const double* tf_matrix=0);	//complete
	/// Add a Cube in cylindrical coordinates to scene
	/*! \param *dCoords Set points as room diagonal (Rmin,Rmax,Amin,Amax,Zmin,Zmax) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)  */
	void AddCylindricalCube(double *dCoords, double *dRGB, double dOpacity, const double* tf_matrix=0);
	void AddCylindricalCube(const double *dStart, const double *dStop, double *dRGB, double dOpacity, const double* tf_matrix=0);
	/// Add a Plane to scene
	/*! \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)  */
	void AddPlane(double *dOrigin, double* dP1, double* dP2, double *dRGB, double dOpacity, const double* tf_matrix=0);	//complete
	/// Add a discrete object (many cubes) to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn. \param uiQtyCoords Set quantity of Points X,Y,Z. \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) */
	void AddDisc(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity, const double* tf_matrix=0);
	/// Add a closed polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param dExtrusionVector Give extrusion vector (X,Y,Z) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddClosedPoly(double *dCoords, unsigned int uiQtyCoords, double *dExtrusionVector, double *dRGB, double dOpacity, const double* tf_matrix=0);//complete
	/// Add a linear polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param LineWidth Set line width \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddLinePoly(const double *dCoords, unsigned int uiQtyCoords,unsigned int LineWidth, double *dRGB, double dOpacity, const double* tf_matrix=0);//complete
	/// Add a linear polygon as tubes to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param TubeRadius Set tube radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddTubePoly(const double *dCoords, unsigned int uiQtyCoords, double TubeRadius, double *dRGB, double dOpacity, int iResolution=8, const double* tf_matrix=0);//complete
	/// Add a cylinder to scene
	/*!  \param *dCenterAxis Set Base Point in 3D space \param *dExtrusionVector Set height (absolute value) and direction \param fRadius Set Radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddCylinder(const double *dCenterAxis, const double *dExtrusionVector, float fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0); //complete
	/*!  \param *dAxisStart Cylinder axis start point \param *dAxisStop Cylinder axis end point \param fRadius Set Radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddCylinder2(const double *dAxisStart, const double* dAxisStop, float fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0); //complete
	/*! \brief Add a cylindrical shell to the scene. \param *dAxisStart Cylinder axis start point \param *dAxisStop Cylinder axis end point \param r_i Set inner radius \param r_o Set outer radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddCylindricalShell(const double *dAxisStart, const double* dAxisStop, double r_i, double r_o, double *dRGB, double dOpacity, int iResolution, const double *tf_matrix=0);
	/// Add a Sphere to scene
	/*! \param *dCenter Set Center Point in 3D space \param fRadius Set Radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddSphere(const double *dCenter, double fRadius, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0);	//complete
	/// Add a Spherical shell to scene
	/*! \param *dCenter Set Center Point in 3D space \param r_i Set inner radius \param r_o Set outer radius \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddSphericalShell(const double *dCenter, double r_i, double r_o, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0);	//complete
	/// Add an arrow to scene
	/*! \param *dStart Set start point \param *dEnd Set end point \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddArrow(double *dStart, double *dEnd, double *dRGB, double dOpacity, int iResolution=6, const double* tf_matrix=0);
	/// Add a text-label to scene
	/*! \param *cText Set text for the label \param *dCoords Set point of label origin \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param dscale Scale the size of the label*/
	void AddLabel(char *cText, double *dCoords, double *dRGB, double dOpacity, double dscale=1.0, const double* tf_matrix=0);
	/// Add a rotated polygon to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for polygon. \param uiQtyCoords Set Quantity of Points X,Y,Z \param *fRotAxis Set rotation axis (X1,X2,Y1,Y2,Z1,Z2) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddRotationalPoly(const double *dCoords, unsigned int uiQtyCoords, const double *fRotAxis, double StartStopAngle[2], double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0);//complete
	/// Add a rotated circle (torus) to scene
	/*! \param dPoint Set point in 3D space \param fRadius Set radius of the torus \param *fRotAxis Set rotation axis (X1,X2,Y1,Y2,Z1,Z2) \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness) \param iResolution Set resolution of discretisation*/
	void AddRotationalSolid(const double *dPoint, double fRadius, const double *fRotAxis, double *dRGB, double dOpacity, int iResolution, const double* tf_matrix=0); //complete
	/// Add a surface (triangles) to scene
	/*! \param *dCoords Set Points as X1,X2,...,Xn,Y1,Y2,...,Yn,Z1,Z2,...Zn for triangles of surface. \param uiQtyCoords Set Quantity of Points X,Y,Z \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddSurface(double *dCoords, unsigned int uiQtyCoords, double *dRGB, double dOpacity, const double* tf_matrix=0);//complete
	/// Add a STL-object to scene
	/*! \param *Filename Set filename of STL object \param *dCenter Set point of origin for STL object \param *dRGB Set RGB Colors (range 0 to 1 for red, green, blue) \param dOpacity Set opacity (0 complete transparency to 1 complete opaqueness)*/
	void AddSTLObject(const char *Filename, double *dCenter, double *dRGB, double dOpacity, const double* tf_matrix=0);//complete

	//! Add the given polydata to the scene
	vtkActor* AddPolyData(vtkAlgorithmOutput* polydata_port, double *dRGB, double dOpacity, const double* tf_matrix=0);
	//! Add the given polydata to the scene
	vtkActor* AddPolyData(vtkPolyData* polydata, double *dRGB, double dOpacity, const double* tf_matrix=0);

	void SetOpacity2All(double opacity);

	//! Write the collected poly-data into a vtk xml file (should be a *.vtp)
	void WritePolyData2File(const char* filename, double scale=1.0);

	//! Write the collected poly-data into a stl file (should be a *.stl)
	void WritePolyData2STL(const char* filename, double scale=1.0);

	//! Write the collected poly-data into a ply file (should be a *.ply)
	void WritePolyData2PLY(const char* filename, double scale=1.0);

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
	double DistancePointLine(const double *dpoint, const double *dstart, const double *dvector, double *dFootpoint);
	/// Calculate distance between one point and another 
	/*! \param *dpoint1 Set first point \param *dpoint2 Set second point \return Methode returns distance */
	double DistancePointPoint(const double *dpoint1, const double *dpoint2);

	//! A Collection of all incoming poly-data
	vtkAppendPolyData* m_PolyDataCollection;

	double m_ArcDelta;

	//! This internal methode will transform cylindrical coords into cartesian coords used by vtk.
	double* TransformCylindricalCoords(const double* in, double* out, unsigned int nrPoints=1);
};

#endif
