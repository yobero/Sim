compile :
	gcc main.c -lm

run : compile
	./a.out

runR : compile
	./a.out
	Rscript essai.R
	eog plot.jpg

clear :
	rm a.out
	rm donnee
	rm plot.jpg
