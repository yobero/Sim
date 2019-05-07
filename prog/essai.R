g = read.table("donnee")
attach(g)
jpeg("plot.jpg")
temps=V1
moy=V2
plot(temps,moy,type="l",,xlab="temps",ylab="nombre moyen de client",col="red")
dev.off()
