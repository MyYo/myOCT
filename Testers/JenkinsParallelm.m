myCluster = parcluster('delazerdatrial1')
start(myCluster)
wait(myCluster) % Wait for the cluster to be ready to accept job submissions
myPool=parpool(myCluster) %Create parallel pool on cluster
myCluster
myPool
delete(myPool)
shutdown(myCluster) 
wait(myCluster)
myCluster
myPool
exit(0)