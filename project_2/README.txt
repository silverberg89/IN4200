IN3200/IN4200 Partial Exam, Spring 2019
By: Fred Marcus John Silverberg

################ Data

External C library package for JPEG images.
from: https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/one_folder.tar

Object for RGB case: mona_lisa_noisy.jpg
from: https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/mona_lisa_noisy.jpg

Object for Greyscale case: mona_lisa_noisy.jpg
from: https://en.wikipedia.org/wiki/File:Mona_Lisa.jpg

################ Files

Folder: (serial_code)
	Makefile 		// For compiling
	serial_main.c 		// Hold projects source code
	functions.c 		// Hold implemented functions 
	mona_lisa_noisy.jpg 	// Working object

Folder: (parallel_code)
	Makefile
	parallel_main.c
	functions.c
	mona_lisa_noisy.jpg
	log.ompi_info  		// MPI related

Folder: (rgb_code)
	Makefile
	rgb_main.c
	functions.c
	mona_lisa_noisy.jpg
	log.ompi_info

Folder: (simple-jpeg) 		// JPEG library package
	
project_report_15319.pdf 	// Short report of project
README.txt
Makefile
._simple-jpeg

################ Compilation

For serial_code:
	1. Open terminal in folder
	2. In terminal, compile with:	'$ make
	3. In terminal, run with:	'$ ./serial_main KAPPA ITERS input_filename.jpg output_filename.jpg
	Ex:				'$ ./serial_main 0.2 100 mona_lisa_noisy.jpg mona_lisa_smooth.jpg

For parallel_code:
	1. Open terminal in folder
	2. In terminal, compile with:	'$ make
	3. In terminal, run with:	'$ mpirun -n PROCESSORS ./parallel_main KAPPA ITERS input_filename.jpg output_filename.jpg
	Ex:				'$ mpirun -n 3 ./parallel_main 0.2 100 mona_lisa_noisy.jpg mona_lisa_smooth.jpg

For rgb_code:
	1. Open terminal in folder
	2. In terminal, compile with:	'$ make
	3. In terminal, run with:	'$ mpirun -n PROCESSORS ./parallel_main KAPPA ITERS input_filename.jpg output_filename.jpg
	Ex:				'$ mpirun -n 3 ./rgb_main 0.1 10 mona_lisa_noisy.jpg mona_lisa_smooth.jpg
