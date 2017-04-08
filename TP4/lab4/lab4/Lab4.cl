__kernel void HeatDynamicTransfer(__global double *matrixParInit, __global double *matrixParFinal, int n, int m, int np, double td, double h)
{
	//Get global_id
	int id = get_global_id(0);

	//Get indexes according to global_id
	int i = (int)(id / m);
	int j = (id%m);

	// if indexes are on one of the boundaries
	if ((i==0) || (i==n-1) || (j==0) || (j==m-1))
	{
		matrixParFinal[i*m + j] = 0.00;
	}
	else
	{
		// Find value and neighbors
		double value = matrixParInit[i*m + j];
		double left_neighbor = matrixParInit[i*m + (j - 1)];
		double top_neighbor = matrixParInit[(i + 1)*m + j)];
		double right_neighbor = matrixParInit[i*m + (j + 1)];
		double bottom_neighbor = matrixParInit[(i - 1)*m + j)];

		// Calculate coefficients (mostly for debug purposes)
		double coeff_val = 1 - ((4 * td) / (h*h));
		double coeff_neighbor = (td / (h*h));

		// Calculate new value
		matrixParFinal[i*m + j] = (coeff_val*value) + (coeff_neighbor*(left_neighbor + top_neighbor + right_neighbor + bottom_neighbor));
	}
}