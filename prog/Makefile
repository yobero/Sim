compile1 :
	gcc mainQ1.c -o q1 -lm

run1 : compile1
	./q1

compile2:
	gcc mainQ2.c -o q2 -lm

run2: compile2
	./q2

compile3 :
	gcc mainQ3.c -o q3 -lm

run3 : compile3
	./q3

runR : compile1 compile2
		./q1
		./q2
		./q3
		Rscript essai.R
		eog plot.jpg
R :
	Rscript essai.R

clear : compile1 compile2 compile3
	rm q1
	rm q2
	rm q3
	rm resultat1.txt
	rm resultat2.txt
	rm resultat3.txt
	rm plot.jpg
