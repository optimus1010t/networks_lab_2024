char domain[MAX_DOMAIN] = "iitkgp.edu\0"; 
<!-- what is this domain exactly i dont get this -->
if(listen(sockfd,5)<0){
        perror("Error in listening\n");
        exit(0);
}
<!-- why not this instead of just listen(sockfd,5) -->