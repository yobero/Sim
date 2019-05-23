compile1 :
	gcc mainQ1.c -o q1 -lm

run1 : compile1
	./q1

compile2:
	gcc mainQ2.c -o q2 -lm

run2: compile2
	./q2

runR : compile1 compile2
		./q1
		./q2
		Rscript essai.R
		eog plot.jpg

clear :
	rm q1
	rm q2
	rm resultat1.txt
	rm resultat2.txt
	rm donnee
	rm plot.jpg
