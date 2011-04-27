#include "../opencv_matlab_interop.h"

#include "calib3d/calib3d.hpp"

#define DEBUG

/* RHS arguments
   prhs[0]: object points
   prhs[1]: image points
   prhs[2]: focal length

   outputs:
   plhs[0]: rotation matrix
   plhs[1]: translation matrix
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    ASSERT_NUM_RHS_ARGS_GTE(3);
    ASSERT_NUM_LHS_ARGS_EQUALS(2);

    const mxArray *object_points_mx = prhs[0];
    const mxArray *image_points_mx = prhs[1];
    const mxArray *focal_length_mx = prhs[2];

    ASSERT_IS_DOUBLE(object_points_mx);
    ASSERT_IS_DOUBLE(image_points_mx);
    ASSERT_IS_DOUBLE(focal_length_mx);
    ASSERT_IS_SCALAR(focal_length_mx);
    double focal_length = SCALAR_GET_DOUBLE(focal_length_mx);
#ifdef DEBUG
    mexPrintf("focal length = %f\n", focal_length);
#endif

    //error checking
    if (mxGetM(object_points) != 3) {
        mexErrMsgTxt("object points should have first dimension size 3");
    }
    else if (mxGetM(image_points) != 2) {
        mexErrMsgTxt("image points should have first dimension size 2");
    }
    int num_points = mxGetN(object_points);
    if (mxGetN(image_points) != num_points) {
        mexErrMsgTxt("number of points in image_points and object_points do not match");
    }
#ifdef DEBUG
    mexPrintf("%d points supplied\n", num_points);
#endif

    //if we have 4 arguments, get number of iterations from it
    unsigned int num_iterations = 100;
    if (nrhs > 3) {
        const mxArray *iterations_mx = prhs[3];
        ASSERT_IS_SCALAR(iterations_mx);
        ASSERT_IS_SINT32(iterations_mx);
        num_iterations = SCALAR_GET_SINT32(iterations_mx);
    }

#ifdef DEBUG
    mexPrintf("num_iterations = %d\n", num_iterations);
#endif

    //the first point in the object array has to be (0,0,0)
    double *object_points_ptr = mxGetPr(object_points);
    if (object_points_ptr[0] != 0.0 || 
        object_points_ptr[1] != 0.0 ||
        object_points_ptr[2] != 0.0) {
        mexErrMsgTxt("first point in object_points must be (0,0,0)\n");
    }


    //make the cvPositObject - uses code from http://opencv.willowgarage.com/wiki/Posit
    std::vector<CvPoint3D32f> model_points;
    for (unsigned int i=0; i < num_points; i++) {
        model_points.push_back(cvPoint3D32f(object_points_ptr[0], 
                                            object_points_ptr[1],
                                            object_points_ptr[2]));
        object_points_ptr += 3;
    }
    CvPOSITObject *positObject = cvCreatePOSITObject( &model_points[0], num_points );

    //put the image points into a similar structure
    double *image_points_ptr = mxGetPr(image_points);
    std::vector<CvPoint2D32f> image_points;
    for (unsigned int i=0; i < num_points; i++) {
        image_points.push_back( cvPoint2D32f(image_points_ptr[0], image_points_ptr[1]));
        image_points_ptr += 2;
    }
    
    CvMatr32f rotation_matrix = new float[9];
    CvVect32f translation_vector = new float[3];

    CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f);
    cvPOSIT(positObject, &image_points[0], focal_length, rotation_matrix, translation_vector);
    
    mexPrintf("cvPOSIT returned successfully\n");

    cvReleasePOSITObject(&positObject);
    delete rotation_matrix;
    delete translation_vector;
        
