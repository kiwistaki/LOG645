#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <CL\cl.h>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength);

int main(int argc, char* argv[]) 
{
	int n, m, np;
	int i, j, k;
	double td, h;
	typedef vector<vector<double>> matrix;
	clock_t timeStartSeq, timeEndSeq, timeStartPar, timeEndPar;
	double timeSeq, timePar;

	if(argc == 6)
	{
		n = atoi(argv[1]);
		m = atoi(argv[2]);
		np = atoi(argv[3]);
		td = atof(argv[4]);
		h = atof(argv[5]);
	}
	else 
	{
		cout << "Invalid number of arguments. End of process." << endl;
		return EXIT_FAILURE;
	}
	////////////////////////////////////////////////////////////////////
	// Sequential
	////////////////////////////////////////////////////////////////////
	//Set matrix
	matrix matrixSeq[2] = { matrix(n, vector<double>(m)), matrix(n, vector<double>(m)) };
	timeStartSeq = clock();
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
		{
			matrixSeq[0][i][j] = i * (m - i - 1) * j * (n - j - 1);
			matrixSeq[1][i][j] = i * (m - i - 1) * j * (n - j - 1);
		}
	}

	//Sequential process
	int idxC;
	for (k = 1; k <= np; k++)
	{
		for (i = 1; i < n - 1; i++)
		{
			for (j = 1; j < m - 1; j++)
			{
				double coeff1 = 1 - (4 * td / (h*h));
				double coeff2 = td / (h*h);
				int idx = (k + 1) % 2;
				idxC = k % 2;
				matrixSeq[idxC][i][j] = coeff1 * matrixSeq[idx][i][j] + 
					coeff2 * (matrixSeq[idx][i-1][j] + matrixSeq[idx][i+1][j] + 
						matrixSeq[idx][i][j-1] + matrixSeq[idx][i][j+1]);
			}
		}
	}
	timeEndSeq = clock();
	timeSeq = (timeEndSeq - timeStartSeq) / (double)CLOCKS_PER_SEC;

	cout << "Sequential matrix" << endl;
	cout << "Temps : " << timeSeq << endl;
	cout << "----------------------------------------------" << endl;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
		{
			cout.precision(2);
			cout << matrixSeq[idxC][i][j] << "\t";
		}
		cout << endl;
	}

	///////////////////////////////////////////////////////////////////
	//Parallel
	//////////////////////////////////////////////////////////////////
	//Set matrix
	double *matrixParInit  = (double*)malloc(n * m * sizeof(double));
	double *matrixParFinal = (double*)malloc(n * m * sizeof(double));
	for(int i = 0; i < n;i++)
	{
		for (int j = 0; j < m; j++) 
		{
			matrixParInit[i*m+j] = i * (m - i - 1) * j * (n - j - 1);
		}
	}

	//Get kernel program
	size_t sizeProg;
	char* kernelSource = oclLoadProgSource("./Lab4.cl", "", &sizeProg);

	//set OpenCL parameters
	cl_platform_id plat_id = NULL;
	cl_device_id dev_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem initMatrixObj = NULL;
	cl_mem finalMatrixObj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_uint dev_num;
	cl_uint plat_num;
	cl_int fdb;

	// Get platform and device information for context
	fdb = clGetPlatformIDs(1, &plat_id, &plat_num);
	fdb = clGetDeviceIDs(plat_id, CL_DEVICE_TYPE_DEFAULT, 1, &dev_id, &dev_num);
	
	// Create context with device information
	context = clCreateContext(NULL, 1, &dev_id, NULL, NULL, &fdb);

	// Create command queue
	command_queue = clCreateCommandQueue(context, dev_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &fdb);
	
	// Create buffer for matrices
	initMatrixObj = clCreateBuffer(context, CL_MEM_READ_WRITE, n*m * sizeof(double), NULL, &fdb);
	finalMatrixObj = clCreateBuffer(context, CL_MEM_READ_WRITE, n*m * sizeof(double), NULL, &fdb);
	
	// Create and build program from CL kernel
	program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, (const size_t *)&sizeProg, &fdb);
	fdb = clBuildProgram(program, 1, &dev_id, NULL, NULL, NULL);

	// Create and Set OpenCL kernel
	kernel = clCreateKernel(program, "HeatDynamicTransfer", &fdb);
	fdb = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&initMatrixObj);
	fdb = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&finalMatrixObj);
	fdb = clSetKernelArg(kernel, 2, sizeof(cl_int), (void *)&n);
	fdb = clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&m);
	fdb = clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&np);
	fdb = clSetKernelArg(kernel, 5, sizeof(cl_double), (void *)&td);
	fdb = clSetKernelArg(kernel, 6, sizeof(cl_double), (void *)&h);

	//Copy initial matrix in buffer
	fdb = clEnqueueWriteBuffer(command_queue, initMatrixObj, CL_TRUE, 0, n*m * sizeof(double), matrixParInit, 0, NULL, NULL);

	// Probe initial clock time
	timeStartPar = clock();

	size_t global_mem_size = n * m;
	for (int k = 1; k <= np; k++)
	{
		// Execute Parallel kernel
		fdb = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_mem_size, NULL, 0, NULL, NULL);

		// Receive and copy data from kernel and set next buffer
		fdb = clEnqueueReadBuffer(command_queue, finalMatrixObj, CL_TRUE, 0, n * m * sizeof(double), matrixParFinal, 0 , NULL, NULL);
		fdb = clEnqueueWriteBuffer(command_queue, initMatrixObj, CL_TRUE, 0, n * m * sizeof(double), matrixParFinal, 0, NULL, NULL);
	}
	
	// Probe final clock time
	timeEndPar = clock();
	timePar = (timeEndPar - timeStartPar) / (double)CLOCKS_PER_SEC;

	// Print final matrix
	cout << "Parallel matrix" << endl;
	cout << "Temps : " << timePar << endl;
	cout << "----------------------------------------------" << endl;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < m; j++)
		{
			cout.precision(2);
			cout << matrixParFinal[i*m+j] << "\t";
		}
		cout << endl;
	}

	// Display result
	cout << "Acceleration = " << timeSeq / timePar << endl;

	// Free memory
	fdb = clFlush(command_queue);
	fdb = clFinish(command_queue);
	fdb = clReleaseKernel(kernel);
	fdb = clReleaseContext(context);
	fdb = clReleaseProgram(program);
	fdb = clReleaseMemObject(initMatrixObj);
	fdb = clReleaseMemObject(finalMatrixObj);
	fdb = clReleaseCommandQueue(command_queue);
	free(kernelSource);

	system("pause");

	return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//! Loads a Program file and prepends the cPreamble to the code.
//!
//! @return the source string if succeeded, 0 otherwise
//! @param cFilename program filename
//! @param cPreamble code that is prepended to the loaded file, typically a set of #defines or a header
//! @param szFinalLength returned length of the code string
//////////////////////////////////////////////////////////////////////////////
char* oclLoadProgSource(const char* cFilename, const char* cPreamble, size_t* szFinalLength)
{
	// locals
	FILE* pFileStream = NULL;
	size_t szSourceLength;
	// open the OpenCL source code file
	if (fopen_s(&pFileStream, cFilename, "rb") != 0)
	{
		return NULL;
	}
	size_t szPreambleLength = strlen(cPreamble);
	// get the length of the source code
	fseek(pFileStream, 0, SEEK_END);
	szSourceLength = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);
	// allocate a buffer for the source code string and read it in
	char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
	memcpy(cSourceString, cPreamble, szPreambleLength);
	if (fread((cSourceString)+szPreambleLength, szSourceLength, 1, pFileStream) != 1)
	{
		fclose(pFileStream);
		free(cSourceString);
		return 0;
	}
	// close the file and return the total length of the combined (preamble + source) string
	fclose(pFileStream);
	if (szFinalLength != 0)
	{
		*szFinalLength = szSourceLength + szPreambleLength;
	}
	cSourceString[szSourceLength + szPreambleLength] = '\0';
	return cSourceString;
}