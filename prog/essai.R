g = read.table("resultat2.txt")
jpeg("tempsMoy.jpg")
lambda=g$V1
moy2=g$V2
tmoy2=g$V3
t902=g$V4


h=read.table("resultat1.txt")
moy1=h$V2
tmoy1=h$V3
t901=h$V4

f = read.table("resultat3.txt")
moy3=f$V2
tmoy3=f$V3
t903=f$V4

plot(lambda,tmoy2,type="l",,xlab="lambda",ylab="temps moyen d'attente",col="red")
lines(lambda,tmoy1,type="l",col="blue")
lines(lambda,tmoy3,type="l",col="green")
legend("topleft",legend=c("model 1","model 2","model 3"),col=c("blue","red","green"),lty=1:1,cex=0.8)

dev.off()
