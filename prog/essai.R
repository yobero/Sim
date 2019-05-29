g = read.table("resultat2.txt")
jpeg("tempsMoy.jpg")
lambda=g$V1
tmoy2=g$V2
t902=g$V3
tmoy2Th=g$V4
t90Th=g$V5


h=read.table("resultat1.txt")
tmoy1=h$V2
t901=h$V3
tmoy1Th=h$V4

f = read.table("resultat3.txt")
tmoy3=f$V2
t903=f$V3
tmoy3Th=f$V4

plot(lambda,tmoy2,type="l",xlab="lambda",ylab="temps moyen d'attente",col="red")
lines(lambda,tmoy1,type="l",col="blue")
lines(lambda,tmoy3,type="l",col="green")
legend("topleft",legend=c("model 1","model 2","model 3"),col=c("blue","red","green"),lty=1:1,cex=0.8)

jpeg("t90.jpg")

plot(lambda,t902,type="l",xlab="lambda",ylab="t90 du temps d'attente",col="red")
lines(lambda,t901,type="l",col="blue")
lines(lambda,t903,type="l",col="green")
legend("topleft",legend=c("model 1","model 2", "model 3"),col=c("blue","red","green"),lty=1:1, cex=0.8)

jpeg("attenteM1.jpg")

plot(lambda,tmoy1Th,type="l",xlab="lambda",ylab="temps moyen d'attente",col="blue")
lines(lambda,tmoy1,type="l",col="red")
legend("topleft",legend=c("experimental","theorique"),col=c("red","blue"),lty=1:1,cex=0.8)

jpeg("attenteM2.jpg")

plot(lambda,tmoy2,type="l",xlab="lambda",ylab="temps moyen d'attente",col="red")
lines(lambda,tmoy2Th,type="l",col="blue")
legend("topleft",legend=c("experimental","theorique"),col=c("red","blue"),lty=1:1,cex=0.8)

jpeg("attenteM3.jpg")

plot(lambda,tmoy3,type="l",xlab="lambda",ylab="temps moyen d'attente",col="red")
lines(lambda,tmoy3Th,type="l",col="blue")
legend("topleft",legend=c("experimental","theorique"),col=c("red","blue"),lty=1:1,cex=0.8)
