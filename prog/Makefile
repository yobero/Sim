compile1 :
	gcc mainQ1.c -lm

run1 : compile1
	./a.out

runR : compile
	./a.out
	Rscript essai.R
	eog plot.jpg

clear :
	rm a.out
	rm donnee
	rm plot.jpg
