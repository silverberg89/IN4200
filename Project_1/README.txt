IN3200/IN4200 Partial Exam, Spring 2019
By: Fred Marcus John Silverberg

################ Data

webgraph.txt
from: https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/in3200_in4200_partial_exam_v19.pdf

100nodes_graph.txt
from: https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/100nodes_graph.txt

web-NotreDame.txt
from: https://snap.stanford.edu/data/web-NotreDame.html

################ Files

PE_main_fmsilver.c
PE_functions_fmsilver.c
PE_report_fmsilver.pdf
README.txt
webgraph.txt
100nodes_graph.txt
web-NotreDame.txt

################ Compilation

1. Open terminal
2. In terminal, compile with:	 '$ gcc PE_main_fmsilver.c -o PE_main_fmsilver -lm -fopenmp'
3. In terminal, run with:	 '$ ./PE_main_fmsilver web-NotreDame.txt 1.0e-20 0.85 10'

################ Times for 4 threads @ 3.2 Ghz:
webgraph.txt
	Everything takes under: 	0.01 sec

100nodes_graph.txt
	Everything takes under: 	0.01 sec

web-NotreDame.txt
	Everything for [M=Nodes]:	~  65 sec
	Everything for [M<10000]:	~ 5.0 sec
	Everything for [M<1000]:	~ 1.0 sec
	Everything for [M<100]:		~ 0.7 sec

